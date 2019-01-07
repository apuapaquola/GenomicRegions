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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "prb.h"

/* Creates and returns a new table
   with comparison function |compare| using parameter |param|
   and memory allocator |allocator|.
   Returns |NULL| if memory allocation failed. */
struct prb_table *
prb_create (struct libavl_allocator *allocator)
{
  struct prb_table *tree;

  if (allocator == NULL)
    allocator = &prb_allocator_default;

  tree = allocator->libavl_malloc (allocator, sizeof *tree);
  if (tree == NULL)
    return NULL;

  tree->prb_root = NULL;
  tree->prb_alloc = allocator;
  tree->prb_count = 0;
  
  tree->freelist = NULL;
    
  return tree;
}


void prb_fix_maxhigh_up_to_root(struct prb_table *tree, struct prb_node *p)
{
  assert(p!=NULL);

  /* don't fix an invalid root (due to root hack) */
  if(p==((struct prb_node *) &tree->prb_root))
    return;
  
  int firsttime=1;
  p->prb_maxhigh=p->prb_high;
  for (; p != NULL; p = p->prb_parent)
  {
    prb_int mh=p->prb_high;
    int dir;
    for(dir=0; dir<=1; dir++)
      if(p->prb_link[dir]!=NULL)
	if(p->prb_link[dir]->prb_maxhigh > mh)
	  mh=p->prb_link[dir]->prb_maxhigh;
    
    if(mh==p->prb_maxhigh && !firsttime)
      break; /* optimization: stop if maxhigh will not change */
    firsttime=0;
    p->prb_maxhigh=mh;
  }
}

  
/* Inserts |item| into |tree| and returns a pointer to |item|'s node.
   Returns |NULL| in case of memory allocation failure. */
