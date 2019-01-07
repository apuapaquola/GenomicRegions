#include <stdio.h>
#include <stdlib.h>
#include "prb_types.h"
#include "prb.h"

#include <Judy.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

Pvoid_t populate_tree_merging_intervals(char *fn)
{
  FILE* fp=fopen(fn,"r");
  if(fp==NULL)
  {
    fprintf(stderr,"Could not open file %s.\n",fn);
    exit(1);
  }

  Pvoid_t   PJArray = (PWord_t)NULL;  // Judy array.
  PWord_t   PElement;                 // Judy array element.
  struct prb_table* tree;

  char chrom[1024];
  prb_int low, high;

  int line_number=0;
  int d;
  while((d=fscanf(fp,"%1023s%ld%ld", chrom, &low, &high))!=EOF)
  {
    line_number++;
    
    if(d!=3)
    {
      fprintf(stderr,"Parse error near line %d.\n",line_number);
      exit(1);
    }
    high--; // internally, all coordinates are 0-based, bed files are 0-based for the start coordinate and 1-based for the end.
    
    JSLG(PElement, PJArray, chrom);
    if(PElement==NULL)
    {
      JSLI(PElement, PJArray, chrom);
      *PElement= (Word_t) prb_create(&prb_allocator_default);
    }
    tree= (struct prb_table*) *PElement;
    prb_nonoverlapping_insert(tree, low, high);
  }
  fclose(fp);
  return PJArray;
}

typedef struct
{
  char *chrom;
  prb_int low;
  prb_int high;
} poic_state_t;

void print_overlapping_interval_callback(void *_state, void *_p)
{
  poic_state_t *s=(poic_state_t *) _state;
  struct prb_node *p=(struct prb_node *)_p;

  printf("%s\t%ld\t%ld\n", s->chrom, MAX(s->low, p->prb_low), 1+MIN(s->high, p->prb_high));
}


void print_overlaps(Pvoid_t PJArray, char *fn)
{
  FILE* fp=fopen(fn,"r");
  if(fp==NULL)
  {
    fprintf(stderr,"Could not open file %s.\n",fn);
    exit(1);
  }

  PWord_t   PElement;                 // Judy array element.
  struct prb_table* tree;

  char chrom[1024];
  prb_int low, high;

  int line_number=0;
  int d;
  while((d=fscanf(fp,"%1023s%ld%ld", chrom, &low, &high))!=EOF)
  {
    line_number++;
    
    if(d!=3)
    {
      fprintf(stderr,"Parse error near line %d.\n",line_number);
      exit(1);
    }
    high--;

    JSLG(PElement, PJArray, chrom);

    if(PElement!=NULL)
    {
      tree= (struct prb_table*) *PElement;
      poic_state_t s;
      s.chrom=chrom;
      s.low=low;
      s.high=high;
      prb_search(tree->prb_root, low, high, print_overlapping_interval_callback, &s);
    }
  }
  fclose(fp);
}



int main(int argc, char **argv)
{
  if(argc!=3)
  {
    fprintf(stderr,"Usage: $argv[0] fn1 fn2 > outfile\n");
    exit(1);
  }
  
  Pvoid_t PJArray = populate_tree_merging_intervals(argv[1]);
  print_overlaps(PJArray, argv[2]);


  /* PWord_t PElement; */
  /* struct prb_table* tree; */
  /* char chrom[1024]; */
  /* chrom[0]='\0'; */
  /* JSLF(PElement, PJArray, chrom); */
  /* while(PElement != NULL) */
  /* { */
  /*   printf("%s\n", chrom); */
  /*   tree= (struct prb_table*) *PElement; */
  /*   prb_print(tree->prb_root); */
  /*   JSLN(PElement, PJArray, chrom); */
  /* } */
  /* //Word_t    Bytes;  JSLFA(Bytes, PJArray);   */
  return 0;

}
