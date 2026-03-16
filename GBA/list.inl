/* Copyripht 2019 moecmks (agalis01@outlook.com)
   This file is part of ArchBoyAdvance.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef LIST_H
#define LIST_H 1

#include "stdint.h"
#include <assert.h>
#include <malloc.h>

#ifndef null_ 
# ifdef __cplusplus
#  ifdef __BORLANDC__
#   define null_ 0
#  else 
#   define null_ nullptr
#  endif 
# else 
#  define null_ (void *)0
# endif 
#endif 
#define cx_inline static

struct list_chunk {
  struct list_chunk *next;
  struct list_chunk *prev;

  union {
    void *dat_ptr;
    intptr_t dat_int;
    uintptr_t dat_uint;
  };
};
struct list_ {
  struct list_chunk head;
  intptr_t count;
  intptr_t step_b[4];
  intptr_t step_phase[4];
  intptr_t usr_data;
  intptr_t usr_data2;
  struct list_chunk *chunk_step[4];
};

cx_inline 
int list_init_ (struct list_ **list__) {
  struct list_ *p = (struct list_ *) malloc (sizeof (struct list_));
  assert (list__ != null_);
  assert (p != null_);

  p->count = 0;
  p->head.dat_int = 0;
  p->head.next =
  p->head.prev = (struct list_chunk *)null_;
  p->usr_data = 0;
  p->usr_data2 = 0;

  *list__ = p;
  return 0;
}

cx_inline 
int list_init_min_ (struct list_ **list__) {
  struct list_ *p = (struct list_ *) malloc (sizeof (struct list_chunk) + sizeof (intptr_t));
  assert (list__ != null_);
  assert (p != null_);

  p->count = 0;
  p->head.dat_int = 0;
  p->head.next =
  p->head.prev = (struct list_chunk *)null_;

  *list__ = p;
  return 0;
}

cx_inline 
void list_uninit_ (struct list_ **list__, void (*node_dtor)(void *, void *), void *param) {
  assert (list__ != null_);
  if (list__ != null_) {
    intptr_t id;
    struct list_ *p = *list__;
    struct list_chunk *u, *v;
    *list__ = (struct list_ *)null_;
    if (p->count <= 0) {
      free (p);
      return ;
    }
    u = p->head.next;
    for (id = 0; id != p->count; id++) {
      v = u->next;
      if (node_dtor != null_)
        node_dtor (u->dat_ptr, param);
      /* free chunk block. */
      free (u);
      u = v;
    }
    /* free main list- chunk */
    free (p);
  }
}


cx_inline /* after list_insert, The previous element position at that location will be + 1 */
int list_insert_ (struct list_ *list__, int pos /* if pos equal -1, insert tail*/, void *obj) {
  struct list_chunk *p;
  struct list_chunk *u;
  assert (list__ != null_);
  assert (list__->count  >= pos || pos == -1);

  if (pos == -1)
    pos = list__->count;
  if (list__->count == 0) {
    /* insert first elem. */
    p = (struct list_chunk *) malloc (sizeof (struct list_chunk));
    p->next = 
    p->prev = p;
    p->dat_ptr = obj;

    list__->head.prev = 
    list__->head.next = p;
  } else {
    /* normal insert */
    intptr_t id;
    p = (struct list_chunk *) malloc (sizeof (struct list_chunk));
    p->dat_ptr = obj;

    u = list__->head.next;
    for (id = 0; id != pos; id++) 
      u = u->next;
    if (pos == 0)
      list__->head.prev =
      list__->head.next = p;
    p->next =u;
    p->prev =u->prev;
    u->prev->next = p;
    u->prev = p;
  }
  list__->count ++;
  return 0;
}

