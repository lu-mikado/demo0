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
 
#ifndef MBUS_INL
#define MBUS_INL 1

#include "eeprom.inl"
#include "flash.inl"

#define HALFWORD_MASK 0xFFFFFFE 
#define WORD_MASK 0xFFFFFFC
#define BYTE_MASK 0xFFFFFFF
#define IRAM_MASK 0x7FFF
#define WRAM_MASK 0x3FFFF
#define OAM_MASK 0x3FF 
#define PAL_MASK 0x3FF 
#define VRAM_MASK 0x1FFFF 
#define VRAM_MASK_Vx17FFF_x20000 0x17FFF 
#define BIOS_MASK 0x3FFF
#define GAME_PAK_MASK 0x1FFFFFF
#define IOMAP_MASK 0x7FF 
#define IOMAP2_MASK 0xFFFFFF 
#define SRAM_MASK 0xFFFF 

void  agb_trace_breakpoint_write (struct gba *agb, uint32_t addr_, int size /* size must 2^n*/);
void  agb_trace_breakpoint_read (struct gba *agb, uint32_t addr_, int size /* size must 2^n*/);

void gba_reset_waitstate (struct gba *agb, uint16_t wait204) {
  /* 
  Bit   Expl.
  0-1   SRAM Wait Control          (0..3 = 4,3,2,8 cycles)
  2-3   Wait State 0 First Access  (0..3 = 4,3,2,8 cycles)
  4     Wait State 0 Second Access (0..1 = 2,1 cycles)
  5-6   Wait State 1 First Access  (0..3 = 4,3,2,8 cycles)
  7     Wait State 1 Second Access (0..1 = 4,1 cycles; unlike above WS0)
  8-9   Wait State 2 First Access  (0..3 = 4,3,2,8 cycles)
  10    Wait State 2 Second Access (0..1 = 8,1 cycles; unlike above WS0,WS1)
  11-12 PHI Terminal Output        (0..3 = Disable, 4.19MHz, 8.38MHz, 16.78MHz)
  13    Not used
  14    Game Pak Prefetch Buffer (Pipe) (0=Disable, 1=Enable)
  15    Game Pak Type Flag  (Read Only) (0=GBA, 1=CGB) (IN35 signal)
  16-31 Not used
  */

  const int32_t sram_wait[4] = { 4, 3, 2, 8 };
  const int32_t prom0_wait_noseq[4] = { 4, 3, 2, 8 };
  const int32_t prom0_wait_seq[2] = { 2, 1 };
  const int32_t prom1_wait_noseq[4] = { 4, 3, 2, 8 };
  const int32_t prom1_wait_seq[2] = { 4, 1 };
  const int32_t prom2_wait_noseq[4] = { 4, 3, 2, 8 };
  const int32_t prom2_wait_seq[2] = { 8, 1 };
  
  agb->mem.sram_wait = sram_wait[BDIFFUSE (wait204, 0, 3)];
  agb->mem.pwait_noseq[0] = prom0_wait_noseq[BDIFFUSE (wait204, 2, 3)];
  agb->mem.pwait_seq[0] = prom0_wait_seq[BDIFFUSE (wait204, 4, 1)];
  agb->mem.pwait_noseq[1] = prom1_wait_noseq[BDIFFUSE (wait204, 5, 3)];
  agb->mem.pwait_seq[1] = prom1_wait_seq[BDIFFUSE (wait204, 7, 1)];
  agb->mem.pwait_noseq[2] = prom2_wait_noseq[BDIFFUSE (wait204, 8, 3)];
  agb->mem.pwait_seq[2] = prom2_wait_seq[BDIFFUSE (wait204, 10, 1)];
}