struct prb_node*
prb_insert (struct prb_table *tree, prb_int _low, prb_int _high)
{
  struct prb_node *p; /* Traverses tree looking for insertion point. */
  struct prb_node *q; /* Parent of |p|; node at which we are rebalancing. */
  struct prb_node *n; /* Newly inserted node. */
  int dir;            /* Side of |q| on which |n| is inserted. */

  assert (tree != NULL && _low<=_high);

  for (q = NULL, p = tree->prb_root; p != NULL; q = p, p = p->prb_link[dir])
  {
    dir = _low <= p->prb_low ? 0 : 1;
  }

  //n = tree->prb_alloc->libavl_malloc (tree->prb_alloc, sizeof *p);
  n = prb_alloc_node(tree);
  if (n == NULL)
    return NULL;

  tree->prb_count++;
  n->prb_link[0] = n->prb_link[1] = NULL;
  n->prb_parent = q;

  if (q != NULL)
    q->prb_link[dir] = n;
  else
    tree->prb_root = n;
  n->prb_color = PRB_RED;
  
  /* Some interval tree code.. */
  n->prb_low=_low;
  n->prb_high=_high;
  prb_fix_maxhigh_up_to_root(tree, n);
  /* ..here. */
  
  q = n;
  for (;;)
    {
      struct prb_node *f; /* Parent of |q|. */
      struct prb_node *g; /* Grandparent of |q|. */

      f = q->prb_parent;
      if (f == NULL || f->prb_color == PRB_BLACK)
        break;

      g = f->prb_parent;
      if (g == NULL)
        break;

      if (g->prb_link[0] == f)
        {
          struct prb_node *y = g->prb_link[1];
          if (y != NULL && y->prb_color == PRB_RED)
            {
              f->prb_color = y->prb_color = PRB_BLACK;
              g->prb_color = PRB_RED;
              q = g;
            }
          else
            {
	      struct prb_node *h; /* Great-grandparent of |q|. */
	      
              h = g->prb_parent;
              if (h == NULL)
                h = (struct prb_node *) &tree->prb_root;

              if (f->prb_link[1] == q)
	      {
 		  f->prb_link[1] = q->prb_link[0];
                  q->prb_link[0] = f;
                  g->prb_link[0] = q;
                  f->prb_parent = q;
                  if (f->prb_link[1] != NULL)
                    f->prb_link[1]->prb_parent = f;
		  
		  prb_fix_maxhigh_up_to_root(tree, f);
		  
                  f = q;
                }

	      
              g->prb_color = PRB_RED;
              f->prb_color = PRB_BLACK;

              g->prb_link[0] = f->prb_link[1];
              f->prb_link[1] = g;
	      h->prb_link[h->prb_link[0] != g] = f;
	      
              f->prb_parent = g->prb_parent;
              g->prb_parent = f;
              if (g->prb_link[0] != NULL)
                g->prb_link[0]->prb_parent = g;

	      prb_fix_maxhigh_up_to_root(tree, g);
	      
              break;
            }
        }
      else
        {
          struct prb_node *y = g->prb_link[0];
          if (y != NULL && y->prb_color == PRB_RED)
            {
              f->prb_color = y->prb_color = PRB_BLACK;
              g->prb_color = PRB_RED;
              q = g;
            }
          else
            {
              struct prb_node *h; /* Great-grandparent of |q|. */

              h = g->prb_parent;
              if (h == NULL)
                h = (struct prb_node *) &tree->prb_root;

              if (f->prb_link[0] == q)
                {
                  f->prb_link[0] = q->prb_link[1];
                  q->prb_link[1] = f;
                  g->prb_link[1] = q;
                  f->prb_parent = q;
                  if (f->prb_link[0] != NULL)
                    f->prb_link[0]->prb_parent = f;
		  
		  prb_fix_maxhigh_up_to_root(tree, f);
		  
                  f = q;
                }

              g->prb_color = PRB_RED;
              f->prb_color = PRB_BLACK;

              g->prb_link[1] = f->prb_link[0];
              f->prb_link[0] = g;
	      h->prb_link[h->prb_link[0] != g] = f;
	      
              f->prb_parent = g->prb_parent;
              g->prb_parent = f;
              if (g->prb_link[1] != NULL)
                g->prb_link[1]->prb_parent = g;
	      
	      prb_fix_maxhigh_up_to_root(tree, g);
	      
              break;
            }
        }
    }
  tree->prb_root->prb_color = PRB_BLACK;
  
  return n;
}

struct prb_node* prb_insert_item (struct prb_table *tree, prb_int _low,  prb_int _high, void *_item)
{
  struct prb_node *p=prb_insert(tree, _low, _high);
  if(p!=NULL)
    p->prb_item=_item;
  return p;
}


/* Deletes from |tree| and returns an item matching |item|.
   Returns a null pointer if no matching item found. */
