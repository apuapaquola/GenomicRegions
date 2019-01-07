#include <stdio.h>
#include <stdlib.h>
#include "prb_types.h"
#include "prb.h"

#include <Judy.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

void print_callback(void *_state, void *_p)
{
  struct prb_node *p=(struct prb_node *)_p;

  printf("%s\t%ld\t%ld\n", (char *) _state, ((struct prb_node *)p)->prb_low, ((struct prb_node *)p)->prb_high+1);
}

int main(int argc, char **argv)
{
  if(argc!=1)
  {
    fprintf(stderr,"Usage: $argv[0] < infile > outfile\n");
    exit(1);
  }
  
  Pvoid_t   PJArray = (PWord_t)NULL;  // Judy array.
  PWord_t   PElement;                   // Judy array element.

  struct prb_table* tree;

  char chrom[1024];
  prb_int low, high;

  int line_number=0;
  int d;
  while((d=scanf("%1023s%ld%ld", chrom, &low, &high))!=EOF)
  {
    line_number++;
    
    if(d!=3)
    {
      fprintf(stderr,"Parse error near line %d.\n",line_number);
      exit(1);
    }

    high--;

    JSLG(PElement, PJArray, chrom);
    if(PElement==NULL)
    {
      JSLI(PElement, PJArray, chrom);
      *PElement= (Word_t) prb_create(&prb_allocator_default);
    }
    tree= (struct prb_table*) *PElement;
    prb_nonoverlapping_insert(tree, low, high);
  }
  
  chrom[0]='\0';
  JSLF(PElement, PJArray, chrom);
  while(PElement != NULL)
  {
    tree= (struct prb_table*) *PElement;
    prb_traverse(tree->prb_root, print_callback, (void*) chrom);

    JSLN(PElement, PJArray, chrom);
  }
  //Word_t    Bytes;  JSLFA(Bytes, PJArray);  
  return 0;

}


/* It's faster than bedtools mergeBed

apua@loggapua:~/lixo/bedtools$ wc -l alu.bed 
1194734 alu.bed

apua@loggapua:~/lixo/bedtools$ time ~/ucsc/lib/red_black_interval_tree/merge_intervals_judy < alu.bed  > a

real	0m1.838s
user	0m1.792s
sys	0m0.044s

apua@loggapua:~/lixo/bedtools$ time /opt/BEDTools-Version-2.16.2/bin/mergeBed -i alu.bed > aa

real	0m2.986s
user	0m2.944s
sys	0m0.032s

apua@loggapua:~/lixo/bedtools$ wc -l aa
1126256 aa
apua@loggapua:~/lixo/bedtools$ wc -l a
1126346 a

apua@loggapua:~/lixo/bedtools$ grep -v chr a | sort | md5sum
ebce76f9d7ef3fb2307aeb99e9e8aa0f  -
apua@loggapua:~/lixo/bedtools$ cut -f 2,3 aa | sort | md5sum
ebce76f9d7ef3fb2307aeb99e9e8aa0f  -

*/