dinline
void agb_io_wb (struct gba *const agb, uint32_t addr,  uint8_t value0) {

  const uint32_t value = value0;
  
  switch (addr & IOMAP2_MASK) {
  case 0x000: case 0x002: case 0x004: case 0x008: case 0x00A:
  case 0x00C: case 0x00E: case 0x010:
  case 0x012: case 0x014: 
  case 0x016: case 0x018:
  case 0x01A: case 0x01C: 
  case 0x01E: case 0x020: case 0x022:
  case 0x024: 
  case 0x026: case 0x028: 
  case 0x02A: 
  case 0x02C: 
  case 0x02E: 
  case 0x030: 
  case 0x032: case 0x034: 
  case 0x036: 
  case 0x038: case 0x03A:  case 0x03C: case 0x03E: 
  case 0x040: case 0x042: case 0x044: 
  case 0x046: 
  case 0x048: case 0x04A: 
  case 0x04C: case 0x050: 
  case 0x052:
  case 0x054: 
  case 0x000+1: case 0x002+1: case 0x004+1: case 0x008+1: case 0x00A+1:
  case 0x00C+1: case 0x00E+1: case 0x010+1:
  case 0x012+1: case 0x014+1: 
  case 0x016+1: case 0x018+1:
  case 0x01A+1: case 0x01C+1: 
  case 0x01E+1: case 0x020+1: case 0x022+1:
  case 0x024+1: 
  case 0x026+1: case 0x028+1: 
  case 0x02A+1: 
  case 0x02C+1: 
  case 0x02E+1: 
  case 0x030+1: 
  case 0x032+1: case 0x034+1: 
  case 0x036+1: 
  case 0x038+1: case 0x03A+1:  case 0x03C+1: case 0x03E+1: 
  case 0x040+1: case 0x042+1: case 0x044+1: 
  case 0x046+1: 
  case 0x048+1: case 0x04A+1: 
  case 0x04C+1: case 0x050+1: 
  case 0x052+1:
  case 0x054+1: gpu_write (& agb->gpu, addr, value, IO_WRITE_8); break;

  case 0x060:
  case 0x062:
  case 0x064:
  case 0x068:
  case 0x06C:
  case 0x070:
  case 0x072:
  case 0x074:
  case 0x078:
  case 0x07C: 
  case 0x080: 
  case 0x082: 
  case 0x084: 
  case 0x088: 
  case 0x090: case 0x0092: case 0x094: case 0x096: 
  case 0x098: case 0x009A: case 0x09C: case 0x0A0: case 0x0A2: case 0xA4: case 0xA6:
  case 0x09E: 
  case 0x060+1:
  case 0x062+1:
  case 0x064+1:
  case 0x068+1:
  case 0x06C+1:
  case 0x070+1:
  case 0x072+1:
  case 0x074+1:
  case 0x078+1:
  case 0x07C+1: 
  case 0x080+1: 
  case 0x082+1: 
  case 0x084+1: 
  case 0x088+1: 
  case 0x090+1: case 0x0092+1: case 0x094+1: case 0x096+1: 
  case 0x098+1: case 0x009A+1: case 0x09C+1: case 0x0A0+1: case 0x0A2+1: case 0xA4+1: case 0xA6+1:
  case 0x09E+1:  apu_write_b (& agb->apu, addr, value); break;

  case DMA0_CONTROL+0: dma_writeIO (agb, 0, value      | agb->dma.component[0].control.blk & 0xFF00); break;
  case DMA1_CONTROL+0: dma_writeIO (agb, 1, value      | agb->dma.component[1].control.blk & 0xFF00); break;
  case DMA2_CONTROL+0: dma_writeIO (agb, 2, value      | agb->dma.component[2].control.blk & 0xFF00); break;
  case DMA3_CONTROL+0: dma_writeIO (agb, 3, value      | agb->dma.component[3].control.blk & 0xFF00); break;
  case DMA0_CONTROL+1: dma_writeIO (agb, 0, value << 8 | agb->dma.component[0].control.blk & 0x00FF); break;
  case DMA1_CONTROL+1: dma_writeIO (agb, 1, value << 8 | agb->dma.component[1].control.blk & 0x00FF); break;
  case DMA2_CONTROL+1: dma_writeIO (agb, 2, value << 8 | agb->dma.component[2].control.blk & 0x00FF); break;
  case DMA3_CONTROL+1: dma_writeIO (agb, 3, value << 8 | agb->dma.component[3].control.blk & 0x00FF); break;

  case DMA0_COUNTER+0: agb->dma.component[0].counter.blk8[0] = value; break;
  case DMA1_COUNTER+0: agb->dma.component[1].counter.blk8[0] = value; break;
  case DMA2_COUNTER+0: agb->dma.component[2].counter.blk8[0] = value; break;
  case DMA3_COUNTER+0: agb->dma.component[3].counter.blk8[0] = value; break;
  case DMA0_COUNTER+1: agb->dma.component[0].counter.blk8[1] = value; break;
  case DMA1_COUNTER+1: agb->dma.component[1].counter.blk8[1] = value; break;
  case DMA2_COUNTER+1: agb->dma.component[2].counter.blk8[1] = value; break;
  case DMA3_COUNTER+1: agb->dma.component[3].counter.blk8[1] = value; break;

  case DMA0SRC_LO+0: agb->dma.component[0].source.blk8[0] = value; break;
  case DMA0SRC_LO+1: agb->dma.component[0].source.blk8[1] = value; break;
  case DMA0SRC_HI+0: agb->dma.component[0].source.blk8[2] = value; break;
  case DMA0SRC_HI+1: agb->dma.component[0].source.blk8[3] = value; break;

  case DMA1SRC_LO+0: agb->dma.component[1].source.blk8[0] = value; break;
  case DMA1SRC_LO+1: agb->dma.component[1].source.blk8[1] = value; break;
  case DMA1SRC_HI+0: agb->dma.component[1].source.blk8[2] = value; break;
  case DMA1SRC_HI+1: agb->dma.component[1].source.blk8[3] = value; break;
  
  case DMA2SRC_LO+0: agb->dma.component[2].source.blk8[0] = value; break;
  case DMA2SRC_LO+1: agb->dma.component[2].source.blk8[1] = value; break;
  case DMA2SRC_HI+0: agb->dma.component[2].source.blk8[2] = value; break;
  case DMA2SRC_HI+1: agb->dma.component[2].source.blk8[3] = value; break;
  
  case DMA3SRC_LO+0: agb->dma.component[3].source.blk8[0] = value; break;
  case DMA3SRC_LO+1: agb->dma.component[3].source.blk8[1] = value; break;
  case DMA3SRC_HI+0: agb->dma.component[3].source.blk8[2] = value; break;
  case DMA3SRC_HI+1: agb->dma.component[3].source.blk8[3] = value; break;
 
  case DMA0DST_LO+0: agb->dma.component[0].destin.blk8[0] = value; break;
  case DMA0DST_LO+1: agb->dma.component[0].destin.blk8[1] = value; break;
  case DMA0DST_HI+0: agb->dma.component[0].destin.blk8[2] = value; break;
  case DMA0DST_HI+1: agb->dma.component[0].destin.blk8[3] = value; break;

  case DMA1DST_LO+0: agb->dma.component[1].destin.blk8[0] = value; break;
  case DMA1DST_LO+1: agb->dma.component[1].destin.blk8[1] = value; break;
  case DMA1DST_HI+0: agb->dma.component[1].destin.blk8[2] = value; break;
  case DMA1DST_HI+1: agb->dma.component[1].destin.blk8[3] = value; break;
  
  case DMA2DST_LO+0: agb->dma.component[2].destin.blk8[0] = value; break;
  case DMA2DST_LO+1: agb->dma.component[2].destin.blk8[1] = value; break;
  case DMA2DST_HI+0: agb->dma.component[2].destin.blk8[2] = value; break;
  case DMA2DST_HI+1: agb->dma.component[2].destin.blk8[3] = value; break;
  
  case DMA3DST_LO+0: agb->dma.component[3].destin.blk8[0] = value; break;
  case DMA3DST_LO+1: agb->dma.component[3].destin.blk8[1] = value; break;
  case DMA3DST_HI+0: agb->dma.component[3].destin.blk8[2] = value; break;
  case DMA3DST_HI+1: agb->dma.component[3].destin.blk8[3] = value; break;

  case TIMER0_RELOAD_COUNTER+0: agb->timer.component[0].reload.blk8[0] = value; break;
  case TIMER1_RELOAD_COUNTER+0: agb->timer.component[1].reload.blk8[0] = value; break;
  case TIMER2_RELOAD_COUNTER+0: agb->timer.component[2].reload.blk8[0] = value; break;
  case TIMER3_RELOAD_COUNTER+0: agb->timer.component[3].reload.blk8[0] = value; break;
  case TIMER0_RELOAD_COUNTER+1: agb->timer.component[0].reload.blk8[1] = value; break;
  case TIMER1_RELOAD_COUNTER+1: agb->timer.component[1].reload.blk8[1] = value; break;
  case TIMER2_RELOAD_COUNTER+1: agb->timer.component[2].reload.blk8[1] = value; break;
  case TIMER3_RELOAD_COUNTER+1: agb->timer.component[3].reload.blk8[1] = value; break;
  
  case TIMER0_CONTROL+0: timer_write_ctl (& agb->timer, 0, value      | agb->timer.component[0].control.blk & 0xFF00); break;
  case TIMER1_CONTROL+0: timer_write_ctl (& agb->timer, 1, value      | agb->timer.component[1].control.blk & 0xFF00); break;
  case TIMER2_CONTROL+0: timer_write_ctl (& agb->timer, 2, value      | agb->timer.component[2].control.blk & 0xFF00); break;
  case TIMER3_CONTROL+0: timer_write_ctl (& agb->timer, 3, value      | agb->timer.component[3].control.blk & 0xFF00); break;
  case TIMER0_CONTROL+1: timer_write_ctl (& agb->timer, 0, value << 8 | agb->timer.component[0].control.blk & 0x00FF); break;
  case TIMER1_CONTROL+1: timer_write_ctl (& agb->timer, 1, value << 8 | agb->timer.component[1].control.blk & 0x00FF); break;
  case TIMER2_CONTROL+1: timer_write_ctl (& agb->timer, 2, value << 8 | agb->timer.component[2].control.blk & 0x00FF); break;
  case TIMER3_CONTROL+1: timer_write_ctl (& agb->timer, 3, value << 8 | agb->timer.component[3].control.blk & 0x00FF); break;

  case 0x120: case 0x124: case 0x128:
  case 0x122: case 0x126: 
  case 0x12A: sio_write (& agb->sio, addr, value); break;

  case 0x132: 
  case 0x133: controller_write_byte (& agb->joypad, addr, value); break;

  case 0x200: agb->arm7.ie.blk8[0] = value; reset_int_magic (agb); break;
  case 0x201: agb->arm7.ie.blk8[1] = value & 0x3F; reset_int_magic (agb); break;
  case 0x202: agb->arm7.ifs.blk8[0]&= ~value; reset_int_magic (agb); break;
  case 0x203: agb->arm7.ifs.blk8[1]&= ~(value & 0x3F | ~0x3F); reset_int_magic (agb); break;
  case 0x208: agb->arm7.ime.blk8[0] = value & 1; reset_int_magic (agb); break;
  case 0x209: agb->arm7.ime.blk8[1] = 0; break;
  case 0x20A: agb->arm7.ime.blk8[2] = 0; break;
  case 0x20B: agb->arm7.ime.blk8[3] = 0; break;

  case 0x300: agb->sundry.post.blk = value;  break;
  case 0x301: agb->sundry.halt.blk = 0x80000000; agb->sundry.halt.blk8[0] = value; break;
  case 0x204: 
  case 0x205: 
  case 0x206: 
  case 0x207:  
    agb->mem.wait.blk8[(addr & IOMAP2_MASK) - 0x204] = value; 
    gba_reset_waitstate (agb, agb->mem.wait.blk16[0]);
    break;
  case 0x410:
  case 0x411:
  case 0x412:
  case 0x413:
    agb->sundry.undoc_410.blk8[(addr & IOMAP2_MASK) - 0x410] = value;
    break;
  default:
    break;
  }
  fto_uint8pv (agb->mem.IOMap[addr & IOMAP_MASK]) = value;
}

