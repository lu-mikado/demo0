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
#include <intrin.h>
#include "cpu.inl"
#include "apu.inl"
#include "dma.inl"
#include "timer.inl"
#include "gpu.inl"
#include "serial.inl"
#include "controller.inl"
#include "mbus.inl"
#include "arm7tdmi.inl"

gba_impl int callc  
gba_setdriver (struct gba *agb, struct driver *drv) {
  memcpy (& agb->platform_io, drv, sizeof (struct driver));
  return 0;
}

static
void agb_trap_ (struct gba *agb, struct breakpoint *bp) {

  agb->dbg.trap = true;
  if (agb->platform_io.bp_hit != false)
    agb->platform_io.bp_hit (agb, bp);
  coroutine_swito (agb->carrier, true, COROUTINE_EXEC_LINK, null);
  return ;
}

finline 
struct breakpoint *serach_addr_breakpoint (struct gba *agb, uint32_t addr_) {
  uint32_t addr = addr_ & 0xFFFFFFF;
  switch (addr >> 24)  {
  case 0:
    if (addr < 0x4000)
      return agb->dbg.breakpoint_slot[addr >> 1];
    else 
  case 1:
      return null;
  case 2:
    return agb->dbg.breakpoint_slot[16 * 1024/2 + ((addr & WRAM_MASK) >> 1)];
  case 3:
    return agb->dbg.breakpoint_slot[272 * 1024/2 + ((addr & IRAM_MASK) >> 1)];
  case 8:
  case 9: 
  case 10:
  case 11: 
  case 12:
  case 13: 
     return agb->dbg.breakpoint_slot[304 * 1024/2 + ((addr & GAME_PAK_MASK) >> 1)];
  default:
    FORCE_BREAK ();
    break;
  }
  FORCE_BREAK ();
  return null;
}

void gba_reset_waitstate2 (struct gba *agb, uint32_t internalMem800) {
  /* 
  Bit   Expl.
  0     Disable 32K+256K WRAM (0=Normal, 1=Disable) (when off: empty/prefetch)
  1-3   Unknown          (Read/Write-able)
  4     Unknown          (Always zero, not used or write only)
  5     Enable 256K WRAM (0=Disable, 1=Normal) (when off: mirror of 32K WRAM)
  6-23  Unknown          (Always zero, not used or write only)
  24-27 Wait Control WRAM 256K (0-14 = 15..1 Waitstates, 15=Lockup)
  28-31 Unknown          (Read/Write-able)
  */
  //agb->mem.wram_wait = 15 - BDIFFUSE (internalMem800, 24, 15);
  //assert (agb->mem.wram_wait != 0);
}

gba_impl void callc 
gba_set_addr_breakpoint_slot (struct gba *agb, uint32_t addr_,  struct breakpoint *bp) {
  uint32_t addr = addr_ & 0xFFFFFFE;
  switch (addr >> 24)  {
  case 0:
    if (addr < 0x4000)
      agb->dbg.breakpoint_slot[addr >> 1] = bp;
    else 
  case 1:
     break;
  case 2:
    agb->dbg.breakpoint_slot[16 * 1024/2 + ((addr & WRAM_MASK) >> 1)] = bp;
  case 3:
    agb->dbg.breakpoint_slot[272 * 1024/2 + ((addr & IRAM_MASK) >> 1)] = bp;
  case 8:
  case 9: 
  case 10:
  case 11: 
  case 12:
  case 13: 
     agb->dbg.breakpoint_slot[304 * 1024/2 + ((addr & GAME_PAK_MASK) >> 1)]  = bp;
  default:
    break;
  }
}

gba_impl kable callc  
gba_iscode_addr (struct gba *agb, uint32_t addr_) {
  uint32_t addr = addr_ & 0xFFFFFFF;
  switch (addr >> 24)  {
  case 0:
    if (addr < 0x4000)
  case 2:
  case 3:
  case 8:
  case 9: 
  case 10:
  case 11: 
  case 12:
  case 13: 
      return true;
    else 
  default:
      return false;
  }
  return false;
}

gba_impl struct breakpoint *callc
gba_get_addr_breakpoint_slot (struct gba *agb, uint32_t addr_) {
  uint32_t addr = addr_ & 0xFFFFFFE;
  switch (addr >> 24)  {
  case 0:
    if (addr < 0x4000)
      return agb->dbg.breakpoint_slot[addr >> 1];
    else 
  case 1:
     break;
  case 2:
    return agb->dbg.breakpoint_slot[16 * 1024/2 + ((addr & WRAM_MASK) >> 1)];
  case 3:
    return agb->dbg.breakpoint_slot[272 * 1024/2 + ((addr & IRAM_MASK) >> 1)];
  case 8:
  case 9: 
  case 10:
  case 11: 
  case 12:
  case 13: 
     return agb->dbg.breakpoint_slot[304 * 1024/2 + ((addr & GAME_PAK_MASK) >> 1)];
  default:
    break;
  }
  return null;
}

