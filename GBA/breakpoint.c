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

/* FIXME: Bad code */
#include "gba.h"
#include "cpu.inl"
#include "flag_malloc.inl"

#define NODE_VAL_MEM 0
#define NODE_VAL_CONSTANT 1
#define NODE_VAL_ARM7_REGISTER 2
#define NODE_VAL_NW 3
#define NODE_VAL_OMEM 4
#define NODE_VAL_CNTCUR 5
#define NODE_VAL_CNTSET 6
#define NODE_VAL_REASON 7
#define NODE_VAL_CORRECT_PC 8
#define NODE_VAL_TIMER_RELOAD 9
#define NODE_VAL_INT_ENUM 10
#define NODE_VAL_OUT 11
#define NODE_VAL_UB -1
typedef int NODE_VAL_TYPE;

#define ARM7_FIELD_CUR 1
#define ARM7_FIELD_SVC 2
#define ARM7_FIELD_IRQ 4
#define ARM7_FIELD_USR 6
#define ARM7_FIELD_SYS ARM7_FIELD_USR
typedef int ARM_FIELD_MASK;

#define SLINK_POST 0 /* +*/
#define SLINK_NEG 1 /*-*/
#define SLINK_NOT 2 /*~*/
#define SLINK_NOT_BOOL 3 /*!*/
typedef int SLINK_TYPE;

#define MEM_BLOCK_BYTE 0
#define MEM_BLOCK_HALFWORD 1 
#define MEM_BLOCK_WORD 2
#define MEM_BLOCK_UB -1
typedef int MEM_BLOCK;

#define RES_EOF -1
#define RES_ERROR_ -2
#define RES_NOMATCH_INTERRUPT -2
#define RES_OK 0

#define LINK_BASE 0
#define LINK_ABOVE LINK_BASE
#define LINK_BELOW (LINK_ABOVE+1)
#define LINK_ABOVE_EQUAL (LINK_BELOW+1)
#define LINK_BELOW_EQUAL (LINK_ABOVE_EQUAL+1)
#define LINK_EQUAL (LINK_BELOW_EQUAL+1)
#define LINK_NOTEQUAL (LINK_EQUAL+1) 
#define LINK_AND (LINK_NOTEQUAL+1)
#define LINK_OR (LINK_AND+1) 
#define LINK_NOT (LINK_OR+1)
#define LINK_LEFT_ENTRY (LINK_NOT+1) /* ( */
#define LINK_RIGHT_ENTRY (LINK_LEFT_ENTRY+1) /* ) */
#define LINK_XOR (LINK_RIGHT_ENTRY+1)
#define LINK_SHR (LINK_XOR+1) 
#define LINK_SHL (LINK_SHR+1)
#define LINK_BOOL_OR (LINK_SHL+1)
#define LINK_BOOL_AND (LINK_BOOL_OR+1)
#define LINK_ADD (LINK_BOOL_AND+1)
#define LINK_SUB (LINK_ADD+1)
#define LINK_MUL (LINK_SUB+1)
#define LINK_MOD (LINK_MUL+1)
#define LINK_DIV (LINK_MOD+1)
typedef int LINK_TYPE;

/* pri. 
    +, -, ~, ! (perfix).
    *
    -, + 
    >>, << 
    >=, <=, <,>, 
    !=, == 
    & (bit)
    ^
    |
    && 
    ||  */
#define LINK_PRI_MUL 0
#define LINK_PRI_DIV 0
#define LINK_PRI_MOD 0
#define LINK_PRI_ADD 1
#define LINK_PRI_SUB LINK_PRI_ADD
#define LINK_PRI_SHL 2
#define LINK_PRI_SHR LINK_PRI_SHL

#define LINK_PRI_ABOVE 3
#define LINK_PRI_ABOVE_EQUAL LINK_PRI_ABOVE
#define LINK_PRI_BELOW LINK_PRI_ABOVE
#define LINK_PRI_BELOW_EQUAL LINK_PRI_ABOVE

#define LINK_PRI_EQUAL 4
#define LINK_PRI_NOT_EQUAL LINK_PRI_EQUAL

#define LINK_PRI_AND 5 
#define LINK_PRI_XOR 6
#define LINK_PRI_OR 7
#define LINK_PRI_BOOL_AND 8
#define LINK_PRI_BOOL_OR 9
typedef int LINK_PRI;