dinline
uint16_t agb_io_rhw (struct gba *const agb, uint32_t addr) {

  switch (addr = addr & IOMAP2_MASK & HALFWORD_MASK) {
  case 0x000: case 0x002: case 0x004: case 0x006: case 0x008: case 0x00A:
  case 0x00C: case 0x00E: 
  case 0x048: case 0x04A: 
  case 0x050: 
  case 0x052: 
    return gpu_read (& agb->gpu, addr);
  case 0x010:
  case 0x012:
  case 0x014:
  case 0x016:
  case 0x018:
  case 0x01A:
  case 0x01C:
  case 0x01E:
  case 0x020:
  case 0x022:
  case 0x024:
  case 0x026:
  case 0x028:
  case 0x02A:
  case 0x02C: 
  case 0x02E: 
  case 0x030:
  case 0x032:
  case 0x034:
  case 0x036:
  case 0x038:
  case 0x03A:
  case 0x03C:
  case 0x03E:
  case 0x040:
  case 0x042:
  case 0x044:
  case 0x046:
  case 0x04C:
  case 0x04E:
  case 0x056:
  case 0x058:
  case 0x05A:
  case 0x05C:
  case 0x05E:
    /* bound load. stuff,  feature from mGBA */
    return agb->arm7.opcode[1];

  case 0x060:
  case 0x062:
  case 0x064:
  case 0x068:
  case 0x06C:
  case 0x070:
  case 0x072:
  case 0x074:
  case 0x078:
  case 0x07C: 
  case 0x080: 
  case 0x082: 
  case 0x084: 
  case 0x088: 
  case 0x090: case 0x0092: case 0x094: case 0x096: 
  case 0x098: case 0x009A: case 0x09C: 
  case 0x09E: 
    return apu_read (& agb->apu, addr); 

  case DMA0_CONTROL: return agb->dma.component[0].control.blk;
  case DMA1_CONTROL: return agb->dma.component[1].control.blk;
  case DMA2_CONTROL: return agb->dma.component[2].control.blk;
  case DMA3_CONTROL: return agb->dma.component[3].control.blk;
  case DMA0_COUNTER: return 0;
  case DMA1_COUNTER: return 0;
  case DMA2_COUNTER: return 0;
  case DMA3_COUNTER: return 0;

  case TIMER0_RELOAD_COUNTER: return timer_adjust_counter (& agb->timer, 0);
  case TIMER1_RELOAD_COUNTER: return timer_adjust_counter (& agb->timer, 1);
  case TIMER2_RELOAD_COUNTER: return timer_adjust_counter (& agb->timer, 2);
  case TIMER3_RELOAD_COUNTER: return timer_adjust_counter (& agb->timer, 3);
  case TIMER0_CONTROL: return agb->timer.component[0].control.blk;
  case TIMER1_CONTROL: return agb->timer.component[1].control.blk;
  case TIMER2_CONTROL: return agb->timer.component[2].control.blk;
  case TIMER3_CONTROL: return agb->timer.component[3].control.blk;

  case 0x120: case 0x124: case 0x128:
  case 0x122: case 0x126: 
  case 0x12A: return sio_read (& agb->sio, addr);

  case 0x130: 
  case 0x132: return controller_read (& agb->joypad, addr);

  case 0x200: return agb->arm7.ie.blk & 0x3FFF;
  case 0x202: return agb->arm7.ifs.blk & 0x3FFF;
  case 0x204: return agb->mem.wait.blk16[0];
  case 0x206: return agb->mem.wait.blk16[1];

  case 0x208: return agb->arm7.ime.blk16[0] & 1;
  case 0x20A: return 0;
  case 0x300: return agb->sundry.halt.blk16[0] << 8 | agb->sundry.post.blk;
  case 0x410: return agb->sundry.undoc_410.blk16[0];
  case 0x412: return agb->sundry.undoc_410.blk16[1];

  case 0x000800: case 0x010800: case 0x020800: case 0x030800:
  case 0x040800: case 0x050800: case 0x060800: case 0x070800:
  case 0x080800: case 0x090800: case 0x0A0800: case 0x0B0800:
  case 0x0C0800: case 0x0D0800: case 0x0E0800:
  case 0x0F0800:
  case 0x100800: case 0x110800: case 0x120800: case 0x130800:
  case 0x140800: case 0x150800: case 0x160800: case 0x170800:
  case 0x180800: case 0x190800: case 0x1A0800: case 0x1B0800:
  case 0x1C0800: case 0x1D0800: case 0x1E0800:
  case 0x1F0800:
  case 0x200800: case 0x210800: case 0x220800: case 0x230800:
  case 0x240800: case 0x250800: case 0x260800: case 0x270800:
  case 0x280800: case 0x290800: case 0x2A0800: case 0x2B0800:
  case 0x2C0800: case 0x2D0800: case 0x2E0800:
  case 0x2F0800:
  case 0x300800: case 0x310800: case 0x320800: case 0x330800:
  case 0x340800: case 0x350800: case 0x360800: case 0x370800:
  case 0x380800: case 0x390800: case 0x3A0800: case 0x3B0800:
  case 0x3C0800: case 0x3D0800: case 0x3E0800:
  case 0x3F0800:
  case 0x400800: case 0x410800: case 0x420800: case 0x430800:
  case 0x440800: case 0x450800: case 0x460800: case 0x470800:
  case 0x480800: case 0x490800: case 0x4A0800: case 0x4B0800:
  case 0x4C0800: case 0x4D0800: case 0x4E0800:
  case 0x4F0800:
  case 0x500800: case 0x510800: case 0x520800: case 0x530800:
  case 0x540800: case 0x550800: case 0x560800: case 0x570800:
  case 0x580800: case 0x590800: case 0x5A0800: case 0x5B0800:
  case 0x5C0800: case 0x5D0800: case 0x5E0800:
  case 0x5F0800:
  case 0x600800: case 0x610800: case 0x620800: case 0x630800:
  case 0x640800: case 0x650800: case 0x660800: case 0x670800:
  case 0x680800: case 0x690800: case 0x6A0800: case 0x6B0800:
  case 0x6C0800: case 0x6D0800: case 0x6E0800:
  case 0x6F0800:
  case 0x700800: case 0x710800: case 0x720800: case 0x730800:
  case 0x740800: case 0x750800: case 0x760800: case 0x770800:
  case 0x780800: case 0x790800: case 0x7A0800: case 0x7B0800:
  case 0x7C0800: case 0x7D0800: case 0x7E0800:
  case 0x7F0800:
  case 0x800800: case 0x810800: case 0x820800: case 0x830800:
  case 0x840800: case 0x850800: case 0x860800: case 0x870800:
  case 0x880800: case 0x890800: case 0x8A0800: case 0x8B0800:
  case 0x8C0800: case 0x8D0800: case 0x8E0800:
  case 0x8F0800:
  case 0x900800: case 0x910800: case 0x920800: case 0x930800:
  case 0x940800: case 0x950800: case 0x960800: case 0x970800:
  case 0x980800: case 0x990800: case 0x9A0800: case 0x9B0800:
  case 0x9C0800: case 0x9D0800: case 0x9E0800:
  case 0x9F0800:
  case 0xA00800: case 0xA10800: case 0xA20800: case 0xA30800:
  case 0xA40800: case 0xA50800: case 0xA60800: case 0xA70800:
  case 0xA80800: case 0xA90800: case 0xAA0800: case 0xAB0800:
  case 0xAC0800: case 0xAD0800: case 0xAE0800:
  case 0xAF0800:
  case 0xB00800: case 0xB10800: case 0xB20800: case 0xB30800:
  case 0xB40800: case 0xB50800: case 0xB60800: case 0xB70800:
  case 0xB80800: case 0xB90800: case 0xBA0800: case 0xBB0800:
  case 0xBC0800: case 0xBD0800: case 0xBE0800:
  case 0xBF0800:
  case 0xC00800: case 0xC10800: case 0xC20800: case 0xC30800:
  case 0xC40800: case 0xC50800: case 0xC60800: case 0xC70800:
  case 0xC80800: case 0xC90800: case 0xCA0800: case 0xCB0800:
  case 0xCC0800: case 0xCD0800: case 0xCE0800:
  case 0xCF0800:
  case 0xD00800: case 0xD10800: case 0xD20800: case 0xD30800:
  case 0xD40800: case 0xD50800: case 0xD60800: case 0xD70800:
  case 0xD80800: case 0xD90800: case 0xDA0800: case 0xDB0800:
  case 0xDC0800: case 0xDD0800: case 0xDE0800:
  case 0xDF0800:
  case 0xE00800: case 0xE10800: case 0xE20800: case 0xE30800:
  case 0xE40800: case 0xE50800: case 0xE60800: case 0xE70800:
  case 0xE80800: case 0xE90800: case 0xEA0800: case 0xEB0800:
  case 0xEC0800: case 0xED0800: case 0xEE0800:
  case 0xEF0800:
  case 0xF00800: case 0xF10800: case 0xF20800: case 0xF30800:
  case 0xF40800: case 0xF50800: case 0xF60800: case 0xF70800:
  case 0xF80800: case 0xF90800: case 0xFA0800: case 0xFB0800:
  case 0xFC0800: case 0xFD0800: case 0xFE0800:
  case 0xFF0800:
    return agb->mem.imc.blk16[0];
  case 0x000802: case 0x010802: case 0x020802: case 0x030802:
  case 0x040802: case 0x050802: case 0x060802: case 0x070802:
  case 0x080802: case 0x090802: case 0x0A0802: case 0x0B0802:
  case 0x0C0802: case 0x0D0802: case 0x0E0802:
  case 0x0F0802:
  case 0x100802: case 0x110802: case 0x120802: case 0x130802:
  case 0x140802: case 0x150802: case 0x160802: case 0x170802:
  case 0x180802: case 0x190802: case 0x1A0802: case 0x1B0802:
  case 0x1C0802: case 0x1D0802: case 0x1E0802:
  case 0x1F0802:
  case 0x200802: case 0x210802: case 0x220802: case 0x230802:
  case 0x240802: case 0x250802: case 0x260802: case 0x270802:
  case 0x280802: case 0x290802: case 0x2A0802: case 0x2B0802:
  case 0x2C0802: case 0x2D0802: case 0x2E0802:
  case 0x2F0802:
  case 0x300802: case 0x310802: case 0x320802: case 0x330802:
  case 0x340802: case 0x350802: case 0x360802: case 0x370802:
  case 0x380802: case 0x390802: case 0x3A0802: case 0x3B0802:
  case 0x3C0802: case 0x3D0802: case 0x3E0802:
  case 0x3F0802:
  case 0x400802: case 0x410802: case 0x420802: case 0x430802:
  case 0x440802: case 0x450802: case 0x460802: case 0x470802:
  case 0x480802: case 0x490802: case 0x4A0802: case 0x4B0802:
  case 0x4C0802: case 0x4D0802: case 0x4E0802:
  case 0x4F0802:
  case 0x500802: case 0x510802: case 0x520802: case 0x530802:
  case 0x540802: case 0x550802: case 0x560802: case 0x570802:
  case 0x580802: case 0x590802: case 0x5A0802: case 0x5B0802:
  case 0x5C0802: case 0x5D0802: case 0x5E0802:
  case 0x5F0802:
  case 0x600802: case 0x610802: case 0x620802: case 0x630802:
  case 0x640802: case 0x650802: case 0x660802: case 0x670802:
  case 0x680802: case 0x690802: case 0x6A0802: case 0x6B0802:
  case 0x6C0802: case 0x6D0802: case 0x6E0802:
  case 0x6F0802:
  case 0x700802: case 0x710802: case 0x720802: case 0x730802:
  case 0x740802: case 0x750802: case 0x760802: case 0x770802:
  case 0x780802: case 0x790802: case 0x7A0802: case 0x7B0802:
  case 0x7C0802: case 0x7D0802: case 0x7E0802:
  case 0x7F0802:
  case 0x800802: case 0x810802: case 0x820802: case 0x830802:
  case 0x840802: case 0x850802: case 0x860802: case 0x870802:
  case 0x880802: case 0x890802: case 0x8A0802: case 0x8B0802:
  case 0x8C0802: case 0x8D0802: case 0x8E0802:
  case 0x8F0802:
  case 0x900802: case 0x910802: case 0x920802: case 0x930802:
  case 0x940802: case 0x950802: case 0x960802: case 0x970802:
  case 0x980802: case 0x990802: case 0x9A0802: case 0x9B0802:
  case 0x9C0802: case 0x9D0802: case 0x9E0802:
  case 0x9F0802:
  case 0xA00802: case 0xA10802: case 0xA20802: case 0xA30802:
  case 0xA40802: case 0xA50802: case 0xA60802: case 0xA70802:
  case 0xA80802: case 0xA90802: case 0xAA0802: case 0xAB0802:
  case 0xAC0802: case 0xAD0802: case 0xAE0802:
  case 0xAF0802:
  case 0xB00802: case 0xB10802: case 0xB20802: case 0xB30802:
  case 0xB40802: case 0xB50802: case 0xB60802: case 0xB70802:
  case 0xB80802: case 0xB90802: case 0xBA0802: case 0xBB0802:
  case 0xBC0802: case 0xBD0802: case 0xBE0802:
  case 0xBF0802:
  case 0xC00802: case 0xC10802: case 0xC20802: case 0xC30802:
  case 0xC40802: case 0xC50802: case 0xC60802: case 0xC70802:
  case 0xC80802: case 0xC90802: case 0xCA0802: case 0xCB0802:
  case 0xCC0802: case 0xCD0802: case 0xCE0802:
  case 0xCF0802:
  case 0xD00802: case 0xD10802: case 0xD20802: case 0xD30802:
  case 0xD40802: case 0xD50802: case 0xD60802: case 0xD70802:
  case 0xD80802: case 0xD90802: case 0xDA0802: case 0xDB0802:
  case 0xDC0802: case 0xDD0802: case 0xDE0802:
  case 0xDF0802:
  case 0xE00802: case 0xE10802: case 0xE20802: case 0xE30802:
  case 0xE40802: case 0xE50802: case 0xE60802: case 0xE70802:
  case 0xE80802: case 0xE90802: case 0xEA0802: case 0xEB0802:
  case 0xEC0802: case 0xED0802: case 0xEE0802:
  case 0xEF0802:
  case 0xF00802: case 0xF10802: case 0xF20802: case 0xF30802:
  case 0xF40802: case 0xF50802: case 0xF60802: case 0xF70802:
  case 0xF80802: case 0xF90802: case 0xFA0802: case 0xFB0802:
  case 0xFC0802: case 0xFD0802: case 0xFE0802:
  case 0xFF0802:
    return agb->mem.imc.blk16[1];
  default:
    DEBUG_OUT ("unknow io read halfword !addr:%08x\n", addr); 
    return fto_uint16pv (agb->mem.IOMap[addr & IOMAP_MASK]);
  }
}

