#include <stdio.h>
#include <stdlib.h>
#include "prb_types.h"
#include "prb.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

int main(int argc, char **argv)
{
  if(argc!=1)
  {
    fprintf(stderr,"Usage: $argv[0] < infile > outfile\n");
    exit(1);
  }
  
  struct prb_table* tree= prb_create(&prb_allocator_default);
  struct prb_node *p;

  prb_int low, high;

  while(scanf("%ld%ld", &low, &high)!=EOF)
  {
    prb_nonoverlapping_insert(tree, low, high);
  }
  
  prb_print(tree->prb_root);
  return 0;
}
