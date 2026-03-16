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

#include "gba.h"

struct coroutine {
  int (callc *startup)(void *param);
  void *param;
  void *stack;
  int size;
  struct context *link;
  struct context *current; 
  struct context *cache;
  struct context *init;
  struct context *exitus;
  struct context *temp;
};

struct context *coroutine_getctx (struct coroutine *co, COROUTINE_EXEC_OBJ e_obj) {
  switch (e_obj) {
  case COROUTINE_EXEC_CACHE:
    return co->cache;
  case COROUTINE_EXEC_CURRENT:
    return co->current;
  case COROUTINE_EXEC_EXITUS:
    return co->exitus;
  case COROUTINE_EXEC_INIT:
    return co->init;
  case COROUTINE_EXEC_LINK:
    return co->link;
  default:
    assert (0);
    break;
  }
  return null;
}

void coroutine_setctx (struct coroutine *co, COROUTINE_EXEC_OBJ e_obj, struct context * ctx) {
  switch (e_obj) {
  case COROUTINE_EXEC_CACHE:
    context_copy  (co->cache, ctx);
    break;
  case COROUTINE_EXEC_CURRENT:
    context_copy  (co->current, ctx);
    break;
  case COROUTINE_EXEC_EXITUS:
    context_copy  (co->exitus, ctx);
    break;
  case COROUTINE_EXEC_INIT:
    context_copy  (co->init, ctx);
    break;
  case COROUTINE_EXEC_LINK:
    context_copy  (co->link, ctx);
    break;
  default:
    assert (0);
    break;
  }
}

int coroutine_ctor (struct coroutine **co, int stack_size) {
  struct coroutine *p = (struct coroutine *) malloc (sizeof (struct  coroutine));
  assert (p != null);

  p->size = stack_size;
  p->stack = malloc (stack_size);
  assert (p->stack  != null);
  assert (co != null);

  context_init (& p->link);
  context_init (& p->current);
  context_init (& p->exitus);
  context_init (& p->init);
  context_init (& p->cache);
  context_init (& p->temp);

  context_get (p->link);
  context_get (p->current);
  context_get (p->exitus);
  context_get (p->init);
  context_get (p->cache);
  context_get (p->temp);

  p->startup = null;
  p->param = null;

  * co = p;
  return 0;
}

void coroutine_dtor (struct coroutine *p) {
  if (p != null) {
    if (p->link) context_uninit (& p->link);
    if (p->current) context_uninit (& p->current);
    if (p->exitus) context_uninit (& p->exitus);
    if (p->init) context_uninit (& p->init);
    if (p->cache) context_uninit (& p->cache);
    if (p->temp) context_uninit (& p->temp);
    if (p->stack != null) free (p->stack);
    if (p != null) free (p);
  }
}

void coroutine_setcall (struct coroutine *co, int (callc *startup)(void *param), void *param) {
  co->startup = startup;
  co->param = param;
}

void coroutine_swito (struct coroutine *co, kable breakout, COROUTINE_EXEC_OBJ e_obj, struct context *ctx) {
  struct context *ctxptr;
  assert (co != null);
  if (e_obj == COROUTINE_EXEC_USER)
    ctxptr = ctx;
  else 
    ctxptr = coroutine_getctx (co, e_obj);
  assert (ctxptr != null);
  if (breakout != false) {
    context_copy (co->temp, ctxptr);
    context_swap2 (co->temp, co->current);
  } else {
    context_copy (co->current, ctx);
    context_set (co->current);
  }
}

static void 
 callc coroutine_bear (struct coroutine *co) {
  context_copy (co->exitus, co->link);
  context_get (co->init);
  if (co->startup != null)
    co->startup (co->param);
  context_set (co->link);
}

void coroutine_run (struct coroutine *co) {
  uintptr_t *stack = & ((uintptr_t *)co->stack)[co->size/sizeof (uintptr_t)-32];
  stack = (uintptr_t *)((uintptr_t)stack & ~((uintptr_t)stack & 7));
  stack[1] = (uintptr_t) co;  /* XXX: Only for x86-32 and wow64 */

  context_setpc (co->current, coroutine_bear);
  context_setstack (co->current, (uintptr_t) stack);
  context_swap2 (co->current, co->link);
}

void SampleCallback (void *param) {
  printf ("Hello World\n");
}

#if 0
context_t ctx;

void setit (void) {

  printf ("Red World\n");
  context_swap2 (ctx, ctx);
  printf ("White World\n");
  context_swap2 (ctx, ctx);
}

void setit2 (void) {

  printf ("Black World\n");
  context_swap2 (ctx, ctx);
  printf ("Green World\n");
  context_swap2 (ctx, ctx);
}

int main (void) {

  context_init (& ctx);
  context_get (ctx);
  context_setpc (ctx, setit2);
  setit ();

  printf ("Cyan World\n");
  // context_swap2 (ctx, ctx);
  context_uninit (& ctx);
  return 0;
}



#endif


#if 0
int main (void) {
  struct coroutine *p;

  coroutine_ctor (&p, 400000);
  p->startup = SampleCallback;
  p->param = null;

  coroutine_run (p);
  coroutine_dtor (p);
  return 0;
}
                             


#endif