void agb_trace_breakpoint_read (struct gba *agb, uint32_t addr_, int size /* size must 2^n*/) {

  if (agb->dbg.debug != false) {
     int id;
     struct list_chunk *pv;
     uint32_t mask = ~(size - 1);
     uint32_t addr = addr_ & mask;

     /* check addr breakpoint*/
     LIST_EACH_ (agb->dbg.read_bp_list, id, pv) {
       struct breakpoint *m_request = (struct breakpoint *) pv->dat_ptr;
       if (m_request->enable != false
         && (m_request->address & mask)== addr) {
          if (gba_breakpoint_expr_run (agb, m_request) != false) {
            if (++m_request->match_loop >= m_request->match_expect) {
              m_request->match_loop = 0;
              agb_trap_ (agb, m_request);
              return ;
            }
          }
       }
     }
  }
}

void agb_trace_breakpoint_write (struct gba *agb, uint32_t addr_, int size /* size must 2^n*/) {

  if (agb->dbg.debug != false) {
     int id;
     struct list_chunk *pv;
     uint32_t mask = ~(size - 1);
     uint32_t addr = addr_ & mask;

     /* check addr breakpoint*/
     LIST_EACH_ (agb->dbg.write_bp_list, id, pv) {
       struct breakpoint *m_request = (struct breakpoint *) pv->dat_ptr;
       if (m_request->enable != false
         && (m_request->address & mask)== addr) {
          if (gba_breakpoint_expr_run (agb, m_request) != false) {
            if (++m_request->match_loop >= m_request->match_expect) {
              m_request->match_loop = 0;
              agb_trap_ (agb, m_request);
              return ;
            }
          }
       }
     }
  }
}

void agb_irq_breakpoint (struct gba *agb, uint16_t mask) {

  if (agb->dbg.debug != false) {
    if (agb->arm7.ime.blk != 0
      && agb->arm7.ie.blk & mask)
    {
     int id;
     struct list_chunk *pv;

     for (id = 0; id != 16; id++) {
       int mask_mapper = 1 << id;
       if (mask & mask_mapper)
         break;
     }
     assert (id != 16);
     agb->dbg.int_enum = id;

     /* check int breakpoint*/
     LIST_EACH_ (agb->dbg.int_bp_list, id, pv) {
       struct breakpoint *i_request = (struct breakpoint *) pv->dat_ptr;
       if (i_request->enable != false) {
          if (gba_breakpoint_expr_run (agb, i_request) != false) {
            if (++i_request->match_loop >= i_request->match_expect) {
              i_request->match_loop = 0;
              agb_trap_ (agb, i_request);
              return ;
            }
          }
       }
     }
    }
  }
}

finline void
check_setlcd_stat (struct gba *agb, kable cond, int32_t stat_bit, int32_t irq_enbit, int32_t irq_bit) {
  if (cond) {
    agb->gpu.status.blk |= stat_bit;
    if (agb->gpu.status.blk & irq_enbit) {
      agb->arm7.ifs.blk |= irq_bit;
      check_halt (agb, irq_bit);
      agb_irq_breakpoint (agb, irq_bit);
    }
  } else {
    agb->gpu.status.blk &= ~stat_bit;
  }
}