void prb_delete (struct prb_table *tree, struct prb_node *p)
{
  /* p is the node to delete. */
  struct prb_node *q; /* Parent of |p|. */
  struct prb_node *f; /* Node at which we are rebalancing. */
  int dir;            /* Side of |q| on which |p| is a child;
                         side of |f| from which node was deleted. */

  assert (tree != NULL && p != NULL);
  assert (tree->prb_root != NULL);

  q = p->prb_parent;
  if (q == NULL)
  {
    /* this is the root hack */
    q = (struct prb_node *) &tree->prb_root;
    dir = 0;
  }
  else
  {
    dir=q->prb_link[0]!=p;
  }

  if (p->prb_link[1] == NULL)
    {
      q->prb_link[dir] = p->prb_link[0];
      if (q->prb_link[dir] != NULL)
        q->prb_link[dir]->prb_parent = p->prb_parent;

      f = q;
    }
  else
    {
      enum prb_color t;
      struct prb_node *r = p->prb_link[1];

      if (r->prb_link[0] == NULL)
        {
          r->prb_link[0] = p->prb_link[0];
          q->prb_link[dir] = r;
          r->prb_parent = p->prb_parent;
          if (r->prb_link[0] != NULL)
            r->prb_link[0]->prb_parent = r;

          t = p->prb_color;
          p->prb_color = r->prb_color;
          r->prb_color = t;

          f = r;
          dir = 1;
        }
      else
        {
          struct prb_node *s = r->prb_link[0];
          while (s->prb_link[0] != NULL)
            s = s->prb_link[0];
          r = s->prb_parent;
          r->prb_link[0] = s->prb_link[1];
          s->prb_link[0] = p->prb_link[0];
          s->prb_link[1] = p->prb_link[1];
          q->prb_link[dir] = s;
          if (s->prb_link[0] != NULL)
            s->prb_link[0]->prb_parent = s;
          s->prb_link[1]->prb_parent = s;
          s->prb_parent = p->prb_parent;
          if (r->prb_link[0] != NULL)
            r->prb_link[0]->prb_parent = r;

          t = p->prb_color;
          p->prb_color = s->prb_color;
          s->prb_color = t;

          f = r;
          dir = 0;
        }
    }


  /* Some interval tree code.. */
  prb_fix_maxhigh_up_to_root(tree, f);
  /* ..here. */
  
  if (p->prb_color == PRB_BLACK)
    {
      for (;;)
        {
          struct prb_node *x; /* Node we want to recolor black if possible. */
          struct prb_node *g; /* Parent of |f|. */
          struct prb_node *t; /* Temporary for use in finding parent. */

          x = f->prb_link[dir];
          if (x != NULL && x->prb_color == PRB_RED)
            {
              x->prb_color = PRB_BLACK;
              break;
            }

          if (f == (struct prb_node *) &tree->prb_root)
            break;

          g = f->prb_parent;
          if (g == NULL)
            g = (struct prb_node *) &tree->prb_root;

          if (dir == 0)
            {
              struct prb_node *w = f->prb_link[1];

              if (w->prb_color == PRB_RED)
                {
                  w->prb_color = PRB_BLACK;
                  f->prb_color = PRB_RED;

                  f->prb_link[1] = w->prb_link[0];
                  w->prb_link[0] = f;
                  g->prb_link[g->prb_link[0] != f] = w;

                  w->prb_parent = f->prb_parent;
                  f->prb_parent = w;

                  g = w;
                  w = f->prb_link[1];

                  w->prb_parent = f;

		  prb_fix_maxhigh_up_to_root(tree, w);
                }

              if ((w->prb_link[0] == NULL
                   || w->prb_link[0]->prb_color == PRB_BLACK)
                  && (w->prb_link[1] == NULL
                      || w->prb_link[1]->prb_color == PRB_BLACK))
                {
                  w->prb_color = PRB_RED;
                }
              else
                {
                  if (w->prb_link[1] == NULL
                      || w->prb_link[1]->prb_color == PRB_BLACK)
                    {
                      struct prb_node *y = w->prb_link[0];
                      y->prb_color = PRB_BLACK;
                      w->prb_color = PRB_RED;
                      w->prb_link[0] = y->prb_link[1];
                      y->prb_link[1] = w;
                      if (w->prb_link[0] != NULL)
                        w->prb_link[0]->prb_parent = w;
                      w = f->prb_link[1] = y;
                      w->prb_link[1]->prb_parent = w;

		      prb_fix_maxhigh_up_to_root(tree, w->prb_link[1]);
                    }

                  w->prb_color = f->prb_color;
                  f->prb_color = PRB_BLACK;
                  w->prb_link[1]->prb_color = PRB_BLACK;

                  f->prb_link[1] = w->prb_link[0];
                  w->prb_link[0] = f;
                  g->prb_link[g->prb_link[0] != f] = w;

                  w->prb_parent = f->prb_parent;
                  f->prb_parent = w;
                  if (f->prb_link[1] != NULL)
                    f->prb_link[1]->prb_parent = f;

		  prb_fix_maxhigh_up_to_root(tree, f);
                  break;
                }
            }
          else
            {
              struct prb_node *w = f->prb_link[0];

              if (w->prb_color == PRB_RED)
                {
                  w->prb_color = PRB_BLACK;
                  f->prb_color = PRB_RED;

                  f->prb_link[0] = w->prb_link[1];
                  w->prb_link[1] = f;
                  g->prb_link[g->prb_link[0] != f] = w;

                  w->prb_parent = f->prb_parent;
                  f->prb_parent = w;

                  g = w;
                  w = f->prb_link[0];

                  w->prb_parent = f;
		  
		  prb_fix_maxhigh_up_to_root(tree, w);
                }

              if ((w->prb_link[0] == NULL
                   || w->prb_link[0]->prb_color == PRB_BLACK)
                  && (w->prb_link[1] == NULL
                      || w->prb_link[1]->prb_color == PRB_BLACK))
                {
                  w->prb_color = PRB_RED;
                }
              else
                {
                  if (w->prb_link[0] == NULL
                      || w->prb_link[0]->prb_color == PRB_BLACK)
                    {
                      struct prb_node *y = w->prb_link[1];
                      y->prb_color = PRB_BLACK;
                      w->prb_color = PRB_RED;
                      w->prb_link[1] = y->prb_link[0];
                      y->prb_link[0] = w;
                      if (w->prb_link[1] != NULL)
                        w->prb_link[1]->prb_parent = w;
                      w = f->prb_link[0] = y;
                      w->prb_link[0]->prb_parent = w;
		      prb_fix_maxhigh_up_to_root(tree, w->prb_link[0]);
                    }

                  w->prb_color = f->prb_color;
                  f->prb_color = PRB_BLACK;
                  w->prb_link[0]->prb_color = PRB_BLACK;

                  f->prb_link[0] = w->prb_link[1];
                  w->prb_link[1] = f;
                  g->prb_link[g->prb_link[0] != f] = w;

                  w->prb_parent = f->prb_parent;
                  f->prb_parent = w;
                  if (f->prb_link[0] != NULL)
                    f->prb_link[0]->prb_parent = f;

		  prb_fix_maxhigh_up_to_root(tree, f);
                  break;
                }
            }

          t = f;
          f = f->prb_parent;
          if (f == NULL)
            f = (struct prb_node *) &tree->prb_root;
          dir = f->prb_link[0] != t;
        }
    }

  //tree->prb_alloc->libavl_free (tree->prb_alloc, p);
  prb_free_node(tree, p);
  tree->prb_count--;
}

