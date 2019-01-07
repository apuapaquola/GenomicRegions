/* Produced by texiweb from libavl.w. */

/* libavl - library for manipulation of binary trees.
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Free Software
   Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.
*/
/* 
   Modified by Apuã Paquola to implement interval trees.
*/

#ifndef PRB_H
#define PRB_H 1

#include <stddef.h>

#include "prb_types.h"

/* Function types. */
typedef void prb_item_func (void *prb_item, void *prb_param);
typedef void *prb_copy_func (void *prb_item, void *prb_param);

#ifndef LIBAVL_ALLOCATOR
#define LIBAVL_ALLOCATOR
/* Memory allocator. */
struct libavl_allocator
  {
    void *(*libavl_malloc) (struct libavl_allocator *, size_t libavl_size);
    void (*libavl_free) (struct libavl_allocator *, void *libavl_block);
  };
#endif

/* Default memory allocator. */
extern struct libavl_allocator prb_allocator_default;
void *prb_malloc (struct libavl_allocator *, size_t);
void prb_free (struct libavl_allocator *, void *);

/* Maximum PRB height. */
#ifndef PRB_MAX_HEIGHT
#define PRB_MAX_HEIGHT 128
#endif

/* Tree data structure. */
struct prb_table
  {
    struct prb_node *prb_root;        /* Tree's root. */
    struct libavl_allocator *prb_alloc; /* Memory allocator. */
    size_t prb_count;                  /* Number of items in tree. */

    struct prb_node *freelist;        /* Free list for allocating nodes */
  };

/* Color of a red-black node. */
enum prb_color
  {
    PRB_BLACK,   /* Black. */
    PRB_RED      /* Red. */
  };

/* A red-black tree with parent pointers node. */
struct prb_node
  {
    struct prb_node *prb_link[2];  /* Subtrees. */
    struct prb_node *prb_parent;   /* Parent. */
    unsigned char prb_color;       /* Color. */

    prb_int prb_low;
    prb_int prb_high;
    
    prb_int prb_maxhigh;

    void *prb_item;
    
  };

/* Table functions. */
struct prb_table *prb_create (struct libavl_allocator *);

void prb_destroy (struct prb_table *);

void prb_empty (struct prb_table *);

struct prb_node* prb_insert (struct prb_table *tree, prb_int _low,
			     prb_int _high);

struct prb_node* prb_insert_item (struct prb_table *tree, prb_int _low,  prb_int _high, void *_item);

void prb_delete (struct prb_table *tree, struct prb_node *p);

void prb_fix_maxhigh(struct prb_table *tree, struct prb_node *p);

void prb_print(struct prb_node *p);

prb_int prb_sum_of_interval_lengths(struct prb_node *p);

struct prb_node* prb_find_overlapping_interval_in_a_nonoverlapping_tree(const struct prb_table *tree, prb_int _low, prb_int _high);

struct prb_node* prb_nonoverlapping_insert (struct prb_table *tree, prb_int _low, prb_int _high);


void prb_search(const struct prb_node *p,
		prb_int _low,
		prb_int _high,
		void (*callback)(void *, void *),
		void *state);

void prb_traverse(const struct prb_node *p,
		  void (*callback)(void *, void *),
		  void *state);


/* Freelist-based node allocation */
struct prb_node* prb_alloc_node (struct prb_table *tree);

void prb_free_node (struct prb_table *tree, struct prb_node* p);


#define prb_count(table) ((size_t) (table)->prb_count)


#endif /* prb.h */