dinline int32_t
dev_tick (struct gba *agb, int32_t clks) {

  struct arm7 *const arm = & agb->arm7;
  union iopad_blk32 *const regs = & arm->regs[0];

  while (clks > 0) {
    int32_t consume = 0;
    
    while (consume < 32) {
      /* =========================== DMA =================================== */
      if (agb->dma.active != null) {
        if (agb->dma.active->fifo != null) {
          if (agb->dma.active->fifo->count <= 16) {
            int32_t id;
            for (id = 0; id != 4; id++) {
              union iopad_blk32 sndfifo;
              consume += agb_mbus_rw (agb, agb->dma.active->source_internal.blk, & sndfifo.blk, true) + 2;

              channel_fifo_push (agb->dma.active->fifo, sndfifo.sblk8[0], 0);
              channel_fifo_push (agb->dma.active->fifo, sndfifo.sblk8[1], 0);
              channel_fifo_push (agb->dma.active->fifo, sndfifo.sblk8[2], 0);
              channel_fifo_push (agb->dma.active->fifo, sndfifo.sblk8[3], 0);

              agb->dma.active->source_internal.blk += agb->dma.active->source_vec;
            }
          }
          channel_fifo_pop (agb->dma.active->fifo);

          if (agb->dma.active->control.blk & 0x4000) { 
            agb->arm7.ifs.blk |= IRQ_REQUEST_DMA_START << agb->dma.active->chan;    
            check_halt (agb, IRQ_REQUEST_DMA_START << agb->dma.active->chan);       
          }    
          if (agb->dma.active->control.blk & 1 << 9) {
            agb->dma.active->ready = false;
          } else {
            agb->dma.active->control.blk &= ~0x8000;
          }
          agb->dma.active = null;
          dma_re_dispatch (agb);
        } else {
          if (agb->dma.active->control.blk & 1 << 10) {
            /* transmission word */
            uint32_t memory;
            consume += agb_mbus_rw (agb, agb->dma.active->source_internal.blk, & memory, true);
            consume += agb_mbus_ww (agb, agb->dma.active->destin_internal.blk, memory, true) + 2;
          } else {
            /* transmission halfword */
            uint16_t memory;
            consume += agb_mbus_rhw (agb, agb->dma.active->source_internal.blk, & memory, true);
            consume += agb_mbus_whw (agb, agb->dma.active->destin_internal.blk, memory, true) + 2;
          }
          agb->dma.active->source_internal.blk += agb->dma.active->source_vec;
          agb->dma.active->destin_internal.blk += agb->dma.active->destin_vec;
      
          if (--agb->dma.active->counter_internal.blk == 0) {
            if (agb->dma.active->control.blk & 0x4000) { 
              agb->arm7.ifs.blk |= IRQ_REQUEST_DMA_START << agb->dma.active->chan;    
              check_halt (agb, IRQ_REQUEST_DMA_START << agb->dma.active->chan);       
            }    
            if (agb->dma.active->control.blk & 1 << 9) {
              agb->dma.active->ready = false;
            } else {
              agb->dma.active->control.blk &= ~0x8000;
            }
            agb->dma.active = null;
            dma_re_dispatch (agb);
          }
        }
      } else {
        /* devticks with cpu ?*/
        if ((agb->sundry.halt.blk & 0x80000000) == 0) {
          if (agb->dbg.debug != false) {
            /* check addr breakpoint */
            struct breakpoint *dev_bp = serach_addr_breakpoint (agb, arm7_getpc (& agb->arm7));
            if (dev_bp != null
              && dev_bp->enable != false 
              && gba_breakpoint_expr_run (agb, dev_bp) != false
              && ++dev_bp->match_loop >= dev_bp->match_expect)
            {
              agb->dbg.weak_bp.enable = false;
              dev_bp->match_loop = 0;
              agb_trap_ (agb, dev_bp);
            }
            /* check weak breakpoint */
            else if (agb->dbg.weak_bp.enable != false) {
              switch (agb->dbg.weak_bp.reason) {
              case BREAKPOINT_REASON_STEP_OVER:
                if (agb->dbg.weak_bp.usr_data != 0) {
                  /* address breakpoint */
              case BREAKPOINT_REASON_STEP_OUT:
              case BREAKPOINT_REASON_CURSOR: 
                  if (arm7_getpc (& agb->arm7) == agb->dbg.weak_bp.address) {
                    agb->dbg.weak_bp.enable = false;
                    agb_trap_ (agb, & agb->dbg.weak_bp);
                  }
                } else {
                   /* next instruction */
              case BREAKPOINT_REASON_STEP_IN:
                  agb->dbg.weak_bp.enable = false;
                  agb_trap_ (agb, & agb->dbg.weak_bp);
                }  
              default:
                break;
              }
            }
          }
          /* arm7 decoce */
          consume += cpu_tick (agb);
          /* #include "arm7tdmi2.inl" */
        } else if  ((agb->sundry.halt.blk & 0x80000080) == 0x80000000) {
          consume += 16;
        } else {
          sio_tick (& agb->sio, 16);
          clks -= 16;
          FORCE_BREAK ();
          continue ;
        }
      } 
    }
    clks -= consume;
    
    apu_tick (& agb->apu, consume);
    
    /* ============================ timer ================================ */
    if (1) {
      uint32_t cascade[3] = { 0, 0, 0};
      
      /* =================== timer 0, special:fifo =============================== */
      if (agb->timer.igniter.item[0] == 1) {
        agb->timer.component[0].phase += consume;
        while (agb->timer.component[0].phase >= agb->timer.component[0].overflow) {
          agb->timer.component[0].phase -= agb->timer.component[0].overflow;
          /* reload timer reload register */
          agb->timer.component[0].overflow = agb->timer.component[0].trigger * (0x10000 - agb->timer.component[0].reload.blk);
          /* check FIFO Request */
          if (agb->apu.settings.nr52.blk & 0x80) {
            if ((agb->apu.settings.snd_ctl.blk & 1 << 10) == 0) {
              /* check FIFO-A DMA1 Request */
              if ((agb->dma.component[1].control.blk & 0xB000) == 0xB000) {
                agb->dma.component[1].ready = true;
                dma_re_dispatch (agb);
              }
            } 
            if ((agb->apu.settings.snd_ctl.blk & 1 << 14) == 0) {
              /* check FIFO-B DMA2 Request */
              if ((agb->dma.component[2].control.blk & 0xB000) == 0xB000) {
                agb->dma.component[2].ready = true;
                dma_re_dispatch (agb);
              }
            } 
          }
  #undef CHECK_TIMER_INTERRUPT
  #define  CHECK_TIMER_INTERRUPT(n)     do  {      if (agb->timer.component[n].control.blk & 0x40) { \
                /* gba_timer overflow, set interrupt flags*/\
                agb->arm7.ifs.blk |= (IRQ_REQUEST_TIMER_START << (n));\
                  /* check resume halt */\
                check_halt (agb, (IRQ_REQUEST_TIMER_START << (n)));\
                /*check timet irq breakpoint */\
                agb_irq_breakpoint (agb, (IRQ_REQUEST_TIMER_START << (n)));\
  } } while (0)
          cascade[0]++;
          CHECK_TIMER_INTERRUPT (0);
        } 
      }
      /* =================== timer 1, special:fifo, cascade =============================== */
      if (agb->timer.igniter.item[1] != 0) {
        if (agb->timer.igniter.item[1] == 1)
          agb->timer.component[1].phase += consume;
        else 
          agb->timer.component[1].phase += cascade[0];
        while (agb->timer.component[1].phase >= agb->timer.component[1].overflow) { 
          agb->timer.component[1].phase -= agb->timer.component[1].overflow;
          /* reload timer reload register */
          agb->timer.component[1].overflow = agb->timer.component[1].trigger * (0x10000 - agb->timer.component[1].reload.blk);
          /* check FIFO Request */
          if (agb->apu.settings.nr52.blk & 0x80) {
            if ((agb->apu.settings.snd_ctl.blk & 1 << 10) != 0) {
              /* check FIFO-A DMA1 Request */
              if ((agb->dma.component[1].control.blk & 0xB000) == 0xB000) {
                agb->dma.component[1].ready = true;
                dma_re_dispatch (agb);
              }
            } 
            if ((agb->apu.settings.snd_ctl.blk & 1 << 14) != 0) {
              /* check FIFO-B DMA2 Request */
              if ((agb->dma.component[2].control.blk & 0xB000) == 0xB000) {
                agb->dma.component[2].ready = true;
                dma_re_dispatch (agb);
              }
            } 
          } 
          cascade[1]++;
          CHECK_TIMER_INTERRUPT (1);
        }       
      }
      /* =================== timer 2, cascade =============================== */
      if (agb->timer.igniter.item[2] != 0) {
        if (agb->timer.igniter.item[2] == 1)
          agb->timer.component[2].phase += consume;
        else 
          agb->timer.component[2].phase += cascade[1];
        while (agb->timer.component[2].phase >= agb->timer.component[2].overflow) { 
          agb->timer.component[2].phase -= agb->timer.component[2].overflow;
          /* reload timer reload register */
          agb->timer.component[2].overflow = agb->timer.component[2].trigger * (0x10000 - agb->timer.component[2].reload.blk);
          cascade[2]++;
          CHECK_TIMER_INTERRUPT (2);
        }       
      }
      /* =================== timer 3, cascade =============================== */
      if (agb->timer.igniter.item[3] != 0) {
        if (agb->timer.igniter.item[3] == 1)
          agb->timer.component[3].phase += consume;
        else 
          agb->timer.component[3].phase += cascade[2];
        while (agb->timer.component[3].phase >= agb->timer.component[3].overflow) { 
          agb->timer.component[3].phase -= agb->timer.component[3].overflow;
          /* reload timer reload register */
          agb->timer.component[3].overflow = agb->timer.component[3].trigger * (0x10000 - agb->timer.component[3].reload.blk);
          CHECK_TIMER_INTERRUPT (3);
        }       
      }
    }
    sio_tick (& agb->sio, consume);
  }
  return clks;
}