cx_inline 
int list_insert_tail_ (struct list_ *list__, void *obj) {
  struct list_chunk *p;
  assert (list__ != null_);

  if (list__->count == 0) {
    p = (struct list_chunk *) malloc (sizeof (struct list_chunk));
    p->next = 
    p->prev = p;
    p->dat_ptr = obj;

    list__->head.prev = 
    list__->head.next = p;
  } else {
    p = (struct list_chunk *) malloc (sizeof (struct list_chunk));
    p->dat_ptr = obj;
    p->next =list__->head.next;
    p->prev =list__->head.next->prev;
    list__->head.next->prev->next = p;
    list__->head.next->prev = p;
  }
  list__->count ++;
  return 0;
}

cx_inline 
void list_remove_ (struct list_ *list__, int pos /* if pos equal -1, remove tail*/, void (*node_dtor)(void *obj, void *param), void *param) {
  struct list_chunk *u;
  assert (list__ != null_);
  assert (list__->count - 1 >= pos || pos == -1);

  if (pos == -1)
    pos = list__->count -1;
  if (list__->count == 0) {
    assert (0);
  } else {
    /* normal remove */
    intptr_t id;

    u = list__->head.next;
    for (id = 0; id != pos; id++) 
      u = u->next;
    if (pos == 0)
      list__->head.next = 
      list__->head.prev = u->next;
    if (node_dtor != null_)
      node_dtor (u->dat_ptr, param);
    u->prev->next = u->next;
    u->next->prev = u->prev;   
    if (--list__->count == 0)
      list__->head.next = 
      list__->head.prev = (struct list_chunk *)null_;
    free (u);
  }
}

cx_inline 
void list_remove_body_ (struct list_ *list__, struct list_chunk *chunk) {
  struct list_chunk *u;
  assert (list__ != null_);

  if (list__->count > 0) {
    u = chunk;
    if (list__->head.next == u)
      list__->head.next = 
      list__->head.prev = u->next;
    u->prev->next = u->next;
    u->next->prev = u->prev;   
    if (--list__->count == 0)
      list__->head.next = 
      list__->head.prev = (struct list_chunk *)null_;
    free (u);
  }
}

cx_inline 
void list_remove_cmp_ (struct list_ *list__, void *obj, void (*node_dtor)(void *obj, void *param), void *param) {
  int id;
  struct list_chunk *u;
  assert (list__ != null_);

  u = list__->head.next;
  for (id = 0; id != list__->count; id++) {
    if (u->dat_ptr == obj) {
      if (node_dtor != null_)
        node_dtor (u->dat_ptr, param);
      /*remove chunk */
      if (id == 0)
        list__->head.next = 
        list__->head.prev = u->next;
      u->prev->next = u->next;
      u->next->prev = u->prev;   
      if (--list__->count == 0)
        list__->head.next = 
        list__->head.prev = (struct list_chunk *)null_;
      free (u);
      return ;
    }
    u = u->next;
  }
}

cx_inline 
struct list_chunk *list_elem_list (struct list_ *list__, int pos /* if pos equal -1, get tail*/) {
  struct list_chunk *u;
  assert (list__ != null_);
  assert (list__->count - 1 >= pos || pos == -1);

  if (pos == -1)
    pos = list__->count -1;
  if (list__->count == 0) {
    assert (0);
  } else {
    /* normal elem */
    intptr_t id;

    u = list__->head.next;
    for (id = 0; id != pos; id++) 
      u = u->next;
    return u;
  }
  return null_;
}

cx_inline 
void *list_elem_ (struct list_ *list__, int pos /* if pos equal -1, get tail*/) {

  struct list_chunk *u = list_elem_list (list__, pos);
  if (u == null_)
    return null_;
  else 
    return u->dat_ptr;
}

cx_inline 
struct list_chunk *list_elem_list_base_list (struct list_chunk *lchunk, int offset) {
  int id;
  struct list_chunk *u;
  assert (lchunk != null_);

  u = lchunk;
  for (id = 0; id != offset; id++)
    u = u->next;
  return u;
}

cx_inline 
int list_elem_pos_ (struct list_ *list__, void *cmp_ptr) {
  intptr_t id;
  struct list_chunk *u;
  assert (list__ != null_);

  u = list__->head.next;
  for (id = 0; id != list__->count; id++)  { 
    if (u->dat_ptr == cmp_ptr)
      return id;
    else ;
    u = u->next;
  }
  return -1;
}

