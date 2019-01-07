#include <stdio.h>
#include <stdlib.h>
#include "prb_types.h"
#include "prb.h"

#include <Judy.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

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
  

  prb_int total_intervals=0;
  prb_int total_basepairs=0;

  chrom[0]='\0';
  JSLF(PElement, PJArray, chrom);
  while(PElement != NULL)
  {
    tree= (struct prb_table*) *PElement;
    total_intervals+=tree->prb_count;
    total_basepairs+=prb_sum_of_interval_lengths(tree->prb_root);
    JSLN(PElement, PJArray, chrom);
  }
  printf("%ld\t%ld\n", total_intervals, total_basepairs);
  //Word_t    Bytes;  JSLFA(Bytes, PJArray);  
  return 0;

}