int callc
gba_frame_ (struct gba *agb) {

  /*cpu freq := 2^24 (16777216)
   * vbl freq := 59.727hz 
   * hbl freq := 13.618khz 
   * scanline (with dummy) := 228 
   * visual scanline := 160 
   * clks/per frame := 280898.35417818
   * clks/per scanline := 1232.0103253429
   * clks/per visual scanline render := 960.0161075599
   * clks/hbl := 271.994217783
   */
  int cnt;
  int32_t clks = agb->sundry.clks_left;

  for (cnt =0 ; cnt != 160; cnt++) {
    /* visual scanline 0~ 159 */
    agb->gpu.line.blk = cnt;
    agb->gpu.status.blk &= ~(GPU_STAT_HBL | GPU_STAT_LYCS | GPU_STAT_VBL);
    gpu_scanline (& agb->gpu);
    check_setlcd_stat (agb, agb->gpu.line.blk == (agb->gpu.status.blk >> 8), GPU_STAT_LYCS, GPU_STAT_LYC_IRQ, IRQ_REQUEST_LY);
    clks = dev_tick (agb, 960 + clks);
    /* hblank period, stuff hbl and dma */
    dma_hbl_stuff (agb);
    check_setlcd_stat (agb, true, GPU_STAT_HBL, GPU_STAT_HBL_IRQ, IRQ_REQUEST_HBL);
    clks = dev_tick (agb, 272 + clks);
    gpu_dispose_inhblank (& agb->gpu, true);
  }
  /* vblank period */
  agb->gpu.line.blk = 160;
  agb->gpu.status.blk &= ~(GPU_STAT_HBL | GPU_STAT_LYCS | GPU_STAT_VBL);
  check_setlcd_stat (agb, true, GPU_STAT_VBL, GPU_STAT_VBL_IRQ, IRQ_REQUEST_VBL);
  check_setlcd_stat (agb, 160 == (agb->gpu.status.blk >> 8), GPU_STAT_LYCS, GPU_STAT_LYC_IRQ, IRQ_REQUEST_LY);
  dma_vbl_stuff (agb);
  gpu_dispose_invblank (& agb->gpu);
  clks = dev_tick (agb, 960 + clks);
  /* hblank period, stuff hbl */
  check_setlcd_stat (agb, true, GPU_STAT_HBL, GPU_STAT_HBL_IRQ, IRQ_REQUEST_HBL);
  clks = dev_tick (agb, 274 + clks);
  /* post drv infos */
  /* update Device's output */
  controller_update (& agb->joypad);
  apu_buffer_reset (& agb->apu);
  agb->platform_io.sound_post (agb, & agb->apu.sndbuf);
  agb->platform_io.video_post (agb, & agb->gpu.vid_buf);
  agb->platform_io.vsync (agb); 

  for (cnt =161 ; cnt != 227; cnt++) {
    /* invisual scanline 161~ 226 */
    agb->gpu.line.blk = cnt;
    agb->gpu.status.blk &= ~(GPU_STAT_HBL | GPU_STAT_LYCS);
    check_setlcd_stat (agb, agb->gpu.line.blk == (agb->gpu.status.blk >> 8), GPU_STAT_LYCS, GPU_STAT_LYC_IRQ, IRQ_REQUEST_LY);
    clks = dev_tick (agb, 960 + clks);
    /* hblank period, stuff hbl */
    check_setlcd_stat (agb, true, GPU_STAT_HBL, GPU_STAT_HBL_IRQ, IRQ_REQUEST_HBL);
    clks = dev_tick (agb, 272 + clks);
  }
  /* last line period */
  agb->gpu.line.blk = 227;
  agb->gpu.status.blk &= ~(GPU_STAT_HBL | GPU_STAT_LYCS | GPU_STAT_VBL);
  check_setlcd_stat (agb, 227 == (agb->gpu.status.blk >> 8), GPU_STAT_LYCS, GPU_STAT_LYC_IRQ, IRQ_REQUEST_LY);
  clks = dev_tick (agb, 960 + clks);
  /* hblank period, stuff hbl and dma */
  dma_hbl_stuff (agb);
  check_setlcd_stat (agb, true, GPU_STAT_HBL, GPU_STAT_HBL_IRQ, IRQ_REQUEST_HBL);
  clks = dev_tick (agb, 272 + clks);
  agb->sundry.clks_left = clks;
  
  gpu_dispose_inhblank (& agb->gpu, true);
  return 0;
} /* FIXME:maybe BUG in VBlank */