cx_inline 
struct list_chunk *list_step_init (struct list_ *list__, int slot, int base_pos, int step_phase) {
  assert (list__ != null_);

  list__->step_b[slot] = base_pos;
  list__->step_phase[slot] = step_phase;
  list__->chunk_step[slot] = list_elem_list (list__, base_pos);

  if (list__->chunk_step == null_)
    return null_;
  else
    return list__->chunk_step[slot];
}

#define LIST_STEP_ADV_AFTER 0
#define LIST_STEP_ADV_BEFORE 1

cx_inline 
struct list_chunk *list_step (struct list_ *list__, int slot, int startup_type) {
  intptr_t id;
  struct list_chunk *u = list__->chunk_step[slot];
  struct list_chunk *t = u;
  assert (list__ != null_);

  for (id = 0; id != list__->step_phase[slot]; id++)
    list__->chunk_step[slot] = list__->chunk_step[slot]->next;
  if (startup_type == LIST_STEP_ADV_BEFORE)
    return list__->chunk_step[slot];
  else if  (startup_type == LIST_STEP_ADV_AFTER)
    return t;
  else 
    assert (0);
  return null_;
}
/* macro public: LIST_NODE <-  per node */
#define LIST_CLEAR_BEGIN(/*  struct list_ * */ list_r_, type)\
do {\
  struct list_chunk *save = null_;\
  struct list_chunk *poll;\
  struct list_ *lroot = ((struct list_ *)list_r_);\
  int id;\
  poll = lroot->head.next;\
  for (id = 0; id != lroot->count; id++) {\
    type LIST_NODE;\
    save = poll->next;\
    LIST_NODE = (type )poll->dat_ptr

#define LIST_CLEAR_END()\
    free (poll);\
    poll = save; \
  } \
  lroot->count = 0;\
} while (0)

#define LIST_DESTROY_BEGIN(/*  struct list_ * */ list_r_, type)\
  LIST_CLEAR_BEGIN (list_r_, type)

#define LIST_DESTROY_END(list_r_)\
    free (poll);\
    poll = save; \
  } \
  free (list_r_);\
} while (0)

#define LIST_EACH_(/*  struct list_ * */ list_r_, /* int */ id, /* struct list_chunk * */p) \
  for (id = 0, p = ((struct list_ *)(list_r_))->head.next; (id) != ((struct list_ *)(list_r_))->count; (id)++, (p) = (p)->next)

/* macro public: LIST_NODE <-  per node 
               public: LIST_SAVE <-   backsave next
               public: LIST_POLL <-   curent list item
*/
#define LIST_EACH2_BEGIN(/*  struct list_ * */ list_r_, type)\
do {\
  struct list_chunk *LIST_SAVE = null_;\
  struct list_chunk *LIST_POLL;\
  struct list_ *lroot = ((struct list_ *)list_r_);\
  int LIST_ID;\
  LIST_POLL = lroot->head.next;\
  for (LIST_ID = 0; LIST_ID != lroot->count; LIST_ID++) {\
    type LIST_NODE;\
    LIST_SAVE = LIST_POLL->next;\
    LIST_NODE = (type )LIST_POLL->dat_ptr

#define LIST_EACH2_END()\
    LIST_POLL = LIST_SAVE; \
  } \
} while (0)

#if 0

#include <stdio.h>

void elem_out (void *elem) {
  int n = (int)elem;
  printf ("elem:%d\n", n);
}
int main (void) {
  int id;
  struct list_ *k;
  int sig = list_init_ (& k);
  assert (sig == 0);

  for (id = 0; id != 5533; id++) 
    list_insert_ (k, -1, (void *)id);
    /* list_remove (k, 2, (void *)id); */
  list_uninit_ (& k, elem_out);
  return 0;
}


#endif 

#endif