dinline
void agb_io_whw (struct gba *const agb, uint32_t addr,  uint16_t value) {

  switch (addr & IOMAP2_MASK & 0xFFFFFFE) {
  case 0x000: case 0x002: case 0x004: case 0x008: case 0x00A:
  case 0x00C: case 0x00E: case 0x010:
  case 0x012: case 0x014: 
  case 0x016: case 0x018:
  case 0x01A: case 0x01C: 
  case 0x01E: case 0x020: case 0x022:
  case 0x024: 
  case 0x026: case 0x028: 
  case 0x02A: 
  case 0x02C: 
  case 0x02E: 
  case 0x030: 
  case 0x032: case 0x034: 
  case 0x036: 
  case 0x038: case 0x03A:  case 0x03C: case 0x03E: 
  case 0x040: case 0x042: case 0x044: 
  case 0x046: 
  case 0x048: case 0x04A: 
  case 0x04C: case 0x050: 
  case 0x052:
  case 0x054: gpu_write (& agb->gpu, addr, value, IO_WRITE_16); break;

  case 0x060:
  case 0x062:
  case 0x064:
  case 0x068:
  case 0x06C:
  case 0x070:
  case 0x072:
  case 0x074:
  case 0x078:
  case 0x07C: 
  case 0x080: 
  case 0x082: 
  case 0x084: 
  case 0x088: 
  case 0x090: case 0x0092: case 0x094: case 0x096: 
  case 0x098: case 0x009A: case 0x09C: case 0x0A0: case 0x0A2: case 0xA4: case 0xA6:
  case 0x09E: apu_write (& agb->apu, addr, value); break;

  case DMA0_CONTROL: dma_writeIO (agb, 0, value); break;
  case DMA1_CONTROL: dma_writeIO (agb, 1, value); break;
  case DMA2_CONTROL: dma_writeIO (agb, 2, value); break;
  case DMA3_CONTROL: dma_writeIO (agb, 3, value); break;

  case DMA0_COUNTER: agb->dma.component[0].counter.blk = value; break;
  case DMA1_COUNTER: agb->dma.component[1].counter.blk = value; break;
  case DMA2_COUNTER: agb->dma.component[2].counter.blk = value; break;
  case DMA3_COUNTER: agb->dma.component[3].counter.blk = value; break;

  case DMA0SRC_LO: agb->dma.component[0].source.blk16[0] = value; break;
  case DMA1SRC_LO: agb->dma.component[1].source.blk16[0] = value; break;
  case DMA2SRC_LO: agb->dma.component[2].source.blk16[0] = value; break;
  case DMA3SRC_LO: agb->dma.component[3].source.blk16[0] = value; break;
  
  case DMA0SRC_HI: agb->dma.component[0].source.blk16[1] = value; break;
  case DMA1SRC_HI: agb->dma.component[1].source.blk16[1] = value; break;
  case DMA2SRC_HI: agb->dma.component[2].source.blk16[1] = value; break;
  case DMA3SRC_HI: agb->dma.component[3].source.blk16[1] = value; break;

  case DMA0DST_LO: agb->dma.component[0].destin.blk16[0] = value; break;
  case DMA1DST_LO: agb->dma.component[1].destin.blk16[0] = value; break;
  case DMA2DST_LO: agb->dma.component[2].destin.blk16[0] = value; break;
  case DMA3DST_LO: agb->dma.component[3].destin.blk16[0] = value; break;
  
  case DMA0DST_HI: agb->dma.component[0].destin.blk16[1] = value; break;
  case DMA1DST_HI: agb->dma.component[1].destin.blk16[1] = value; break;
  case DMA2DST_HI: agb->dma.component[2].destin.blk16[1] = value; break;
  case DMA3DST_HI: agb->dma.component[3].destin.blk16[1] = value; break;
  
  case TIMER0_RELOAD_COUNTER: agb->timer.component[0].reload.blk = value; break;
  case TIMER1_RELOAD_COUNTER: agb->timer.component[1].reload.blk = value; break;
  case TIMER2_RELOAD_COUNTER: agb->timer.component[2].reload.blk = value; break;
  case TIMER3_RELOAD_COUNTER: agb->timer.component[3].reload.blk = value; break;

  case TIMER0_CONTROL: timer_write_ctl (& agb->timer, 0, value); break;
  case TIMER1_CONTROL: timer_write_ctl (& agb->timer, 1, value); break;
  case TIMER2_CONTROL: timer_write_ctl (& agb->timer, 2, value); break;
  case TIMER3_CONTROL: timer_write_ctl (& agb->timer, 3, value); break;

  case 0x120: case 0x124: case 0x128:
  case 0x122: case 0x126: 
  case 0x12A: sio_write (& agb->sio, addr, value); break;

  case 0x132: controller_write (& agb->joypad, addr, value); break;

  case 0x200: agb->arm7.ie.blk = value & 0x3FFF; reset_int_magic (agb); break;
  case 0x202: agb->arm7.ifs .blk&= ~(value & 0x3FFF); reset_int_magic (agb); break;
  case 0x208: agb->arm7.ime.blk16[0] = value & 1; reset_int_magic (agb); break;
  case 0x20A: agb->arm7.ime.blk16[1] = 0; break;
  case 0x300: 
    agb->sundry.post.blk = (uint8_t) value; 
    break;
  case 0x204: 
    agb->mem.wait.blk16[0] = value; 
    gba_reset_waitstate (agb, value);
  default:
    break;
  }
  fto_uint16pv (agb->mem.IOMap[addr & IOMAP_MASK]) = value;
}