gba_impl uint16_t callc  
gba_fast_rhw (struct gba *agb, uint32_t addr) {
  return gba_fast_rhw_ (agb, addr);
}

#if 0

#endif 

int gba_set_rtc (struct gba *agb) {
  return 0;
}

gba_impl int callc  
gba_set_bios (struct gba *agb, void *bios_bin, int size) {

  memset (agb->mem.bios, 0, sizeof (agb->mem.bios));
  memcpy (agb->mem.bios, bios_bin, size);
  return 0;
}

gba_impl int callc  
gba_fetchrom (struct gba *gba, FILE *fp) {

  int id;
  int res;
  uint8_t title[13];

  struct header header;
# include "backup.inl"
  res = fseek (fp, 0, SEEK_SET);
  if (res != 0)
    return -1;

  res = fread (& header, sizeof (struct header), 1, fp);
  if (res > 1)
    return -1;
  if (header.fixed != 0x96)
    return -1;

  fseek (fp, 0, SEEK_END);
  res = (int) ftell (fp);
  fseek (fp, 0, SEEK_SET);

  if (res <= 0)
    return -1;
  else {}

  memset (title, 0, sizeof (title));
  memcpy (& gba->rom_header, & header, sizeof (struct header));
  memcpy (title, header.title, 12);
  memset (& gba->sundry, 0, sizeof (gba->sundry));
  memset (& gba->mem.IOMap[0], 0, sizeof (gba->mem.IOMap));
  memset (& gba->mem.SRam[0], 0, sizeof (gba->mem.SRam));
  memset (& gba->mem.PRom[0], 0, sizeof (gba->mem.PRom));
  memset (& gba->mem.IRam[0], 0, sizeof (gba->mem.IRam));
  memset (& gba->mem.WRam[0], 0, sizeof (gba->mem.WRam));
  memset (& gba->mem.save_backup.flash, 0, sizeof (gba->mem.save_backup.flash));
  memset (& gba->mem.save_backup.eeprom, 0, sizeof (gba->mem.save_backup.eeprom));

  res = fread (gba->mem.PRom, res, 1, fp);
  assert (res == 1);

  /* Link each */
  gba->arm7.agb = gba;
  gba->joypad.agb = gba;
  gba->apu.agb = gba;
  gba->gpu.agb = gba;
  gba->sio.agb = gba;
  gba->rc_clock.agb = gba;

  gba->mem.eeprom_ptr = null;
  gba->mem.flash_ptr = null;
  gba->rc_clock.support = false;
  gba->mem.backup_id = SAVE_BACKUP_NONE;
  /* serach database, set backup device */

  for (id = 0; id != sizeof (backup_data)/ sizeof (backup_data[0]); id++) {
    if (strcmp (backup_data[id].title, (const char *)title) == 0) {
      gba->mem.backup_id = backup_data[id].backup_id;
      switch (backup_data[id].backup_id) {
      case SAVE_BACKUP_EEPROM_512B:
        eeprom_init (& gba->mem.save_backup.eeprom, EEPROM_CAPACITY_512B);
        gba->mem.eeprom_ptr = & gba->mem.save_backup.eeprom;
        gba->mem.eeprom_ptr->agb = gba;
        break;
      case SAVE_BACKUP_EEPROM_8K:
        eeprom_init (& gba->mem.save_backup.eeprom, EEPROM_CAPACITY_8K);
        gba->mem.eeprom_ptr = & gba->mem.save_backup.eeprom;
        gba->mem.eeprom_ptr->agb = gba;
        break;
      case SAVE_BACKUP_FLASH_AT29LV512:
      case SAVE_BACKUP_FLASH_MACRONIX_128K:
      case SAVE_BACKUP_FLASH_MACRONIX_64K:
      case SAVE_BACKUP_FLASH_SANYO:
      case SAVE_BACKUP_FLASH_SST39LF_VF512:
      case SAVE_BACKUP_FLASH_PANASONIC:
        flash_init (& gba->mem.save_backup.flash, backup_data[id].backup_id);
        gba->mem.flash_ptr = & gba->mem.save_backup.flash;
        gba->mem.flash_ptr->agb = gba;
        break;
      default:
        break;
      }
      if (backup_data[id].rt_clock_support)
        gba_set_rtc (gba);
      break;
    }
  }
  gba_reset_waitstate (gba, 0);

  /* Reset AGB Device. */
  cpu_reset (& gba->arm7);
  dma_reset (& gba->dma);
  controller_reset (& gba->joypad);
  apu_reset (& gba->apu);
  gpu_reset (& gba->gpu);
  sio_reset (& gba->sio);

  /* Link each */
  gba->arm7.agb = gba;
  gba->joypad.agb = gba;
  gba->apu.agb = gba;
  gba->gpu.agb = gba;
  gba->sio.agb = gba; 
  gba->rc_clock.agb = gba;

  gba->sundry.halt.blk = 0;

  cpsr_to_fast (& gba->arm7.cpsr, & gba->arm7.cpsr_fast);

  reset_int_magic (gba);
  // @TEST:=
  // gba->dbg.debug = true;
  return 0;
}