static 
const struct _tag_keyword {
  const char *token;
  MEM_BLOCK mblk;
  uint32_t toklen;
  uint32_t addr;
} keyword [] = {
#define SET_KEYWORD_INIT(token, addr)\
  { token, MEM_BLOCK_UB , sizeof(token) - 1, addr }
#define SET_KEYWORDIO8_INIT(token, addr)\
  { token, MEM_BLOCK_BYTE, sizeof(token) - 1, 0x4000000 + (addr) }
#define SET_KEYWORDIO16_INIT(token, addr)\
  { token, MEM_BLOCK_HALFWORD, sizeof(token) - 1, 0x4000000 + (addr) }
#define SET_KEYWORDIO32_INIT(token, addr)\
  { token, MEM_BLOCK_WORD, sizeof(token) - 1, 0x4000000 + (addr) }

#define KEYWORD_PC_ID 0
SET_KEYWORD_INIT("pc", 0),
#define KEYWORD_LR_ID (KEYWORD_PC_ID + 1)
SET_KEYWORD_INIT("lr", 0),
#define KEYWORD_SP_ID (KEYWORD_LR_ID + 1)
SET_KEYWORD_INIT("sp", 0),
#define KEYWORD_CPSR_ID (KEYWORD_SP_ID + 1)
SET_KEYWORD_INIT("cpsr", 0),
#define KEYWORD_SPSR_ID (KEYWORD_CPSR_ID+1)
SET_KEYWORD_INIT("spsr", 0),

#define KEYWORD_CNTCUR_ID (KEYWORD_SPSR_ID + 1)
SET_KEYWORD_INIT("cntcur", 0),
#define KEYWORD_CNTSET_ID (KEYWORD_CNTCUR_ID + 1)
SET_KEYWORD_INIT("cntset", 0),
#define KEYWORD_REASON_ID (KEYWORD_CNTSET_ID + 1)
SET_KEYWORD_INIT("reason", 0),
#define KEYWORD_NEWWRITE_ID (KEYWORD_REASON_ID + 1)
SET_KEYWORD_INIT("nw", 0),
#define KEYWORD_OLDMEM_ID (KEYWORD_NEWWRITE_ID + 1)
SET_KEYWORD_INIT("om", 0),
#define KEYWORD_INT_ENUM_ID (KEYWORD_OLDMEM_ID + 1)
SET_KEYWORD_INIT("int_enum", 0),

  /* GPU IO */
SET_KEYWORDIO16_INIT("dispcnt", 0),
SET_KEYWORDIO16_INIT("gswap", 2),
SET_KEYWORDIO16_INIT("dispstat", 4),
SET_KEYWORDIO16_INIT("vcount", 6),
SET_KEYWORDIO16_INIT("bg0cnt", 8),
SET_KEYWORDIO16_INIT("bg1cnt", 10),
SET_KEYWORDIO16_INIT("bg2cnt", 12),
SET_KEYWORDIO16_INIT("bg3cnt", 14),
SET_KEYWORDIO16_INIT("bg0hofs", 16),
SET_KEYWORDIO16_INIT("bg0vofs", 0x12),
SET_KEYWORDIO16_INIT("bg1hofs", 0x14),
SET_KEYWORDIO16_INIT("bg1vofs", 0x16),
SET_KEYWORDIO16_INIT("bg2hofs", 0x18),
SET_KEYWORDIO16_INIT("bg2vofs", 0x1A),
SET_KEYWORDIO16_INIT("bg3hofs", 0x1C),
SET_KEYWORDIO16_INIT("bg3vofs", 0x1E),
SET_KEYWORDIO16_INIT("bg2pa", 0x20),
SET_KEYWORDIO16_INIT("bg2pb", 0x22),
SET_KEYWORDIO16_INIT("bg2pc", 0x24),
SET_KEYWORDIO16_INIT("bg2pd", 0x26),
SET_KEYWORDIO32_INIT("bg2x", 0x28),
SET_KEYWORDIO32_INIT("bg2y", 0x2C),
SET_KEYWORDIO32_INIT("bg3pa", 0x30),
SET_KEYWORDIO16_INIT("bg3pb", 0x32),
SET_KEYWORDIO16_INIT("bg3pc", 0x34),
SET_KEYWORDIO16_INIT("bg3pd", 0x36),
SET_KEYWORDIO32_INIT("bg3x", 0x38),
SET_KEYWORDIO32_INIT("bg3y", 0x3C),
SET_KEYWORDIO16_INIT("win0h", 0x40),
SET_KEYWORDIO16_INIT("win1h", 0x42),
SET_KEYWORDIO16_INIT("win0v", 0x44),
SET_KEYWORDIO16_INIT("win1v", 0x46),
SET_KEYWORDIO16_INIT("winin", 0x48),
SET_KEYWORDIO16_INIT("winout", 0x4A),
SET_KEYWORDIO16_INIT("mosaic", 0x4C),
SET_KEYWORDIO16_INIT("bldcnt", 0x50),
SET_KEYWORDIO16_INIT("bldv", 0x52),
SET_KEYWORDIO16_INIT("bldy", 0x54),
  /* APU IO */
SET_KEYWORDIO16_INIT("nr10", 0x60),
SET_KEYWORDIO16_INIT("nr11_12", 0x62),
SET_KEYWORDIO16_INIT("nr13_14", 0x64),
SET_KEYWORDIO16_INIT("nr21_22", 0x68),
SET_KEYWORDIO16_INIT("nr23_24", 0x6C),
SET_KEYWORDIO16_INIT("nr30", 0x70),
SET_KEYWORDIO16_INIT("nr31_32", 0x72),
SET_KEYWORDIO16_INIT("nr33_34", 0x74),
SET_KEYWORDIO16_INIT("nr41_42", 0x78),
SET_KEYWORDIO16_INIT("nr43_44", 0x7C),
SET_KEYWORDIO16_INIT("nr50_51", 0x80),
SET_KEYWORDIO16_INIT("mixdma", 0x82),/* 4000082h */
SET_KEYWORDIO16_INIT("nr52", 0x84),
SET_KEYWORDIO16_INIT("sndpwm", 0x88),
SET_KEYWORDIO32_INIT("fifo_a", 0xA0),
SET_KEYWORDIO32_INIT("fifo_b", 0xA4),
/* DMA */
SET_KEYWORDIO32_INIT("dma0sad", 0xB0),
SET_KEYWORDIO32_INIT("dma0dad", 0xB4),
SET_KEYWORDIO16_INIT("dma0cnt", 0xB8),
SET_KEYWORDIO16_INIT("dma0ctl", 0xBA),
SET_KEYWORDIO32_INIT ("dma1sad", 0xBC),
SET_KEYWORDIO32_INIT ("dma1dad", 0xC0),
SET_KEYWORDIO16_INIT ("dma1cnt", 0xC4),
SET_KEYWORDIO16_INIT ("dma1ctl",0xC6),
SET_KEYWORDIO32_INIT ("dma2sad",0xC8),
SET_KEYWORDIO32_INIT ("dma2dad", 0xCC),
SET_KEYWORDIO16_INIT ("dma2cnt",0xD0),
SET_KEYWORDIO16_INIT ("dma2ctl",0xD2),
SET_KEYWORDIO32_INIT ("dma3sad", 0xD4),
SET_KEYWORDIO32_INIT ("dma3dad",0xD8),
SET_KEYWORDIO16_INIT ("dma3cnt",0xDC),
SET_KEYWORDIO16_INIT ("dma3ctl", 0xDE),
  /* TIMER */
SET_KEYWORDIO16_INIT("timer0cnt", 0x100),
SET_KEYWORDIO16_INIT("timer0ctl", 0x102),
SET_KEYWORDIO16_INIT("timer0cnt", 0x104),
SET_KEYWORDIO16_INIT("timer0ctl", 0x106),
SET_KEYWORDIO16_INIT("timer0cnt", 0x108),
SET_KEYWORDIO16_INIT("timer0ctl", 0x10A),
SET_KEYWORDIO16_INIT("timer0cnt", 0x10C),
SET_KEYWORDIO16_INIT("timer0ctl", 0x10E),

  /* SIO TODO:*/
  /* KEY */
SET_KEYWORDIO16_INIT("keyinput", 0x130),
SET_KEYWORDIO16_INIT("keyctl", 0x132),
  /* Interrupt and misc */
SET_KEYWORDIO16_INIT("ie", 0x200),
SET_KEYWORDIO16_INIT("if", 0x202),
SET_KEYWORDIO16_INIT("wait", 0x204),
SET_KEYWORDIO32_INIT("wait", 0x208),
SET_KEYWORDIO8_INIT("postflg", 0x300),
SET_KEYWORDIO8_INIT("halt", 0x301),
SET_KEYWORDIO32_INIT("imc", 0x800),
};
struct cond_bp_node {
  union {
    uint32_t addr;
    uint32_t number;
    uint32_t register_id;
  };
  union {
    MEM_BLOCK mem_block;
    ARM_FIELD_MASK reg_field;
  };
  struct list_ *slink_chain;
  struct list_ *child_expr_chain;
  struct list_chunk **pri_slot;
  NODE_VAL_TYPE n_type;
};

struct cond_bp_link {
  LINK_TYPE link;
  LINK_PRI op_pri; /* 0......$, 0 is max */
  int pos;
};

#define CHUNK_TYPE_LINK 0
#define CHUNK_TYPE_NODE 1
#define CHUNK_TYPE_CHILD_CHAIN 2
typedef int CHUNK_TYPE;