/* Frees storage allocated for |tree|.
   If |destroy != NULL|, applies it to each data item in inorder. */
void
prb_destroy (struct prb_table *tree)
{
  struct prb_node *p, *q;

  assert (tree != NULL);

  for (p = tree->prb_root; p != NULL; p = q)
    if (p->prb_link[0] == NULL)
      {
        q = p->prb_link[1];
        tree->prb_alloc->libavl_free (tree->prb_alloc, p);
      }
    else
      {
        q = p->prb_link[0];
        p->prb_link[0] = q->prb_link[1];
        q->prb_link[1] = p;
      }

  for (p = tree->freelist; p != NULL; p = q)
  {
    q=p->prb_parent;
    tree->prb_alloc->libavl_free (tree->prb_alloc, p);
  }

  tree->prb_alloc->libavl_free (tree->prb_alloc, tree);
}

/* Deletes all nodes of |tree|, leaving it empty. Storage is returned to the free list. */
void
prb_empty (struct prb_table *tree)
{
  struct prb_node *p, *q;

  assert (tree != NULL);

  for (p = tree->prb_root; p != NULL; p = q)
    if (p->prb_link[0] == NULL)
      {
        q = p->prb_link[1];
        prb_free_node(tree, p);
      }
    else
      {
        q = p->prb_link[0];
        p->prb_link[0] = q->prb_link[1];
        q->prb_link[1] = p;
      }

  tree->prb_root=NULL;
  tree->prb_count=0;
}