gba_impl int callc  
gba_fetchrom_ansi (struct gba *agb, const char *filename) {
  FILE *fd = fopen (filename, "rb");
  if (fd != null) {
    int sig= gba_fetchrom (agb, fd);
    fclose (fd);
    return sig;
  }
  return -1;
}

gba_impl int callc  
gba_fetchrom_unicode (struct gba *agb, const wchar_t *filename) {
#ifdef _WIN32
  FILE *fd = _wfopen (filename, L"rb");
#else 
#error "gba_fetchrom_unicode unimpl"
#endif 
  if (fd != null) {
    int sig= gba_fetchrom (agb, fd);
    fclose (fd);
    return sig;
  }
  return -1;
}

gba_impl void callc  
gba_destroy (struct gba **agb) {
  if (agb != null) {
    struct gba *p = *agb;
    if (p != null) {
      free (p);
    }
    *agb = null;
  }
}

gba_impl int callc  
gba_create (struct gba **agb) {
  struct gba *p = (struct gba *)malloc (sizeof (struct gba));
  memset (p, 0, sizeof (struct gba));
  assert (agb != null);

#define EM_1M (0x400*1024)
#define EMULTOR_CORE_STACK_SIZE (EM_1M* 8)
  /* init coroutine */
  coroutine_ctor (& p->carrier, EMULTOR_CORE_STACK_SIZE);

  list_init_ (& p->dbg.addr_bp_list);
  list_init_ (& p->dbg.read_bp_list);
  list_init_ (& p->dbg.write_bp_list);
  list_init_ (& p->dbg.int_bp_list);
  list_init_ (& p->dbg.soft_int_bp_list);

#undef EM_1M
#undef EMULTOR_CORE_STACK_SIZE

  gba_setgpu_coltype (p, GPU_COL_DEFAULT);

  *agb = p;
  return 0;
}

