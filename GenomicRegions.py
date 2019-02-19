#!/usr/bin/env python
__author__ = 'Apua Paquola'

import sqlite3
import os
import tempfile
import subprocess
import shutil
import hashlib
import re
import math
from scipy.stats import binom

class GenomicRegions:
    def __init__(self):
        self.base_dir = '/ceph/users/apua/tests/genomicregions/GenomicRegions'
        self.cache_dir = self.base_dir + '/cache'
        self.tmp_dir = self.base_dir + '/tmp'
        self.con = sqlite3.connect(self.base_dir + '/GenomicRegions.sqlite', timeout=3600)
        #self.con.isolation_level = 'EXCLUSIVE'
        self.bin_dir = '/ceph/opt/GenomicRegions/red_black_interval_tree'

        
    def init_db(self):
        #self.con.execute('BEGIN EXCLUSIVE')
        self.con.execute('''create table genomic_region (
        name text primary key,
        parent text,
        construction text,
        bedfile text,
        intervals integer,
        basepairs integer,
        constructed text
        )
        ''')
        self.con.commit()


    def canonical_name(self, region_name):
        if region_name is None:
            return None 
        else:
            return '%'.join(sorted(set(region_name.split('%'))))


    def cache_path(self, region_name):
        region_name = self.canonical_name(region_name)
        h = hashlib.sha1(region_name.encode()).hexdigest()
        return '/'.join([self.cache_dir, h[0:2], h[2:4]])


    def region_exists(self, region_name):
        cur = self.con.cursor()
        cur.execute('select count(*) from genomic_region where name=?', [self.canonical_name(region_name)])
        r = cur.fetchone()[0] > 0
        cur.close()
        return r
        
    def add_genomic_region(self, name, parent, construction):
        new_name = self.canonical_name('%'.join([x for x in [parent, name] if x is not None]))
        
        if not self.region_exists(new_name):
            bedfile = self.cache_path(new_name) + '/' + new_name + '.bed'
            #self.con.execute('BEGIN EXCLUSIVE')
            self.con.execute('''insert into genomic_region (name, parent, construction, bedfile) values (?,?,?,?)''', (new_name, self.canonical_name(parent), construction, bedfile))
            self.con.commit()
            self.construct_genomic_region(new_name)
            #self.con.execute('BEGIN EXCLUSIVE')
            self.update_genomic_region_stats(new_name)
            self.con.commit()

        return new_name


    def intersect(self, parent, region):
        return self.add_genomic_region(region, parent, '%intersect ' + region)

        
    def construct_genomic_region(self, name):
        olddir = os.getcwd()
        cname = self.canonical_name(name)
        try:
            cur = self.con.cursor()
            cur.execute('select construction, parent, bedfile from genomic_region where name=?', [cname])
            construction, parent, bedfile = cur.fetchone()
            tmpdir = tempfile.mkdtemp(dir=self.tmp_dir)
            os.chdir(tmpdir)
            
            m = re.search('^%intersect\s+(\S+)$', construction)
            if m is not None:
                iname = self.canonical_name(m.group(1))
                cur.execute('select bedfile from genomic_region where name=?', [iname])
                ibed = cur.fetchone()[0]
                os.symlink(ibed, 'region.bed')
            else:
                subprocess.check_call(construction, shell=True)


            if parent is None:
                subprocess.check_call(self.bin_dir + '/merge_intervals_judy < region.bed > output.bed', shell=True)
            else:
                cur.execute('select bedfile from genomic_region where name=?', [parent])
                parentbed = cur.fetchone()[0]
                subprocess.check_call(self.bin_dir + '/intersectbed region.bed ' + parentbed + ' > output.bed', shell=True)


            assert(os.path.realpath(bedfile).startswith(self.base_dir + '/cache/'))
            os.renames('output.bed', bedfile)
        finally:
            #print(construction)

            os.chdir(olddir)
            assert(os.path.realpath(tmpdir).startswith(self.base_dir + '/tmp/'))
            shutil.rmtree(tmpdir)


    def update_genomic_region_stats(self, name):
        cname = self.canonical_name(name)
        cur = self.con.cursor()
        cur.execute('select bedfile from genomic_region where name=?', [cname])
        bedfile = cur.fetchone()[0]
        line = subprocess.check_output(self.bin_dir + '/bedcoverage < %s' % bedfile, shell=True).decode().rstrip()
        (intervals, basepairs) = line.split('\t')

        cur.execute('''update genomic_region set intervals=?, basepairs=?, constructed=datetime('now') where name=?''', (intervals, basepairs, cname))
        


    def point_statistics(self, parent_region, test_region, point_region):
        cur = self.con.cursor()
        r = dict()

        basepair_query = 'select basepairs from genomic_region where name=?'

        cur.execute(basepair_query, [self.canonical_name(parent_region)])
        r['basepairs_in_parent_region'] = cur.fetchone()[0]

        cur.execute(basepair_query, [self.canonical_name(parent_region + '%' + test_region)])
        r['basepairs_in_test_region'] = cur.fetchone()[0]

        cur.execute(basepair_query, [self.canonical_name(parent_region + '%' + point_region)])
        r['points_in_parent_region'] = cur.fetchone()[0]

        cur.execute(basepair_query, [self.canonical_name(parent_region + '%' + test_region + '%' + point_region)])
        r['points_in_test_region'] = cur.fetchone()[0]

        
        if r['basepairs_in_parent_region'] == 0:
            r['test_region_basepair_ratio'] = 1
        else:
            r['test_region_basepair_ratio'] = r['basepairs_in_test_region'] / \
                                              r['basepairs_in_parent_region']
        
        if r['points_in_parent_region'] == 0:
            r['test_region_point_count_ratio'] = 1
            r['enrichment_pvalue'] = 1
            r['depletion_pvalue'] = 1
            r['log2_enrichment_ratio'] = 0
            r['enrichment_ratio'] = 1.0
        else:
            r['test_region_point_count_ratio'] = r['points_in_test_region'] / r['points_in_parent_region']
            
            r['depletion_pvalue'] = binom.cdf(k=r['points_in_test_region'], 
                                              n=r['points_in_parent_region'],
                                              p=r['test_region_basepair_ratio'])

            r['enrichment_pvalue'] = 1 - \
                binom.cdf(k=r['points_in_test_region'] - 1,
                          n=r['points_in_parent_region'],
                          p=r['test_region_basepair_ratio'])

            r['enrichment_ratio'] = r['test_region_point_count_ratio'] /\
                                    r['test_region_basepair_ratio']

            if r['points_in_test_region'] == 0:
                r['log2_enrichment_ratio'] = float('-inf')
            else:
                r['log2_enrichment_ratio'] = math.log2(r['enrichment_ratio'])
            
            return r
            
        