struct pn_block {
  CHUNK_TYPE ctype;
  union {
    struct cond_bp_link link;
    struct cond_bp_node node;
  };
};

struct token_buf {
  const char *buf;
  intptr_t pos;
  intptr_t num;
};

/* breakpoint_cond:: breakpoint_expr*/
struct breakpoint_expr {
  struct pn_block tree_root;
  struct list_ *auto_dtor;
  struct token_buf token_buf;
};

kable tkbuf_overflow (struct token_buf *p) {
  if (p->pos <= p->num -1)
    return false;
  else 
    return true;
}

int tkbuf_skip_space (struct token_buf *p) {
  int n_pos = p->pos;
  while (true) {
    if (p->pos <= p->num -1) {
      if (p->buf[p->pos++] == ' ')
        continue;
      p->pos --;
      return RES_OK;
    } else {
      p->pos = n_pos;
      return RES_EOF;
    }
  }
}

void tkbuf_init (struct token_buf *buf, const char *charbuf) {
  buf->buf = charbuf;
  buf->pos = 0;
  buf->num = strlen (charbuf);
}

finline
const char *tkbuf_getchptr (struct token_buf *p) {
  return & p->buf[p->pos];
}

finline
int tkbuf_getch_re (struct token_buf *p) {
  return p->num - p->pos;
}

finline
int tkbuf_varlen_bcur (struct token_buf *p, int *pstart, int *pend /*pos +1 */) {
  int n = p->pos;
  int nstart = -1;
  int sig = -1;
  kable ch_exist = false;
  while (n < p->num) {
    char ch = p->buf[n];
    if (ch == ' ') {
      if (ch_exist == false)
        continue ;
      sig = 0;
      break;
    } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
      if (nstart == -1)
        nstart = n;
      ch_exist = true;
    } else if ((ch >= '0' && ch <= '9')) {
      if (ch_exist != false)
        continue ;
      return -1;
    } else {
      if (ch_exist != false) {
        sig = 0;
        break;
      }
      return -1;
    }
    n++;
  }
  if (ch_exist == false || n == nstart)
    sig = -1;
  if (!sig) {
    if (pstart != null)
      *pstart = nstart;
    if (pend != null)
      *pend = n;
  }
  return sig;
}

finline
void tkbuf_backup_n (struct token_buf *p, int n) {
  p->pos -= n;
}

int tkbuf_get_ch (struct token_buf *p, char *ch, kable popit) {
  if (tkbuf_overflow (p) == false) {
    * ch = *tkbuf_getchptr (p);
    if (popit != false)
      p->pos ++;
    return RES_OK;
  } else {
    return RES_EOF;
  }
}

finline
int tkbuf_get_ch_ex (struct token_buf *p, 
                               char *chrecv, 
                               int *source_pos, 
                               int *after_pos, 
                               const kable overflow_return_error_,
                               const kable pure_space_return_error,
                               const kable space_filter, 
                               const kable popit, 
                               const kable tail_scan, 
                               const kable tail_scan_expect_ch) {

  int n_pos = p->pos;
  int sig = RES_ERROR_;
  int temp_pos = p->pos;
  kable space =true;

  if (source_pos != null)
    *source_pos = n_pos;

  if (space_filter != false) {
    while (true) {
      char ch = p->buf[temp_pos];
      if (temp_pos <= p->num -1) {
        temp_pos++;
        if (ch == ' ')
          continue;
        else 
          space = false;

        if (chrecv != null)
          *chrecv = ch;
        if (after_pos != null)
          *after_pos = temp_pos;
        if (popit != false)
          p->pos = temp_pos;
        if (tail_scan != false) {
          space = true;
          while (true) {
            char ch = p->buf[temp_pos];
            if (temp_pos <= p->num -1)
              temp_pos++;
            if (ch != ' ') {
              space = false;
              break;
            }
          }
          if (tail_scan_expect_ch != false)
            if (space == false)
              return RES_OK;
            else 
              return RES_ERROR_;
          else 
            if (space == false)
              return RES_ERROR_;
            else 
              return RES_OK;
        }
        return RES_OK;
      } 
    }
    if (popit == false)
      p->pos = n_pos;
    if (after_pos != null)
      *after_pos = p->num;

    if (overflow_return_error_ != false)
      sig = RES_ERROR_;
    else 
      sig = RES_OK;
    if (pure_space_return_error != false)
      if (space != false)
         sig = RES_ERROR_;
      else ;
    else;
    return sig;
  } else {
    char ch = p->buf[temp_pos];
    if (temp_pos <= p->num -1) {
      temp_pos++;
      if (ch != ' ')
        space = false;

      if (chrecv != null)
        *chrecv = ch;
      if (after_pos != null)
        *after_pos = temp_pos;
      if (popit != false)
        p->pos = temp_pos;
      if (tail_scan != false) {
        space = true;
        while (true) {
          char ch = p->buf[temp_pos];
          if (temp_pos <= p->num -1)
            temp_pos++;
          if (ch != ' ') {
            space = false;
            break;
          }
        }
        if (tail_scan_expect_ch != false)
          if (space == false)
            return RES_OK;
          else 
            return RES_ERROR_;
        else 
          if (space == false)
            return RES_ERROR_;
          else 
            return RES_OK;
      }
      return RES_OK;
    } 
    if (popit == false)
      p->pos = n_pos;
    if (after_pos != null)
      *after_pos = p->num;

    if (overflow_return_error_ != false)
      sig = RES_ERROR_;
    else 
      sig = RES_OK;
    if (pure_space_return_error != false)
      if (space != false)
         sig = RES_ERROR_;
      else ;
    else;
    return sig;
  }
}

kable tkbuf_assert_ch2 (struct token_buf *p, char ch, kable space_filter, kable popit) {
  int n_pos = p->pos;
  if (space_filter != false) {
    if (    tkbuf_skip_space (p) != RES_OK
      ||   *tkbuf_getchptr (p) != ch) {
      p->pos = n_pos;
      return false;
    }
    p->pos ++;
    if (popit == false)
      p->pos = n_pos;
    return true;
  } else {
    if (tkbuf_overflow (p) == false) {
      if (*tkbuf_getchptr (p) == ch) {
        if (popit != false)
            p->pos ++;
        else 
            p->pos = n_pos;
        return true;
      }
    }
  }
  return false;
}

kable tkbuf_ensize (struct token_buf *p, int length) {
  int n =  p->num - p->pos;
  if (n >= length)
    return true;
  else 
    return false;
}


int tkbuf_strcmp (struct token_buf *p, const char *cmpv, kable add_pos) {
  int n = strlen (cmpv);
  if (tkbuf_ensize (p, n)) {
    if (strncmp (cmpv, tkbuf_getchptr (p), n) == 0) {
      if (add_pos != false)
        p->pos += n;
      else ;
      return RES_OK;
    }
    return RES_NOMATCH_INTERRUPT;
  }
  return RES_EOF;
}