gba_impl int callc  
gba_stepover (struct gba *agb) {
  /* XXX:for step over, only test bl instructions */
  uint32_t opcode;
  uint32_t eip;

  agb->dbg.weak_bp.enable  = true;
  agb->dbg.weak_bp.reason = BREAKPOINT_REASON_STEP_OVER;
  agb->dbg.weak_bp.usr_data = 0;

  /* check current arm7tdmi 's opcode. */
  opcode = agb->arm7.opcode[0];
  eip = arm7_getpc (& agb->arm7);
  agb->dbg.weak_bp.address = eip + 4;

  if (arm7_thumbmode (& agb->arm7) != false) {
    /* check is thumb bl instruction */
    /* see ARM Architecture Reference Manual:: A7.1.17 BL, BLX (1) */
    if ((opcode & 0xF000) == 0xF000) {
      agb->dbg.weak_bp.usr_data = 1;
    }
  } else if ((opcode & 0xF000000) == 0xB000000) {
      /* check is arm7 bl instruction */
      /* see ARM Architecture Reference Manual:: A4.1.5 B, BL */
    agb->dbg.weak_bp.usr_data = 2;
  }
  return 0;
}

gba_impl int callc  
gba_stepout (struct gba *agb) {
  /* XXX:for step out, only  current lr register*/
  if (agb->dbg.debug != false) {
    agb->dbg.weak_bp.enable = true;
    agb->dbg.weak_bp.reason = BREAKPOINT_REASON_STEP_OUT;
    agb->dbg.weak_bp.address = agb->arm7.regs[14].blk & 0xFFFFFFE; /* maybe... with lr's thumb flags ?? */
    agb->dbg.weak_bp.usr_data = 1;
  }
  return 0;
}

gba_impl int callc  
gba_breakpoint_set_stepin (struct dev_infos *dev, struct gba *agb) {
  if (agb->dbg.debug != false) {
    agb->dbg.weak_bp.enable = true;
    agb->dbg.weak_bp.reason = BREAKPOINT_REASON_STEP_IN;
  }
  return 0;
}

void gba_breakpoint_set_brk (struct dev_infos *dev, struct gba *agb) {
  if (agb->dbg.debug != false) {
    agb->dbg.weak_bp.enable = true;
    agb->dbg.weak_bp.reason = BREAKPOINT_REASON_INTERRUPT;
  }
}

gba_impl int callc  
gba_cursor_bp (struct gba *agb, uint32_t address) {
  if (agb->dbg.debug != false) {
    agb->dbg.weak_bp.enable = true;
    agb->dbg.weak_bp.reason = BREAKPOINT_REASON_CURSOR;
    agb->dbg.weak_bp.address = address & 0xFFFFFFE;
  }
  return 0;
}

gba_impl int callc  
gba_frame (struct gba *agb) {
  if (agb->dbg.trap != false) {
    /* resume from suspend. reset context*/
    agb->dbg.trap = false;
    context_swap2 (coroutine_getctx (agb->carrier, COROUTINE_EXEC_CURRENT), 
                             coroutine_getctx (agb->carrier, COROUTINE_EXEC_LINK));
  } else {
    coroutine_setcall (agb->carrier, ( int (*)(void *))gba_frame_, agb);
    coroutine_run (agb->carrier);
  }
  if (agb->dbg.trap != false)
    return -1;
  else 
    return 0;
}

gba_impl int callc  
gba_setgpu_coltype (struct gba *agb, GPU_COL_MOLD col_mold) {
  if (col_mold == GPU_COL16_X1B5G5R5)
    agb->gpu.palette16_b = & agb->gpu.palette[0];
  else if (col_mold == GPU_COL16_X1R5G5B5)
    agb->gpu.palette16_b = & agb->gpu.palette2[0];
  else 
    assert (0);
  agb->gpu.palette16_b = & agb->gpu.palette[0];
  return 0;
}