dinline int32_t 
agb_mbus_rb (struct gba *const agb, uint32_t addr_n, uint8_t *const memory, const kable seq) {
  
  const uint32_t addr = addr_n & BYTE_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb_trace_breakpoint_read (agb, addr, 1);

  switch (bank)  {
  case 0:
    if (addr < 0x4000) {
      const uint8_t bios_pro[4] = { 0x00,  0xF0, 0x29, 0xE1 };
      
      if ((agb->arm7.regs[15].blk & 0x0F000000) == 0)
        * memory = agb->mem.bios[addr];
      else 
        * memory = bios_pro[addr & 3];
    } else {
  case 1:
      * memory = 0;
    }
    return 0;
  case 2:
    * memory = agb->mem.WRam[addr & WRAM_MASK];
    return agb->mem.wram_wait;
  case 4: 
    if (addr_n & 1)
      * memory =  (uint8_t) (agb_io_rhw (agb, addr_n) >> 8);
    else 
      * memory =  (uint8_t) (agb_io_rhw (agb, addr_n) & 0xFF);
    return 0;
  case 3: 
    * memory =  agb->mem.IRam[addr & IRAM_MASK];
    return 0;
  case 5: 
    * memory = agb->gpu.palette[addr & PAL_MASK];
    return 0;
  case 6:
    if (addr_n & 0x10000) 
      * memory =  agb->gpu.vram[addr & VRAM_MASK_Vx17FFF_x20000];
    else 
      * memory =  agb->gpu.vram[addr & VRAM_MASK];
    return 0;
  case 7: 
    * memory = agb->gpu.oam[addr & OAM_MASK];
    return 0;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
    * memory = agb->mem.PRom[addr & GAME_PAK_MASK];
    return seq ? agb->mem.pwait_seq[bank-8 >> 1] : agb->mem.pwait_noseq[bank-8 >> 1];
  case 14:
  case 15:
    if (addr >= 0xE000000 
      && addr <= 0xE00FFFF
      && agb->mem.flash_ptr != null)
      return flash_read (agb->mem.flash_ptr, addr & 0xFFFF, memory);
    else {
      * memory = agb->mem.SRam[addr & SRAM_MASK];
      return agb->mem.sram_wait;
    }
  default:
    assert (0);
    break;
  }
  return 0;
}