int tkbuf_cmp_perch_or (struct token_buf *p, char *recvch, int *pos, const char *cmpv, kable space_filter, kable add_pos) {
  int n = strlen (cmpv);
  int n_pos = p->pos;
  int id ;
  char ch;

  if (space_filter != false)
    if (tkbuf_skip_space (p) != RES_OK)
      return RES_EOF;
    else ;
  else ;

  if (tkbuf_overflow (p) == false) {
    tkbuf_get_ch (p, & ch, false);
    for (id = 0; id != n; id++)  {
      if (ch == cmpv[id]) {
        if (recvch != null)
          *recvch = ch;
        if (add_pos != false)
          p->pos++;
        else 
          p->pos = n_pos;
        if (pos != null)
          *pos =id;
        return RES_OK;
      }
    }
    p->pos = n_pos;
    return RES_NOMATCH_INTERRUPT;
  }
  p->pos = n_pos;
  return RES_EOF;
}

int tkbuf_tryget_hex (struct token_buf *p, int *numb, kable space_filter, kable add_pos) {
  int n_pos= p->pos;
  int b_pos;

  if (space_filter != false)
    if (tkbuf_skip_space (p) != RES_OK)
      return RES_EOF;
    else ;
  else ;

  b_pos= p->pos;
  while (true) {
    if ( tkbuf_cmp_perch_or (p, null, null, "0123456789abcdef", false, true) != RES_OK)
      break;
    else ;
  }
  if (b_pos == p->pos) {
    p->pos = n_pos;
    return RES_EOF;
  }
  if (add_pos == false)
    p->pos = n_pos; /*resume it .*/
  if (numb != null)
    *numb = strtol (& p->buf[b_pos], null, 16); /* XXX: range check */
  return RES_OK;
}

int tkbuf_tryget_dec (struct token_buf *p, int *numb, kable space_filter, kable add_pos) {
  int n_pos= p->pos;
  int b_pos;

  if (space_filter != false)
    if (tkbuf_skip_space (p) != RES_OK)
      return -1;
    else ;
  else ;

  b_pos= p->pos;
  while (true) {
    if ( tkbuf_cmp_perch_or (p, null, null, "0123456789", false, true) != RES_OK)
      break;
    else ;
  }
  if (b_pos == p->pos) {
    p->pos = n_pos;
    return RES_EOF;
  }
  if (add_pos == false)
    p->pos = n_pos; /*resume it .*/
  if (numb != null)
    *numb = strtol (& p->buf[b_pos], null, 10); /* XXX: range check */
  return RES_OK;
}

/* node- pure.  only 3 type. 
   type 1 - regsiter :<- [irq_|svc_|usr_|sys_] r[R] 0~15, 16(spsr), 99(cpsr)
   type 2 - constant :<- hex numb 
   type 3 - membus|devio :<- hex numb:[b|h|w]
  */

