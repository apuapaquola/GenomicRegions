# GenomicRegions



This tool supports:
- Fast interval operations on genomic regions.
- Statistical enrichment analysis.
- A cache to store previoulsy computed genomic regions.

### C code: Red-black interval trees

We modified the "prb" implementation of red-black trees in the GNU libavl package so it supports interval tree operations.
 
### Python code: 

- Creation and manipulation of genomic regions.
- Cache for previoulsy computed genomic regions.
- Enrichment analysis based on the binomial distribution.

### Installation Prerequisites:

- Judy C library for dynamic arrays.
- SQLite
- Numpy/Scipy

### To do:

- pip installation script
- documentation

### License:

GNU GPL