dinline int32_t 
agb_mbus_rhw (struct gba *const agb, uint32_t addr_n, uint16_t *const memory, const kable seq) {
  
  const uint32_t addr = addr_n & HALFWORD_MASK;
  const uint32_t bank = addr >> 24 & 15;
  
  agb_trace_breakpoint_read (agb, addr, 2);

  switch (bank)  {
  case 0:
    if (addr < 0x4000) {
      const uint8_t bios_pro[4] = { 0x00,  0xF0, 0x29, 0xE1 };
      
      if ((agb->arm7.regs[15].blk & 0x0F000000) == 0)
        * memory = * (uint16_t *) & agb->mem.bios[addr];
      else 
        * memory = * (uint16_t *) & bios_pro[addr & 2];
    } else {
  case 1:
      * memory = 0;
    }
    return 0;
  case 2:
    * memory = * (uint16_t *) & agb->mem.WRam[addr & WRAM_MASK];
    return agb->mem.wram_wait;
  case 4: 
    * memory =  agb_io_rhw (agb, addr_n);
    return 0;
  case 3: 
    * memory = * (uint16_t *) & agb->mem.IRam[addr  & IRAM_MASK];
    return 0;
  case 5: 
    * memory = * (uint16_t *) & agb->gpu.palette[addr & PAL_MASK];
    return 0;
  case 6:
    if (addr & 0x10000) 
      * memory =  * (uint16_t *) & agb->gpu.vram[addr & VRAM_MASK_Vx17FFF_x20000];
    else 
      * memory =  * (uint16_t *) & agb->gpu.vram[addr & VRAM_MASK];
    return 0;
  case 7: 
    * memory = * (uint16_t *) & agb->gpu.oam[addr & OAM_MASK];
    return 0;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
    * memory = * (uint16_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    return seq ? agb->mem.pwait_seq[bank-8 >> 1] : agb->mem.pwait_noseq[bank-8 >> 1];
  case 13:
    if (agb->mem.eeprom_ptr != null)
      * memory =  eeprom_read (agb->mem.eeprom_ptr, addr & GAME_PAK_MASK);
    else 
      * memory = * (uint16_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    return seq ? agb->mem.pwait_seq[2] : agb->mem.pwait_noseq[2];
  case 14:
  case 15:
    {
      const uint16_t bsrc = agb->mem.SRam[addr_n & SRAM_MASK];
    * memory = bsrc | bsrc << 8;
    }
    return (agb->mem.sram_wait << 1) + 1;
  default:
    assert (0);
    break;
  }
  return 0;
}

finline int32_t 
agb_mbus_rw (struct gba *const agb, uint32_t addr_n, uint32_t *const memory, const kable seq) {
  
  const uint32_t addr = addr_n & WORD_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb_trace_breakpoint_read (agb, addr, 4);

  switch (bank)  {
  case 0:
    if (addr < 0x4000) {
      const uint8_t bios_pro[4] = { 0x00,  0xF0, 0x29, 0xE1 };
      
      if ((agb->arm7.regs[15].blk & 0x0F000000) == 0)
        * memory = * (uint32_t *) & agb->mem.bios[addr];
      else 
        * memory = * (uint32_t *) & bios_pro[0];
    } else {
  case 1:
      * memory = 0;
    }
    return 0;
  case 2:
    * memory = * (uint32_t *) & agb->mem.WRam[addr & WRAM_MASK];
    return (agb->mem.wram_wait << 1) + 1;
  case 4: 
    {
      const uint32_t base = addr_n & 0xFFFFFFC;
      const uint32_t lo = agb_io_rhw (agb, base) & 0xFFFF;
      const uint32_t hi = agb_io_rhw (agb, base + 2) & 0xFFFF;
      * memory = lo | hi << 16;
    }
    return 0;
  case 3: 
    * memory =  * (uint32_t *) & agb->mem.IRam[addr  & IRAM_MASK];
    return 0;
  case 5: 
    * memory = * (uint32_t *) & agb->gpu.palette[addr & PAL_MASK];
    return 1;
  case 6:
    if (addr & 0x10000) 
      * memory =  * (uint32_t *) & agb->gpu.vram[addr & VRAM_MASK_Vx17FFF_x20000];
    else 
      * memory =  * (uint32_t *) & agb->gpu.vram[addr & VRAM_MASK];
    return 1;
  case 7: 
    * memory = * (uint32_t *) & agb->gpu.oam[addr & OAM_MASK];
    return 0;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
    * memory = * (uint32_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    {
      const uint32_t pw_bank = bank-8 >> 1;
      if (seq != false)
        return (agb->mem.pwait_seq[pw_bank] << 1) + 1;
      else 
        return agb->mem.pwait_seq[pw_bank] + agb->mem.pwait_noseq[pw_bank] + 1;
    }
  case 14:
  case 15:
    {
      const uint32_t bsrc = agb->mem.SRam[addr_n & SRAM_MASK];
    * memory = bsrc | bsrc << 8 | bsrc << 16 | bsrc << 24;
    }
    return (agb->mem.sram_wait << 2) + 1;
  default:
    assert (0);
    break;
  }
  return 0;
}

dinline int32_t 
agb_mbus_wb (struct gba *const agb, uint32_t addr_n, const uint8_t memory, const kable seq) {
  
  const uint32_t addr = addr_n & BYTE_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb_trace_breakpoint_write (agb, addr, 1);

  switch (bank)  {
  case 2:
    agb->mem.WRam[addr & WRAM_MASK] = memory;
    return agb->mem.wram_wait;
  case 4: 
    agb_io_wb (agb, addr_n, memory);
  case 0:
    return 0;
  case 3: 
    agb->mem.IRam[addr  & IRAM_MASK] = memory;
    return 0;
  case 5: 
    * (uint16_t *) & agb->gpu.palette[addr & PAL_MASK] = (uint16_t) memory | (uint16_t) memory << 8; 
    gpu_adjust_palette64 (& agb->gpu, addr);
    return 0;
  case 6:
    if (addr & 0x10000) 
      ;
    else 
      * (uint16_t *) & agb->gpu.vram[addr & VRAM_MASK] =  (uint16_t) memory | (uint16_t) memory << 8; 
    return 0;
  case 7: 
    return 0;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
    return seq ? agb->mem.pwait_seq[bank-8 >> 1] : agb->mem.pwait_noseq[bank-8 >> 1];
  case 14:
  case 15:
    if (addr >= 0xE000000 
      && addr <= 0xE00FFFF
      && agb->mem.flash_ptr != null)
      return flash_write (agb->mem.flash_ptr, addr & 0xFFFF, memory);
    else {
      agb->mem.SRam[addr & SRAM_MASK] = memory;
      return agb->mem.sram_wait;
    }
  default:
    assert (0);
    break;
  }
  return 0;
}

dinline int32_t 
agb_mbus_whw (struct gba *const agb, uint32_t addr_n, const uint16_t memory, const kable seq) {
  
  const uint32_t addr = addr_n & HALFWORD_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb_trace_breakpoint_write (agb, addr, 2);

  switch (bank)  {
    return 0;
  case 2:
    * (uint16_t *) & agb->mem.WRam[addr & WRAM_MASK] = memory;
    return agb->mem.wram_wait;
  case 4: 
    agb_io_whw (agb, addr_n, memory);
    return 0;
  case 3: 
    * (uint16_t *) & agb->mem.IRam[addr  & IRAM_MASK] = memory;
  case 0:
    return 0;
  case 5: 
    * (uint16_t *) & agb->gpu.palette[addr & PAL_MASK] = memory;
    gpu_adjust_palette64 (& agb->gpu, addr);
    return 0;
  case 6:
    if (addr & 0x10000) 
      * (uint16_t *) & agb->gpu.vram[addr & VRAM_MASK_Vx17FFF_x20000] =  memory; 
    else 
      * (uint16_t *) & agb->gpu.vram[addr & VRAM_MASK] =  memory; 
    return 0;
  case 7: 
    * (uint16_t *) & agb->gpu.oam[addr & OAM_MASK] = memory;
    return 0;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
    return seq ? agb->mem.pwait_seq[bank-8 >> 1] : agb->mem.pwait_noseq[bank-8 >> 1];
  case 13:
    if (agb->mem.eeprom_ptr != null)
      eeprom_write (agb->mem.eeprom_ptr, addr & GAME_PAK_MASK, memory);
    return seq ? agb->mem.pwait_seq[bank-8 >> 1] : agb->mem.pwait_noseq[bank-8 >> 1];
  case 14:
  case 15:
     * (uint16_t *) & agb->mem.SRam[addr & SRAM_MASK] = memory & 0xFF | (memory & 0xFF) << 8;
    return agb->mem.sram_wait;
  default:
    break;
  }
  return 0;
}

dinline int32_t 
agb_mbus_ww (struct gba *const agb, uint32_t addr_n, const uint32_t memory, const kable seq) {
  
  const uint32_t addr = addr_n & WORD_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb_trace_breakpoint_write (agb, addr, 4);

  switch (bank)  {
  case 2:
    * (uint32_t *) & agb->mem.WRam[addr & WRAM_MASK] = memory;
    return agb->mem.wram_wait * 2 + 1;
  case 4: 
    {
      const uint32_t base = addr_n & 0xFFFFFFC;
      agb_io_whw (agb, base, memory & 0xFFFF);
      agb_io_whw (agb, base + 2, memory >> 16 & 0xFFFF);
    }
    return 0;
  case 3: 
    * (uint32_t *) & agb->mem.IRam[addr  & IRAM_MASK] = memory;
  case 0:
    return 0;
  case 5: 
    * (uint32_t *) & agb->gpu.palette[addr & PAL_MASK] = memory;
    gpu_adjust_palette64 (& agb->gpu, addr);
    return 1;
  case 6:
    if (addr & 0x10000) 
      * (uint32_t *) & agb->gpu.vram[addr & VRAM_MASK_Vx17FFF_x20000] =  memory; 
    else 
      * (uint32_t *) & agb->gpu.vram[addr & VRAM_MASK] =  memory; 
    return 1;
  case 7: 
    * (uint32_t *) & agb->gpu.oam[addr & OAM_MASK] = memory;
    return 0;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
    {
      const uint32_t pw_bank = bank-8 >> 1;
      
      if (seq != false)
        return (agb->mem.pwait_seq[pw_bank] << 1) + 1;
      else 
        return agb->mem.pwait_seq[pw_bank] + agb->mem.pwait_noseq[pw_bank] + 1;
    }
  case 14:
  case 15:
    if (addr_n & 3)
      * (uint32_t *) & agb->mem.SRam[addr & SRAM_MASK]  = 0;
    else 
      * (uint32_t *) & agb->mem.SRam[addr & SRAM_MASK]  = memory;
    return agb->mem.sram_wait * 4 + 3;
  default:
    break;
  }
  return 0;
}

finline 
int32_t thumb_fecth_n (struct gba *const agb, const uint32_t addr_0, uint16_t *const accept) {

  const uint32_t addr = addr_0 & HALFWORD_MASK;
  const uint32_t bank = addr_0 >> 24 & 15;

  switch (bank)  {
  case 0: * accept = * (uint16_t *) & agb->mem.bios[addr]; return 0;
  case 3: * accept = * (uint16_t *) & agb->mem.IRam[addr & IRAM_MASK]; return 0;
  case 2: * accept = * (uint16_t *) & agb->mem.WRam[addr & WRAM_MASK]; return agb->mem.wram_wait;
  case 8: case 9: case 10: case 11: case 12: case 13:
    * accept = * (uint16_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    return agb->mem.pwait_noseq[bank-8 >> 1];
  default:
    FORCE_BREAK ();
    break;
  }
  return 0;
}

finline 
int32_t thumb_fecth_s (struct gba *const agb, const uint32_t addr_0, uint16_t *const accept) {

  const uint32_t addr = addr_0 & HALFWORD_MASK;
  const uint32_t bank = addr_0 >> 24 & 15;

  switch (bank)  {
  case 0: * accept = * (uint16_t *) & agb->mem.bios[addr]; return 0;
  case 3: * accept = * (uint16_t *) & agb->mem.IRam[addr & IRAM_MASK]; return 0;
  case 2: * accept = * (uint16_t *) & agb->mem.WRam[addr & WRAM_MASK]; return agb->mem.wram_wait;
  case 8: case 9: case 10: case 11: case 12: case 13:
    * accept = * (uint16_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    return agb->mem.pwait_seq[bank-8 >> 1];
  default:
    FORCE_BREAK ();
    break;
  }
  return 0;
}

finline 
int32_t arm7_fecth_n (struct gba *const agb, const uint32_t addr_0, uint32_t *const accept) {
  const uint32_t addr = addr_0 & WORD_MASK;
  const uint32_t bank = addr_0 >> 24 & 15;

  switch (bank)  {
  case 0: * accept = * (uint32_t *) & agb->mem.bios[addr]; return 0;
  case 3: * accept = * (uint32_t *) & agb->mem.IRam[addr & IRAM_MASK]; return 0;
  case 2: * accept = * (uint32_t *) & agb->mem.WRam[addr & WRAM_MASK]; return agb->mem.wram_wait * 2 + 1;
  case 8: case 9: case 10: case 11: case 12: case 13:
    * accept = * (uint32_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    return agb->mem.pwait_noseq[bank-8 >> 1] + agb->mem.pwait_seq[bank-8 >> 1] + 1;
  default:
    FORCE_BREAK ();
    break;
  }
  return 0;
}

finline 
int32_t arm7_fecth_s (struct gba *const agb, const uint32_t addr_0, uint32_t *const accept) {
  const uint32_t addr = addr_0 & WORD_MASK;
  const uint32_t bank = addr_0 >> 24 & 15;

  switch (bank)  {
  case 0: * accept = * (uint32_t *) & agb->mem.bios[addr]; return 0;
  case 3: * accept = * (uint32_t *) & agb->mem.IRam[addr & IRAM_MASK]; return 0;
  case 2: * accept = * (uint32_t *) & agb->mem.WRam[addr & WRAM_MASK]; return agb->mem.wram_wait * 2 + 1;
  case 8: case 9: case 10: case 11: case 12: case 13:
    * accept = * (uint32_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    return agb->mem.pwait_seq[bank-8 >> 1] + agb->mem.pwait_seq[bank-8 >> 1] + 1;
  default:
    FORCE_BREAK ();
    break;
  }
  return 0;
}

finline 
int32_t thumb_flush (struct gba *const agb) {

  const uint32_t addr = agb->arm7.regs[15].blk & HALFWORD_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb->arm7.regs[15].blk = (agb->arm7.regs[15].blk & -2) + 2;
  switch (bank)  {
  case 0:
    agb->arm7.opcode[0] = * (uint16_t *) & agb->mem.bios[addr];
    agb->arm7.opcode[1] = * (uint16_t *) & agb->mem.bios[addr+2];
    return 1;
  case 2:
    agb->arm7.opcode[0] = * (uint16_t *) & agb->mem.WRam[addr & WRAM_MASK];
    agb->arm7.opcode[1] = * (uint16_t *) & agb->mem.WRam[addr + 2 & WRAM_MASK];
    return agb->mem.wram_wait * 2 + 1;
  case 3: 
    agb->arm7.opcode[0] = * (uint16_t *) & agb->mem.IRam[addr & IRAM_MASK];
    agb->arm7.opcode[1] = * (uint16_t *) & agb->mem.IRam[addr + 2 & IRAM_MASK];
    return 1;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
    agb->arm7.opcode[0] = * (uint16_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    agb->arm7.opcode[1] = * (uint16_t *) & agb->mem.PRom[addr + 2 & GAME_PAK_MASK];
    {
      const uint32_t pw_bank = bank-8 >> 1;
      return agb->mem.pwait_seq[pw_bank] 
        + agb->mem.pwait_noseq[pw_bank]  + 1;
    }
  default:
    assert (0);
    break;
  }
  return 0;
}

finline 
int32_t arm7_flush (struct gba *const agb) {

  const uint32_t addr = agb->arm7.regs[15].blk & WORD_MASK;
  const uint32_t bank = addr >> 24 & 15;

  agb->arm7.regs[15].blk = (agb->arm7.regs[15].blk & -4) + 4;
  switch (bank)  {
  case 0:
    agb->arm7.opcode[0] = * (uint32_t *) & agb->mem.bios[addr];
    agb->arm7.opcode[1] = * (uint32_t *) & agb->mem.bios[addr+4];
    return 1;
  case 2:
    agb->arm7.opcode[0] = * (uint32_t *) & agb->mem.WRam[addr & WRAM_MASK];
    agb->arm7.opcode[1] = * (uint32_t *) & agb->mem.WRam[addr + 4 & WRAM_MASK];
    return agb->mem.wram_wait * 4 + 3;
  case 3: 
    agb->arm7.opcode[0] = * (uint32_t *) & agb->mem.IRam[addr & IRAM_MASK];
    agb->arm7.opcode[1] = * (uint32_t *) & agb->mem.IRam[addr + 4 & IRAM_MASK];
    return 1;
  case 8:
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
    agb->arm7.opcode[0] = * (uint32_t *) & agb->mem.PRom[addr & GAME_PAK_MASK];
    agb->arm7.opcode[1] = * (uint32_t *) & agb->mem.PRom[addr + 4 & GAME_PAK_MASK];
    {
      const uint32_t pw_bank = bank-8 >> 1;
      return (agb->mem.pwait_seq[pw_bank] << 1) + agb->mem.pwait_seq[pw_bank] 
        + agb->mem.pwait_noseq[pw_bank]  + 3;
    }
  default:
    assert (0);
    break;
  }
  return 0;
}

dinline int32_t
agb_stack_ww (struct gba *agb, uint32_t addr, uint32_t value, kable seq, uint32_t r_id) {
  uint32_t w_addr = addr & 0xFFFFFFF;
  agb->mem.novel_access = value;
  
  if (! (w_addr >= 0x3000000 && w_addr <= 0x3FFFFFF))
    return agb_mbus_ww (agb, addr& 0xFFFFFFF, value, seq);
  else {
    struct stack_trace_chunk *const stc = & agb->dbg.stack_trace[ (w_addr & 0x7FFF) >> 2];
    agb_trace_breakpoint_write (agb, addr, 4);

    stc->r_id = r_id;
    stc->status = 1;
    if (agb->arm7.regs[16].blk & 0x20000000) {
       stc->do_addr = (agb->arm7.regs[15].blk & -2) - 4;
       stc->thumb = true;
    } else {
       stc->do_addr = (agb->arm7.regs[15].blk & -4) - 8;
       stc->thumb = false;
    }
   * (uint32_t *)& agb->mem.IRam [w_addr & 0x7FFC] = value;
   return 0;
  }
}

dinline uint32_t 
agb_stack_rw (struct gba *agb, uint32_t addr, uint32_t *const memory,  kable seq) {
  uint32_t r_addr = addr & 0xFFFFFFF;
  if (! (r_addr >= 0x3000000 && r_addr <= 0x3FFFFFF))
    return  agb_mbus_rw (agb, r_addr, memory, seq);
  else {
    struct stack_trace_chunk *const stc = & agb->dbg.stack_trace[ (r_addr & 0x7FFF) >> 2];
    agb_trace_breakpoint_read (agb, addr, 4);

    stc->status = 0;
    if (agb->arm7.regs[16].blk & 0x20000000) {
       stc->do_addr = (agb->arm7.regs[15].blk & -2) - 4;
       stc->thumb = true;
    } else {
       stc->do_addr = (agb->arm7.regs[15].blk & -4) - 8;
       stc->thumb = false;
    }
    * memory = * (uint32_t *)& agb->mem.IRam [r_addr & 0x7FFC];
    return 0;
  }
}

dinline uint16_t 
gba_fast_rhw_ (struct gba *const agb, uint32_t addr) {
  
  uint32_t value;
  addr &= 0xFFFFFFF;
  switch (addr >> 24)  {
  case 0:
  case 1:
    return fto_uint16pv (agb->mem.bios[addr & BIOS_MASK]);
  case 2:
    return fto_uint16pv (agb->mem.WRam[addr & WRAM_MASK]);
  case 4:
    if (addr < 0x4000800) {
      switch (addr & 0x7FE) {
        /* GPU IO */
      case 0x000: return agb->gpu.ctl.blk;
      case 0x002: return agb->gpu.col_swap.blk;
      case 0x004: return agb->gpu.status.blk;
      case 0x006: return agb->gpu.line.blk;
      case 0x008: return agb->gpu.chan[0].ctl.blk;
      case 0x00A: return agb->gpu.chan[1].ctl.blk;
      case 0x00C: return agb->gpu.chan[2].ctl.blk;
      case 0x00E: return agb->gpu.chan[3].ctl.blk;
      case 0x010: return agb->gpu.chan[0].loopy_x.blk;
      case 0x012: return agb->gpu.chan[0].loopy_y.blk;
      case 0x014: return agb->gpu.chan[1].loopy_x.blk;
      case 0x016: return agb->gpu.chan[1].loopy_y.blk;
      case 0x018: return agb->gpu.chan[2].loopy_x.blk;
      case 0x01A: return agb->gpu.chan[2].loopy_y.blk;
      case 0x01C: return agb->gpu.chan[3].loopy_x.blk;
      case 0x01E: return agb->gpu.chan[3].loopy_y.blk;
      case 0x020: return agb->gpu.chan[2].dx.blk;
      case 0x022: return agb->gpu.chan[2].dmx.blk;
      case 0x024: return agb->gpu.chan[2].dy.blk;
      case 0x026: return agb->gpu.chan[2].dmy.blk;
      case 0x028: return agb->gpu.chan[2].ref_x.blk16[0];
      case 0x02A: return agb->gpu.chan[2].ref_x.blk16[1];
      case 0x02C: return agb->gpu.chan[2].ref_y.blk16[0];
      case 0x02E: return agb->gpu.chan[2].ref_y.blk16[1];
      case 0x030: return agb->gpu.chan[2].dx.blk;
      case 0x032: return agb->gpu.chan[2].dmx.blk;
      case 0x034: return agb->gpu.chan[2].dy.blk;
      case 0x036: return agb->gpu.chan[2].dmy.blk;
      case 0x038: return agb->gpu.chan[2].ref_x.blk16[0];
      case 0x03A: return agb->gpu.chan[2].ref_x.blk16[1];
      case 0x03C: return agb->gpu.chan[2].ref_y.blk16[0];
      case 0x03E: return agb->gpu.chan[2].ref_y.blk16[1];
      case 0x040: return agb->gpu.clip_x[0].blk;
      case 0x042: return agb->gpu.clip_x[1].blk;
      case 0x044: return agb->gpu.clip_y[0].blk;
      case 0x046: return agb->gpu.clip_y[1].blk;
      case 0x048: return agb->gpu.win_in.blk;
      case 0x04A: return agb->gpu.win_out.blk;
      case 0x04C: return agb->gpu.mosaic.blk;
      case 0x04E: return agb->gpu.ugpio_4E.blk;
      case 0x050: return agb->gpu.bld_ctl.blk;
      case 0x052: return agb->gpu.bld_args.blk;
      case 0x054: return agb->gpu.bri_args.blk;
      case 0x056: return agb->gpu.ugpio_56.blk;
        /* APU IO*/
      case 0x060: return agb->apu.squ_ch[0].nrx0.blk;
      case 0x062: return agb->apu.squ_ch[0].nrx1_x2.blk;
      case 0x064: return agb->apu.squ_ch[0].nrx3_x4.blk;
      case 0x068: return agb->apu.squ_ch[1].nrx1_x2.blk;
      case 0x06C: return agb->apu.squ_ch[1].nrx3_x4.blk;
      case 0x070: return agb->apu.wave_ch.nr30.blk;
      case 0x072: return agb->apu.wave_ch.nr31_32.blk;
      case 0x074: return agb->apu.wave_ch.nr33_34.blk;
      case 0x078: return agb->apu.noise_ch.nr41_42.blk;
      case 0x07C: return agb->apu.noise_ch.nr43_44.blk;
      case 0x080: return agb->apu.settings.nr50_51.blk;
      case 0x082: return agb->apu.settings.snd_ctl.blk;
      case 0x084: return agb->apu.settings.nr52.blk;
      case 0x088: return agb->apu.settings.pwm_ctl.blk;
        /* TODO: Bank 4bit PCM and FIFO buffer */
        /* DMA */
      case 0x0BA: return agb->dma.component[0].control.blk;
      case 0x0C6: return agb->dma.component[1].control.blk;
      case 0x0D2: return agb->dma.component[2].control.blk;
      case 0x0DE: return agb->dma.component[3].control.blk;
      case 0x0B0: return agb->dma.component[0].source.blk16[0]; 
      case 0x0B2: return agb->dma.component[0].source.blk16[1];
      case 0x0B4: return agb->dma.component[0].destin.blk16[0] ;
      case 0x0B6: return agb->dma.component[0].destin.blk16[1];
      case 0x0B8: return agb->dma.component[0].counter.blk;
      case 0x0BC: return agb->dma.component[1].source.blk16[0] ;
      case 0x0BE: return agb->dma.component[1].source.blk16[1];
      case 0x0C0: return agb->dma.component[1].destin.blk16[0] ;
      case 0x0C2: return agb->dma.component[1].destin.blk16[1];
      case 0x0C4: return agb->dma.component[1].counter.blk;
      case 0x0C8: return agb->dma.component[2].source.blk16[0] ;
      case 0x0CA: return agb->dma.component[2].source.blk16[1];
      case 0x0CC: return agb->dma.component[2].destin.blk16[0] ;
      case 0x0CE: return agb->dma.component[2].destin.blk16[1];
      case 0x0D0: return agb->dma.component[2].counter.blk;
      case 0x0D4: return agb->dma.component[3].source.blk16[0] ;
      case 0x0D6: return agb->dma.component[3].source.blk16[1];
      case 0x0D8: return agb->dma.component[3].destin.blk16[0] ;
      case 0x0DA: return agb->dma.component[3].destin.blk16[1];
      case 0x0DC: return agb->dma.component[3].counter.blk;
      case 0x0E0: return agb->dma.ugpio_E0.blk ;
        /* timer */
      case 0x100: return agb->timer.component[0].counter.blk; 
      case 0x104: return agb->timer.component[1].counter.blk; 
      case 0x108: return agb->timer.component[2].counter.blk; 
      case 0x10C: return agb->timer.component[3].counter.blk; 
      case 0x102: return agb->timer.component[0].control.blk; 
      case 0x106: return agb->timer.component[1].control.blk; 
      case 0x10A: return agb->timer.component[2].control.blk; 
      case 0x10E: return agb->timer.component[3].control.blk; 

      case 0x130: return agb->joypad.input.blk;
      case 0x132: return agb->joypad.ctl.blk;

      case 0x200: return agb->arm7.ie.blk;
      case 0x202: return agb->arm7.ifs.blk;
      case 0x204: return agb->mem.wait.blk;
      case 0x208: return agb->arm7.ime.blk & 1; 
      case 0x20A: return 0;
      case 0x300:
        if (addr & 1)
          return agb->sundry.halt.blk8[0];
        else 
          return agb->sundry.post.blk;
      default:
        return fto_uint16pv (agb->mem.IOMap [addr & -2]);
      }
    } else {
      uint32_t xc800  = addr & 0x400FFFF;
      if (xc800 >= 0x4000800 && xc800 <= 0x4000803) 
        return agb->mem.imc.blk16[xc800 - 0x4000800 >> 1];
      else
        return fto_uint16pv (agb->mem.IOMap [addr & IOMAP2_MASK]);
    }
    break;
  case 3: 
    return fto_uint16pv (agb->mem.IRam[addr & IRAM_MASK]);
  case 5: 
    return fto_uint16pv (agb->gpu.palette[addr & PAL_MASK]);
  case 6:
    if (addr & 0x10000) 
      return fto_uint16pv (agb->gpu.vram[addr & VRAM_MASK_Vx17FFF_x20000]);
    else 
      return fto_uint16pv (agb->gpu.vram[addr & VRAM_MASK]);
    break;
  case 7: 
    return fto_uint16pv (agb->gpu.oam[addr & OAM_MASK]);
  case 8:
  case 9: 
  case 10:
  case 11: 
  case 12:
  case 13: 
    return fto_uint16pv (agb->mem.PRom[addr & GAME_PAK_MASK]);
  case 15:
  case 14:
    value = agb->mem.SRam[addr & SRAM_MASK];
    return value | value << 8;
  default:
     assert (0);
  }
  return 0;
}

#if 0
#undef HALFWORD_MASK  
#undef WORD_MASK 
#undef BYTE_MASK 
#undef IRAM_MASK 
#undef WRAM_MASK 
#undef OAM_MASK  
#undef PAL_MASK  
#undef VRAM_MASK  
#undef BIOS_MASK 
#undef GAME_PAK_MASK
#undef IOMAP_MASK 
#undef IOMAP2_MASK 
#undef SRAM_MASK 
#endif 

#endif