void build_token_tree (struct token_buf *token_, 
                                    struct list_ *malloc_list,
                                 struct list_ *parent_list, 
                                   struct cond_bp_node *parent,
                                     kable root,
                                 kable *interrupt, 
                                 int *result, 
                                 char *err_buf) {
#define BTT_SET_EXIT(/* kable */ result_, /* const char * */ err_infos) do { \
                                   *interrupt = true;\
                                   *result = result_;\
                                   memcpy (err_buf, err_infos, strlen (err_infos) + 1); \
                                   return ;\
                                 } while (0)
   int id;
   int n_arc;
   int n_link = 0;
   int xorgate = 0; /* 0:<- serach node 1:<- serach link */
   kable first = true;
   kable rentry = false;
   struct pn_block *blk = null;
   struct list_chunk *clist = null;
   const char *arm7field[3] = { "svc_r", "usr_r", "irq_r" };
   if (*interrupt || tkbuf_overflow (token_) != false)
     return ;
   else do {
     char ch;
     
     int n = tkbuf_get_ch (token_, & ch, true);
     if (n == RES_EOF) {
btt_exit:
       if (root == false && rentry == false) 
         BTT_SET_EXIT (-1, "mismatch (|)");
       if (xorgate != 1 || !(parent_list->count & 1))
         BTT_SET_EXIT (-1, "errors in xorgate");
       if (first != false)
         BTT_SET_EXIT (-1, "errors in empty data");
       /* operate pri sort.*/
       n_arc = parent_list->count / 2;
       if (parent_list->count > 1) {
         int u, v;
         parent->pri_slot = (struct list_chunk **) flag_malloc (malloc_list, sizeof (struct list_chunk *) * n_arc, null);
         clist = list_step_init (parent_list, 0, 1, 2);
         assert (clist != null);

         for (id = 0; id < n_arc;  id++) {
            struct list_chunk *pb = (struct list_chunk *) list_step (parent_list, 0, LIST_STEP_ADV_AFTER);
            struct pn_block *pblk = (struct pn_block *)pb->dat_ptr;
            assert (((struct pn_block *)pb->dat_ptr)->ctype == CHUNK_TYPE_LINK);
            pblk->link.op_pri = pblk->link.op_pri << 16 + id;
            parent->pri_slot[id] = pb;
         }
         for (u = 0; u < n_arc - 1;  u++) {
           struct list_chunk * pu = parent->pri_slot[u];
           for (v = u + 1; v < n_arc; v++) {
             struct list_chunk * pv = parent->pri_slot[v];
             if (((struct pn_block *)pu->dat_ptr)->link.op_pri > ((struct pn_block *)pv->dat_ptr)->link.op_pri) {
               struct list_chunk * temp  = parent->pri_slot[u];
               pu = parent->pri_slot[u] = parent->pri_slot[v];
               parent->pri_slot[v] = temp;
             }
           }
         }
         for (id = 0; id < n_arc;  id++) {
            struct list_chunk *pb = parent->pri_slot[id];
            struct pn_block *pblk = (struct pn_block *)parent->pri_slot[id]->dat_ptr;
            assert (((struct pn_block *)pb->dat_ptr)->ctype == CHUNK_TYPE_LINK);
            pblk->link.op_pri >>= 16;
         }
       }
       return ;
     } else {
       if (ch == ' ')
         continue ;
       else if (xorgate == 0) {
         /*collect self link symbols. */
         blk = (struct pn_block *)flag_malloc (malloc_list, sizeof(struct pn_block), null);
         assert (blk != null);
         memset (blk, 0, sizeof (struct pn_block));
         blk->ctype = CHUNK_TYPE_NODE;
         list_init_ (& blk->node.slink_chain);
         blk->node.reg_field = ARM7_FIELD_CUR;
         blk->node.n_type = NODE_VAL_ARM7_REGISTER;
         /* insert parent it. */
         list_insert_ (parent_list, -1, blk);
         /* a - b + c *v */
         while (true) {
           if (ch == ' ')
             ;
           else if (ch == '-')
             list_insert_ (blk->node.slink_chain, 0, (void*) SLINK_NEG);
           else if (ch == '+')
             list_insert_ (blk->node.slink_chain, 0, (void*) SLINK_POST);
           else if (ch == '~')
             list_insert_ (blk->node.slink_chain, 0, (void*) SLINK_NOT);
           else if (ch == '!')
             list_insert_ (blk->node.slink_chain, 0, (void*) SLINK_NOT_BOOL);
           else 
             break ;
           n =  tkbuf_get_ch (token_, & ch, true);
           if (n == RES_EOF)
             BTT_SET_EXIT (-1, "errors in collect self link");
         }
         
         /* serach node */
        switch (ch) {
        case '@': /* expect keyword */
          {
            int nstart;
            int nend;
            int keylen;
            int id;

            if (tkbuf_varlen_bcur (token_, & nstart, & nend) < 0)
              BTT_SET_EXIT (-1, "miss content after @");
            /* trymatch keyword */
            keylen = nend - nstart;
            token_->pos = nend;

            blk->node.n_type = NODE_VAL_UB;
            for (id = 0; id != sizeof (keyword)/ sizeof (keyword[0]); id++) {
              if (keylen != keyword[id].toklen)
                continue ;
              if (strncmp (keyword[id].token, & token_->buf[nstart], keylen) == 0) {
                switch (id) {
                case KEYWORD_PC_ID:
                  blk->node.n_type = NODE_VAL_CORRECT_PC;
                  break;
                case KEYWORD_LR_ID:
                  blk->node.n_type = NODE_VAL_ARM7_REGISTER;
                  blk->node.register_id = 14;
                  blk->node.reg_field = ARM7_FIELD_CUR;
                  break;
                case KEYWORD_SP_ID:
                  blk->node.n_type = NODE_VAL_ARM7_REGISTER;
                  blk->node.register_id = 13;
                  blk->node.reg_field = ARM7_FIELD_CUR;
                  break;
                case KEYWORD_CPSR_ID:
                  blk->node.n_type = NODE_VAL_ARM7_REGISTER;
                  blk->node.register_id = 99;
                  blk->node.reg_field = ARM7_FIELD_CUR;
                  break;
                case KEYWORD_SPSR_ID:
                  blk->node.n_type = NODE_VAL_ARM7_REGISTER;
                  blk->node.register_id = 16;
                  blk->node.reg_field = ARM7_FIELD_CUR;
                  break;
                case KEYWORD_INT_ENUM_ID:
                  blk->node.n_type = NODE_VAL_INT_ENUM;
                  break;
                case KEYWORD_CNTCUR_ID:
                  blk->node.n_type = NODE_VAL_CNTCUR;
                  break;
                case KEYWORD_CNTSET_ID:
                  blk->node.n_type = NODE_VAL_CNTSET;
                  break;
                case KEYWORD_REASON_ID:
                  blk->node.n_type = NODE_VAL_REASON;
                  break;
                case KEYWORD_NEWWRITE_ID:
                  blk->node.n_type = NODE_VAL_NW;
                  break;
                case KEYWORD_OLDMEM_ID:
                  blk->node.n_type = NODE_VAL_OMEM;
                  break;
                default:
                  blk->node.addr = keyword[id].addr;
                  blk->node.mem_block = keyword[id].mblk;
                  blk->node.n_type = NODE_VAL_MEM;
                  break;
                }
                break;
              }
            }
            if (blk->node.n_type == NODE_VAL_UB)
              BTT_SET_EXIT (-1, "mismatch content after @");
          }
          break;
        case 'i': /* expect irq_r */
          blk->node.reg_field = ARM7_FIELD_IRQ;
          id = 2;
          goto field_cmp;
        case 's':/* expect svc_r  */
          blk->node.reg_field = ARM7_FIELD_SVC;
          id = 0;
          goto field_cmp;
        case 'u':/* expect usr_r  */
          blk->node.reg_field = ARM7_FIELD_USR;
          id = 1;
field_cmp:
          tkbuf_backup_n (token_, 1);
          if (tkbuf_strcmp (token_, arm7field[id], true) != 0) 
            BTT_SET_EXIT (-1, "errors in collect arm7 field perfix mask");
        case 'r':
          /* try get hex..*/
          if (tkbuf_tryget_dec (token_, (int *)& blk->node.register_id, false, true) != RES_OK) 
            BTT_SET_EXIT (-1, "errors in collect arm7 's number id");
          if ( !((blk->node.register_id >= 0 && blk->node.register_id <= 16) || blk->node.register_id == 99)) 
            BTT_SET_EXIT (-1, "errors in collect arm7 's number id- range errors.");
          break;
        case '(': /* expect chil chain expr. */
          blk->ctype = CHUNK_TYPE_CHILD_CHAIN;
          list_init_ (& blk->node.child_expr_chain);

          build_token_tree (token_, malloc_list,
                   blk->node.child_expr_chain,
                   & blk->node,
                   false,
                   interrupt, 
                   result,
                   err_buf);

          if (*interrupt != false)
            return ;
          /* check expr..... (((((( mid  ))))))), mid is empty ??*/
          if (blk->node.child_expr_chain == null 
            || blk->node.child_expr_chain->count <= 0)
            BTT_SET_EXIT (-1, "errors in nesting entry no content.");
            break;
        default: /* expect hex number | hex number:w|h|b */
          tkbuf_backup_n (token_, 1);
          if (tkbuf_strcmp (token_, "0x", true) == RES_OK) 
             if (tkbuf_tryget_hex (token_, (int *)& blk->node.number, false, true) != RES_OK) 
              BTT_SET_EXIT (-1, "errors in collect hex number | hex number:w|h|b");
             else ;
          else 
             if (tkbuf_tryget_dec (token_, (int *)& blk->node.number, false, true) != RES_OK) 
              BTT_SET_EXIT (-1, "errors in collect hex number | hex number:w|h|b");
             else ;
         blk->node.n_type = NODE_VAL_CONSTANT;
         if (tkbuf_assert_ch2 (token_, ':', true, true)) {
           if (tkbuf_cmp_perch_or (token_, null, & n_arc, "whb", true, true) == RES_OK) {
             blk->node.n_type = NODE_VAL_MEM;
             if (n_arc == 0) 
               blk->node.mem_block = MEM_BLOCK_WORD;
             else if (n_arc == 1)
               blk->node.mem_block = MEM_BLOCK_HALFWORD;
             else if (n_arc == 2)
               blk->node.mem_block = MEM_BLOCK_BYTE;
             else assert (0);
           } else BTT_SET_EXIT (-1, "errors in miss mem size flag w|h|b");
         }
         break;
        }
       } else {
         if (ch == ')') {
          rentry = true;
          if (root == true) 
            BTT_SET_EXIT (-1, "errors in mismatch (, ).");
          goto btt_exit ;
         }

         blk = (struct pn_block *)flag_malloc (malloc_list, sizeof(struct pn_block), null);
         assert (blk != null);
         memset (blk, 0, sizeof (struct pn_block));
         blk->ctype = CHUNK_TYPE_LINK;
         list_insert_ (parent_list, -1, blk);

         blk->link.pos = n_link++;
         blk->ctype = CHUNK_TYPE_LINK;
         switch (ch) {
         case '-':
           blk->link.link = LINK_SUB;
           blk->link.op_pri = LINK_PRI_SUB;
           break ;
         case '+':
           blk->link.link = LINK_ADD;
           blk->link.op_pri = LINK_PRI_ADD;
           break ;
         case '%':
           blk->link.link = LINK_MOD;
           blk->link.op_pri = LINK_PRI_MOD;
           break ;
         case '/':
           blk->link.link = LINK_DIV;
           blk->link.op_pri = LINK_PRI_DIV;
           break ;
         case '>': /* expect null | = | >*/
           blk->link.link = LINK_ABOVE;
           blk->link.op_pri = LINK_PRI_ABOVE;
           if (tkbuf_assert_ch2 (token_, '=', false, true) != false) {
             blk->link.link = LINK_ABOVE_EQUAL;
             blk->link.op_pri = LINK_PRI_ABOVE_EQUAL;
           } else if (tkbuf_assert_ch2 (token_, '>', false, true) != false) {
             blk->link.link = LINK_SHR;
             blk->link.op_pri = LINK_PRI_SHR;
           } 
           break;
         case '<': /* expect null | = | < */
           blk->link.link = LINK_BELOW;
           blk->link.op_pri = LINK_PRI_BELOW;
           if (tkbuf_assert_ch2 (token_, '=', false, true) != false) {
             blk->link.link = LINK_BELOW_EQUAL;
             blk->link.op_pri = LINK_PRI_BELOW_EQUAL;
           } else if (tkbuf_assert_ch2 (token_, '<', false, true) != false) {
             blk->link.link = LINK_SHL;
             blk->link.op_pri = LINK_PRI_SHL;
           } 
           break;
         case '=': /* expect = */
           if (tkbuf_assert_ch2 (token_, '=', false, true) != false) {
             blk->link.link = LINK_EQUAL;
             blk->link.op_pri = LINK_PRI_EQUAL;
           } else {
             BTT_SET_EXIT (-1, "errors in collect == ");
           }
           break;
         case '*':
           blk->link.link = LINK_MUL;
           blk->link.op_pri = LINK_PRI_MUL;
           break ;
         case '&': /* expect null | & */
           blk->link.link = LINK_AND;
           blk->link.op_pri = LINK_PRI_AND;
           if (tkbuf_assert_ch2 (token_, '&', false, true) != false) {
             blk->link.link = LINK_BOOL_AND;
             blk->link.op_pri = LINK_PRI_BOOL_AND;
           } 
           break;
         case '^':
           blk->link.link = LINK_XOR;
           blk->link.op_pri = LINK_PRI_XOR;
           break ;
         case '|': /* expect null | | */
           blk->link.link = LINK_OR;
           blk->link.op_pri = LINK_PRI_OR;
           if (tkbuf_assert_ch2 (token_, '|', false, true) != false) {
             blk->link.link = LINK_BOOL_OR;
             blk->link.op_pri = LINK_PRI_BOOL_OR;
           } 
           break;
         default:
            BTT_SET_EXIT (-1, "errors in misc char. ");
         }
       }
       first = false;
       xorgate ^= 1;
     }
   } while (true);
}

