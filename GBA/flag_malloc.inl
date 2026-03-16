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

#ifndef FLAG_MALLOC_H
#define FLAG_MALLOC_H 1

#include "list.inl"

struct flag_chunk {
  void *mem;
  int size;
};

cx_inline
void *flag_malloc (struct list_ *list__, size_t num, void* (*malloc_)(size_t )) {
  void *chunk;
  struct flag_chunk *fc = (struct flag_chunk *) malloc (sizeof (struct flag_chunk));
  if (malloc_ == null_)
   chunk = malloc (num);
  else 
    chunk = malloc_ (num);
  fc->mem =chunk;
  fc->size = num;
  list__->usr_data += num;

  list_insert_ (list__, 0, fc);
  return chunk;
}

cx_inline 
void flag_free_callback (void *node, void *param) {
  struct flag_chunk *n = (struct flag_chunk *)node;
  if (param != null_)
    ((void (*)(void *)) param) (n->mem);
  else 
    free (n->mem);
  free (n);
}

cx_inline 
void flag_free_callback2 (void *node, void *param) {
  struct flag_chunk *n = (struct flag_chunk *)node;
  free (n);
}

/*
Using this set assignment implies that the free function will not be used, otherwise errors may occur.
cx_inline
void *flag_free (struct list_ *list__, void *ptr, void (*free_)(void * )) {
  if (free_ != null_)
   free_ (ptr);
  else 
    free (ptr);
  list_remove_ (list__, list_elem_pos_ (list__, ptr), flag_free_callback, null_);
}
*/

cx_inline 
void flag_malloc_uninit (struct list_ **list__, void (*free_)(void * )) {
  if (list__ != null) 
    list_uninit_ (list__, flag_free_callback2, (void *)free_);
  else ;
}

#endif