void prb_print(struct prb_node *p)
{
  if(p==NULL) return;
  prb_print(p->prb_link[0]);
  printf("%ld\t%ld\n", p->prb_low, p->prb_high);
  prb_print(p->prb_link[1]);
}

prb_int prb_sum_of_interval_lengths(struct prb_node *p)
{
  if(p==NULL) return 0;
  prb_int r=0;
  r+=prb_sum_of_interval_lengths(p->prb_link[0]);
  r+=p->prb_high-p->prb_low+1;
  r+=prb_sum_of_interval_lengths(p->prb_link[1]);
  return r;
}


/* Allocates |size| bytes of space using |malloc()|.
   Returns a null pointer if allocation fails. */
void *
prb_malloc (struct libavl_allocator *allocator, size_t size)
{
  assert (allocator != NULL && size > 0);
  return malloc (size);
}

/* Frees |block|. */
void
prb_free (struct libavl_allocator *allocator, void *block)
{
  assert (allocator != NULL && block != NULL);
  free (block);
}

/* Default memory allocator that uses |malloc()| and |free()|. */
struct libavl_allocator prb_allocator_default =
  {
    prb_malloc,
    prb_free
  };

struct prb_node* prb_alloc_node (struct prb_table *tree)
{
  struct prb_node *p;
  if(tree->freelist==NULL)
  {
    p = tree->prb_alloc->libavl_malloc (tree->prb_alloc, sizeof *p);
  }
  else
  {
    p=tree->freelist;
    tree->freelist=p->prb_parent;
  }
  return p;
}

void prb_free_node (struct prb_table *tree, struct prb_node* p)
{
  assert (p != NULL);
  p->prb_parent=tree->freelist;
  tree->freelist=p;
}


struct prb_node* prb_find_overlapping_interval_in_a_nonoverlapping_tree(const struct prb_table *tree, prb_int _low, prb_int _high)
{
  struct prb_node *p;
  assert (tree != NULL && _low<=_high);
  
  for (p = tree->prb_root; p != NULL; )
  {
    if(_low <= p->prb_low)
      if(p->prb_low <= _high)
	return p;
      else
	p = p->prb_link[0];
    else
      if(_low <= p->prb_high)
	return p;
      else
	p = p->prb_link[1];
  }
  
  return NULL;
}

/* Inserts |item| into |tree| and returns a pointer to |item|'s node.
   If overlapping intervals are found, they are deleted, and a new
   interval that contains all deleted intervals plus the newly
   inserted one is created. Returns |NULL| in case of memory
   allocation failure. */
struct prb_node*
prb_nonoverlapping_insert (struct prb_table *tree, prb_int _low, prb_int _high)
{
  struct prb_node* nn;

  while((nn=prb_find_overlapping_interval_in_a_nonoverlapping_tree(tree, _low, _high))!=NULL)
  {
    if(_low>=nn->prb_low && _high<=nn->prb_high) return nn;
    
    _low=  _low < nn->prb_low ? _low : nn->prb_low;
    _high= _high > nn->prb_high ? _high : nn->prb_high;


    prb_delete(tree, nn);
  }
  return prb_insert(tree, _low, _high);
}


void prb_search(const struct prb_node *p,
		prb_int _low,
		prb_int _high,
		void (*callback)(void *, void *),
		void *state)
{
  assert(_low<=_high);
  if(p==NULL) return;
  if(_low > p->prb_maxhigh) return;

  prb_search(p->prb_link[0], _low, _high, callback, state);

  if(p->prb_high >= _low && _high>=p->prb_low)
    (*callback)(state, (void*) p);
  
  if(_high >= p->prb_low)
    prb_search(p->prb_link[1], _low, _high, callback, state);
  
  return;
}


void prb_traverse(const struct prb_node *p,
		  void (*callback)(void *, void *),
		  void *state)
{
  if(p==NULL) return;
  prb_traverse(p->prb_link[0], callback, state);
  (*callback)(state, (void*) p);
  prb_traverse(p->prb_link[1], callback, state);
}