finline
int expr_node_val (struct gba *gba, struct breakpoint *dev_bp,  struct pn_block *node) {
  uint32_t out;
  assert (node->ctype == CHUNK_TYPE_NODE);

  switch (node->node.n_type) {
  case NODE_VAL_CNTCUR:
    out = dev_bp->match_loop;
    break;
  case NODE_VAL_INT_ENUM:
    out = gba->dbg.int_enum & 15;
    break;
  case NODE_VAL_CNTSET:
    out = dev_bp->match_expect;
    break;
  case NODE_VAL_CORRECT_PC:
    if (gba->arm7.regs[16].blk & 0x20000000)
      out = gba->arm7.regs[15].blk - 4;
    else 
      out = gba->arm7.regs[15].blk - 8;
    break;
  case NODE_VAL_NW:
    out = gba->mem.novel_access;
    break;
  case NODE_VAL_OMEM:
    if (node->node.mem_block == MEM_BLOCK_BYTE)
      out = gba_fast_rhw (gba, node->node.addr) & 0xFF;
    else if (node->node.mem_block == MEM_BLOCK_HALFWORD)
      out = gba_fast_rhw (gba, node->node.addr & -2);
    else {
      uint32_t lo = gba_fast_rhw (gba, node->node.addr & -2) & 0xFFFF;
      uint32_t hi = gba_fast_rhw (gba, (node->node.addr & -2) + 2) & 0xFFFF;
      out = lo | hi << 16;
    }
    break;
  case NODE_VAL_CONSTANT:
    out = node->node.number;
    break;
  case NODE_VAL_ARM7_REGISTER:
    switch (node->node.register_id) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 15:
      out = gba->arm7.regs[node->node.register_id].blk;
      break;
    case 99:
      out = gba_get_cur_spsr (& gba->arm7);
      break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      /* TODO: FIQ Stuff */
      out = gba->arm7.regs[node->node.register_id].blk;
      break;
    case 13:
    case 14:
      if (node->node.reg_field & ARM7_FIELD_CUR)
        out = gba->arm7.regs[node->node.register_id].blk;
      else if (node->node.reg_field & ARM7_FIELD_SVC)
        out = gba->arm7.r1314_t[node->node.register_id - 13 + R1314b_MGR].blk;
      else if (node->node.reg_field & ARM7_FIELD_USR)
        out = gba->arm7.r1314_t[node->node.register_id - 13 + R1314b_SYSUSER].blk;
      else if (node->node.reg_field & ARM7_FIELD_IRQ)
        out = gba->arm7.r1314_t[node->node.register_id - 13 + R1314b_IRQ].blk;
      else 
        assert (0);
      break;
    case 16:
      if (node->node.reg_field & ARM7_FIELD_CUR)
        switch (gba->arm7.regs[16].blk >> 24 & 0x1F) {
        case 0x10:
        case 0x1F:
          out = 0;
          break;
        case 0x13:
          out = gba->arm7.spsr_t[SPSRb_MGR].blk;
          break;
        case 0x12:
          out = gba->arm7.spsr_t[SPSRb_IRQ].blk;
          break;
        default:
          assert (0);
        }
      else if (node->node.reg_field & ARM7_FIELD_SVC)
        out = gba->arm7.spsr_t[SPSRb_MGR].blk;
      else if (node->node.reg_field & ARM7_FIELD_USR)
        out = gba->arm7.spsr_t[SPSRb_SYSUSER].blk;
      else if (node->node.reg_field & ARM7_FIELD_IRQ)
        out = gba->arm7.spsr_t[SPSRb_IRQ].blk;
      else 
        assert (0);
      break;
    default:
      assert (0);
    }
    break;
  case NODE_VAL_MEM:
    switch (node->node.mem_block) {
    case MEM_BLOCK_WORD:
      out = gba_fast_rhw (gba, node->node.addr & -4);
      out = out |  ((uint32_t)  gba_fast_rhw (gba, (node->node.addr & -4) + 2) )  << 16;
      break;
    case MEM_BLOCK_HALFWORD:
      out = gba_fast_rhw (gba, node->node.addr);
      break;
    case MEM_BLOCK_BYTE:
      out = gba_fast_rhw (gba, node->node.addr) & 0xFF;
      break;
    default:
      assert (0);
    }
    break;
  default:
    assert (0);
  }
  /*self link stuff */
  if (node->node.slink_chain != null 
    && node->node.slink_chain->count > 0)
  {
    int id;
    struct list_chunk *pv;

    LIST_EACH_(node->node.slink_chain, id, pv) {
      switch ((SLINK_TYPE)pv->dat_uint) {
      case SLINK_NEG: 
        out = -out; 
        continue ;
      case SLINK_NOT:
        out = ~out; 
        continue;
      case SLINK_NOT_BOOL: 
        out = !out; 
        continue;
      case SLINK_POST: 
        continue;
      default:
        assert (0);
      }
    }
  }
  return out;
}