gba_impl int callc  
gba_stepin (struct gba *agb) {
  agb->dbg.weak_bp.reason = BREAKPOINT_REASON_STEP_IN;
  agb->dbg.weak_bp.enable = true;
  agb->dbg.weak_bp.usr_data = 0;
  return 0;
}

gba_impl int callc  
gba_setdebug (struct gba *agb, kable enable) {
  agb->dbg.debug = enable;
  return 0;
}

gba_impl int callc  
gba_read_battery (struct gba *agb, FILE *fd) {

  if (fd == NULL)
    return -1;
  else {
    int size;
    fseek (fd, 0, SEEK_END);
    size = ftell (fd);
    fseek (fd, 0, SEEK_SET);

    switch (agb->mem.backup_id) {
    case SAVE_BACKUP_EEPROM_512B:
    case SAVE_BACKUP_EEPROM_8K:
      if (size != 0x2000)
        return -1;
      fread (& agb->mem.save_backup.eeprom.memory[0], 1, size, fd);
      break;
    case SAVE_BACKUP_GM76V256CLLFW10:
    case SAVE_BACKUP_MB85R256:
      if (size != 0x10000)
        return -1;
      fread (& agb->mem.SRam[0], 1, size, fd);
      break;
    case SAVE_BACKUP_FLASH_AT29LV512:
    case SAVE_BACKUP_FLASH_MACRONIX_128K:
    case SAVE_BACKUP_FLASH_MACRONIX_64K:
    case SAVE_BACKUP_FLASH_SANYO:
    case SAVE_BACKUP_FLASH_SST39LF_VF512:
    case SAVE_BACKUP_FLASH_PANASONIC:
      if (size != 0x20000)
        return -1;
      fread (& agb->mem.flash_ptr->memory[0], 1, size, fd);
      break;
    default:
      break;
    }
    fseek (fd, 0, SEEK_SET);
  }
  return 0;
}
gba_impl int callc 
gba_save_battery (struct gba *agb, FILE *fd) {

  if (fd == NULL)
    return -1;
  else {
    fseek (fd, 0, SEEK_SET);
    switch (agb->mem.backup_id) {
    case SAVE_BACKUP_EEPROM_512B:
    case SAVE_BACKUP_EEPROM_8K:
      fwrite (& agb->mem.save_backup.eeprom.memory[0], 1, 0x2000, fd);
      break;
    case SAVE_BACKUP_GM76V256CLLFW10:
    case SAVE_BACKUP_MB85R256:
      fwrite (& agb->mem.SRam[0], 1, 0x10000, fd);
      break;
    case SAVE_BACKUP_FLASH_AT29LV512:
    case SAVE_BACKUP_FLASH_MACRONIX_128K:
    case SAVE_BACKUP_FLASH_MACRONIX_64K:
    case SAVE_BACKUP_FLASH_SANYO:
    case SAVE_BACKUP_FLASH_SST39LF_VF512:
    case SAVE_BACKUP_FLASH_PANASONIC:
      fwrite (& agb->mem.flash_ptr->memory[0], 1, 0x20000, fd);
      break;
    default:
      break;
    }
    fseek (fd, 0, SEEK_SET);
  }
  return 0;
}

gba_impl int callc  
gba_set_apubuffer (struct gba *agb, APU_SAMPLE_TYPE sample, int sample_sec) {
  agb->apu.sndbuf.sample_request = sample;

  switch (sample) {
  case APU_SAMPLE_TYPE_UINT8:
  case APU_SAMPLE_TYPE_SINT8:
    agb->apu.sndbuf.blk_depth = 8;
    agb->apu.sndbuf.blk_align = 2;
    break;
  case APU_SAMPLE_TYPE_FLOAT:
    agb->apu.sndbuf.blk_depth = 32;
    agb->apu.sndbuf.blk_align = 8;
    break;
  case APU_SAMPLE_TYPE_UINT16:
  case APU_SAMPLE_TYPE_SINT16:
    agb->apu.sndbuf.blk_depth = 16;
    agb->apu.sndbuf.blk_align = 4;
    break;
  }
  agb->apu.sndbuf.blk_nums = sample_sec / 60;
  agb->apu.sndbuf.byte = agb->apu.sndbuf.blk_nums * 4;
  agb->apu.sam_trigger = (uint32_t) (AGB_CPU_FREQ_f / 60.0 / (double) agb->apu.sndbuf.blk_nums);
  agb->apu.sam_phase = 0;
  agb->apu.sam_start = 0;

  memset (& agb->apu.sndbuf.buf[0], 0, sizeof (agb->apu.sndbuf.buf));
  return 0;
}