def test0():
    gr = GenomicRegions()
    gr.init_db()
    print(gr.add_genomic_region('hg19', None, '''cut -f 1,2 /ceph/genome/human/hg19/_m/hg19.fa.fai | perl -pe 's/\t/\t0\t/' > region.bed'''))
    print(gr.add_genomic_region('chr1_10_1000', 'hg19', 'echo "chr1\t10\t1000" > region.bed'))
    print(gr.add_genomic_region('chr1_1_222', 'hg19', 'echo "chr1\t1\t222" > region.bed'))
    print(gr.add_genomic_region('chr1_10_1000', 'hg19%chr1_1_222', '%intersect hg19%chr1_10_1000'))


    print(gr.add_genomic_region('points', 'hg19', '(echo "chr1\t22\t23"; echo "chrX\t22\t23"; ) > region.bed'))

    print(gr.add_genomic_region('chr1_10_1000', 'hg19%chr1_1_222', '%intersect hg19%chr1_10_1000'))
    
    print(gr.intersect('hg19%points', 'hg19%chr1_1_222'))
    r = gr.point_statistics('hg19', 'chr1_1_222', 'points')
    print(r)


def test1():
    gr = GenomicRegions()
    gr.init_db()
    print(gr.add_genomic_region('hg19', None, '''cut -f 1,2 /ceph/genome/human/hg19/_m/hg19.fa.fai | perl -pe 's/\t/\t0\t/' > region.bed'''))
    print(gr.add_genomic_region('a75', None, '''cp /ceph/users/apua/tests/genomicregions/a75/a75.bed region.bed'''))
    print(gr.intersect('hg19', 'a75'))
    

if __name__ == '__main__':
    test1()