static
int expr_eval (struct gba *gba,    struct breakpoint *dev_bp, struct pn_block *parnet)
{
  int n_arc = parnet->node.child_expr_chain->count / 2;
  int n_cfa;
  uint32_t out;
  struct phase_pt {
    struct phase_pt *finger_left;
    struct phase_pt *finger_right;
    uint32_t diffuse;
  } *pt = null, pt_stk_blk[12];

  if (n_arc) {
    if (n_arc <= sizeof (pt_stk_blk) / sizeof (pt_stk_blk[0]))
      pt = & pt_stk_blk[0];
    else 
      pt = (struct phase_pt *)malloc (n_arc * sizeof (struct phase_pt));

    memset (pt, 0, sizeof (struct phase_pt) * n_arc);
    for (n_cfa = 0; n_cfa != n_arc; n_cfa++) {
      struct list_chunk *pl = parnet->node.pri_slot[n_cfa];
      struct pn_block *pn = (struct pn_block *) pl->dat_ptr;
      struct pn_block *pn_l = (struct pn_block *) pl->prev->dat_ptr;
      struct pn_block *pn_r = (struct pn_block *) pl->next->dat_ptr;
      kable ref_l = false;
      kable ref_r = false;
      uint32_t val_l;
      uint32_t val_r;

      /* check left val*/
      if (pn->link.pos == 0 || pt[pn->link.pos-1].finger_left == null)
        if (pn_l->ctype == CHUNK_TYPE_CHILD_CHAIN)
          val_l = expr_eval (gba, dev_bp, pn_l);
        else 
          val_l = expr_node_val (gba, dev_bp, pn_l);
      else {
        val_l = pt[pn->link.pos-1].finger_left->diffuse;
        ref_l = true;
      }

      if (pn->link.pos == n_arc - 1 || pt[pn->link.pos+1].finger_left == null)
        if (pn_r->ctype == CHUNK_TYPE_CHILD_CHAIN)
          val_r = expr_eval (gba, dev_bp, pn_r);
        else 
          val_r = expr_node_val (gba, dev_bp, pn_r);
      else {
        val_r = pt[pn->link.pos+1].finger_right->diffuse;
        ref_r = true;
      }

      switch (pn->link.link) {
      case LINK_ABOVE:
        out = !! (val_l > val_r);
        break;
      case LINK_BELOW:
        out = !! (val_l < val_r);
        break;
      case LINK_ABOVE_EQUAL:
        out = !! (val_l >= val_r);
        break;
      case LINK_BELOW_EQUAL:
        out = !! (val_l <= val_r);
        break;
      case LINK_EQUAL:
        out = !! (val_l == val_r);
        break;
      case LINK_NOTEQUAL:
        out = !! (val_l != val_r);
        break;
      case LINK_AND:
        out = val_l & val_r;
        break;
      case LINK_OR:
        out = val_l | val_r;
        break;
      case LINK_XOR:
        out = val_l ^ val_r;
        break;
      case LINK_SHR:
        out = val_l >> (val_r & 31);
        break;
      case LINK_SHL:
        out = val_l << (val_r & 31);
        break;
      case LINK_BOOL_OR:
        out = val_l || val_r;
        break;
      case LINK_BOOL_AND:
        out = val_l && val_r;
        break;
      case LINK_ADD:
        out = val_l + val_r;
        break;
      case LINK_SUB:
        out = val_l - val_r;
        break;
      case LINK_MUL:
        out = val_l * val_r;
        break;
      case LINK_DIV:
        if (val_l == 0)
          out = 0;
        else if (val_r == 0)
          out = UINT32_MAX;
        else
          out = val_l / val_r;
        break;
      case LINK_MOD:
        if (val_l == 0 || val_r == 0)
          out = 0;
        else
          out = val_l % val_r;
        break;
      default:
        assert (0);
      }
      /* check ref cross */
      if (ref_l != false) {
         pt[pn->link.pos-1].finger_left->diffuse = out;
         pt[pn->link.pos-1].finger_right = & pt[pn->link.pos];
      }
      if (ref_r != false) {
        pt[pn->link.pos+1].finger_right->diffuse = out;
        pt[pn->link.pos+1].finger_left= & pt[pn->link.pos];
      }
      pt[pn->link.pos].diffuse = out;
      pt[pn->link.pos].finger_left = & pt[pn->link.pos];
      pt[pn->link.pos].finger_right = & pt[pn->link.pos];
    }
    if (pt != & pt_stk_blk[0])
      free (pt);
    else ;
  } else {
      struct list_chunk *pl = parnet->node.child_expr_chain->head.next;
      struct pn_block *pn = (struct pn_block *) pl->dat_ptr;

      if (pn->ctype == CHUNK_TYPE_CHILD_CHAIN)
        return expr_eval (gba, dev_bp, pn);
      else 
        return expr_node_val (gba, dev_bp, pn);
  }
  return out;
}

