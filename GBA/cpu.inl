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
 
#ifndef CPU_INL
#define CPU_INL 1

finline void 
cpu_reset (struct arm7 *cpu) {

  cpu->nextNoSeqFetch = 0;
  cpu->ifs.blk = 0;
  cpu->ime.blk = 0;
  cpu->ie.blk = 0;

  memset (& cpu->r1314_t[0], 0, sizeof (cpu->r1314_t));
  memset (& cpu->r812_t[0], 0, sizeof (cpu->r812_t));
  memset (& cpu->spsr_t[0], 0, sizeof (cpu->spsr_t));

  /* Set CPU Management mode and IRQ, FIQ disable *.*/
  cpu->cpsr.mode = ARM7_MODE_MGR | 0x10;

  cpu->regs[15].blk = 0x8;
  cpu->regs[14].blk = 0;
  cpu->regs[13].blk = 0x3007F00;
  cpu->regs[12].blk = 0;
  cpu->regs[11].blk = 0;
  cpu->regs[10].blk = 0;
  cpu->regs[9].blk = 0;
  cpu->regs[8].blk = 0;
  cpu->regs[7].blk = 0;
  cpu->regs[6].blk = 0;
  cpu->regs[5].blk = 0;
  cpu->regs[4].blk = 0;
  cpu->regs[3].blk = 0;
  cpu->regs[2].blk = 0;
  cpu->regs[1].blk = 0;
  cpu->regs[0].blk = 0;

  /* Prefetch  opcode */
  cpu->opcode[0] = * (uint32_t *)& cpu->agb->mem.bios[0];
  cpu->opcode[1] = * (uint32_t *)& cpu->agb->mem.bios[4];

#ifdef _RESET_STRAT_GAMEPAK
  /* Set CPU Management mode and IRQ, FIQ disable *.*/
  cpu->cpsr.mode = ARM7_MODE_SYS | 0x10;

  cpu->regs[15] = 0x8000008;
  cpu->regs[14] = 0;
  cpu->regs[13] = 0x3007F00;
  cpu->regs[12] = 0;
  cpu->regs[11] = 0;
  cpu->regs[10] = 0;
  cpu->regs[9] = 0;
  cpu->regs[8] = 0;
  cpu->regs[7] = 0;
  cpu->regs[6] = 0;
  cpu->regs[5] = 0;
  cpu->regs[4] = 0;
  cpu->regs[3] = 0;
  cpu->regs[2] = 0;
  cpu->regs[1] = 0;
  cpu->regs[0] = 0;

  /* Prefetch  opcode */
  cpu->opcode[0] = * (uint32_t *)& cpu->agb->mem.PRom[0];
  cpu->opcode[1] = * (uint32_t *)& cpu->agb->mem.PRom[4];


#endif 
}

static 
uint32_t arm7_current_std_spsr (struct arm7 *arm7) {

  switch (arm7->cpsr_fast.mode & 15) {
  case ARM7_MODE_USER:
  case ARM7_MODE_SYS:
    return 0;
  case ARM7_MODE_FIQ:
    return arm7->spsr_t[SPSRb_FIQ].blk;
  case ARM7_MODE_IRQ:
    return arm7->spsr_t[SPSRb_IRQ].blk;
  case ARM7_MODE_MGR:
    return arm7->spsr_t[SPSRb_MGR].blk;
  case ARM7_MODE_UNDEF:
    return arm7->spsr_t[SPSRb_UDEF].blk;
  case ARM7_MODE_ABT:
    return arm7->spsr_t[SPSRb_ABT].blk;
  default:
    return 0;
    assert (0);
  }
}

finline uint32_t 
arm7_getpc (struct arm7 *arm7) {

  if (arm7->cpsr_fast.thumb != false)
    return (arm7->regs[15].blk & 0xFFFFFFE) - 4;
  else 
    return (arm7->regs[15].blk & 0xFFFFFFC) - 8;
}

finline kable 
arm7_thumbmode (struct arm7 *arm7) {

  if (arm7->cpsr_fast.thumb != false)
    return true;
  else 
    return false;
}

finline uint32_t 
arm7_getpc_internal (struct arm7 *arm7) {

  if (arm7->cpsr_fast.thumb != false)
    return (arm7->regs[15].blk & 0xFFFFFFE) - 2;
  else 
    return (arm7->regs[15].blk & 0xFFFFFFC) - 4;
}

#endif