int gba_breakpoint_expr_init (struct gba *agb, struct breakpoint_expr **recv, const char *expr, char *err_buf, int buf_max) {

  char err_buf_[960];
  struct breakpoint_expr *co= (struct breakpoint_expr *) malloc (sizeof (struct breakpoint_expr));
  int result = 0;
  kable interrupt = false;
  assert (co != null);
  assert (recv != null);

  list_init_ (& co->auto_dtor);
  memset (& co->tree_root, 0, sizeof (struct pn_block));
  tkbuf_init (& co->token_buf, expr);
  list_init_ (& co->tree_root.node.child_expr_chain);

  build_token_tree (& co->token_buf, co->auto_dtor, co->tree_root.node.child_expr_chain, & co->tree_root.node,
    true, & interrupt, & result, & err_buf[0]);

  if (result != 0) {
    int n = strlen (err_buf);
    if (err_buf != null)
      memcpy (err_buf_, err_buf, n);
    err_buf_[n] = 0;
    flag_malloc_uninit (& co->auto_dtor, null);
    free (co);
    return -1;
  }
  *recv = co;
  return 0;
}

void gba_breakpoint_expr_uninit (struct breakpoint_expr *recv) {

  if (recv != null) {
    if (recv->auto_dtor != null)
      flag_malloc_uninit (& recv->auto_dtor, null);
    free (recv);
  }
}

kable gba_breakpoint_expr_run (struct gba *agb, struct breakpoint *dev_bp) {
  if (dev_bp->expr != null)
    return !! expr_eval (agb, dev_bp, & dev_bp->expr->tree_root);
  else 
    return true;
}

gba_impl int callc
gba_breakpoint_insert (struct gba *agb, struct breakpoint *breakpoint) {
  assert (agb != null);
  assert (breakpoint != null);

  switch (breakpoint->reason)  {
  case BREAKPOINT_REASON_R15:
    list_insert_tail_  (agb->dbg.addr_bp_list, breakpoint);
    break;
  case BREAKPOINT_REASON_MEM_READ:
    list_insert_tail_  (agb->dbg.read_bp_list, breakpoint);
    break;
  case BREAKPOINT_REASON_MEM_WRITE:
    list_insert_tail_  (agb->dbg.write_bp_list, breakpoint);
    break;
  case BREAKPOINT_REASON_HARDWARE_INTERRUPT:
    list_insert_tail_  (agb->dbg.int_bp_list, breakpoint);
    break;
  case BREAKPOINT_REASON_SOFTWARE_INTERRUPT:
    list_insert_tail_  (agb->dbg.soft_int_bp_list, breakpoint);
    break;
  default:
    assert (0);
    break;
  }
  return 0;
}
static 
void gba_breakpoint_destroy_savebody (struct breakpoint *breakpoint) {
  assert (breakpoint != null);

  if (breakpoint->comment != null)
    free (breakpoint->comment);
  if (breakpoint->describe != null)
    free (breakpoint->describe);

  if (breakpoint->expr != null) {
    flag_malloc_uninit (& breakpoint->expr->auto_dtor, null);
    free (breakpoint->expr);
  }
  breakpoint->comment = null;
  breakpoint->describe = null;
  breakpoint->expr = null;
}
gba_impl void callc
gba_breakpoint_destroy (struct breakpoint *breakpoint) {
  gba_breakpoint_destroy_savebody (breakpoint);
  free (breakpoint);
}
gba_impl int callc  
gba_breakpoint_remove (struct gba *agb, struct breakpoint *breakpoint) {

  struct list_ *p= null;

  assert (agb != null);
  assert (breakpoint != null);

  switch (breakpoint->reason)  {
  case BREAKPOINT_REASON_R15:
    p = agb->dbg.addr_bp_list;
    break;
  case BREAKPOINT_REASON_MEM_READ:
    p = agb->dbg.read_bp_list;
    break;
  case BREAKPOINT_REASON_MEM_WRITE:
    p = agb->dbg.write_bp_list;
    break;
  case BREAKPOINT_REASON_HARDWARE_INTERRUPT:
    p = agb->dbg.int_bp_list;
    break;
  case BREAKPOINT_REASON_SOFTWARE_INTERRUPT:
    p = agb->dbg.soft_int_bp_list;
  default:
    break;
  }
  assert (p != null);

  LIST_EACH2_BEGIN (p, struct breakpoint *);
    if (LIST_NODE == breakpoint) {
      gba_breakpoint_destroy (LIST_NODE);
      list_remove_body_ (p, LIST_POLL);
      return 0;
    }
  LIST_EACH2_END ();
  return 0;
}



gba_impl struct breakpoint * callc  
gba_breakpoint_create (struct gba *agb, kable add_todbglist, BREAKPOINT_REASON reason, kable enable, uint32_t address, int size, char *describe, int desclen, int match_cnt, char *comment, int comlen, char *err_buf, int buf_max) {
  struct breakpoint *db = (struct breakpoint *) malloc (sizeof (struct breakpoint));
  assert (db != null);

  db->enable = enable;
  db->reason = reason;
  db->describe = null;
  db->desclen = 0;
  db->size = size;
  db->comment = null;
  db->comlen = 0;
  db->expr = null;
  db->address = address;
  db->match_loop = 0;
  db->match_expect = match_cnt <= 0 ? 1 :match_cnt ;

  if (comment != null 
    && comlen > 0) {
    db->comment = (char *)malloc (sizeof (char) * desclen);
    memcpy (db->comment, comment, comlen *sizeof(char) + 1);
    db->comment[desclen] = 0;
  }
  if (describe != null 
    && desclen > 0) {
    db->describe = (char *)malloc (sizeof (char) * desclen + 4);
    memcpy (db->describe, describe, desclen *sizeof(char));
    db->describe[desclen] = 0;
    if (gba_breakpoint_expr_init (agb, & db->expr, describe, err_buf, buf_max) != 0) {
      gba_breakpoint_destroy (db);
      return null;
    }
  }
  if (add_todbglist != false) 
    gba_breakpoint_insert (agb, db);
  return db;
}

gba_impl int callc  
gba_breakpoint_modify (struct gba *agb, struct breakpoint *breakpoint_, kable enable, uint32_t address, int size, char *describe, int desclen, int match_cnt, char *comment, int comlen, char *err_buf, int buf_max) {
  struct breakpoint *devbp = gba_breakpoint_create (agb, false, breakpoint_->reason, 
                                                     enable, address, size, describe, 
                                                     desclen, match_cnt, comment, comlen, 
                                                     err_buf, buf_max);
  if (devbp == null)
    return -1;
  
  gba_breakpoint_destroy_savebody (breakpoint_);
  memcpy (breakpoint_, devbp, sizeof (struct breakpoint));

  free (devbp);
  return 0;
}

#if 0
int main (void) {
  struct gba *gb;
  struct token_buf tkb;
  struct breakpoint_cond *cd;
  struct expr_breakpoint *co;
  char arcbuf[123];
  // const char *chars = "((512 + 0x50*(7 + (555>>2)>>2 <<3==3>>1 +((4))*7)))  *10+(0x2559+0x1f)*2";
  const char *chars = "123456:w";
  int len = strlen (chars);


  gba_create ( &gb);
  cd = gba_add_breakpoint_cond (gb,chars, len, arcbuf);
  co = (struct expr_breakpoint *) cd->expr_breakpoint;

  expr_eval (gb, & co->tree_root);

  return 0;
}
                             


#endif