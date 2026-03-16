/* This program is free software; you can redistribute it and/or modify
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

/* TODO: LDM, STM seq, I cycle cache operate, 
            pc + 12 in some memory access,
    noseq fetch after       some memory write
       empty operate in some list-opcode 
       
       arm7 alu with s, r15 adjust pc in thumb/arm7 
       list-memory with s, r15 adjust pc in thumb/arm7 
       
       cmp no-adjust r15 pipeline.
*/

/* CPU-Contribution: jsmolka's fasm arm7 test source code (https://github.com/jsmolka/eggvance/tree/master/roms) 
                     Advance Debugger 0.36 (https://www.gbadev.org/tools.php?showinfo=1264)
                     VBA arm7tdmi source 
                     nocash$GBA-debug's arm7 doc 
                     ARM Architecture Reference Manual
                     ARM7TDMI Revision: r4p1
                     ARM® Developer Suite     
                     and etc. 
 */

#ifndef ARM7TDMI_INL
#define ARM7TDMI_INL 1
  
/* =================== ARM7 HELPER =================== */
struct arm_alu_rot_imm32 {
  uint32_t immed_8:8;
  uint32_t rorate_imm:4;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t s:1;
  uint32_t opcode:4;
  uint32_t _0_0_1:3;
  uint32_t cond:4;
};
struct arm_alu_shift_imm5 {
  uint32_t rm:4;
  uint32_t _0:1;
  uint32_t shift:2;
  uint32_t shift_imm:5;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t s:1;
  uint32_t opcode:4;
  uint32_t _0_0_0:3;
  uint32_t cond:4;
};
struct arm_alu_shift_rs {
  uint32_t rm:4;
  uint32_t _1:1;
  uint32_t shift:2;
  uint32_t _0:1;
  uint32_t rs:4;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t s:1;
  uint32_t opcode:4;
  uint32_t _0_0_0:3;
  uint32_t cond:4;
};
struct arm_atomic_memory_access {
  uint32_t rm:4;
  uint32_t _0_0_0_0_1_0_0_1:8;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t _0_0:2;
  uint32_t b:1;
  uint32_t _0_0_0_1_0:5;
  uint32_t cond:4; 
};
struct arm_memory_access_ubw_imm12 {
  uint32_t offset12:12;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t l:1;
  uint32_t w:1;
  uint32_t b:1;
  uint32_t u:1;
  uint32_t p:1;
  uint32_t _0_1_0:3;
  uint32_t cond:4;
};
struct arm_memory_access_ubw_reg {
  uint32_t rm:4;
  uint32_t _0_0_0_0_0_0_0_0:8;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t l:1;
  uint32_t w:1;
  uint32_t b:1;
  uint32_t u:1;
  uint32_t p:1;
  uint32_t _0_1_1:3;
  uint32_t cond:4;
};
struct arm_memory_access_ubw_scaled {
  uint32_t rm:4;
  uint32_t _0:1;
  uint32_t shift:2;
  uint32_t shift_imm:5;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t l:1;
  uint32_t w:1;
  uint32_t b:1;
  uint32_t u:1;
  uint32_t p:1;
  uint32_t _0_1_1:3;
  uint32_t cond:4;
};
struct arm_memory_access_sbh_imm8 {
  uint32_t imm_l:4;
  uint32_t _1:1;
  uint32_t h:1;
  uint32_t s:1;
  uint32_t __1:1;
  uint32_t imm_h:4;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t l:1;
  uint32_t w:1;
  uint32_t ___1:1;
  uint32_t u:1;
  uint32_t p:1;
  uint32_t _0_0_0:3;
  uint32_t cond:4;
};
struct arm_memory_access_sbh_reg {
  uint32_t rm:4;
  uint32_t _1:1;
  uint32_t h:1;
  uint32_t s:1;
  uint32_t __1:1;
  uint32_t sbz:4;
  uint32_t rd:4;
  uint32_t rn:4;
  uint32_t l:1;
  uint32_t w:1;
  uint32_t _0:1;
  uint32_t u:1;
  uint32_t p:1;
  uint32_t _0_0_0:3;
  uint32_t cond:4;
};
struct arm_list_memory_access {
  uint32_t r0:1;
  uint32_t r1:1;
  uint32_t r2:1;
  uint32_t r3:1;
  uint32_t r4:1;
  uint32_t r5:1;
  uint32_t r6:1;
  uint32_t r7:1;
  uint32_t r8:1;
  uint32_t r9:1;
  uint32_t r10:1;
  uint32_t r11:1;
  uint32_t r12:1;
  uint32_t r13:1; /* sp */
  uint32_t r14:1; /* lr */
  uint32_t r15:1; /* pc */
  uint32_t rn:4;
  uint32_t l:1;
  uint32_t w:1;
  uint32_t s:1;
  uint32_t u:1;
  uint32_t p:1;
  uint32_t _1_0_0:3;
  uint32_t cond:4;
};
struct arm_mul {
  uint32_t rm:4;
  uint32_t _1_0_0_1:4;
  uint32_t rs:4;
  uint32_t rn:4;
  uint32_t rd:4;
  uint32_t s:1;
  uint32_t a:1;
  uint32_t  _0_0_0_0_0_0:6;
  uint32_t cond:4;
};
struct arm_mul_long {
  uint32_t rm:4;
  uint32_t _1_0_0_1:4;
  uint32_t rs:4;
  uint32_t rd_lo:4;
  uint32_t rd_hi:4;
  uint32_t s:1;
  uint32_t a:1;
  uint32_t u:1;
  uint32_t  _0_0_0_0_1:5;
  uint32_t cond:4;
};
struct arm_msr_rot_imm32 {
  uint32_t imm8:8;
  uint32_t rotate_imm:4;
  uint32_t sbo:4;
  uint32_t field_c:1;
  uint32_t field_x:1;
  uint32_t field_s:1;
  uint32_t field_f:1;
  uint32_t _1_0:2;
  uint32_t r:1;
  uint32_t _0_0_1_1_0:5;
  uint32_t cond:4;
};
struct arm_msr_reg {
  uint32_t rm:4;
  uint32_t _0_0_0_0:4;
  uint32_t sbz:4;
  uint32_t sbo:4;
  uint32_t field_c:1;
  uint32_t field_x:1;
  uint32_t field_s:1;
  uint32_t field_f:1;
  uint32_t _1_0:2;
  uint32_t r:1;
  uint32_t _0_0_0_1_0:5;
  uint32_t cond:4;
};
struct arm_branch {
  uint32_t sign_imm:23;
  uint32_t sign_bit:1;
  uint32_t l:1;
  uint32_t _1_0_1:3;
  uint32_t cond:4;
};
struct arm_branch_exchange {
  uint32_t rm:4;
  uint32_t _0_0_0_1:4; 
  uint32_t sbo:12;
  uint32_t _0_0_0_1_0_0_1_0:8;
  uint32_t cond:4;
};
/* =================== THUMB HELPER =================== */
struct thumb_alu {
  uint16_t rd_rn:3;
  uint16_t rm_rs:3;
  uint16_t opcode:4;
  uint16_t _0_1_0_0_0_0:6;
};
struct thumb_branch {
  uint16_t imm10:10;
  uint16_t sign_bit:1;
  uint16_t h:2;
  uint16_t _1_1_1:3;
};



finline kable 
arm7_privilege_mode (struct gba *const agb) {
  const uint32_t mode = agb->arm7.cpsr_fast.mode & 0x0F;
  if (mode != 0)
    return true;
  else 
    return false;
}

finline 
struct psr *arm7_mode_spsr (struct gba *const agb, const uint32_t mode) {
  struct arm7 *const arm = & agb->arm7;
  switch (mode & 0x0F) {
  case ARM7_MODE_USER: return & arm->spsr_t[SPSRb_SYSUSER];
  case ARM7_MODE_SYS: return & arm->spsr_t[SPSRb_SYSUSER];
  case ARM7_MODE_FIQ: return & arm->spsr_t[SPSRb_FIQ];
  case ARM7_MODE_MGR: return & arm->spsr_t[SPSRb_MGR];
  case ARM7_MODE_ABT: return & arm->spsr_t[SPSRb_ABT];
  case ARM7_MODE_UNDEF: return & arm->spsr_t[SPSRb_UDEF];
  case ARM7_MODE_IRQ: return & arm->spsr_t[SPSRb_IRQ];
  default: assert (0); break;
  }
  return null;
}

finline 
struct psr *arm7_cur_spsr (struct gba *const agb) {
  struct arm7 *const arm = & agb->arm7;
  return arm7_mode_spsr (agb, arm->cpsr_fast.mode & 0x0F);
}

finline 
void cpsr_to_fast (struct psr *std, struct cpsr_f *fast) {
  
  fast->flags.blk = 0;
  fast->fiq_disable = false;
  fast->irq_disable = false;
  fast->mode = 0;
  fast->thumb = false;

  if (std->blk & ARM7_FLAG_N)
    fast->flags.sig.n = 0x80;
  if (std->blk & ARM7_FLAG_Z)
    fast->flags.sig.z = 0x80;
  if (std->blk & ARM7_FLAG_C)
    fast->flags.sig.c = 0x80;
  if (std->blk & ARM7_FLAG_V)
    fast->flags.sig.v = 0x80;

  if (std->blk & ARM7_FIQ_DIS_MASK)
    fast->fiq_disable = true;
  if (std->blk & ARM7_IRQ_DIS_MASK)
    fast->irq_disable = true;
  if (std->blk & ARM7_OPCODE_THUMB_MASK)
    fast->thumb = true;

  fast->mode = std->mode & 0xF | ARM7_MODE_HSET_MASK;
}

finline void                           
bank_swi (struct gba *const agb, const uint32_t new_mode) {
  struct arm7 *const arm = & agb->arm7;
  const uint32_t mode_cur = arm->cpsr_fast.mode & 0x0F;
  const uint32_t mode_to = new_mode & 0x0F;
  union iopad_blk32 *const regs = & arm->regs[0];

  if ( arm7_mode_spsr (agb, mode_cur)
      != arm7_mode_spsr (agb, mode_to) )
  {
    if (mode_cur == ARM7_MODE_FIQ
      || mode_to == ARM7_MODE_FIQ)
    {
      if (mode_cur == ARM7_MODE_FIQ) {
        arm->r812_t[R812b_FIQ+0].blk = regs[8].blk;
        arm->r812_t[R812b_FIQ+1].blk = regs[9].blk;
        arm->r812_t[R812b_FIQ+2].blk = regs[10].blk;
        arm->r812_t[R812b_FIQ+3].blk = regs[11].blk;
        arm->r812_t[R812b_FIQ+4].blk = regs[12].blk;
        
        regs[8].blk = arm->r812_t[R812b_EXCEPT_FIQ+0].blk;
        regs[9].blk = arm->r812_t[R812b_EXCEPT_FIQ+1].blk;
        regs[10].blk = arm->r812_t[R812b_EXCEPT_FIQ+2].blk;
        regs[11].blk = arm->r812_t[R812b_EXCEPT_FIQ+3].blk;
        regs[12].blk = arm->r812_t[R812b_EXCEPT_FIQ+4].blk;
      } else {
        arm->r812_t[R812b_EXCEPT_FIQ+0].blk = regs[8].blk;
        arm->r812_t[R812b_EXCEPT_FIQ+1].blk = regs[9].blk;
        arm->r812_t[R812b_EXCEPT_FIQ+2].blk = regs[10].blk;
        arm->r812_t[R812b_EXCEPT_FIQ+3].blk = regs[11].blk;
        arm->r812_t[R812b_EXCEPT_FIQ+4].blk = regs[12].blk;
        
        regs[8].blk = arm->r812_t[R812b_FIQ+0].blk;
        regs[9].blk = arm->r812_t[R812b_FIQ+1].blk;
        regs[10].blk = arm->r812_t[R812b_FIQ+2].blk;
        regs[11].blk = arm->r812_t[R812b_FIQ+3].blk;
        regs[12].blk = arm->r812_t[R812b_FIQ+4].blk;
      }
    }

    switch (mode_cur) {
    case ARM7_MODE_USER:
    case ARM7_MODE_SYS: 
      arm->r1314_t[R1314b_SYSUSER+0].blk = regs[13].blk;
      arm->r1314_t[R1314b_SYSUSER+1].blk = regs[14].blk;
      break;
    case ARM7_MODE_MGR: 
      arm->r1314_t[R1314b_MGR+0].blk = regs[13].blk;
      arm->r1314_t[R1314b_MGR+1].blk = regs[14].blk;
      break;
    case ARM7_MODE_ABT: 
      arm->r1314_t[R1314b_ABT+0].blk = regs[13].blk;
      arm->r1314_t[R1314b_ABT+1].blk = regs[14].blk;
      break;
    case ARM7_MODE_UNDEF: 
      arm->r1314_t[R1314b_UDEF+0].blk = regs[13].blk;
      arm->r1314_t[R1314b_UDEF+1].blk = regs[14].blk;
      break;
    case ARM7_MODE_IRQ: 
      arm->r1314_t[R1314b_IRQ+0].blk = regs[13].blk;
      arm->r1314_t[R1314b_IRQ+1].blk = regs[14].blk;
      break;
    case ARM7_MODE_FIQ: 
      arm->r1314_t[R1314b_FIQ+0].blk = regs[13].blk;
      arm->r1314_t[R1314b_FIQ+1].blk = regs[14].blk;
      break;
    default: 
      assert (0); 
      break;
    }

    switch (mode_to) {
    case ARM7_MODE_USER:
    case ARM7_MODE_SYS: 
      regs[13].blk = arm->r1314_t[R1314b_SYSUSER+0].blk;
      regs[14].blk = arm->r1314_t[R1314b_SYSUSER+1].blk;
      break;
    case ARM7_MODE_MGR: 
      regs[13].blk = arm->r1314_t[R1314b_MGR+0].blk;
      regs[14].blk = arm->r1314_t[R1314b_MGR+1].blk;
      break;
    case ARM7_MODE_ABT: 
      regs[13].blk = arm->r1314_t[R1314b_ABT+0].blk;
      regs[14].blk = arm->r1314_t[R1314b_ABT+1].blk;
      break;
    case ARM7_MODE_UNDEF: 
      regs[13].blk = arm->r1314_t[R1314b_UDEF+0].blk;
      regs[14].blk = arm->r1314_t[R1314b_UDEF+1].blk;
      break;
    case ARM7_MODE_IRQ: 
      regs[13].blk = arm->r1314_t[R1314b_IRQ+0].blk;
      regs[14].blk = arm->r1314_t[R1314b_IRQ+1].blk;
      break;
    case ARM7_MODE_FIQ: 
      regs[13].blk = arm->r1314_t[R1314b_FIQ+0].blk;
      regs[14].blk = arm->r1314_t[R1314b_FIQ+1].blk;
      break;
    default: 
      assert (0); 
      break;
    }
  }
}

finline 
void cpsr_to_std (struct psr *std, struct cpsr_f *fast) {

  std->blk = 0;

  if (fast->flags.sig.n != 0)
    std->blk |= ARM7_FLAG_N;
  if (fast->flags.sig.z != 0)
    std->blk |= ARM7_FLAG_Z;
  if (fast->flags.sig.c != 0)
    std->blk |= ARM7_FLAG_C;
  if (fast->flags.sig.v != 0)
    std->blk |= ARM7_FLAG_V;
  if (fast->fiq_disable != false)
    std->blk |= ARM7_FIQ_DIS_MASK;
  if (fast->irq_disable != false)
    std->blk |= ARM7_IRQ_DIS_MASK;
  if (fast->thumb != false)
    std->blk |= ARM7_OPCODE_THUMB_MASK;
  std->blk |= fast->mode | ARM7_MODE_HSET_MASK;
}

finline 
uint32_t cpsr_to_std2 (struct psr *std, struct cpsr_f *fast) {

  uint32_t blk = 0;

  if (fast->flags.sig.n != 0)
    blk |= ARM7_FLAG_N;
  if (fast->flags.sig.z != 0)
    blk |= ARM7_FLAG_Z;
  if (fast->flags.sig.c != 0)
    blk |= ARM7_FLAG_C;
  if (fast->flags.sig.v != 0)
    blk |= ARM7_FLAG_V;
  if (fast->fiq_disable != false)
    blk |= ARM7_FIQ_DIS_MASK;
  if (fast->irq_disable != false)
    blk |= ARM7_IRQ_DIS_MASK;
  if (fast->thumb != false)
    blk |= ARM7_OPCODE_THUMB_MASK;
  blk |= fast->mode | ARM7_MODE_HSET_MASK;
  return blk;
}

finline void 
spsr_to_cpsr (struct gba *const agb) {
  struct psr *spsr = arm7_cur_spsr (agb);
  const uint32_t psrc = spsr->blk;
  
  bank_swi (agb, spsr->mode);
  agb->arm7.cpsr.blk = psrc;
  cpsr_to_fast (& agb->arm7.cpsr, & agb->arm7.cpsr_fast);
  reset_int_magic (agb); 
}

finline 
int32_t mul_clks (uint32_t rs_value) {
  /* multiplier's clks, m
   see ARM7TDMI Technical Reference Manual's 6.20 Instruction speed summary
   m is: 
   1 if bits [31:8] of the multiplier operand (Rs) are all zero or one, else 
   2 if bits [31:16] of the multiplier operand (Rs) are all zero or one, else 
   3 if bits [31:24] of the multiplier operand (Rs) are all zero or all one, else 
   4. */
          
  if (rs_value & 0x80000000)                                    
    rs_value = ~rs_value;
  
  if ((rs_value & 0xFFFFFF00) == 0)                         
    return 1;                            
  else if ((rs_value & 0xFFFF0000) == 0)                    
    return 2;
  else if ((rs_value & 0xFF000000) == 0)                    
    return 3;                             
  else                                                
    return 4;    
}

#define ROT_IMM32_FAST(/* uint32_t */ imm8_, /* uint32_t */ shift_even_)\
  ((imm8_) == 0 ?           (imm8_)                                     \
                :     (     (imm8_) >>      ((shift_even_) << 1)        \
                        |   (imm8_) << 32 - ((shift_even_) << 1)   )  )

#define ROT_IMM32_C(/* uint32_t */ imm8_, /* uint32_t */ shift_even_, oprand, carry)\
do {                                                           \
  if (shift_even_ == 0)                                        \
    oprand = imm8_;                                            \
  else {                                                       \
    const uint32_t shift_0 = (shift_even_) << 1;                   \
    const uint32_t imm32_0 = (imm8_) >> shift_0 | (imm8_) << 32 - shift_0; \
    if (imm32_0 & ARM7_NEG_BIT)                     \
      carry = 0x80;                                               \
    else                                                       \
      carry = 0;                                               \
    oprand = imm32_0;                                            \
  }                                                            \
} while (0) 
  
#define NZ_RESET_FAST(/* uint32_t */ output0)           \
do {                                                   \
  const uint32_t out0 = (output0);                      \
  arm->cpsr_fast.flags.blk16[1] = out0 >> 16 & 0x8000; \
  if (out0 == 0)                                       \
    arm->cpsr_fast.flags.blk8[2] = 0x80;                \
} while (0)

/* ============================ ARM7 Rotate imm32 alu ============================== */

#define ISA_ARM7_ALU_NO_SIGN_ROT_IMM32(/* alu_out */ OP_alu /* (rn, oprand) */ )               \
do {                                                                                                \
  const struct arm_alu_rot_imm32 *const m_code = (const struct arm_alu_rot_imm32 *const) & OP_code; \
  const uint32_t immed_8 = m_code->immed_8;                                                         \
  const uint32_t rorate_imm = m_code->rorate_imm;                                                   \
  const uint32_t oprand2 = ROT_IMM32_FAST (immed_8, rorate_imm);                                 \
  const uint32_t rdi = m_code->rd;                                                                  \
  const uint32_t rn = regs[m_code->rn].blk;                                                             \
                                                                                                    \
  regs[rdi].blk = OP_alu (rn, oprand2);                                                            \
                                                                                                    \
  if (rdi == 15) {                                                                                  \
    regs[15].blk &= ARM7_CODE_MASK;                                                                     \
    consume +=  arm7_flush (agb) + 2;                                                               \
  }                                                                                                 \
} while (0)

/* ==================== shift imm5 no sign ==================== */
#define LSL_IMM5_NO_SIGN(rm, shift_imm, oprand)\
 do {                                          \
   oprand = rm << shift_imm;                   \
 } while (0)                                   
#define LSR_IMM5_NO_SIGN(rm, shift_imm, oprand)\
 do {                                          \
   if (shift_imm == 0)                         \
     oprand = 0;                               \
   else                                        \
     oprand = rm >> shift_imm;                 \
 } while (0)                                   
#define ASR_IMM5_NO_SIGN(rm, shift_imm, oprand)\
 do {                                          \
   if (shift_imm == 0)                         \
     if (rm & LSHIFT (1, 31))       \
       oprand = ARM7_UWORD_MAX;                    \
     else                                      \
       oprand = 0;                             \
    else                                       \
      oprand = ((int32_t) rm) >> shift_imm;    \
 } while (0)                                   
#define ROR_IMM5_NO_SIGN(rm, shift_imm, oprand)\
 do {                                          \
   if (shift_imm == 0)                         \
     oprand = rm >> 1 | (arm->cpsr_fast.flags.sig.c != 0? LSHIFT (1, 31) : 0); \
   else                                        \
     oprand = rm >> shift_imm | rm << 32 - shift_imm; \
 } while (0)                                   
   
#define ISA_ARM7_ALU_NO_SIGN_SHIFT_IMM5(/* alu_out */ OP_alu /* (rn, oprand) */,/*void */ shift_imm5_callback /* (const imm5, invariant &) */)  \
do {  \
  const struct arm_alu_shift_imm5 *const m_code = (const struct arm_alu_shift_imm5 *const) & OP_code;\
  const uint32_t shift_imm = m_code->shift_imm & 31;\
  const uint32_t rdi = m_code->rd;\
  const uint32_t rn = regs[m_code->rn].blk;\
  const uint32_t rm = regs[m_code->rm].blk;   \
                                             \
  uint32_t oprand2;                        \
                                             \
  shift_imm5_callback (rm, shift_imm, oprand2);\
                                                  \
  regs[rdi].blk = OP_alu (rn, oprand2);\
  \
  if (rdi == 15) {\
    regs[15].blk &= ARM7_CODE_MASK;\
    consume +=  arm7_flush (agb) + 2;\
  }\
  \
} while (0)

/* ==================== shift rs no sign ==================== */
#define LSL_RS_NO_SIGN(rm, rs_shift, oprand)  \
 do {                                   \
   const uint32_t rs_0 = rs_shift & 255;   \
   if (rs_0 < 32)                         \
     oprand = rm << rs_0;                 \
   else                                 \
     oprand = 0;                        \
 } while (0)                                 
#define LSR_RS_NO_SIGN(rm, rs_shift, oprand) \
 do {                                   \
   const uint32_t rs_0 = rs_shift & 255;   \
   if (rs_0 < 32)                         \
     oprand = rm  >> rs_0;                \
   else                                 \
     oprand = 0;                        \
 } while (0)                                   
#define ASR_RS_NO_SIGN(rm, rs_shift, oprand)\
 do {                                   \
   const uint32_t rs_0 = rs_shift & 255;   \
   if (rs_0 < 32)                         \
     oprand = ((int32_t) rm) >> rs_0;     \
   else                                 \
     if (rm & LSHIFT (1, 31))\
       oprand = ARM7_UWORD_MAX;             \
     else                               \
       oprand = 0;                      \
 } while (0)                                
#define ROR_RS_NO_SIGN(rm, rs_shift, oprand)\
 do {                                   \
   const uint32_t rs_0 = rs_shift & 255;   \
   if (rs_0 == 0)                         \
     oprand = rm;                       \
   else if ((rs_0 & 31) == 0)             \
     oprand = rm;                       \
   else {                               \
     const uint32_t rs_f = rs_0 & 31;     \
     oprand = rm >> rs_f | rm << 32 - rs_f; \
   }                                    \
 } while (0)                            \
 
#define ISA_ARM7_ALU_NO_SIGN_SHIFT_RS(OP_alu /* (rd, rn, oprand) */, shift_rs_callback /* (invariant &) */)  \
do {  \
  const struct arm_alu_shift_rs *const m_code = (const struct arm_alu_shift_rs *const) & OP_code;\
  const uint32_t rdi = m_code->rd;\
  const uint32_t rn = m_code->rn == 15 ? regs[m_code->rn].blk + 4 : regs[m_code->rn].blk;                \
  const uint32_t rm = m_code->rm == 15 ? regs[m_code->rm].blk + 4 : regs[m_code->rm].blk;                \
  const uint32_t rs = regs[m_code->rs].blk8[0];   \
                                             \
  uint32_t oprand2;                        \
                                           \
                                             \
  shift_rs_callback (rm, rs, oprand2);\
                                                  \
  regs[rdi].blk = OP_alu (rn, oprand2);\
  \
  if (rdi == 15) {\
    regs[15].blk &= ARM7_CODE_MASK;\
    consume +=  arm7_flush (agb) + 3;\
  }\
  \
} while (0)

/* ==================== arm7 alu main no sign ==================== */
#define OP_AND(rn, oprand) (rn & oprand)
#define OP_EOR(rn, oprand) (rn ^ oprand)
#define OP_SUB(rn, oprand) (rn - oprand)
#define OP_RSB(rn, oprand) (oprand - rn)
#define OP_ADD(rn, oprand) (rn + oprand)
#define OP_ADC(rn, oprand) (rn + oprand + (arm->cpsr_fast.flags.sig.c != 0? 1 : 0))
#define OP_SBC(rn, oprand) (rn - oprand - (arm->cpsr_fast.flags.sig.c != 0? 0 : 1))
#define OP_RSC(rn, oprand) (oprand - rn - (arm->cpsr_fast.flags.sig.c != 0? 0 : 1))
#define OP_ORR(rn, oprand) (rn | oprand)
#define OP_MOV(rn, oprand) (oprand)
#define OP_BIC(rn, oprand) (rn & ~oprand)
#define OP_MVN(rn, oprand) (~oprand)

/* ==================== arm7 alu sign ==================== */
#define ISA_ARM7_ALU_SIGN_ROT_IMM32(/* alu_out */ OP_alu /* (rn, oprand, output) */,           \
                                    /* bool */    OP_w,                                     \
                                    /* bool */    OP_c )                            \
do {                                                                                                \
  const struct arm_alu_rot_imm32 *const m_code = (const struct arm_alu_rot_imm32 *const) & OP_code; \
  const uint32_t immed_8 = m_code->immed_8;            \
  const uint32_t rorate_imm = m_code->rorate_imm;      \
  const uint32_t rdi = m_code->rd;                     \
  const uint32_t rn = regs[m_code->rn].blk;                \
  const uint32_t prev_cpsr = arm->cpsr_fast.flags.blk;            \
  uint32_t oprand2;                                    \
                                                       \
  if (OP_c != false) {                 \
    oprand2 = ROT_IMM32_FAST (immed_8, rorate_imm);    \
                                                       \
  } else {                                             \
    uint32_t carry = arm->cpsr_fast.flags.sig.c;                      \
                                                       \
    ROT_IMM32_C (immed_8, rorate_imm, oprand2, carry); \
                                                       \
    arm->cpsr_fast.flags.sig.c = carry;                               \
  }                                                    \
  OP_alu (rn, oprand2, regs[rdi].blk);                \
  if (OP_w != false && rdi == 15) {            \
    arm->cpsr_fast.flags.blk = prev_cpsr;                         \
                                                       \
    /* flush pipeline, switch cpu mode */              \
    spsr_to_cpsr (agb);                                \
    if (arm->cpsr_fast.thumb != false) {                        \
      /* FIMXE: internal adjust pc only for arm7 ???*/ \
      const uint32_t pc = regs[15].blk & THUMB_CODE_MASK;  \
      consume  += thumb_flush (agb) + 3;               \
      regs[15].blk = pc;                                   \
    }                                                  \
    else                                               \
      consume  += arm7_flush (agb) + 3;                \
  }                                                    \
} while (0)
  
/* ==================== shift imm5 sign ==================== */
#define LSL_IMM5_SIGN(rm, shift_imm, oprand, carry)\
 do {                                          \
   if (imm5 != 0)                              \
     carry = rm & LSHIFT (1, 32 - shift_imm) ? 0x80 : 0; \
   oprand = rm << shift_imm;                   \
 } while (0)                                
#define LSR_IMM5_SIGN(rm, shift_imm, oprand, carry)\
 do {                                          \
   if (shift_imm == 0) {                       \
     carry = (rm & ARM7_NEG_BIT) ? 0x80 : 0;\
     oprand = 0;                               \
   } else  {                                   \
     carry = (rm & LSHIFT (1, shift_imm - 1)) ? 0x80 : 0; \
     oprand = rm >> shift_imm;                 \
   }                                           \
 } while (0)                                   
#define ASR_IMM5_SIGN(rm, shift_imm, oprand, carry)\
 do {                                          \
   if (shift_imm == 0)   {                     \
     if (rm & LSHIFT (1, 31)) {     \
       oprand = ARM7_UWORD_MAX;                    \
       carry = 0x80;                              \
     } else  {                                 \
       oprand = 0;                             \
       carry = 0;                              \
     }                                         \
   } else  {                                   \
     carry = rm & LSHIFT (1, shift_imm - 1) ? 0x80 : 0; \
     oprand = ((int32_t) rm) >> shift_imm;     \
   }                                           \
 } while (0)                                   
#define ROR_IMM5_SIGN(rm, shift_imm, oprand, carry)                         \
 do {                                                                       \
   if (shift_imm == 0)  {                                                   \
     if (rm & 1)                                                            \
       carry = 0x80;                                                     \
     else                                                                   \
       carry = 0;                                                     \
     oprand = rm >> 1 | (arm->cpsr_fast.flags.sig.c != 0? LSHIFT (1, 31) : 0); \
   } else   {                                                               \
     carry = rm & LSHIFT (1, shift_imm - 1) ? 0x80 : 0;             \
     oprand = rm >> shift_imm | rm << 32 - shift_imm;                       \
   }                                                                        \
 } while (0)                                   
   
#define ISA_ARM7_ALU_SIGN_SHIFT_IMM5(/* alu_out */ OP_alu /* (rn, oprand, output) */\
                                   ,/*void */ shift_imm5_callback /* (rm, shift_imm, oprand) */ \
                                   ,/*void */ shift_imm5_c_callback /* (rm, shift_imm, oprand, carry) */ \
                                   ,/*bool*/ OP_w \
                                   ,/*bool*/ OP_c )  \
do {  \
  const struct arm_alu_shift_imm5 *const m_code = (const struct arm_alu_shift_imm5 *const) & OP_code; \
  const uint32_t imm5 = m_code->shift_imm;             \
  const uint32_t rdi = m_code->rd;                     \
  const uint32_t rn = regs[m_code->rn].blk;                \
  const uint32_t rm = regs[m_code->rm].blk;                \
  const uint32_t prev_cpsr = arm->cpsr_fast.flags.blk;            \
  uint32_t oprand2;                                    \
                                                       \
  if (OP_c != false) {                 \
    shift_imm5_callback (rm, imm5,  oprand2);          \
                                                       \
  } else {                                             \
    uint32_t carry = arm->cpsr_fast.flags.sig.c;                      \
                                                        \
    shift_imm5_c_callback (rm, imm5,  oprand2, carry);  \
                                                       \
    arm->cpsr_fast.flags.sig.c = carry;                               \
  }                                                    \
  OP_alu (rn, oprand2, regs[rdi].blk);                \
  if (OP_w != false && rdi == 15) {            \
    arm->cpsr_fast.flags.blk = prev_cpsr;                         \
                                                       \
    /* flush pipeline, switch cpu mode */              \
    spsr_to_cpsr (agb);                                \
    if (arm->cpsr_fast.thumb != false) {                        \
      /* FIMXE: internal adjust pc only for arm7 ???*/ \
      const uint32_t pc = regs[15].blk & THUMB_CODE_MASK;  \
      consume  += thumb_flush (agb) + 3;               \
      regs[15].blk = pc;                                   \
    }                                                  \
    else                                               \
      consume  += arm7_flush (agb) + 3;                \
  }                                                    \
} while (0)

/* ==================== shift rs sign ==================== */
#define LSL_RS_SIGN(rm, rs_shift , oprand, carry)        \
 do {                                                    \
  const uint32_t rs_0 = rs_shift & 0xFF;                   \
  if (rs_0 == 0)                                           \
    oprand = rm;                                         \
  else if (rs_0 < 32) {                                    \
    carry = rm & LSHIFT (1, 32 - rs_0) ? 0x80 : 0; \
    oprand = rm << rs_0;                                   \
  } else if (rs_0 == 32) {                                 \
    carry = (rm & 1) ? 0x80 : 0;                                      \
    oprand = 0;                                          \
  } else {                                               \
    carry = 0;                                           \
    oprand = 0;                                          \
  }                                                      \
 } while (0)                                 
#define LSR_RS_SIGN(rm, rs_shift, oprand, carry)         \
 do {                                                    \
  const uint32_t rs_0 = rs_shift & 0xFF;                   \
  if (rs_0 == 0)                                           \
    oprand = rm;                                         \
  else if (rs_0 < 32) {                                    \
    carry = rm & LSHIFT (1, rs_0 - 1) ? 0x80 : 0;  \
    oprand = rm >> rs_0;                                   \
  } else if (rs_0 == 32) {                                 \
    carry = rm & LSHIFT (1, 31) ? 0x80 : 0;      \
    oprand = 0;                                          \
  } else {                                               \
    carry = 0;                                           \
    oprand = 0;                                          \
  }                                                      \
 } while (0)                                   
#define ASR_RS_SIGN(rm, rs_shift, oprand, carry)         \
 do {                                                    \
  const uint32_t rs_0 = rs_shift & 0xFF;                   \
  if (rs_0 == 0)                                           \
    oprand = rm;                                         \
  else if (rs_0 < 32) {                                    \
    carry = rm & LSHIFT (1, rs_0 - 1) ? 0x80 : 0;  \
    oprand = (int32_t) rm >> rs_0;                         \
  } else if (rs_0 >= 32) {                                 \
    if (rm & LSHIFT (1, 31)) {                \
      carry = 0x80;                                         \
      oprand = ARM7_UWORD_MAX;                           \
    } else {                                             \
      carry = 0;                                         \
      oprand = 0;                                        \
    }                                                    \
  }                                                      \
 } while (0)                                
#define ROR_RS_SIGN(rm, rs_shift, oprand, carry)         \
 do {                                                    \
  const uint32_t rs_0 = rs_shift & 0xFF;                   \
  if (rs_0 == 0)                                           \
    oprand = rm;                                         \
  else if ( (rs_0 & 31) == 0) {                            \
    carry = rm & LSHIFT (1, 31) ? 0x80 : 0;      \
    oprand = rm;                                         \
  } else {                                               \
    const uint32_t rs_f = rs_0 & 31;                       \
     carry = rm & LSHIFT (1, rs_f - 1) ? 0x80 : 0;\
    oprand = rm >> rs_f | rm << 32 - rs_f;               \
  }                                                      \
 } while (0)                            
 
#define ISA_ARM7_ALU_SIGN_SHIFT_RS(/* alu_out */ OP_alu /* (rn, oprand, output) */\
                                   ,/*void */ shift_rs_callback /* (rm, rs_shift, oprand) */ \
                                   ,/*void */ shift_rs_c_callback /* (rm, rs_shift, oprand, carry) */ \
                                   ,/*bool*/ OP_w \
                                   ,/*bool*/ OP_c )  \
do {  \
  const struct arm_alu_shift_rs *const m_code = (const struct arm_alu_shift_rs *const) & OP_code; \
  const uint32_t rs = regs[m_code->rs].blk8[0];                 \
  const uint32_t rdi = m_code->rd;                     \
  const uint32_t rn = m_code->rn == 15 ? regs[m_code->rn].blk + 4 : regs[m_code->rn].blk;                \
  const uint32_t rm = m_code->rm == 15 ? regs[m_code->rm].blk + 4 : regs[m_code->rm].blk;                \
  const uint32_t prev_cpsr = arm->cpsr_fast.flags.blk;            \
  uint32_t oprand2;                                    \
                                                       \
  if (OP_c != false) {                 \
    shift_rs_callback (rm, rs,  oprand2);              \
                                                       \
  } else {                                             \
    uint32_t carry = arm->cpsr_fast.flags.sig.c;                      \
                                                       \
    shift_rs_c_callback (rm, rs,  oprand2, carry);     \
                                                       \
    arm->cpsr_fast.flags.sig.c = carry;                               \
  }                                                    \
  OP_alu (rn, oprand2, regs[rdi].blk);                \
  if (OP_w != false && rdi == 15) {            \
    arm->cpsr_fast.flags.blk = prev_cpsr;                         \
                                                       \
    /* flush pipeline, switch cpu mode */              \
    spsr_to_cpsr (agb);                                \
    if (arm->cpsr_fast.thumb != false) {                        \
      /* FIMXE: internal adjust pc only for arm7 ???*/ \
      const uint32_t pc = regs[15].blk & THUMB_CODE_MASK;  \
      consume  += thumb_flush (agb) + 3;               \
      regs[15].blk = pc;                                   \
    }                                                  \
    else                                               \
      consume  += arm7_flush (agb) + 3;                \
  }                                                    \
} while (0)
/* ==================== arm7 alu main sign ==================== */
#define OP_ANDS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = rn & oprand; \
   output = result;                     \
   NZ_RESET_FAST (result);              \
} while (0)
  
#define OP_EORS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = rn ^ oprand; \
   output = result;                     \
   NZ_RESET_FAST (result);              \
} while (0)

#define OP_SET_SUB_FLAGS()                                         \
  arm->cpsr_fast.flags.blk = res_ptr[0] & ARM7_NEG_BIT;            \
  if (res_ptr[1] == 0)                                             \
    arm->cpsr_fast.flags.sig.c = 0x80;                                 \
  if (res_ptr[0] == 0)                                             \
    arm->cpsr_fast.flags.sig.z = 0x80;                                 \
  if (((lhs ^ res_ptr[0]) & ARM7_NEG_BIT) && ((lhs ^ rhs) & ARM7_NEG_BIT)) \
    arm->cpsr_fast.flags.sig.v = 0x80
    
#define OP_SUBS(rn, oprand, output)      \
do {                                     \
   const uint32_t lhs = rn;              \
   const uint32_t rhs = oprand;          \
   const uint64_t lhs64 = lhs;           \
   const uint64_t rhs64 = rhs;           \
   const uint64_t result = lhs64 - rhs64;\
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                                                    \
   OP_SET_SUB_FLAGS(); \
                                                           \
   output = res_ptr[0];                                    \
} while (0)
  
#define OP_RSBS(rn, oprand, output)         \
do {                                        \
   const uint32_t lhs = oprand;             \
   const uint32_t rhs = rn;                 \
   const uint64_t lhs64 = lhs;              \
   const uint64_t rhs64 = rhs;              \
   const uint64_t result = lhs64 - rhs64;   \
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                                                    \
   OP_SET_SUB_FLAGS(); \
                                                           \
   output = res_ptr[0];                                    \
} while (0)
  
#define OP_SET_ADD_FLAGS()                                          \
  arm->cpsr_fast.flags.blk = res_ptr[0] & ARM7_NEG_BIT;            \
  if (res_ptr[1] != 0)                                             \
    arm->cpsr_fast.flags.sig.c = 0x80;                                 \
  if (res_ptr[0] == 0)                                             \
    arm->cpsr_fast.flags.sig.z = 0x80;                                 \
  if ((!((lhs ^ rhs) & ARM7_NEG_BIT) && ((lhs ^ res_ptr[0]) & ARM7_NEG_BIT))) \
    arm->cpsr_fast.flags.sig.v = 0x80
                                     
#define OP_ADDS(rn, oprand, output)      \
do {                                     \
   const uint32_t lhs = rn;              \
   const uint32_t rhs = oprand;\
   const uint64_t lhs64 = lhs; \
   const uint64_t rhs64 = rhs; \
   const uint64_t result = lhs64 + rhs64;                           \
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                                                    \
   OP_SET_ADD_FLAGS ();\
                                                                    \
   output = res_ptr[0];                                    \
} while (0)
  
#define OP_ADCS(rn, oprand, output)                                                        \
do {                                                \
   const uint64_t carry = arm->cpsr_fast.flags.sig.c != 0 ? 1 : 0; \
   const uint32_t lhs = rn;                         \
   const uint32_t rhs = oprand;                     \
   const uint64_t lhs64 = lhs;                      \
   const uint64_t rhs64 = rhs;                      \
   const uint64_t result = lhs64 + rhs64 + carry;   \
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                         \
   OP_SET_ADD_FLAGS ();\
                                      \
   output = res_ptr[0];               \
} while (0)
  
#define OP_SBCS(rn, oprand, output)      \
do {                                     \
   const uint64_t carry = arm->cpsr_fast.flags.sig.c != 0 ? 0 : 1; \
   const uint32_t lhs = rn;    \
   const uint32_t rhs = oprand;\
   const uint64_t lhs64 = lhs; \
   const uint64_t rhs64 = rhs; \
   const uint64_t result = lhs64 - rhs64 - carry;   \
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                         \
   OP_SET_SUB_FLAGS ();\
                                         \
   output = res_ptr[0];                  \
} while (0)
  
#define OP_RSCS(rn, oprand, output)                 \
do {                                                \
   const uint64_t carry = arm->cpsr_fast.flags.sig.c != 0 ? 0 : 1; \
   const uint32_t lhs = oprand;                     \
   const uint32_t rhs = rn;                         \
   const uint64_t lhs64 = lhs;                      \
   const uint64_t rhs64 = rhs;                      \
   const uint64_t result = lhs64 - rhs64 - carry;   \
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                                    \
   OP_SET_SUB_FLAGS ();\
                                        \
   output = res_ptr[0];                 \
} while (0)
  
#define OP_TSTS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = rn & oprand; \
   NZ_RESET_FAST (result);              \
} while (0)
  
#define OP_TEQS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = rn ^ oprand; \
   NZ_RESET_FAST (result);              \
} while (0)
  
#define OP_CMPS(rn, oprand, output)      \
do {                                     \
   const uint32_t lhs = rn;              \
   const uint32_t rhs = oprand;          \
   const uint64_t lhs64 = lhs;           \
   const uint64_t rhs64 = rhs;           \
   const uint64_t result = lhs64 - rhs64;\
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                                                    \
   OP_SET_SUB_FLAGS ();\
} while (0)
  
#define OP_CMNS(rn, oprand, output)      \
do {                                     \
   const uint32_t lhs = rn;              \
   const uint32_t rhs = oprand;\
   const uint64_t lhs64 = lhs; \
   const uint64_t rhs64 = rhs; \
   const uint64_t result = lhs64 + rhs64;                           \
   const uint32_t *const res_ptr = (const uint32_t *const) & result;\
                                                                    \
   OP_SET_ADD_FLAGS ();\
} while (0)
  
#define OP_ORRS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = rn | oprand; \
   output = result;                     \
   NZ_RESET_FAST (result);              \
} while (0)
  
#define OP_MOVS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = oprand;      \
   output = result;                     \
   NZ_RESET_FAST (result);              \
} while (0)
  
#define OP_BICS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = rn & ~oprand;\
   output = result;                     \
   NZ_RESET_FAST (result);              \
} while (0)
  
#define OP_MVNS(rn, oprand, output)     \
do {                                    \
   const uint32_t result = ~oprand;     \
   output = result;                     \
   NZ_RESET_FAST (result);              \
} while (0)

/* ==================== arm7 memory access ==================== */
    
#define LDRB(rnAddress, output)                        \
  do {                                                       \
    consume += agb_mbus_rb (agb, rnAddress, & output.blk8[0], false);  \
    output.blk = output.blk8[0];                                           \
  } while (0)
    
#define LDRSB(rnAddress, output)                       \
  do {                                                       \
    consume += agb_mbus_rb (agb, rnAddress, & output.blk8[0], false);  \
    output.sblk = output.sblk8[0];             \
  } while (0)
    
#define LDRH(rnAddress, output)                       \
  do {                                                      \
    consume += agb_mbus_rhw (agb, rnAddress, & output.blk16[0], false);  \
    output.blk = output.blk16[0];   \
                                                            \
    if (rnAddress & 1)                                      \
      output.blk = output.blk >> 8 | output.blk << 24;              \
  } while (0)
    
#define LDRSH(rnAddress, output)                      \
  do {                                                      \
    consume += agb_mbus_rhw (agb, rnAddress, & output.blk16[0], false);  \
 \
    if (rnAddress & 1)                                      \
      output.sblk = output.sblk8[1];   \
    else             \
      output.sblk = output.sblk16[0];   \
  } while (0)
    
#define LDR(rnAddress, output)                       \
  do {                                                     \
    const uint32_t rot = (rnAddress & 3) << 3;             \
    consume += agb_mbus_rw (agb, rnAddress, & output.blk, false);\
    if (rot != 0)                                          \
      output.blk = output.blk >> rot| output.blk << 32 - rot;              \
  } while (0)
    
#define STRB(rnAddress, write)                      \
  do {                                                    \
    consume += agb_mbus_wb (agb, rnAddress, write, false); \
  } while (0)
    
#define STRH(rnAddress, write)                      \
  do {                                                    \
    consume += agb_mbus_whw (agb, rnAddress, write, false);\
  } while (0)
    
#define STR(rnAddress, write)                       \
  do {                                                    \
    consume += agb_mbus_ww (agb, rnAddress, write, false); \
  } while (0)

#define THUMB_LDR_ALLB(LDRc, rdi, rni, offset) \
 ARM7_LDR_ALL_B(LDRc, rdi, rni, 1, 1, offset, 0)


#define ARM7_LDR_ALL_B(LDRc, rdi, rni, OP_pre, OP_post, offset, OP_w) \
 do {                                                       \
   if (OP_w != false) {                        \
     uint32_t address;                                      \
     uint32_t rn_w;                                  \
                                                            \
     if (OP_post != false)                            \
       rn_w = regs[rni].blk + offset;                    \
     else                                                   \
       rn_w = regs[rni].blk - offset;                    \
                                                            \
     if (OP_pre != false)                            \
       address = rn_w;                               \
     else                                                   \
       address = regs[rni].blk;                                 \
                                                            \
     LDRc (address, regs[rdi]);                     \
                                                            \
     if (rdi != rni)                                        \
       regs[rni].blk = rn_w;                             \
   } else {                                                 \
     uint32_t address;                                      \
                                                            \
     if (OP_pre != false)                            \
       if (OP_post != false)                          \
         address = regs[rni].blk + offset;                      \
       else                                                 \
         address = regs[rni].blk - offset;                      \
     else                                                   \
       address = regs[rni].blk;                                 \
                                                          \
     LDRc (address, regs[rdi]);                     \
   }                                                        \
   if (rdi == 15) {                                       \
        /* r15 load only for arm7 mode */                 \
       regs[15].blk &= ARM7_CODE_MASK;                        \
       consume += arm7_flush (agb) + 2;                   \
   }                                                      \
 } while (0)

#define THUMB_STR_ALLB(STRc, rdi, rni, offset) \
 ARM7_STR_ALL_B(STRc, rdi, rni, 1, 1, offset, 0)
  
#define ARM7_STR_ALL_B(STRc, rdi, rni, OP_pre, OP_post, offset, OP_w) \
 do {                                                       \
   uint32_t rd_v = rdi == 15 ? regs[rdi].blk + 4 : regs[rdi].blk;\
   if (OP_w != false) {                        \
     uint32_t address;                                      \
     uint32_t rn_w;                                  \
                                                            \
     if (OP_post != false)                            \
       rn_w = regs[rni].blk + offset;                    \
     else                                                   \
       rn_w = regs[rni].blk - offset;                    \
                                                            \
     if (OP_pre != false)                            \
       address = rn_w;                               \
     else                                                   \
       address = regs[rni].blk;                                 \
                                                            \
     STRc (address, rd_v);                          \
                                                            \
     regs[rni].blk = rn_w;                               \
   } else {                                                 \
     uint32_t address;                                      \
                                                            \
     if (OP_pre != false)                            \
       if (OP_post != false)                          \
         address = regs[rni].blk + offset;                      \
       else                                                 \
         address = regs[rni].blk - offset;                      \
     else                                                   \
       address = regs[rni].blk;                                 \
     STRc (address, rd_v);                          \
   }                                                        \
 } while (0)

finline void
isa_arm7_store_spsr_base (struct gba *const agb, 
                                  const uint32_t c,
                                  const uint32_t f,   const uint32_t value)
{
  const struct psr *const nw_psr = (const struct psr *const )& value;
  struct psr *const spsr = arm7_cur_spsr (agb);
  
  if (f != 0) {
    spsr->n = nw_psr->n;
    spsr->z = nw_psr->z;
    spsr->c = nw_psr->c;
    spsr->v = nw_psr->v;
  }
  if (c != 0) {
    spsr->mode = nw_psr->mode; 
    spsr->irq = nw_psr->irq;
    spsr->fiq = nw_psr->fiq;
    spsr->thumb = nw_psr->thumb;
    spsr->mode |= 0x10;
  }
}

dinline void
isa_arm7_store_cpsr_base (struct gba *const agb, 
                                  const uint32_t c,
                                  const uint32_t f,   const uint32_t value)
{
  const struct psr *const nw_psr = (const struct psr *const ) & value;
  struct psr org_psr;

  cpsr_to_std (& agb->arm7.cpsr, & agb->arm7.cpsr_fast);
  org_psr.blk = agb->arm7.cpsr.blk;

  if (arm7_privilege_mode (agb) != false) {
    if (c == 0) {
      if (f != 0) {
        agb->arm7.cpsr.n = nw_psr->n;
        agb->arm7.cpsr.z = nw_psr->z;
        agb->arm7.cpsr.c = nw_psr->c;
        agb->arm7.cpsr.v = nw_psr->v;
      }
    } else {
      const uint32_t nw_mode = nw_psr->mode & 0x0f;
      const uint32_t org_mode = agb->arm7.cpsr.mode & 0x0f;
      
      if (org_mode != nw_mode) {
        /* will swtch mode */
        bank_swi (agb, nw_mode);
      }
        
      agb->arm7.cpsr.mode = nw_psr->mode | 0x10; /* or mode mask */
      agb->arm7.cpsr.irq = nw_psr->irq;
      agb->arm7.cpsr.fiq = nw_psr->fiq;
      agb->arm7.cpsr.thumb = 0;

      if (f != 0) {
        agb->arm7.cpsr.n = nw_psr->n;
        agb->arm7.cpsr.z = nw_psr->z;
        agb->arm7.cpsr.c = nw_psr->c;
        agb->arm7.cpsr.v = nw_psr->v;
      } else {
        agb->arm7.cpsr.n = org_psr.n;
        agb->arm7.cpsr.z = org_psr.z;
        agb->arm7.cpsr.c = org_psr.c;
        agb->arm7.cpsr.v = org_psr.v;
      }
    }
  } else {
    /* in user mode, only write field_f */
    if (f != 0) {
      agb->arm7.cpsr.n = nw_psr->n;
      agb->arm7.cpsr.z = nw_psr->z;
      agb->arm7.cpsr.c = nw_psr->c;
      agb->arm7.cpsr.v = nw_psr->v;
    }
  }
  cpsr_to_fast (& agb->arm7.cpsr, & agb->arm7.cpsr_fast);
  reset_int_magic (agb); 
}

finline 
int32_t isa_thumb_ldmia (struct gba *const agb, 
                                  const uint16_t opcode) {
                                  
   const uint32_t rni = opcode >> 8 & 7;     
   const uint32_t reg_list = opcode & 0xFF;
         uint32_t *const regs = & agb->arm7.regs[0].blk;
         uint32_t rn_poll = regs[rni];
   uint32_t wait_state = 0;

   if (reg_list & 1 << 0) { wait_state += agb_stack_rw (agb, rn_poll, & regs[0], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 1) { wait_state += agb_stack_rw (agb, rn_poll, & regs[1], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 2) { wait_state += agb_stack_rw (agb, rn_poll, & regs[2], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 3) { wait_state += agb_stack_rw (agb, rn_poll, & regs[3], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 4) { wait_state += agb_stack_rw (agb, rn_poll, & regs[4], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 5) { wait_state += agb_stack_rw (agb, rn_poll, & regs[5], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 6) { wait_state += agb_stack_rw (agb, rn_poll, & regs[6], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 7) { wait_state += agb_stack_rw (agb, rn_poll, & regs[7], true) + 1; rn_poll += 4; }

   if ((reg_list & 1 << rni) == 0)
     regs[rni] = rn_poll;
   
   return wait_state + 2;
}

finline 
int32_t isa_thumb_pop (struct gba *const agb, 
                                  const uint16_t opcode) {
                                       
   const uint32_t reg_list = opcode & 0x1FF;
         uint32_t *const regs = & agb->arm7.regs[0].blk;
         uint32_t rn_poll = regs[13];
   uint32_t wait_state = 0;

   if (reg_list & 1 << 0) { wait_state += agb_stack_rw (agb, rn_poll, & regs[0], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 1) { wait_state += agb_stack_rw (agb, rn_poll, & regs[1], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 2) { wait_state += agb_stack_rw (agb, rn_poll, & regs[2], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 3) { wait_state += agb_stack_rw (agb, rn_poll, & regs[3], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 4) { wait_state += agb_stack_rw (agb, rn_poll, & regs[4], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 5) { wait_state += agb_stack_rw (agb, rn_poll, & regs[5], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 6) { wait_state += agb_stack_rw (agb, rn_poll, & regs[6], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 7) { wait_state += agb_stack_rw (agb, rn_poll, & regs[7], true) + 1; rn_poll += 4; }
   if (reg_list & 1 << 8) { wait_state += agb_stack_rw (agb, rn_poll, & regs[15], true);
                            regs[15] &= THUMB_CODE_MASK;
                            wait_state += thumb_flush(agb) + 1; 
                            rn_poll += 4; 
                          }  
   
   regs[13] = rn_poll;
   return wait_state + 2;
}

finline /* push db stack type */
int32_t isa_thumb_push (struct gba *const agb, 
                                  const uint16_t opcode) {
    
   const uint32_t vcount_l[16] = { 0, 1, 1, 2,   
                                1, 2, 2, 3,
                                1, 2, 2, 3,
                                2, 3, 3, 4 };
   const uint32_t reg_list = opcode & 0x1FF;
   const uint32_t vcount = vcount_l[reg_list >> 0 & 15]
                         + vcount_l[reg_list >> 4 & 15]
                         + vcount_l[reg_list >> 8 & 15]
                         + vcount_l[reg_list >>12 & 15];
         uint32_t rn_poll;
         uint32_t rn_wb;
         uint32_t *const regs = & agb->arm7.regs[0].blk;
   uint32_t wait_state = 0;
   
   rn_wb =     regs[13] - vcount * 4;
   rn_poll = rn_wb;
     
   if (reg_list & 1 << 0) { wait_state += agb_stack_ww (agb, rn_poll, regs[0], true, 0) + 1; rn_poll += 4; }
   if (reg_list & 1 << 1) { wait_state += agb_stack_ww (agb, rn_poll, regs[1], true, 1 ) + 1; rn_poll += 4; }
   if (reg_list & 1 << 2) { wait_state += agb_stack_ww (agb, rn_poll, regs[2], true, 2) + 1; rn_poll += 4; }
   if (reg_list & 1 << 3) { wait_state += agb_stack_ww (agb, rn_poll, regs[3], true, 3) + 1; rn_poll += 4; }
   if (reg_list & 1 << 4) { wait_state += agb_stack_ww (agb, rn_poll, regs[4], true, 4) + 1; rn_poll += 4; }
   if (reg_list & 1 << 5) { wait_state += agb_stack_ww (agb, rn_poll, regs[5], true, 5) + 1; rn_poll += 4; }
   if (reg_list & 1 << 6) { wait_state += agb_stack_ww (agb, rn_poll, regs[6], true, 6) + 1; rn_poll += 4; }
   if (reg_list & 1 << 7) { wait_state += agb_stack_ww (agb, rn_poll, regs[7], true, 7) + 1; rn_poll += 4; }
   if (reg_list & 1 << 8) { wait_state += agb_stack_ww (agb, rn_poll, regs[14], true, 14) + 1; rn_poll += 4; }  

   regs[13] = rn_wb;
   return wait_state + 1;
}

finline 
int32_t isa_thumb_stmia (struct gba *const agb, 
                                  const uint16_t opcode) {
                                  
   const uint32_t rni = opcode >> 8 & 7;     
   const uint32_t reg_list = opcode & 0xFF;
         uint32_t *const regs = & agb->arm7.regs[0].blk;
         uint32_t rn_poll = regs[rni];
   uint32_t wait_state = 0;

   if (reg_list & 1 << 0) { wait_state += agb_stack_ww (agb, rn_poll, regs[0], true, 0) + 1; rn_poll += 4; }
   if (reg_list & 1 << 1) { wait_state += agb_stack_ww (agb, rn_poll, regs[1], true, 1) + 1; rn_poll += 4; }
   if (reg_list & 1 << 2) { wait_state += agb_stack_ww (agb, rn_poll, regs[2], true, 2) + 1; rn_poll += 4; }
   if (reg_list & 1 << 3) { wait_state += agb_stack_ww (agb, rn_poll, regs[3], true, 3) + 1; rn_poll += 4; }
   if (reg_list & 1 << 4) { wait_state += agb_stack_ww (agb, rn_poll, regs[4], true, 4) + 1; rn_poll += 4; }
   if (reg_list & 1 << 5) { wait_state += agb_stack_ww (agb, rn_poll, regs[5], true, 5) + 1; rn_poll += 4; }
   if (reg_list & 1 << 6) { wait_state += agb_stack_ww (agb, rn_poll, regs[6], true, 6) + 1; rn_poll += 4; }
   if (reg_list & 1 << 7) { wait_state += agb_stack_ww (agb, rn_poll, regs[7], true, 7) + 1; rn_poll += 4; }

   regs[rni] = rn_poll;
   return wait_state + 1;
}

dinline 
int32_t isa_arm7_list_memory_access_load (struct gba *const agb, 
                                  const uint32_t opcode,
                                  const uint32_t p,
                                  const uint32_t u,
                                  const uint32_t s,
                                  const uint32_t w) {
                                  
   const uint32_t vcount_l[16] = { 0, 1, 1, 2,   
                                1, 2, 2, 3,
                                1, 2, 2, 3,
                                2, 3, 3, 4 };
     
   const uint32_t rni = opcode >> 16 & 15;     
   const uint32_t reg_list = opcode & 0xFFFF;
   const uint32_t vcount = vcount_l[reg_list >> 0 & 15]
                         + vcount_l[reg_list >> 4 & 15]
                         + vcount_l[reg_list >> 8 & 15]
                         + vcount_l[reg_list >>12 & 15];
         uint32_t rn_poll;
         uint32_t rn_wb;
         uint32_t *const regs = & agb->arm7.regs[0].blk;
         struct arm7 *const arm = & agb->arm7;
   const uint32_t arm7_mode = arm->cpsr_fast.mode & 0x0F;
   const struct arm_list_memory_access *const m_code = (const struct arm_list_memory_access *const) & opcode;
   uint32_t wait_state = 0;
   
   if (u != 0) {
     if (p != 0)
       rn_poll = regs[rni]  + 4;
     else 
       rn_poll = regs[rni];
     rn_wb =     regs[rni] + vcount * 4;
   } else {
     rn_wb =     regs[rni] - vcount * 4;
     
     if (p != 0)
       rn_poll = rn_wb;
     else 
       rn_poll = rn_wb + 4;
   }
   if (m_code->r0) { wait_state += agb_stack_rw (agb, rn_poll, & regs[0], true) + 1; rn_poll += 4; }
   if (m_code->r1) { wait_state += agb_stack_rw (agb, rn_poll, & regs[1], true) + 1; rn_poll += 4; }
   if (m_code->r2) { wait_state += agb_stack_rw (agb, rn_poll, & regs[2], true) + 1; rn_poll += 4; }
   if (m_code->r3) { wait_state += agb_stack_rw (agb, rn_poll, & regs[3], true) + 1; rn_poll += 4; }
   if (m_code->r4) { wait_state += agb_stack_rw (agb, rn_poll, & regs[4], true) + 1; rn_poll += 4; }
   if (m_code->r5) { wait_state += agb_stack_rw (agb, rn_poll, & regs[5], true) + 1; rn_poll += 4; }
   if (m_code->r6) { wait_state += agb_stack_rw (agb, rn_poll, & regs[6], true) + 1; rn_poll += 4; }
   if (m_code->r7) { wait_state += agb_stack_rw (agb, rn_poll, & regs[7], true) + 1; rn_poll += 4; }
       
   if (  s != 0 
     && (reg_list & 0x8000) == 0
     && (arm7_mode != ARM7_MODE_USER && arm7_mode != ARM7_MODE_SYS)) {
     /* copy user mode register */
     if (arm7_mode != ARM7_MODE_FIQ) {
       /* not fiq mode */
       if (m_code->r8) { wait_state += agb_stack_rw (agb, rn_poll, & regs[8], true) + 1; rn_poll += 4; }
       if (m_code->r9) { wait_state += agb_stack_rw (agb, rn_poll, & regs[9], true) + 1; rn_poll += 4; }
       if (m_code->r10) { wait_state += agb_stack_rw (agb, rn_poll, & regs[10], true) + 1; rn_poll += 4; }
       if (m_code->r11) { wait_state += agb_stack_rw (agb, rn_poll, & regs[11], true) + 1; rn_poll += 4; }
       if (m_code->r12) { wait_state += agb_stack_rw (agb, rn_poll, & regs[12], true) + 1; rn_poll += 4; }
       if (m_code->r13) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r1314_t[R1314b_SYSUSER+0].blk, true) + 1; rn_poll += 4; }
       if (m_code->r14) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r1314_t[R1314b_SYSUSER+1].blk, true) + 1; rn_poll += 4; }
       if (m_code->r15) { wait_state += agb_stack_rw (agb, rn_poll, & regs[15], true) + 1; rn_poll += 4; }
     } else {
       /* fiq mode */
       if (m_code->r8) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r812_t[R812b_EXCEPT_FIQ+0].blk, true) + 1; rn_poll += 4; }
       if (m_code->r9) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r812_t[R812b_EXCEPT_FIQ+1].blk, true) + 1; rn_poll += 4; }
       if (m_code->r10) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r812_t[R812b_EXCEPT_FIQ+2].blk, true) + 1; rn_poll += 4; }
       if (m_code->r11) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r812_t[R812b_EXCEPT_FIQ+3].blk, true) + 1; rn_poll += 4; }
       if (m_code->r12) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r812_t[R812b_EXCEPT_FIQ+4].blk, true) + 1; rn_poll += 4; }
       if (m_code->r13) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r1314_t[R1314b_SYSUSER+0].blk, true) + 1; rn_poll += 4; }
       if (m_code->r14) { wait_state += agb_stack_rw (agb, rn_poll, & arm->r1314_t[R1314b_SYSUSER+1].blk, true) + 1; rn_poll += 4; }
       if (m_code->r15) { wait_state += agb_stack_rw (agb, rn_poll, & regs[15], true) + 1; rn_poll += 4; }
     } 
   } else {
     /* normal copy (maybe with spsr to cpsr) */
     if (m_code->r8) { wait_state += agb_stack_rw (agb, rn_poll, & regs[8], true) + 1; rn_poll += 4; }
     if (m_code->r9) { wait_state += agb_stack_rw (agb, rn_poll, & regs[9], true) + 1; rn_poll += 4; }
     if (m_code->r10) { wait_state += agb_stack_rw (agb, rn_poll, & regs[10], true) + 1; rn_poll += 4; }
     if (m_code->r11) { wait_state += agb_stack_rw (agb, rn_poll, & regs[11], true) + 1; rn_poll += 4; }
     if (m_code->r12) { wait_state += agb_stack_rw (agb, rn_poll, & regs[12], true) + 1; rn_poll += 4; }
     if (m_code->r13) { wait_state += agb_stack_rw (agb, rn_poll, & regs[13], true) + 1; rn_poll += 4; }
     if (m_code->r14) { wait_state += agb_stack_rw (agb, rn_poll, & regs[14], true) + 1; rn_poll += 4; }
     if (m_code->r15) { wait_state += agb_stack_rw (agb, rn_poll, & regs[15], true) + 1; rn_poll += 4; }
   }
   if (w != 0 && (reg_list & 1 << rni) == 0)
     regs[rni] = rn_wb;
   
   if (s != 0 && (reg_list & 0x8000) != 0) {
     spsr_to_cpsr (agb);
   }
   if (opcode & 0x8000) {
     if (arm->cpsr_fast.thumb != false) {
       const uint32_t pc = regs[15] & THUMB_CODE_MASK;
       regs[15] &= THUMB_CODE_MASK;
       wait_state += thumb_flush (agb) + 3;
       regs[15] = pc;
     } else {
       wait_state += arm7_flush (agb) + 3;
       regs[15] &= ARM7_CODE_MASK;
     }
   }
 /*if ( (reg_list & 1 << rni) == 0)
        regs[rni] &= ARM7_CODE_MASK; 
  */
   return wait_state;
}

dinline 
int32_t isa_arm7_list_memory_access_store (struct gba *const agb, 
                                  const uint32_t opcode,
                                  const uint32_t p,
                                  const uint32_t u,
                                  const uint32_t s,
                                  const uint32_t w) {
                                  
   const uint32_t vcount_l[16] = { 0, 1, 1, 2,   
                                1, 2, 2, 3,
                                1, 2, 2, 3,
                                2, 3, 3, 4 };
     
   const uint32_t rni = opcode >> 16 & 15;     
   const uint32_t reg_list = opcode & 0xFFFF;
   const uint32_t vcount = vcount_l[reg_list >> 0 & 15]
                         + vcount_l[reg_list >> 4 & 15]
                         + vcount_l[reg_list >> 8 & 15]
                         + vcount_l[reg_list >>12 & 15];
         uint32_t rn_poll;
         uint32_t rn_wb;
         union iopad_blk32 *const regs = & agb->arm7.regs[0];
         struct arm7 *const arm = & agb->arm7;
   const uint32_t arm7_mode = arm->cpsr_fast.mode & 0x0F;
   const struct arm_list_memory_access *const m_code = (const struct arm_list_memory_access *const) & opcode;
   uint32_t wait_state = 0;
   
   if (u != 0) {
     if (p != 0)
       rn_poll = regs[rni].blk  + 4;
     else 
       rn_poll = regs[rni].blk;
     rn_wb =     regs[rni].blk + vcount * 4;
   } else {
     rn_wb =     regs[rni].blk - vcount * 4;
     
     if (p != 0)
       rn_poll = rn_wb;
     else 
       rn_poll = rn_wb + 4;
   }
   if (m_code->r0) { wait_state += agb_stack_ww (agb, rn_poll, regs[0].blk, true, 0) + 1; rn_poll += 4; }
   if (m_code->r1) { wait_state += agb_stack_ww (agb, rn_poll, regs[1].blk, true, 1) + 1; rn_poll += 4; }
   if (m_code->r2) { wait_state += agb_stack_ww (agb, rn_poll, regs[2].blk, true, 2) + 1; rn_poll += 4; }
   if (m_code->r3) { wait_state += agb_stack_ww (agb, rn_poll, regs[3].blk, true, 3) + 1; rn_poll += 4; }
   if (m_code->r4) { wait_state += agb_stack_ww (agb, rn_poll, regs[4].blk, true, 4) + 1; rn_poll += 4; }
   if (m_code->r5) { wait_state += agb_stack_ww (agb, rn_poll, regs[5].blk, true, 5) + 1; rn_poll += 4; }
   if (m_code->r6) { wait_state += agb_stack_ww (agb, rn_poll, regs[6].blk, true, 6) + 1; rn_poll += 4; }
   if (m_code->r7) { wait_state += agb_stack_ww (agb, rn_poll, regs[7].blk, true, 7) + 1; rn_poll += 4; }
       
   if (  s != 0 
     && (arm7_mode != ARM7_MODE_USER && arm7_mode != ARM7_MODE_SYS)) {
     /* copy user mode register */
     if (arm7_mode != ARM7_MODE_FIQ) {
       /* not fiq mode */
       if (m_code->r8) { wait_state += agb_stack_ww (agb, rn_poll, regs[8].blk, true, 8) + 1; rn_poll += 4; }
       if (m_code->r9) { wait_state += agb_stack_ww (agb, rn_poll, regs[9].blk, true, 9) + 1; rn_poll += 4; }
       if (m_code->r10) { wait_state += agb_stack_ww (agb, rn_poll, regs[10].blk, true, 10) + 1; rn_poll += 4; }
       if (m_code->r11) { wait_state += agb_stack_ww (agb, rn_poll, regs[11].blk, true, 11) + 1; rn_poll += 4; }
       if (m_code->r12) { wait_state += agb_stack_ww (agb, rn_poll, regs[12].blk, true, 12) + 1; rn_poll += 4; }
       if (m_code->r13) { wait_state += agb_stack_ww (agb, rn_poll, arm->r1314_t[R1314b_SYSUSER+0].blk, true, 13) + 1; rn_poll += 4; }
       if (m_code->r14) { wait_state += agb_stack_ww (agb, rn_poll, arm->r1314_t[R1314b_SYSUSER+1].blk, true, 14) + 1; rn_poll += 4; }
     } else {
       /* fiq mode */
       if (m_code->r8) { wait_state += agb_stack_ww (agb, rn_poll, arm->r812_t[R812b_EXCEPT_FIQ+0].blk, true, 8) + 1; rn_poll += 4; }
       if (m_code->r9) { wait_state += agb_stack_ww (agb, rn_poll, arm->r812_t[R812b_EXCEPT_FIQ+1].blk, true, 9) + 1; rn_poll += 4; }
       if (m_code->r10) { wait_state += agb_stack_ww (agb, rn_poll, arm->r812_t[R812b_EXCEPT_FIQ+2].blk, true, 10) + 1; rn_poll += 4; }
       if (m_code->r11) { wait_state += agb_stack_ww (agb, rn_poll, arm->r812_t[R812b_EXCEPT_FIQ+3].blk, true, 11) + 1; rn_poll += 4; }
       if (m_code->r12) { wait_state += agb_stack_ww (agb, rn_poll, arm->r812_t[R812b_EXCEPT_FIQ+4].blk, true, 12) + 1; rn_poll += 4; }
       if (m_code->r13) { wait_state += agb_stack_ww (agb, rn_poll, arm->r1314_t[R1314b_SYSUSER+0].blk, true, 13) + 1; rn_poll += 4; }
       if (m_code->r14) { wait_state += agb_stack_ww (agb, rn_poll, arm->r1314_t[R1314b_SYSUSER+1].blk, true, 14) + 1; rn_poll += 4; }
     } 
   } else {
     /* normal copy (maybe with spsr to cpsr) */
     if (m_code->r8) { wait_state += agb_stack_ww (agb, rn_poll, regs[8].blk, true, 8) + 1; rn_poll += 4; }
     if (m_code->r9) { wait_state += agb_stack_ww (agb, rn_poll, regs[9].blk, true, 9) + 1; rn_poll += 4; }
     if (m_code->r10) { wait_state += agb_stack_ww (agb, rn_poll, regs[10].blk, true, 10) + 1; rn_poll += 4; }
     if (m_code->r11) { wait_state += agb_stack_ww (agb, rn_poll, regs[11].blk, true, 11) + 1; rn_poll += 4; }
     if (m_code->r12) { wait_state += agb_stack_ww (agb, rn_poll, regs[12].blk, true, 12) + 1; rn_poll += 4; }
     if (m_code->r13) { wait_state += agb_stack_ww (agb, rn_poll, regs[13].blk, true, 13) + 1; rn_poll += 4; }
     if (m_code->r14) { wait_state += agb_stack_ww (agb, rn_poll, regs[14].blk, true, 14) + 1; rn_poll += 4; }
   }
   if (m_code->r15) { wait_state += agb_stack_ww (agb, rn_poll, regs[15].blk + 4, true, 15) + 1; rn_poll += 4; }
   if (w != 0)
     regs[rni].blk = rn_wb;
   /* regs[rni] &= ARM7_CODE_MASK; */
   return wait_state;
}

finline
uint32_t cpu_tick (struct gba *agb) {

  int consume;
#if 0
#define DUMP_REGS_STACK_NUMS 16 

  static uint32_t od_pc = 0;
  static struct arm7 arm_cache[DUMP_REGS_STACK_NUMS];
  static int dump_cnt = 0;
  static kable show =false;

  struct arm7 *const arm = & agb->arm7;
  uint32_t *const regs = & arm->regs[0];
  uint32_t pc;
  uint32_t bank;

  if (arm->cpsr_fast.thumb != false) {
    pc = regs[15] - 4 & -2;
  } else {
    pc = regs[15] - 8 & -4;
  }


  //if (pc == 0x8015944)
  //  __asm int 3
    //  if ( (arm->ie.blk & arm->ifs.blk & 1)
    //    && (arm->ime.blk != 0) 
    //    && (arm->cpsr.irq == 0) ) {
    //      printf ("VBL IRQ_ start...\n");
    //  }
   //   if ( (arm->ie.blk & arm->ifs.blk & 8)
    //    && (arm->ime.blk != 0) 
   //     && (arm->cpsr.irq == 0) ) {
     //     printf ("timing IRQ_ start...\n");
     //     if (show == false)
            
  //  show = true;
   // __asm int 3
   //   }
  if (pc == 0x3002804 && 
    agb->mem.IRam[0x2800] == 0x78) {
    show = true;
   // _asm int 3
  }
  // if (pc == 8)
   // __asm int 3

  // if (regs[13] == 0x3002208
  //  && regs[15] == 0x3002178)
  //  __asm int 3


  memcpy (& arm_cache[dump_cnt], arm, sizeof (struct arm7));
  arm_cache[dump_cnt].regs[15].blk = pc;

  dump_cnt++;
  dump_cnt%= DUMP_REGS_STACK_NUMS;


 // if ( (arm->cpsr.mode & 0x10) == 0)
  //  __asm int 3
  
 // if (show != 0)
   // printf ("code:%07X mode:%d\n", pc, arm->cpsr.thumb);
  //if (pc == 0x80000C8)
  //  __asm int 3
  // if (regs[13] <= 0x3007000)
  //  __asm int 3
  //if ( show)
  //printf ("code:%07x r0:%07x r1:%07x r3:%07x r14:%07x\n", pc, regs[0], regs[1], regs[3], regs[14]);


#endif 

  do {
    struct arm7 *const arm = & agb->arm7;
    union iopad_blk32 *const regs = & arm->regs[0];

    /* check interrupt */
    if (arm->cpsr_fast.thumb != 0) {
      if (arm->int_magic.blk == 0 ) {
          
        uint32_t pipeline_pc = regs[15].blk;

        cpsr_to_std (& arm->cpsr, & arm->cpsr_fast);

        pipeline_pc &= THUMB_CODE_MASK;
        pipeline_pc -= 4;

        bank_swi (agb, ARM7_MODE_IRQ);

        arm->r1314_t[1+R1314b_IRQ].blk = pipeline_pc  + 4;
        arm->spsr_t[SPSRb_IRQ].blk =  arm->cpsr.blk;
        
        arm->cpsr_fast.mode = ARM7_MODE_IRQ | ARM7_MODE_HSET_MASK;
        arm->cpsr_fast.thumb = false;
        arm->cpsr_fast.irq_disable = true;

        arm->regs[14].blk = pipeline_pc  + 4;
        arm->regs[15].blk = 0x18;

        consume = arm7_flush (agb) + 7;
        arm->regs[15].blk = 0x20 - 2;  

        reset_int_magic (agb);
      } else {
        const uint16_t OP_code = arm->opcode[0] & 0xFFFF;
        const uint32_t OP_throat = OP_code >> 6;

        arm->opcode[0] = arm->opcode[1];
        consume =  thumb_fecth_n (agb, regs[15].blk, (uint16_t *const)& arm->opcode[1]);
        regs[15].blk &= THUMB_CODE_MASK;

        switch (OP_throat) {
          
        /* ===================== thumb :: and rd, rm =========================== */
        case 0x100: 
          OP_ANDS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        /* ===================== thumb :: eor rd, rm =========================== */
        case 0x101:
          OP_EORS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        /* ===================== thumb :: lsl rd, rs =========================== */
        case 0x102:
          LSL_RS_SIGN (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk, arm->cpsr_fast.flags.sig.c);
          
          NZ_RESET_FAST (regs[OP_code & 7].blk);
          consume += 2;      
          break;
          
        /* ===================== thumb :: lsr rd, rs =========================== */
        case 0x103:
          LSR_RS_SIGN (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk, arm->cpsr_fast.flags.sig.c);
          
          NZ_RESET_FAST (regs[OP_code & 7].blk);
          consume += 2;      
          break;
          
        /* ===================== thumb :: asr rd, rs =========================== */ 
        case 0x104:
          ASR_RS_SIGN (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk, arm->cpsr_fast.flags.sig.c);
          
          NZ_RESET_FAST (regs[OP_code & 7].blk);
          consume += 2;      
          break;
          
        case 0x105:
          OP_ADCS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        case 0x106:
          OP_SBCS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        /* ===================== thumb :: ror rd, rs =========================== */ 
        case 0x107:
          ROR_RS_SIGN (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk, arm->cpsr_fast.flags.sig.c);
          
          NZ_RESET_FAST (regs[OP_code & 7].blk);
          consume += 2;    
          break;
          
        /* ===================== thumb :: tst rn, rm =========================== */
        case 0x108:
          OP_TSTS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        case 0x109:
          OP_SUBS (0, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
        case 0x10A:
          OP_CMPS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
        case 0x10B:
          OP_CMNS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        /* ===================== thumb :: orr rd, rm =========================== */
        case 0x10C:
          OP_ORRS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        /* ===================== thumb :: mul rd, rm =========================== */
        case 0x10D:
          { 
            const uint32_t rs_value =  regs[OP_code >> 3 & 7].blk;
            const uint32_t output = regs[OP_code & 7].blk * rs_value;

            regs[OP_code & 7].blk = output;
      
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v; 
            arm->cpsr_fast.flags.blk|= output & ARM7_NEG_BIT;
            
            if (output == 0)                                       
              arm->cpsr_fast.flags.blk8[2] = 0x80;                

            consume += mul_clks (rs_value) + 1;
          }
          break;
          
        /* ===================== thumb :: bic rd, rm =========================== */
        case 0x10E:
          OP_BICS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;
          
        /* ===================== thumb :: mvn rd, rm =========================== */
        case 0x10F:
          OP_MVNS (regs[OP_code & 7].blk, regs[OP_code >> 3 & 7].blk, regs[OP_code & 7].blk);
          consume ++;
          break;

        /* ===================== thumb :: b offset 11 =========================== */
        case 0x1C << 5 | 0:
        case 0x1C << 5 | 1:
        case 0x1C << 5 | 2:
        case 0x1C << 5 | 3:
        case 0x1C << 5 | 4:
        case 0x1C << 5 | 5:
        case 0x1C << 5 | 6:
        case 0x1C << 5 | 7:
        case 0x1C << 5 | 8:
        case 0x1C << 5 | 9:
        case 0x1C << 5 |10:
        case 0x1C << 5 |11:
        case 0x1C << 5 |12:
        case 0x1C << 5 |13:
        case 0x1C << 5 |14:
        case 0x1C << 5 |15:
          {
            const struct thumb_branch *const m_code = (const struct thumb_branch *const) & OP_code;
            const uint32_t pipeline_pc = regs[15].blk;
            const uint32_t offset = (uint32_t) m_code->imm10;

            regs[15].blk = pipeline_pc + offset * 2;
            consume += thumb_flush (agb) + 3;
          }
          break;
        
        case 0x1C << 5 |16:
        case 0x1C << 5 |17:
        case 0x1C << 5 |18:
        case 0x1C << 5 |19:
        case 0x1C << 5 |20:
        case 0x1C << 5 |21:
        case 0x1C << 5 |22:
        case 0x1C << 5 |23:
        case 0x1C << 5 |24:
        case 0x1C << 5 |25:
        case 0x1C << 5 |26:
        case 0x1C << 5 |27:
        case 0x1C << 5 |28:
        case 0x1C << 5 |29:
        case 0x1C << 5 |30:
        case 0x1C << 5 |31:
          {
            const struct thumb_branch *const m_code = (const struct thumb_branch *const) & OP_code;
            const uint32_t pipeline_pc = regs[15].blk & -2;
            const uint32_t mask = 0x3FF;
            const uint32_t offset = (uint32_t) m_code->imm10 & mask |  ~mask;

            regs[15].blk = pipeline_pc + offset * 2;
            consume += thumb_flush (agb) + 3;
          }
          break;

        /* ===================== thumb :: bl-step 1 =========================== */
        case 0x1E << 5 | 0:
        case 0x1E << 5 | 1:
        case 0x1E << 5 | 2:
        case 0x1E << 5 | 3:
        case 0x1E << 5 | 4:
        case 0x1E << 5 | 5:
        case 0x1E << 5 | 6:
        case 0x1E << 5 | 7:
        case 0x1E << 5 | 8:
        case 0x1E << 5 | 9:
        case 0x1E << 5 |10:
        case 0x1E << 5 |11:
        case 0x1E << 5 |12:
        case 0x1E << 5 |13:
        case 0x1E << 5 |14:
        case 0x1E << 5 |15:
          {
            const struct thumb_branch *const m_code = (const struct thumb_branch *const) & OP_code;
            const uint32_t pipeline_pc = regs[15].blk & -2;
            const uint32_t offset = m_code->imm10;

            regs[14].blk = pipeline_pc + (offset << 12);
            consume += 1;
          }
          break;
        
        case 0x1E << 5 |16:
        case 0x1E << 5 |17:
        case 0x1E << 5 |18:
        case 0x1E << 5 |19:
        case 0x1E << 5 |20:
        case 0x1E << 5 |21:
        case 0x1E << 5 |22:
        case 0x1E << 5 |23:
        case 0x1E << 5 |24:
        case 0x1E << 5 |25:
        case 0x1E << 5 |26:
        case 0x1E << 5 |27:
        case 0x1E << 5 |28:
        case 0x1E << 5 |29:
        case 0x1E << 5 |30:
        case 0x1E << 5 |31:
          {
            const struct thumb_branch *const m_code = (const struct thumb_branch *const) & OP_code;
            const uint32_t pipeline_pc = regs[15].blk & -2;  
            const uint32_t mask = 0x3FF;
            const uint32_t offset = (uint32_t) m_code->imm10 & mask | ~mask;
            
            regs[14].blk = pipeline_pc + (offset << 12);
            consume += 1;
          }
          break;

        /* ===================== thumb :: bl-step 2 =========================== */
        case 0x1F << 5 | 0:
        case 0x1F << 5 | 1:
        case 0x1F << 5 | 2:
        case 0x1F << 5 | 3:
        case 0x1F << 5 | 4:
        case 0x1F << 5 | 5:
        case 0x1F << 5 | 6:
        case 0x1F << 5 | 7:
        case 0x1F << 5 | 8:
        case 0x1F << 5 | 9:
        case 0x1F << 5 |10:
        case 0x1F << 5 |11:
        case 0x1F << 5 |12:
        case 0x1F << 5 |13:
        case 0x1F << 5 |14:
        case 0x1F << 5 |15:
        case 0x1F << 5 |16:
        case 0x1F << 5 |17:
        case 0x1F << 5 |18:
        case 0x1F << 5 |19:
        case 0x1F << 5 |20:
        case 0x1F << 5 |21:
        case 0x1F << 5 |22:
        case 0x1F << 5 |23:
        case 0x1F << 5 |24:
        case 0x1F << 5 |25:
        case 0x1F << 5 |26:
        case 0x1F << 5 |27:
        case 0x1F << 5 |28:
        case 0x1F << 5 |29:
        case 0x1F << 5 |30:
        case 0x1F << 5 |31:
          {
            const uint32_t save_lrlink = (regs[15].blk & -2) - 2 | 1;
            regs[15].blk = regs[14].blk + (OP_code & 0x7FF) * 2;
            regs[14].blk = save_lrlink;

            consume += thumb_flush (agb) + 3;
          }
          break;
          
        /* ===================== thumb :: stmia =========================== */
        case 0x18 << 5 | 0:
        case 0x18 << 5 | 1:
        case 0x18 << 5 | 2:
        case 0x18 << 5 | 3:
        case 0x18 << 5 | 4:
        case 0x18 << 5 | 5:
        case 0x18 << 5 | 6:
        case 0x18 << 5 | 7:
        case 0x18 << 5 | 8:
        case 0x18 << 5 | 9:
        case 0x18 << 5 |10:
        case 0x18 << 5 |11:
        case 0x18 << 5 |12:
        case 0x18 << 5 |13:
        case 0x18 << 5 |14:
        case 0x18 << 5 |15:
        case 0x18 << 5 |16:
        case 0x18 << 5 |17:
        case 0x18 << 5 |18:
        case 0x18 << 5 |19:
        case 0x18 << 5 |20:
        case 0x18 << 5 |21:
        case 0x18 << 5 |22:
        case 0x18 << 5 |23:
        case 0x18 << 5 |24:
        case 0x18 << 5 |25:
        case 0x18 << 5 |26:
        case 0x18 << 5 |27:
        case 0x18 << 5 |28:
        case 0x18 << 5 |29:
        case 0x18 << 5 |30:
        case 0x18 << 5 |31:
           consume +=  isa_thumb_stmia (agb, OP_code);
           break;
           
        /* ===================== thumb :: ldmia =========================== */
        case 0x19 << 5 | 0:
        case 0x19 << 5 | 1:
        case 0x19 << 5 | 2:
        case 0x19 << 5 | 3:
        case 0x19 << 5 | 4:
        case 0x19 << 5 | 5:
        case 0x19 << 5 | 6:
        case 0x19 << 5 | 7:
        case 0x19 << 5 | 8:
        case 0x19 << 5 | 9:
        case 0x19 << 5 |10:
        case 0x19 << 5 |11:
        case 0x19 << 5 |12:
        case 0x19 << 5 |13:
        case 0x19 << 5 |14:
        case 0x19 << 5 |15:
        case 0x19 << 5 |16:
        case 0x19 << 5 |17:
        case 0x19 << 5 |18:
        case 0x19 << 5 |19:
        case 0x19 << 5 |20:
        case 0x19 << 5 |21:
        case 0x19 << 5 |22:
        case 0x19 << 5 |23:
        case 0x19 << 5 |24:
        case 0x19 << 5 |25:
        case 0x19 << 5 |26:
        case 0x19 << 5 |27:
        case 0x19 << 5 |28:
        case 0x19 << 5 |29:
        case 0x19 << 5 |30:
        case 0x19 << 5 |31:
           consume +=  isa_thumb_ldmia (agb, OP_code);
           break;
           
        /* ===================== thumb :: push  =========================== */
        case 0x5A << 3 | 0:
        case 0x5A << 3 | 1:
        case 0x5A << 3 | 2:
        case 0x5A << 3 | 3:
        case 0x5A << 3 | 4:
        case 0x5A << 3 | 5:
        case 0x5A << 3 | 6:
        case 0x5A << 3 | 7:     
          consume += isa_thumb_push (agb, OP_code);
          break;
        
        /* ===================== thumb :: pop  =========================== */
        case 0x5E << 3 | 0:
        case 0x5E << 3 | 1:
        case 0x5E << 3 | 2:
        case 0x5E << 3 | 3:
        case 0x5E << 3 | 4:
        case 0x5E << 3 | 5:
        case 0x5E << 3 | 6:
        case 0x5E << 3 | 7:     
          consume += isa_thumb_pop (agb, OP_code);
          break;
           
        /* ===================== thumb :: swi =========================== */
        case 0xDF << 2 | 0:
        case 0xDF << 2 | 1:
        case 0xDF << 2 | 2:
        case 0xDF << 2 | 3:
          {
            const uint32_t pipeline_pc = regs[15].blk &  THUMB_CODE_MASK;
            cpsr_to_std (& arm->cpsr, & arm->cpsr_fast);
            
            bank_swi (agb, ARM7_MODE_MGR);

            arm->r1314_t[1+R1314b_MGR].blk = pipeline_pc  - 2;
            arm->spsr_t[SPSRb_MGR].blk =  arm->cpsr.blk;
            arm->regs[14].blk = pipeline_pc  - 2;
            
            arm->cpsr_fast.mode = ARM7_MODE_MGR | ARM7_MODE_HSET_MASK;
            arm->cpsr_fast.thumb = false;
            arm->cpsr_fast.irq_disable = true;

            arm->regs[15].blk = 8;

            consume += arm7_flush (agb) + 3;
            reset_int_magic (agb);     

            arm->regs[15].blk = 14;
          }
          break;
          
        /* ===================== thumb :: add sp, imm7 =========================== */
        case 0x160 << 1 | 0:
        case 0x160 << 1 | 1:
          regs[13].blk += (OP_code & 0x7F) * 4;
          consume++;
          break;
          
        /* ===================== thumb :: sub sp, imm7 =========================== */
        case 0x161 << 1 | 0:
        case 0x161 << 1 | 1:
          regs[13].blk -= (OP_code & 0x7F) * 4;
          consume++;
          break;

        /* =================== thumb :: lsl rd, rm, imm5 ========================= */
        case 0x00 << 5 | 0: 
        case 0x00 << 5 | 1:  
        case 0x00 << 5 | 2:  
        case 0x00 << 5 | 3:  
        case 0x00 << 5 | 4:  
        case 0x00 << 5 | 5:  
        case 0x00 << 5 | 6:  
        case 0x00 << 5 | 7:  
        case 0x00 << 5 | 8:  
        case 0x00 << 5 | 9:  
        case 0x00 << 5 |10:  
        case 0x00 << 5 |11: 
        case 0x00 << 5 |12:  
        case 0x00 << 5 |13:  
        case 0x00 << 5 |14: 
        case 0x00 << 5 |15: 
        case 0x00 << 5 |16:  
        case 0x00 << 5 |17:  
        case 0x00 << 5 |18:  
        case 0x00 << 5 |19: 
        case 0x00 << 5 |20:  
        case 0x00 << 5 |21:  
        case 0x00 << 5 |22:  
        case 0x00 << 5 |23:  
        case 0x00 << 5 |24: 
        case 0x00 << 5 |25: 
        case 0x00 << 5 |26:  
        case 0x00 << 5 |27: 
        case 0x00 << 5 |28:
        case 0x00 << 5 |29:
        case 0x00 << 5 |30:
        case 0x00 << 5 |31:
          {
            const uint32_t rdi = OP_code & 7;
            const uint32_t rm = OP_code >> 3 & 7;
            const uint32_t imm5 = OP_code >> 6 & 31;

            LSL_IMM5_SIGN (regs[rm].blk, imm5, regs[rdi].blk, arm->cpsr_fast.flags.sig.c);
            
            NZ_RESET_FAST (regs[rdi].blk);
          }
          consume ++;
          break;
        
        /* =================== thumb :: lsr rd, rm, imm5 ========================= */
        case 0x01 << 5 | 0: 
        case 0x01 << 5 | 1:  
        case 0x01 << 5 | 2:  
        case 0x01 << 5 | 3:  
        case 0x01 << 5 | 4:  
        case 0x01 << 5 | 5:  
        case 0x01 << 5 | 6:  
        case 0x01 << 5 | 7:  
        case 0x01 << 5 | 8:  
        case 0x01 << 5 | 9:  
        case 0x01 << 5 |10:  
        case 0x01 << 5 |11: 
        case 0x01 << 5 |12:  
        case 0x01 << 5 |13:  
        case 0x01 << 5 |14: 
        case 0x01 << 5 |15: 
        case 0x01 << 5 |16:  
        case 0x01 << 5 |17:  
        case 0x01 << 5 |18:  
        case 0x01 << 5 |19: 
        case 0x01 << 5 |20:  
        case 0x01 << 5 |21:  
        case 0x01 << 5 |22:  
        case 0x01 << 5 |23:  
        case 0x01 << 5 |24: 
        case 0x01 << 5 |25: 
        case 0x01 << 5 |26:  
        case 0x01 << 5 |27: 
        case 0x01 << 5 |28:
        case 0x01 << 5 |29:
        case 0x01 << 5 |30:
        case 0x01 << 5 |31:
          {
            const uint32_t rdi = OP_code & 7;
            const uint32_t rm = OP_code >> 3 & 7;
            const uint32_t imm5 = OP_code >> 6 & 31;

            LSR_IMM5_SIGN (regs[rm].blk, imm5, regs[rdi].blk, arm->cpsr_fast.flags.sig.c);
            
            NZ_RESET_FAST (regs[rdi].blk);
          }
          consume ++;
          break;

        /* =================== thumb :: asr rd, rm, imm5 ========================= */
        case 0x02 << 5 | 0: 
        case 0x02 << 5 | 1:  
        case 0x02 << 5 | 2:  
        case 0x02 << 5 | 3:  
        case 0x02 << 5 | 4:  
        case 0x02 << 5 | 5:  
        case 0x02 << 5 | 6:  
        case 0x02 << 5 | 7:  
        case 0x02 << 5 | 8:  
        case 0x02 << 5 | 9:  
        case 0x02 << 5 |10:  
        case 0x02 << 5 |11: 
        case 0x02 << 5 |12:  
        case 0x02 << 5 |13:  
        case 0x02 << 5 |14: 
        case 0x02 << 5 |15: 
        case 0x02 << 5 |16:  
        case 0x02 << 5 |17:  
        case 0x02 << 5 |18:  
        case 0x02 << 5 |19: 
        case 0x02 << 5 |20:  
        case 0x02 << 5 |21:  
        case 0x02 << 5 |22:  
        case 0x02 << 5 |23:  
        case 0x02 << 5 |24: 
        case 0x02 << 5 |25: 
        case 0x02 << 5 |26:  
        case 0x02 << 5 |27: 
        case 0x02 << 5 |28:
        case 0x02 << 5 |29:
        case 0x02 << 5 |30:
        case 0x02 << 5 |31:
          {
            const uint32_t rdi = OP_code & 7;
            const uint32_t rm = OP_code >> 3 & 7;
            const uint32_t imm5 = OP_code >> 6 & 31;

            ASR_IMM5_SIGN (regs[rm].blk, imm5, regs[rdi].blk, arm->cpsr_fast.flags.sig.c);
            
            NZ_RESET_FAST (regs[rdi].blk);
          }
          consume ++;
          break;
        
        case 0x03 << 5 | 0: 
        case 0x03 << 5 | 1: 
        case 0x03 << 5 | 2: 
        case 0x03 << 5 | 3: 
        case 0x03 << 5 | 4: 
        case 0x03 << 5 | 5: 
        case 0x03 << 5 | 6: 
        case 0x03 << 5 | 7: 
          {
            const uint32_t rd = OP_code & 7;
            const uint32_t rn = OP_code >> 3 & 7;
            const uint32_t rm = OP_code >> 6 & 7;

            OP_ADDS (regs[rn].blk, regs[rm].blk, regs[rd].blk);
          }
          consume++;
          break;

        case 0x03 << 5 | 8: 
        case 0x03 << 5 | 9:
        case 0x03 << 5 |10: 
        case 0x03 << 5 |11: 
        case 0x03 << 5 |12: 
        case 0x03 << 5 |13: 
        case 0x03 << 5 |14:
        case 0x03 << 5 |15: 
          {
            const uint32_t rd = OP_code & 7;
            const uint32_t rn = OP_code >> 3 & 7;
            const uint32_t rm = OP_code >> 6 & 7;

            OP_SUBS (regs[rn].blk, regs[rm].blk, regs[rd].blk);
          }
          consume++;
          break;

        case 0x03 << 5 |16: 
        case 0x03 << 5 |17: 
        case 0x03 << 5 |18: 
        case 0x03 << 5 |19: 
        case 0x03 << 5 |20: 
        case 0x03 << 5 |21: 
        case 0x03 << 5 |22:
        case 0x03 << 5 |23: 
          {
            const uint32_t rd = OP_code & 7;
            const uint32_t rn = OP_code >> 3 & 7;
            const uint32_t imm3 = OP_code >> 6 & 7;

            OP_ADDS (regs[rn].blk, imm3, regs[rd].blk);
          }
          consume++;
          break;

        case 0x03 << 5 |24: 
        case 0x03 << 5 |25:
        case 0x03 << 5 |26: 
        case 0x03 << 5 |27: 
        case 0x03 << 5 |28:
        case 0x03 << 5 |29: 
        case 0x03 << 5 |30: 
        case 0x03 << 5 |31:
          {
            const uint32_t rd = OP_code & 7;
            const uint32_t rn = OP_code >> 3 & 7;
            const uint32_t imm3 = OP_code >> 6 & 7;

            OP_SUBS (regs[rn].blk, imm3, regs[rd].blk);
          }
          consume++;
          break;

  #define MCAS_CASE(n) \
        case 0x04 << 5 | (n) * 4 + 0:\
        case 0x04 << 5 | (n) * 4 + 1:\
        case 0x04 << 5 | (n) * 4 + 2:\
        case 0x04 << 5 | (n) * 4 + 3: OP_MOVS (0, OP_code & 255, regs[n].blk); consume ++; break;\
        case 0x05 << 5 | (n) * 4 + 0:\
        case 0x05 << 5 | (n) * 4 + 1:\
        case 0x05 << 5 | (n) * 4 + 2:\
        case 0x05 << 5 | (n) * 4 + 3: OP_CMPS (regs[n].blk, OP_code & 255, regs[n].blk);    consume ++;  break; \
        case 0x06 << 5 | (n) * 4 + 0:\
        case 0x06 << 5 | (n) * 4 + 1:\
        case 0x06 << 5 | (n) * 4 + 2:\
        case 0x06 << 5 | (n) * 4 + 3: OP_ADDS (regs[n].blk, OP_code & 255, regs[n].blk);    consume ++;  break; \
        case 0x07 << 5 | (n) * 4 + 0:\
        case 0x07 << 5 | (n) * 4 + 1:\
        case 0x07 << 5 | (n) * 4 + 2:\
        case 0x07 << 5 | (n) * 4 + 3: OP_SUBS (regs[n].blk, OP_code & 255, regs[n].blk);    consume ++;  break; \

        MCAS_CASE (0)
        MCAS_CASE (1)
        MCAS_CASE (2)
        MCAS_CASE (3)
        MCAS_CASE (4)
        MCAS_CASE (5)
        MCAS_CASE (6)
        MCAS_CASE (7)

        /* ===================== thumb :: add rd, rm (full regs) =========================== */
        case 0x44 << 2 |0:
        case 0x44 << 2 |1:
        case 0x44 << 2 |2:
        case 0x44 << 2 |3:
          {
            const uint32_t rm = OP_code >> 3 & 15;
            const uint32_t rn_rd = OP_code & 7 | (OP_code >> 7 & 1) << 3;
            regs[rn_rd].blk += regs[rm].blk;
            if  (rn_rd == 15) {
              regs[rn_rd].blk &= THUMB_CODE_MASK; /* FIXME: and -4 ?? */
              consume += thumb_flush (agb) + 3;
            } else 
              consume += 1;
          }
          break;

        /* ===================== thumb :: cmp rn, rm (full regs) =========================== */
        case 0x45 << 2 |0:
        case 0x45 << 2 |1:
        case 0x45 << 2 |2:
        case 0x45 << 2 |3:
          {
            const uint32_t rm = OP_code >> 3 & 15;
            const uint32_t rn_rd = OP_code & 7 | (OP_code >> 7 & 1) << 3;

            OP_CMPS (regs[rn_rd].blk, regs[rm].blk, regs[rm].blk);
            consume ++;
          }
          break;

        /* ===================== thumb :: mov rn, rm (full regs) =========================== */
        case 0x46 << 2 |0:
        case 0x46 << 2 |1:
        case 0x46 << 2 |2:
        case 0x46 << 2 |3:
          {
            const uint32_t rm = OP_code >> 3 & 15;
            const uint32_t rn_rd = OP_code & 7 | (OP_code >> 7 & 1) << 3;
            regs[rn_rd].blk = regs[rm].blk;

            if  (rn_rd == 15) {
              regs[rn_rd].blk &= THUMB_CODE_MASK; /* FIXME: and -4 ?? */
              consume += thumb_flush (agb) + 3;
            } else 
              consume += 1;
          }
          break;

        /* ===================== thumb :: bx rm (full regs) =========================== */
        case 0x47 << 2 |0:
        case 0x47 << 2 |1:
          {
            const uint32_t rm = OP_code >> 3 & 15;
            const uint32_t thumb = regs[rm].blk & 1;
            
            if (thumb != 0) {
              regs[15].blk = regs[rm].blk & THUMB_CODE_MASK;
              consume += thumb_flush (agb) + 3;
              arm->cpsr_fast.thumb = true;
            } else {
              const uint32_t temp_pc = regs[rm].blk & ARM7_CODE_MASK;
              regs[15].blk = temp_pc;
              consume += arm7_flush (agb) + 3;
              regs[15].blk = temp_pc + 8 - 2;
              arm->cpsr_fast.thumb = false;
            }
          }
          break;

        /* =============================== thumb :: ldr rd, pc, imm8 =============================== */
        case 0x09 << 5 | 0: case 0x09 << 5 | 1: case 0x09 << 5 | 2: case 0x09 << 5 | 3: 
        case 0x09 << 5 | 4: case 0x09 << 5 | 5: case 0x09 << 5 | 6: case 0x09 << 5 | 7:
        case 0x09 << 5 | 8: case 0x09 << 5 | 9: case 0x09 << 5 |10: case 0x09 << 5 |11: 
        case 0x09 << 5 |12: case 0x09 << 5 |13: case 0x09 << 5 |14: case 0x09 << 5 |15: 
        case 0x09 << 5 |16: case 0x09 << 5 |17: case 0x09 << 5 |18: case 0x09 << 5 |19:
        case 0x09 << 5 |20: case 0x09 << 5 |21: case 0x09 << 5 |22: case 0x09 << 5 |23: 
        case 0x09 << 5 |24: case 0x09 << 5 |25: case 0x09 << 5 |26: case 0x09 << 5 |27: 
        case 0x09 << 5 |28: case 0x09 << 5 |29: case 0x09 << 5 |30: case 0x09 << 5 |31:
          {
            const uint32_t offset8 = OP_code & 0xFF;
            const uint32_t address = (regs[15].blk & ARM7_CODE_MASK) + offset8 * 4;
            const uint32_t rdi = OP_code >> 8 & 7;
            
            consume += agb_mbus_rw ( agb, address, & regs[rdi].blk, false) + 3;
          }
          break;
        /* =============================== thumb :: str rd, sp, imm8 =============================== */
        case 0x12 << 5 | 0: case 0x12 << 5 | 1: case 0x12 << 5 | 2: case 0x12 << 5 | 3: 
        case 0x12 << 5 | 4: case 0x12 << 5 | 5: case 0x12 << 5 | 6: case 0x12 << 5 | 7: 
        case 0x12 << 5 | 8: case 0x12 << 5 | 9: case 0x12 << 5 |10: case 0x12 << 5 |11: 
        case 0x12 << 5 |12: case 0x12 << 5 |13: case 0x12 << 5 |14: case 0x12 << 5 |15: 
        case 0x12 << 5 |16: case 0x12 << 5 |17: case 0x12 << 5 |18: case 0x12 << 5 |19: 
        case 0x12 << 5 |20: case 0x12 << 5 |21: case 0x12 << 5 |22: case 0x12 << 5 |23:
        case 0x12 << 5 |24: case 0x12 << 5 |25: case 0x12 << 5 |26: case 0x12 << 5 |27: 
        case 0x12 << 5 |28: case 0x12 << 5 |29: case 0x12 << 5 |30: case 0x12 << 5 |31: 
          {
            const uint32_t offset8 = OP_code & 0xFF;
            const uint32_t address = regs[13].blk + offset8 * 4;
            const uint32_t rdi = OP_code >> 8 & 7;
            
            consume += agb_mbus_ww ( agb, address, regs[rdi].blk, false) + 3;
          }
          break;
          
        /* =============================== thumb :: ldr rd, sp, imm8 =============================== */
        case 0x13 << 5 | 0: case 0x13 << 5 | 1: case 0x13 << 5 | 2: case 0x13 << 5 | 3: 
        case 0x13 << 5 | 4: case 0x13 << 5 | 5: case 0x13 << 5 | 6: case 0x13 << 5 | 7: 
        case 0x13 << 5 | 8: case 0x13 << 5 | 9: case 0x13 << 5 |10: case 0x13 << 5 |11: 
        case 0x13 << 5 |12: case 0x13 << 5 |13: case 0x13 << 5 |14: case 0x13 << 5 |15:
        case 0x13 << 5 |16: case 0x13 << 5 |17: case 0x13 << 5 |18: case 0x13 << 5 |19: 
        case 0x13 << 5 |20: case 0x13 << 5 |21: case 0x13 << 5 |22: case 0x13 << 5 |23: 
        case 0x13 << 5 |24: case 0x13 << 5 |25: case 0x13 << 5 |26: case 0x13 << 5 |27: 
        case 0x13 << 5 |28: case 0x13 << 5 |29: case 0x13 << 5 |30: case 0x13 << 5 |31: 
          THUMB_LDR_ALLB (LDR, (OP_code >> 8 & 7), 13, ((OP_code & 0xFF) * 4));
          break;
          
        /* =============================== thumb :: add rd, pc, imm8 =============================== */
        case 0x14 << 5 | 0: case 0x14 << 5 | 1: case 0x14 << 5 | 2: case 0x14 << 5 | 3: 
        case 0x14 << 5 | 4: case 0x14 << 5 | 5: case 0x14 << 5 | 6: case 0x14 << 5 | 7: 
        case 0x14 << 5 | 8: case 0x14 << 5 | 9: case 0x14 << 5 |10: case 0x14 << 5 |11: 
        case 0x14 << 5 |12: case 0x14 << 5 |13: case 0x14 << 5 |14: case 0x14 << 5 |15: 
        case 0x14 << 5 |16: case 0x14 << 5 |17: case 0x14 << 5 |18: case 0x14 << 5 |19:
        case 0x14 << 5 |20: case 0x14 << 5 |21: case 0x14 << 5 |22: case 0x14 << 5 |23: 
        case 0x14 << 5 |24: case 0x14 << 5 |25: case 0x14 << 5 |26: case 0x14 << 5 |27: 
        case 0x14 << 5 |28: case 0x14 << 5 |29: case 0x14 << 5 |30: case 0x14 << 5 |31: 
          regs[OP_code >> 8 & 7].blk = (regs[15].blk & ARM7_CODE_MASK) + (OP_code & 0xFF) * 4; 
          consume++;
          break;
          
        /* =============================== thumb :: add rd, sp, imm8 =============================== */
        case 0x15 << 5 | 0: case 0x15 << 5 | 1: case 0x15 << 5 | 2: case 0x15 << 5 | 3: 
        case 0x15 << 5 | 4: case 0x15 << 5 | 5: case 0x15 << 5 | 6: case 0x15 << 5 | 7: 
        case 0x15 << 5 | 8: case 0x15 << 5 | 9: case 0x15 << 5 |10: case 0x15 << 5 |11: 
        case 0x15 << 5 |12: case 0x15 << 5 |13: case 0x15 << 5 |14: case 0x15 << 5 |15: 
        case 0x15 << 5 |16: case 0x15 << 5 |17: case 0x15 << 5 |18: case 0x15 << 5 |19: 
        case 0x15 << 5 |20: case 0x15 << 5 |21: case 0x15 << 5 |22: case 0x15 << 5 |23:
        case 0x15 << 5 |24: case 0x15 << 5 |25: case 0x15 << 5 |26: case 0x15 << 5 |27: 
        case 0x15 << 5 |28: case 0x15 << 5 |29: case 0x15 << 5 |30: case 0x15 << 5 |31: 
          regs[OP_code >> 8 & 7].blk = regs[13].blk + (OP_code & 0xFF) * 4; 
          consume++;
          break;
             
        /* =============================== thumb :: ldrb rd, rn, imm5 =============================== */
        case 0x0F << 5 | 0: 
        case 0x0F << 5 | 1: 
        case 0x0F << 5 | 2: 
        case 0x0F << 5 | 3:  
        case 0x0F << 5 | 4: 
        case 0x0F << 5 | 5: 
        case 0x0F << 5 | 6: 
        case 0x0F << 5 | 7: 
        case 0x0F << 5 | 8:  
        case 0x0F << 5 | 9: 
        case 0x0F << 5 |10: 
        case 0x0F << 5 |11: 
        case 0x0F << 5 |12:
        case 0x0F << 5 |13: 
        case 0x0F << 5 |14: 
        case 0x0F << 5 |15: 
        case 0x0F << 5 |16: 
        case 0x0F << 5 |17: 
        case 0x0F << 5 |18: 
        case 0x0F << 5 |19:
        case 0x0F << 5 |20: 
        case 0x0F << 5 |21:
        case 0x0F << 5 |22: 
        case 0x0F << 5 |23: 
        case 0x0F << 5 |24: 
        case 0x0F << 5 |25: 
        case 0x0F << 5 |26: 
        case 0x0F << 5 |27: 
        case 0x0F << 5 |28: 
        case 0x0F << 5 |29: 
        case 0x0F << 5 |30: 
        case 0x0F << 5 |31: 
          THUMB_LDR_ALLB (LDRB, (OP_code & 7), (OP_code >> 3 & 7), ((OP_code >> 6 & 31) * 1));
          break;
          
        /* =============================== thumb :: ldrh rd, rn, imm5 =============================== */
        case 0x11 << 5 | 0: 
        case 0x11 << 5 | 1: 
        case 0x11 << 5 | 2:  
        case 0x11 << 5 | 3:  
        case 0x11 << 5 | 4: 
        case 0x11 << 5 | 5: 
        case 0x11 << 5 | 6:  
        case 0x11 << 5 | 7: 
        case 0x11 << 5 | 8: 
        case 0x11 << 5 | 9: 
        case 0x11 << 5 |10: 
        case 0x11 << 5 |11: 
        case 0x11 << 5 |12: 
        case 0x11 << 5 |13:
        case 0x11 << 5 |14: 
        case 0x11 << 5 |15: 
        case 0x11 << 5 |16: 
        case 0x11 << 5 |17: 
        case 0x11 << 5 |18:  
        case 0x11 << 5 |19: 
        case 0x11 << 5 |20: 
        case 0x11 << 5 |21: 
        case 0x11 << 5 |22: 
        case 0x11 << 5 |23: 
        case 0x11 << 5 |24:
        case 0x11 << 5 |25: 
        case 0x11 << 5 |26: 
        case 0x11 << 5 |27: 
        case 0x11 << 5 |28: 
        case 0x11 << 5 |29: 
        case 0x11 << 5 |30: 
        case 0x11 << 5 |31:    
          THUMB_LDR_ALLB (LDRH, (OP_code & 7), (OP_code >> 3 & 7), ((OP_code >> 6 & 31) * 2));
          break;
        
        /* =============================== thumb :: ldr rd, rn, imm5 =============================== */
        case 0x0D << 5 | 0:  
        case 0x0D << 5 | 1: 
        case 0x0D << 5 | 2:  
        case 0x0D << 5 | 3:  
        case 0x0D << 5 | 4: 
        case 0x0D << 5 | 5:
        case 0x0D << 5 | 6: 
        case 0x0D << 5 | 7: 
        case 0x0D << 5 | 8:  
        case 0x0D << 5 | 9: 
        case 0x0D << 5 |10: 
        case 0x0D << 5 |11: 
        case 0x0D << 5 |12:
        case 0x0D << 5 |13: 
        case 0x0D << 5 |14:
        case 0x0D << 5 |15: 
        case 0x0D << 5 |16:  
        case 0x0D << 5 |17: 
        case 0x0D << 5 |18: 
        case 0x0D << 5 |19: 
        case 0x0D << 5 |20: 
        case 0x0D << 5 |21: 
        case 0x0D << 5 |22:  
        case 0x0D << 5 |23:  
        case 0x0D << 5 |24: 
        case 0x0D << 5 |25: 
        case 0x0D << 5 |26:
        case 0x0D << 5 |27: 
        case 0x0D << 5 |28: 
        case 0x0D << 5 |29: 
        case 0x0D << 5 |30: 
        case 0x0D << 5 |31:    
          THUMB_LDR_ALLB (LDR, (OP_code & 7), (OP_code >> 3 & 7), ((OP_code >> 6 & 31) * 4));
          break;
          
        /* =============================== thumb :: strb rd, rn, imm5 =============================== */
        case 0x0E << 5 | 0: 
        case 0x0E << 5 | 1: 
        case 0x0E << 5 | 2:  
        case 0x0E << 5 | 3:  
        case 0x0E << 5 | 4:  
        case 0x0E << 5 | 5: 
        case 0x0E << 5 | 6:  
        case 0x0E << 5 | 7:  
        case 0x0E << 5 | 8: 
        case 0x0E << 5 | 9:  
        case 0x0E << 5 |10: 
        case 0x0E << 5 |11: 
        case 0x0E << 5 |12:
        case 0x0E << 5 |13: 
        case 0x0E << 5 |14: 
        case 0x0E << 5 |15:  
        case 0x0E << 5 |16: 
        case 0x0E << 5 |17: 
        case 0x0E << 5 |18: 
        case 0x0E << 5 |19: 
        case 0x0E << 5 |20: 
        case 0x0E << 5 |21: 
        case 0x0E << 5 |22:
        case 0x0E << 5 |23:
        case 0x0E << 5 |24:  
        case 0x0E << 5 |25: 
        case 0x0E << 5 |26: 
        case 0x0E << 5 |27: 
        case 0x0E << 5 |28: 
        case 0x0E << 5 |29: 
        case 0x0E << 5 |30: 
        case 0x0E << 5 |31: 
          THUMB_STR_ALLB (STRB, (OP_code & 7), (OP_code >> 3 & 7), ((OP_code >> 6 & 31) * 1));
          break;
          
        /* =============================== thumb :: strh rd, rn, imm5 =============================== */
        case 0x10 << 5 | 0: 
        case 0x10 << 5 | 1:  
        case 0x10 << 5 | 2: 
        case 0x10 << 5 | 3: 
        case 0x10 << 5 | 4: 
        case 0x10 << 5 | 5: 
        case 0x10 << 5 | 6: 
        case 0x10 << 5 | 7:  
        case 0x10 << 5 | 8: 
        case 0x10 << 5 | 9: 
        case 0x10 << 5 |10: 
        case 0x10 << 5 |11:  
        case 0x10 << 5 |12: 
        case 0x10 << 5 |13: 
        case 0x10 << 5 |14:  
        case 0x10 << 5 |15: 
        case 0x10 << 5 |16:  
        case 0x10 << 5 |17: 
        case 0x10 << 5 |18:  
        case 0x10 << 5 |19: 
        case 0x10 << 5 |20: 
        case 0x10 << 5 |21: 
        case 0x10 << 5 |22: 
        case 0x10 << 5 |23: 
        case 0x10 << 5 |24: 
        case 0x10 << 5 |25: 
        case 0x10 << 5 |26: 
        case 0x10 << 5 |27: 
        case 0x10 << 5 |28: 
        case 0x10 << 5 |29: 
        case 0x10 << 5 |30: 
        case 0x10 << 5 |31:  
          THUMB_STR_ALLB (STRH, (OP_code & 7), (OP_code >> 3 & 7), ((OP_code >> 6 & 31) * 2));
          break;
          
        /* =============================== thumb :: str rd, rn, imm5 =============================== */
        case 0x0C << 5 | 0: 
        case 0x0C << 5 | 1: 
        case 0x0C << 5 | 2: 
        case 0x0C << 5 | 3: 
        case 0x0C << 5 | 4:  
        case 0x0C << 5 | 5:  
        case 0x0C << 5 | 6:  
        case 0x0C << 5 | 7: 
        case 0x0C << 5 | 8:  
        case 0x0C << 5 | 9:  
        case 0x0C << 5 |10: 
        case 0x0C << 5 |11:  
        case 0x0C << 5 |12: 
        case 0x0C << 5 |13: 
        case 0x0C << 5 |14:  
        case 0x0C << 5 |15: 
        case 0x0C << 5 |16: 
        case 0x0C << 5 |17:  
        case 0x0C << 5 |18: 
        case 0x0C << 5 |19:  
        case 0x0C << 5 |20:  
        case 0x0C << 5 |21: 
        case 0x0C << 5 |22: 
        case 0x0C << 5 |23:  
        case 0x0C << 5 |24: 
        case 0x0C << 5 |25: 
        case 0x0C << 5 |26:  
        case 0x0C << 5 |27:  
        case 0x0C << 5 |28: 
        case 0x0C << 5 |29:
        case 0x0C << 5 |30:
        case 0x0C << 5 |31: 
          THUMB_STR_ALLB (STR, (OP_code & 7), (OP_code >> 3 & 7), ((OP_code >> 6 & 31) * 4));
          break;
     
        /* =============================== thumb :: ldrb rd, rn, rm =============================== */
        case 0x2E << 3 | 0: 
        case 0x2E << 3 | 1:    
        case 0x2E << 3 | 2: 
        case 0x2E << 3 | 3:  
        case 0x2E << 3 | 4: 
        case 0x2E << 3 | 5:   
        case 0x2E << 3 | 6: 
        case 0x2E << 3 | 7: 
          THUMB_LDR_ALLB (LDRB, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
        /* =============================== thumb :: ldrsb rd, rn, rm =============================== */
        case 0x2B << 3 | 0: 
        case 0x2B << 3 | 1:    
        case 0x2B << 3 | 2: 
        case 0x2B << 3 | 3:  
        case 0x2B << 3 | 4: 
        case 0x2B << 3 | 5:  
        case 0x2B << 3 | 6: 
        case 0x2B << 3 | 7: 
          THUMB_LDR_ALLB (LDRSB, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
        /* =============================== thumb :: ldrh rd, rn, rm =============================== */
        case 0x2D << 3 | 0: 
        case 0x2D << 3 | 1:  
        case 0x2D << 3 | 2: 
        case 0x2D << 3 | 3: 
        case 0x2D << 3 | 4: 
        case 0x2D << 3 | 5:  
        case 0x2D << 3 | 6:
        case 0x2D << 3 | 7: 
          THUMB_LDR_ALLB (LDRH, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;

        /* =============================== thumb :: ldrsh rd, rn, rm =============================== */
        case 0x2F << 3 | 0: 
        case 0x2F << 3 | 1:  
        case 0x2F << 3 | 2: 
        case 0x2F << 3 | 3:  
        case 0x2F << 3 | 4:
        case 0x2F << 3 | 5:  
        case 0x2F << 3 | 6: 
        case 0x2F << 3 | 7: 
          THUMB_LDR_ALLB (LDRSH, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
        /* =============================== thumb :: ldr rd, rn, rm =============================== */
        case 0x2C << 3 | 0: 
        case 0x2C << 3 | 1:   
        case 0x2C << 3 | 2:
        case 0x2C << 3 | 3:   
        case 0x2C << 3 | 4: 
        case 0x2C << 3 | 5:    
        case 0x2C << 3 | 6: 
        case 0x2C << 3 | 7: 
          THUMB_LDR_ALLB (LDR, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
        /* =============================== thumb :: strb rd, rn, rm =============================== */
        case 0x2A << 3 | 0: 
        case 0x2A << 3 | 1:   
        case 0x2A << 3 | 2: 
        case 0x2A << 3 | 3:    
        case 0x2A << 3 | 4:
        case 0x2A << 3 | 5:  
        case 0x2A << 3 | 6: 
        case 0x2A << 3 | 7: 
          THUMB_STR_ALLB (STRB, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
        /* =============================== thumb :: strh rd, rn, rm =============================== */
        case 0x29 << 3 | 0: 
        case 0x29 << 3 | 1:  
        case 0x29 << 3 | 2: 
        case 0x29 << 3 | 3:   
        case 0x29 << 3 | 4: 
        case 0x29 << 3 | 5:
        case 0x29 << 3 | 6: 
        case 0x29 << 3 | 7: 
          THUMB_STR_ALLB (STRH, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
        /* =============================== thumb :: str rd, rn, rm =============================== */
        case 0x28 << 3 | 0: 
        case 0x28 << 3 | 1: 
        case 0x28 << 3 | 2: 
        case 0x28 << 3 | 3:  
        case 0x28 << 3 | 4: 
        case 0x28 << 3 | 5: 
        case 0x28 << 3 | 6: 
        case 0x28 << 3 | 7: 
          THUMB_STR_ALLB (STR, (OP_code & 7), (OP_code >> 3 & 7), regs[OP_code >> 6 &7].blk);
          break;
          
  #define THUMB_COND_CASE(cond_code) \
          case 0xD << 6 | (cond_code) << 2 | 0:\
          case 0xD << 6 | (cond_code) << 2 | 1:\
          case 0xD << 6 | (cond_code) << 2 | 2:\
          case 0xD << 6 | (cond_code) << 2 | 3

  #define THUMB_JCC { const int8_t offset = OP_code & 0xFF; const int32_t offset32 = offset; regs[15].blk += offset32 * 2;  consume += thumb_flush (agb) + 2; break; }

          THUMB_COND_CASE (0x00): if (arm->cpsr_fast.flags.sig.z != 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x01): if (arm->cpsr_fast.flags.sig.z == 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x02): if (arm->cpsr_fast.flags.sig.c != 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x03): if (arm->cpsr_fast.flags.sig.c == 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x04): if (arm->cpsr_fast.flags.sig.n != 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x05): if (arm->cpsr_fast.flags.sig.n == 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x06): if (arm->cpsr_fast.flags.sig.v != 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x07): if (arm->cpsr_fast.flags.sig.v == 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x08): if (arm->cpsr_fast.flags.sig.c != 0 && arm->cpsr_fast.flags.sig.z == 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x09): if (arm->cpsr_fast.flags.sig.c == 0 || arm->cpsr_fast.flags.sig.z != 0) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x0A): if (arm->cpsr_fast.flags.sig.n == arm->cpsr_fast.flags.sig.v) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x0B): if (arm->cpsr_fast.flags.sig.n != arm->cpsr_fast.flags.sig.v) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x0C): if (arm->cpsr_fast.flags.sig.z == 0 && (arm->cpsr_fast.flags.sig.n == arm->cpsr_fast.flags.sig.v)) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x0D): if (arm->cpsr_fast.flags.sig.z != 0 || (arm->cpsr_fast.flags.sig.n != arm->cpsr_fast.flags.sig.v)) THUMB_JCC regs[15].blk += 2; return consume + 1;
          THUMB_COND_CASE (0x0E): THUMB_JCC
          default:
            assert (0);
            break;
        }
      }
      regs[15].blk += 2;
    } else {
      if ( (arm->int_magic.blk == 0) ) {
          
        uint32_t pipeline_pc = regs[15].blk;

        cpsr_to_std (& arm->cpsr, & arm->cpsr_fast);

        pipeline_pc &= ARM7_CODE_MASK;
        pipeline_pc -= 8;

        bank_swi (agb, ARM7_MODE_IRQ);

        arm->r1314_t[1+R1314b_IRQ].blk = pipeline_pc  + 4;
        arm->spsr_t[SPSRb_IRQ].blk =  arm->cpsr.blk;
        
        arm->cpsr_fast.mode = ARM7_MODE_IRQ | ARM7_MODE_HSET_MASK;
        arm->cpsr_fast.thumb = false;
        arm->cpsr_fast.irq_disable = true;

        arm->regs[14].blk = pipeline_pc  + 4;
        arm->regs[15].blk = 0x18;

        consume = arm7_flush (agb) + 7;
        arm->regs[15].blk = 0x20 - 4;  

        reset_int_magic (agb);
      } else {
        struct arm7_ldr_sbh_throat {
          uint32_t _1:1;
          uint32_t h:1;
          uint32_t s:1;
          uint32_t __1:1;
          uint32_t l:1;
          uint32_t w:1;
          uint32_t x:1; /* LDR SBH reg | LDR SBH imm must not zero*/
          uint32_t u:1;
          uint32_t p:1;
          uint32_t align:23;
        }; 

        struct arm7_ldr_ubw_throat {
          uint32_t l:1;
          uint32_t w:1;
          uint32_t b:1;
          uint32_t u:1;
          uint32_t p:1;
          uint32_t i:1;
          uint32_t align:26;
        };

        struct arm7_ldm_throat {
          uint32_t l:1;
          uint32_t w:1;
          uint32_t s:1; 
          uint32_t u:1;
          uint32_t p:1;
          uint32_t align:27;
        };
        const uint32_t OP_code = arm->opcode[0];
        const uint32_t OP_throat = OP_code >> 16 & 0xFF0 | OP_code >> 4 & 0xF;
        const uint32_t OP_cond = OP_code & LSHIFT (0x0F, 28);

        arm->opcode[0] = arm->opcode[1];
        consume =  arm7_fecth_n (agb, regs[15].blk, & arm->opcode[1]);
        regs[15].blk &= ARM7_CODE_MASK;

        /* check arm7 cond field */
        switch (OP_cond) {
        case LSHIFT (0x00, 28): if (arm->cpsr_fast.flags.sig.z != 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x01, 28): if (arm->cpsr_fast.flags.sig.z == 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x02, 28): if (arm->cpsr_fast.flags.sig.c != 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x03, 28): if (arm->cpsr_fast.flags.sig.c == 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x04, 28): if (arm->cpsr_fast.flags.sig.n != 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x05, 28): if (arm->cpsr_fast.flags.sig.n == 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x06, 28): if (arm->cpsr_fast.flags.sig.v != 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x07, 28): if (arm->cpsr_fast.flags.sig.v == 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x08, 28): if (arm->cpsr_fast.flags.sig.c != 0 && arm->cpsr_fast.flags.sig.z == 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x09, 28): if (arm->cpsr_fast.flags.sig.c == 0 || arm->cpsr_fast.flags.sig.z != 0) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x0A, 28): if (arm->cpsr_fast.flags.sig.n == arm->cpsr_fast.flags.sig.v) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x0B, 28): if (arm->cpsr_fast.flags.sig.n != arm->cpsr_fast.flags.sig.v) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x0C, 28): if (arm->cpsr_fast.flags.sig.z == 0 && (arm->cpsr_fast.flags.sig.n == arm->cpsr_fast.flags.sig.v)) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x0D, 28): if (arm->cpsr_fast.flags.sig.z != 0 || (arm->cpsr_fast.flags.sig.n != arm->cpsr_fast.flags.sig.v)) break; regs[15].blk += 4; return consume + 1;
        case LSHIFT (0x0E, 28): break;
        case LSHIFT (0x0F, 28):  regs[15].blk += 4; return consume + 1;
        default: assert (0); break;
        }

        /* arm7 opcode decode *.*/
        switch (OP_throat) {
          
        /* ===================== arm7 :: mul =========================== */
        case 0x009:
          {
            const struct arm_mul *const m_code = (const struct arm_mul *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const uint32_t output = rs_value * regs[m_code->rm].blk;
            
            regs[m_code->rd].blk = output;
            
            consume += mul_clks (rs_value) + 1;  
          }
          break;

        /* ===================== arm7 :: muls =========================== */
        case 0x019: 
          {
            const struct arm_mul *const m_code = (const struct arm_mul *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const uint32_t output = rs_value * regs[m_code->rm].blk;
            
            regs[m_code->rd].blk = output;
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v;
            
            if (output == 0)
              arm->cpsr_fast.flags.sig.z = 0x80;
            else if (output & ARM7_NEG_BIT)
              arm->cpsr_fast.flags.sig.n = 0x80;

            consume += mul_clks (rs_value) + 1;  
          }
          break;

        /* ===================== arm7 :: mla =========================== */
        case 0x029:
          {
            const struct arm_mul *const m_code = (const struct arm_mul *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const uint32_t output = rs_value * regs[m_code->rm].blk + regs[m_code->rn].blk;
            
            regs[m_code->rd].blk = output;
            
            consume += mul_clks (rs_value) + 2;  
          }
          break;

        /* ===================== arm7 :: mlas =========================== */
        case 0x039: 
          {
            const struct arm_mul *const m_code = (const struct arm_mul *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const uint32_t output = rs_value * regs[m_code->rm].blk + regs[m_code->rn].blk;
            
            regs[m_code->rd].blk = output;
            
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v;
            
            if (output == 0)
              arm->cpsr_fast.flags.sig.z = 0x80;
            else if (output & ARM7_NEG_BIT)
              arm->cpsr_fast.flags.sig.n = 0x80;

            consume += mul_clks (rs_value) + 2;  
          }
          break;
          
        /* ===================== arm7 :: umull =========================== */
        case 0x089: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const uint64_t rs_value_u64 = rs_value;
            const uint64_t rm_value_u64 = regs[m_code->rm].blk;
            const uint64_t output = rs_value_u64 * rm_value_u64;
            const uint32_t *const res_ptr = (const uint32_t *const) & output;

            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
            
            consume += mul_clks (rs_value) + 2;    
          }
          break;
           
        /* ===================== arm7 :: umulls =========================== */
        case 0x099: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const uint64_t rs_value_u64 = rs_value;
            const uint64_t rm_value_u64 = regs[m_code->rm].blk;
            const uint64_t output = rs_value_u64 * rm_value_u64;
            const uint32_t *const res_ptr = (const uint32_t *const) & output;

            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];

#ifdef UMULLS_DESTROY_FLAG_V        
            arm->cpsr_fast.flags.blk = 0;
#else 
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v;
#endif 
            if (res_ptr[1] & ARM7_NEG_BIT)
              arm->cpsr_fast.flags.sig.n = 0x80;
            else if (output == 0)
              arm->cpsr_fast.flags.sig.z = 0x80;

            consume += mul_clks (rs_value) + 2;    
          }
          break;        
      
        /* ===================== arm7 :: umlal =========================== */
        case 0x0A9: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = arm->regs[m_code->rs].blk;
            const uint64_t rs_value_u64 = rs_value;
            const uint64_t rm_value_u64 = regs[m_code->rm].blk;
            const uint64_t rd_lo_value_u64 = regs[m_code->rd_lo].blk;
            const uint64_t rd_hi_value_u64 = regs[m_code->rd_hi].blk;
            const uint64_t output = rs_value_u64 * rm_value_u64 + (( rd_hi_value_u64 << 32) | rd_lo_value_u64);
            const uint32_t *const res_ptr = (const uint32_t *const) & output;
            
            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
            
            consume += mul_clks (rs_value) + 3;    
          }
          break;
          
        /* ===================== arm7 :: umlals =========================== */
        case 0x0B9: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = arm->regs[m_code->rs].blk;
            const uint64_t rs_value_u64 = rs_value;
            const uint64_t rm_value_u64 = regs[m_code->rm].blk;
            const uint64_t rd_lo_value_u64 = regs[m_code->rd_lo].blk;
            const uint64_t rd_hi_value_u64 = regs[m_code->rd_hi].blk;
            const uint64_t output = rs_value_u64 * rm_value_u64 + (( rd_hi_value_u64 << 32) | rd_lo_value_u64);
            const uint32_t *const res_ptr = (const uint32_t *const) & output;
            
            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
            
#ifdef UMLALS_DESTROY_FLAG_V     
            arm->cpsr_fast.flags.blk = 0;
#else 
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v;
#endif 
            if (res_ptr[1] & ARM7_NEG_BIT)
              arm->cpsr_fast.flags.sig.n = 0x80;
            else if (output == 0)
              arm->cpsr_fast.flags.sig.z = 0x80;

            consume += mul_clks (rs_value) + 3;    
          }
          break; 

        /* ===================== arm7 :: smull =========================== */
        case 0x0C9: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const int64_t rs_value_s64 = regs[m_code->rs].sblk;
            const int64_t rm_value_s64 = regs[m_code->rm].sblk;
            const int64_t output = rs_value_s64 * rm_value_s64;
            const uint32_t *const res_ptr = (const uint32_t *const) & output;
            
            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
            
            consume += mul_clks (rs_value) + 2;      
          }
          break;
          
        /* ===================== arm7 :: smulls =========================== */
        case 0x0D9: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const int64_t rs_value_s64 = regs[m_code->rs].sblk;
            const int64_t rm_value_s64 = regs[m_code->rm].sblk;
            const int64_t output = rs_value_s64 * rm_value_s64;
            const uint32_t *const res_ptr = (const uint32_t *const) & output;
            
            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
                     
#ifdef SMULLS_DESTROY_FLAG_V
            arm->cpsr_fast.flags.blk = 0;
#else 
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v;
#endif 
            if (res_ptr[1] & ARM7_NEG_BIT)
              arm->cpsr_fast.flags.sig.n = 0x80;
            else if (output == 0)
              arm->cpsr_fast.flags.sig.z = 0x80;

            consume += mul_clks (rs_value) + 2;   
          }
          break; 
     
        /* ===================== arm7 :: smlal =========================== */
        case 0x0E9: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const int64_t rs_value_s64 = regs[m_code->rs].sblk;
            const int64_t rm_value_s64 = regs[m_code->rm].sblk;
            const uint64_t rd_lo_value_u64 = regs[m_code->rd_lo].blk;
            const uint64_t rd_hi_value_u64 = regs[m_code->rd_hi].blk;
            const uint64_t output = rs_value_s64 * rm_value_s64 + (int64_t) (( rd_hi_value_u64 << 32) | rd_lo_value_u64);
            const uint32_t *const res_ptr = (const uint32_t *const) & output;
            
            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
            
            consume += mul_clks (rs_value) + 3;        
          }
          break;

        /* ===================== arm7 :: smlals =========================== */
        case 0x0F9: 
          {
            const struct arm_mul_long *const m_code = (const struct arm_mul_long *const) & OP_code;
            const uint32_t rs_value = regs[m_code->rs].blk;
            const int64_t rs_value_s64 = regs[m_code->rs].sblk;
            const int64_t rm_value_s64 = regs[m_code->rm].sblk;
            const uint64_t rd_lo_value_u64 = regs[m_code->rd_lo].blk;
            const uint64_t rd_hi_value_u64 = regs[m_code->rd_hi].blk;
            const uint64_t output = rs_value_s64 * rm_value_s64 + (int64_t) (( rd_hi_value_u64 << 32) | rd_lo_value_u64);
            const uint32_t *const res_ptr = (const uint32_t *const) & output;
            
            regs[m_code->rd_lo].blk = res_ptr[0];
            regs[m_code->rd_hi].blk = res_ptr[1];
            
#ifdef SMLALS_DESTROY_FLAG_V           
            arm->cpsr_fast.flags.blk = 0;
#else 
            arm->cpsr_fast.flags.blk = arm->cpsr_fast.flags.sig.v;
#endif 
            if (res_ptr[1] & ARM7_NEG_BIT)
              arm->cpsr_fast.flags.sig.n = 0x80;
            else if (output == 0)
              arm->cpsr_fast.flags.sig.z = 0x80; 

            consume += mul_clks (rs_value) + 3;    
          }
          break; 
   
        /* ===================== arm7 :: swp =========================== */
        case 0x109: 
          {
            uint32_t memory;
            const struct arm_atomic_memory_access *const m_code = (const struct arm_atomic_memory_access *const) & OP_code;
            const uint32_t rn_address = regs[m_code->rn].blk;
            const uint32_t rn_rorate = (rn_address & 3) << 3;
            
            consume += agb_mbus_rw (agb, rn_address, & memory, false);
            consume += agb_mbus_ww (agb, rn_address, regs[m_code->rm].blk, false) + 4;

            if (rn_rorate != 0)
              regs[m_code->rd].blk = memory >> rn_rorate | memory << 32 - rn_rorate;
            else 
              regs[m_code->rd].blk = memory;
          }
          break;

        /* ==================== arm7 :: swpb ========================== */
        case 0x149:
          {
            uint8_t memory;
            const struct arm_atomic_memory_access *const m_code = (const struct arm_atomic_memory_access *const) & OP_code;
            
            consume += agb_mbus_rb (agb, regs[m_code->rn].blk, & memory, false);
            consume += agb_mbus_wb (agb, regs[m_code->rn].blk, regs[m_code->rm].blk, false) + 4;
            regs[m_code->rd].blk = memory;
          } 
          break;

        /* ================== arm7 :: mrs cpsr ======================== */
        case 0x100:
          cpsr_to_std (& arm->cpsr, & arm->cpsr_fast);
          
          regs[OP_code >> 12 & 15].blk = arm->cpsr.blk;
          consume ++;
          break;

        /* ================== arm7 :: mrs spsr ======================== */
        case 0x140:
          regs[OP_code >> 12 & 15].blk = arm7_cur_spsr (agb)->blk;
          consume ++;
          break;

        /* =============== arm7 :: msr cpsr rot imm32  =============== */
        case 0x320: 
        case 0x321: 
        case 0x322: 
        case 0x323: 
        case 0x324: 
        case 0x325: 
        case 0x326: 
        case 0x327:
        case 0x328: 
        case 0x329: 
        case 0x32A:
        case 0x32B:
        case 0x32C:
        case 0x32D: 
        case 0x32E:
        case 0x32F:
          {
            const struct arm_msr_rot_imm32 *const m_code = (const struct arm_msr_rot_imm32 *const) & OP_code;
            const uint32_t imm32 = ROT_IMM32_FAST (m_code->imm8, m_code->rotate_imm);
            
            isa_arm7_store_cpsr_base (agb, m_code->field_c, m_code->field_f, imm32);
            
            consume ++;
          }
          break;

        /* ============ arm7 :: msr spsr rot imm32 ================== */
        case 0x360: 
        case 0x361: 
        case 0x362: 
        case 0x363:
        case 0x364: 
        case 0x365: 
        case 0x366: 
        case 0x367: 
        case 0x368: 
        case 0x369: 
        case 0x36A: 
        case 0x36B: 
        case 0x36C:
        case 0x36D:
        case 0x36E: 
        case 0x36F:
          {
            const struct arm_msr_rot_imm32 *const m_code = (const struct arm_msr_rot_imm32 *const) & OP_code;
            const uint32_t imm32 = ROT_IMM32_FAST (m_code->imm8, m_code->rotate_imm);
            
            isa_arm7_store_spsr_base (agb, m_code->field_c, m_code->field_f, imm32);
            
            consume ++;
          }
          break;

        /* ===================== arm7 :: msr cpsr reg =========================== */
        case 0x120:
          {
            const struct arm_msr_reg *const m_code = (const struct arm_msr_reg *const) & OP_code;
            
            isa_arm7_store_cpsr_base (agb, m_code->field_c, m_code->field_f, regs[m_code->rm].blk);
            
            consume ++;
          }
          break;

        /* ===================== arm7 :: msr spsr reg =========================== */
        case 0x160: 
          {
            const struct arm_msr_reg *const m_code = (const struct arm_msr_reg *const) & OP_code;
            
            isa_arm7_store_spsr_base (agb, m_code->field_c, m_code->field_f, regs[m_code->rm].blk);
            
            consume ++;
          }
          break;

        /* ===================== arm7 :: swi =========================== */
        case 0xF00: case 0xF01: case 0xF02: case 0xF03:
        case 0xF04: case 0xF05: case 0xF06: case 0xF07:
        case 0xF08: case 0xF09: case 0xF0A: case 0xF0B:
        case 0xF0C: case 0xF0D: case 0xF0E:
        case 0xF0F:
        case 0xF10: case 0xF11: case 0xF12: case 0xF13:
        case 0xF14: case 0xF15: case 0xF16: case 0xF17:
        case 0xF18: case 0xF19: case 0xF1A: case 0xF1B:
        case 0xF1C: case 0xF1D: case 0xF1E:
        case 0xF1F:
        case 0xF20: case 0xF21: case 0xF22: case 0xF23:
        case 0xF24: case 0xF25: case 0xF26: case 0xF27:
        case 0xF28: case 0xF29: case 0xF2A: case 0xF2B:
        case 0xF2C: case 0xF2D: case 0xF2E:
        case 0xF2F:
        case 0xF30: case 0xF31: case 0xF32: case 0xF33:
        case 0xF34: case 0xF35: case 0xF36: case 0xF37:
        case 0xF38: case 0xF39: case 0xF3A: case 0xF3B:
        case 0xF3C: case 0xF3D: case 0xF3E:
        case 0xF3F:
        case 0xF40: case 0xF41: case 0xF42: case 0xF43:
        case 0xF44: case 0xF45: case 0xF46: case 0xF47:
        case 0xF48: case 0xF49: case 0xF4A: case 0xF4B:
        case 0xF4C: case 0xF4D: case 0xF4E:
        case 0xF4F:
        case 0xF50: case 0xF51: case 0xF52: case 0xF53:
        case 0xF54: case 0xF55: case 0xF56: case 0xF57:
        case 0xF58: case 0xF59: case 0xF5A: case 0xF5B:
        case 0xF5C: case 0xF5D: case 0xF5E:
        case 0xF5F:
        case 0xF60: case 0xF61: case 0xF62: case 0xF63:
        case 0xF64: case 0xF65: case 0xF66: case 0xF67:
        case 0xF68: case 0xF69: case 0xF6A: case 0xF6B:
        case 0xF6C: case 0xF6D: case 0xF6E:
        case 0xF6F:
        case 0xF70: case 0xF71: case 0xF72: case 0xF73:
        case 0xF74: case 0xF75: case 0xF76: case 0xF77:
        case 0xF78: case 0xF79: case 0xF7A: case 0xF7B:
        case 0xF7C: case 0xF7D: case 0xF7E:
        case 0xF7F:
        case 0xF80: case 0xF81: case 0xF82: case 0xF83:
        case 0xF84: case 0xF85: case 0xF86: case 0xF87:
        case 0xF88: case 0xF89: case 0xF8A: case 0xF8B:
        case 0xF8C: case 0xF8D: case 0xF8E:
        case 0xF8F:
        case 0xF90: case 0xF91: case 0xF92: case 0xF93:
        case 0xF94: case 0xF95: case 0xF96: case 0xF97:
        case 0xF98: case 0xF99: case 0xF9A: case 0xF9B:
        case 0xF9C: case 0xF9D: case 0xF9E:
        case 0xF9F:
        case 0xFA0: case 0xFA1: case 0xFA2: case 0xFA3:
        case 0xFA4: case 0xFA5: case 0xFA6: case 0xFA7:
        case 0xFA8: case 0xFA9: case 0xFAA: case 0xFAB:
        case 0xFAC: case 0xFAD: case 0xFAE:
        case 0xFAF:
        case 0xFB0: case 0xFB1: case 0xFB2: case 0xFB3:
        case 0xFB4: case 0xFB5: case 0xFB6: case 0xFB7:
        case 0xFB8: case 0xFB9: case 0xFBA: case 0xFBB:
        case 0xFBC: case 0xFBD: case 0xFBE:
        case 0xFBF:
        case 0xFC0: case 0xFC1: case 0xFC2: case 0xFC3:
        case 0xFC4: case 0xFC5: case 0xFC6: case 0xFC7:
        case 0xFC8: case 0xFC9: case 0xFCA: case 0xFCB:
        case 0xFCC: case 0xFCD: case 0xFCE:
        case 0xFCF:
        case 0xFD0: case 0xFD1: case 0xFD2: case 0xFD3:
        case 0xFD4: case 0xFD5: case 0xFD6: case 0xFD7:
        case 0xFD8: case 0xFD9: case 0xFDA: case 0xFDB:
        case 0xFDC: case 0xFDD: case 0xFDE:
        case 0xFDF:
        case 0xFE0: case 0xFE1: case 0xFE2: case 0xFE3:
        case 0xFE4: case 0xFE5: case 0xFE6: case 0xFE7:
        case 0xFE8: case 0xFE9: case 0xFEA: case 0xFEB:
        case 0xFEC: case 0xFED: case 0xFEE:
        case 0xFEF:
        case 0xFF0: case 0xFF1: case 0xFF2: case 0xFF3:
        case 0xFF4: case 0xFF5: case 0xFF6: case 0xFF7:
        case 0xFF8: case 0xFF9: case 0xFFA: case 0xFFB:
        case 0xFFC: case 0xFFD: case 0xFFE:
        case 0xFFF:
          {
            const uint32_t pipeline_pc = regs[15].blk &  ARM7_CODE_MASK;
            cpsr_to_std (& arm->cpsr, & arm->cpsr_fast);
            
            bank_swi (agb, ARM7_MODE_MGR);

            arm->r1314_t[1+R1314b_MGR].blk = pipeline_pc  - 4;
            arm->spsr_t[SPSRb_MGR].blk =  arm->cpsr.blk;
            arm->regs[14].blk = pipeline_pc  - 4;
            
            arm->cpsr_fast.mode = ARM7_MODE_MGR | ARM7_MODE_HSET_MASK;
            arm->cpsr_fast.thumb = false;
            arm->cpsr_fast.irq_disable = true;
 
            arm->regs[15].blk = 8;

            consume += arm7_flush (agb) + 3;
            reset_int_magic (agb);           
          }
          break;

        /* ===================== arm7 :: b =========================== */
        case 0xA00: case 0xA01: case 0xA02: case 0xA03:
        case 0xA04: case 0xA05: case 0xA06: case 0xA07:
        case 0xA08: case 0xA09: case 0xA0A: case 0xA0B:
        case 0xA0C: case 0xA0D: case 0xA0E:
        case 0xA0F:
        case 0xA10: case 0xA11: case 0xA12: case 0xA13:
        case 0xA14: case 0xA15: case 0xA16: case 0xA17:
        case 0xA18: case 0xA19: case 0xA1A: case 0xA1B:
        case 0xA1C: case 0xA1D: case 0xA1E:
        case 0xA1F:
        case 0xA20: case 0xA21: case 0xA22: case 0xA23:
        case 0xA24: case 0xA25: case 0xA26: case 0xA27:
        case 0xA28: case 0xA29: case 0xA2A: case 0xA2B:
        case 0xA2C: case 0xA2D: case 0xA2E:
        case 0xA2F:
        case 0xA30: case 0xA31: case 0xA32: case 0xA33:
        case 0xA34: case 0xA35: case 0xA36: case 0xA37:
        case 0xA38: case 0xA39: case 0xA3A: case 0xA3B:
        case 0xA3C: case 0xA3D: case 0xA3E:
        case 0xA3F:
        case 0xA40: case 0xA41: case 0xA42: case 0xA43:
        case 0xA44: case 0xA45: case 0xA46: case 0xA47:
        case 0xA48: case 0xA49: case 0xA4A: case 0xA4B:
        case 0xA4C: case 0xA4D: case 0xA4E:
        case 0xA4F:
        case 0xA50: case 0xA51: case 0xA52: case 0xA53:
        case 0xA54: case 0xA55: case 0xA56: case 0xA57:
        case 0xA58: case 0xA59: case 0xA5A: case 0xA5B:
        case 0xA5C: case 0xA5D: case 0xA5E:
        case 0xA5F:
        case 0xA60: case 0xA61: case 0xA62: case 0xA63:
        case 0xA64: case 0xA65: case 0xA66: case 0xA67:
        case 0xA68: case 0xA69: case 0xA6A: case 0xA6B:
        case 0xA6C: case 0xA6D: case 0xA6E:
        case 0xA6F:
        case 0xA70: case 0xA71: case 0xA72: case 0xA73:
        case 0xA74: case 0xA75: case 0xA76: case 0xA77:
        case 0xA78: case 0xA79: case 0xA7A: case 0xA7B:
        case 0xA7C: case 0xA7D: case 0xA7E:
        case 0xA7F:
        case 0xA80: case 0xA81: case 0xA82: case 0xA83:
        case 0xA84: case 0xA85: case 0xA86: case 0xA87:
        case 0xA88: case 0xA89: case 0xA8A: case 0xA8B:
        case 0xA8C: case 0xA8D: case 0xA8E:
        case 0xA8F:
        case 0xA90: case 0xA91: case 0xA92: case 0xA93:
        case 0xA94: case 0xA95: case 0xA96: case 0xA97:
        case 0xA98: case 0xA99: case 0xA9A: case 0xA9B:
        case 0xA9C: case 0xA9D: case 0xA9E:
        case 0xA9F:
        case 0xAA0: case 0xAA1: case 0xAA2: case 0xAA3:
        case 0xAA4: case 0xAA5: case 0xAA6: case 0xAA7:
        case 0xAA8: case 0xAA9: case 0xAAA: case 0xAAB:
        case 0xAAC: case 0xAAD: case 0xAAE:
        case 0xAAF:
        case 0xAB0: case 0xAB1: case 0xAB2: case 0xAB3:
        case 0xAB4: case 0xAB5: case 0xAB6: case 0xAB7:
        case 0xAB8: case 0xAB9: case 0xABA: case 0xABB:
        case 0xABC: case 0xABD: case 0xABE:
        case 0xABF:
        case 0xAC0: case 0xAC1: case 0xAC2: case 0xAC3:
        case 0xAC4: case 0xAC5: case 0xAC6: case 0xAC7:
        case 0xAC8: case 0xAC9: case 0xACA: case 0xACB:
        case 0xACC: case 0xACD: case 0xACE:
        case 0xACF:
        case 0xAD0: case 0xAD1: case 0xAD2: case 0xAD3:
        case 0xAD4: case 0xAD5: case 0xAD6: case 0xAD7:
        case 0xAD8: case 0xAD9: case 0xADA: case 0xADB:
        case 0xADC: case 0xADD: case 0xADE:
        case 0xADF:
        case 0xAE0: case 0xAE1: case 0xAE2: case 0xAE3:
        case 0xAE4: case 0xAE5: case 0xAE6: case 0xAE7:
        case 0xAE8: case 0xAE9: case 0xAEA: case 0xAEB:
        case 0xAEC: case 0xAED: case 0xAEE:
        case 0xAEF:
        case 0xAF0: case 0xAF1: case 0xAF2: case 0xAF3:
        case 0xAF4: case 0xAF5: case 0xAF6: case 0xAF7:
        case 0xAF8: case 0xAF9: case 0xAFA: case 0xAFB:
        case 0xAFC: case 0xAFD: case 0xAFE:
        case 0xAFF:
          {
            const struct arm_branch *const m_code = (const struct arm_branch *const) & OP_code;
            const uint32_t pipeline_pc = (regs[15].blk & ARM7_CODE_MASK);
            uint32_t offset;

            if (m_code->sign_bit != 0) {
              const uint32_t mask = 0x7FFFFF;
              offset = (m_code->sign_imm & mask |  ~mask) << 2;
            } else {
              offset = m_code->sign_imm << 2;
            }
            regs[15].blk = pipeline_pc + offset;

            consume += arm7_flush (agb) + 3;
          }
          break;

        /* ===================== arm7 :: bl =========================== */
        case 0xB00: case 0xB01: case 0xB02: case 0xB03:
        case 0xB04: case 0xB05: case 0xB06: case 0xB07:
        case 0xB08: case 0xB09: case 0xB0A: case 0xB0B:
        case 0xB0C: case 0xB0D: case 0xB0E:
        case 0xB0F:
        case 0xB10: case 0xB11: case 0xB12: case 0xB13:
        case 0xB14: case 0xB15: case 0xB16: case 0xB17:
        case 0xB18: case 0xB19: case 0xB1A: case 0xB1B:
        case 0xB1C: case 0xB1D: case 0xB1E:
        case 0xB1F:
        case 0xB20: case 0xB21: case 0xB22: case 0xB23:
        case 0xB24: case 0xB25: case 0xB26: case 0xB27:
        case 0xB28: case 0xB29: case 0xB2A: case 0xB2B:
        case 0xB2C: case 0xB2D: case 0xB2E:
        case 0xB2F:
        case 0xB30: case 0xB31: case 0xB32: case 0xB33:
        case 0xB34: case 0xB35: case 0xB36: case 0xB37:
        case 0xB38: case 0xB39: case 0xB3A: case 0xB3B:
        case 0xB3C: case 0xB3D: case 0xB3E:
        case 0xB3F:
        case 0xB40: case 0xB41: case 0xB42: case 0xB43:
        case 0xB44: case 0xB45: case 0xB46: case 0xB47:
        case 0xB48: case 0xB49: case 0xB4A: case 0xB4B:
        case 0xB4C: case 0xB4D: case 0xB4E:
        case 0xB4F:
        case 0xB50: case 0xB51: case 0xB52: case 0xB53:
        case 0xB54: case 0xB55: case 0xB56: case 0xB57:
        case 0xB58: case 0xB59: case 0xB5A: case 0xB5B:
        case 0xB5C: case 0xB5D: case 0xB5E:
        case 0xB5F:
        case 0xB60: case 0xB61: case 0xB62: case 0xB63:
        case 0xB64: case 0xB65: case 0xB66: case 0xB67:
        case 0xB68: case 0xB69: case 0xB6A: case 0xB6B:
        case 0xB6C: case 0xB6D: case 0xB6E:
        case 0xB6F:
        case 0xB70: case 0xB71: case 0xB72: case 0xB73:
        case 0xB74: case 0xB75: case 0xB76: case 0xB77:
        case 0xB78: case 0xB79: case 0xB7A: case 0xB7B:
        case 0xB7C: case 0xB7D: case 0xB7E:
        case 0xB7F:
        case 0xB80: case 0xB81: case 0xB82: case 0xB83:
        case 0xB84: case 0xB85: case 0xB86: case 0xB87:
        case 0xB88: case 0xB89: case 0xB8A: case 0xB8B:
        case 0xB8C: case 0xB8D: case 0xB8E:
        case 0xB8F:
        case 0xB90: case 0xB91: case 0xB92: case 0xB93:
        case 0xB94: case 0xB95: case 0xB96: case 0xB97:
        case 0xB98: case 0xB99: case 0xB9A: case 0xB9B:
        case 0xB9C: case 0xB9D: case 0xB9E:
        case 0xB9F:
        case 0xBA0: case 0xBA1: case 0xBA2: case 0xBA3:
        case 0xBA4: case 0xBA5: case 0xBA6: case 0xBA7:
        case 0xBA8: case 0xBA9: case 0xBAA: case 0xBAB:
        case 0xBAC: case 0xBAD: case 0xBAE:
        case 0xBAF:
        case 0xBB0: case 0xBB1: case 0xBB2: case 0xBB3:
        case 0xBB4: case 0xBB5: case 0xBB6: case 0xBB7:
        case 0xBB8: case 0xBB9: case 0xBBA: case 0xBBB:
        case 0xBBC: case 0xBBD: case 0xBBE:
        case 0xBBF:
        case 0xBC0: case 0xBC1: case 0xBC2: case 0xBC3:
        case 0xBC4: case 0xBC5: case 0xBC6: case 0xBC7:
        case 0xBC8: case 0xBC9: case 0xBCA: case 0xBCB:
        case 0xBCC: case 0xBCD: case 0xBCE:
        case 0xBCF:
        case 0xBD0: case 0xBD1: case 0xBD2: case 0xBD3:
        case 0xBD4: case 0xBD5: case 0xBD6: case 0xBD7:
        case 0xBD8: case 0xBD9: case 0xBDA: case 0xBDB:
        case 0xBDC: case 0xBDD: case 0xBDE:
        case 0xBDF:
        case 0xBE0: case 0xBE1: case 0xBE2: case 0xBE3:
        case 0xBE4: case 0xBE5: case 0xBE6: case 0xBE7:
        case 0xBE8: case 0xBE9: case 0xBEA: case 0xBEB:
        case 0xBEC: case 0xBED: case 0xBEE:
        case 0xBEF:
        case 0xBF0: case 0xBF1: case 0xBF2: case 0xBF3:
        case 0xBF4: case 0xBF5: case 0xBF6: case 0xBF7:
        case 0xBF8: case 0xBF9: case 0xBFA: case 0xBFB:
        case 0xBFC: case 0xBFD: case 0xBFE:
        case 0xBFF:
          {
            const struct arm_branch *const m_code = (const struct arm_branch *const) & OP_code;
            const uint32_t pipeline_pc = (regs[15].blk & ARM7_CODE_MASK);
            const uint32_t link_reg = pipeline_pc - 4;
            uint32_t offset;

            if (m_code->sign_bit != 0) {
              const uint32_t mask = 0x7FFFFF;
              offset = (m_code->sign_imm & mask |  ~mask) << 2;
            } else {
              offset = m_code->sign_imm << 2;
            }
            regs[14].blk = link_reg;
            regs[15].blk = pipeline_pc + offset;

            consume += arm7_flush (agb) + 3;
          }
          break;
        
        /* ===================== arm7 :: bx =========================== */
        case 0x121:
          {
            const struct arm_branch_exchange *const m_code = (const struct arm_branch_exchange *const) & OP_code;
            const uint32_t rm_value = regs[m_code->rm].blk;

            if (rm_value & 1) {
              /* to thumb mode, flush thumb pipeline */
              regs[15].blk = rm_value & THUMB_CODE_MASK;
              consume += thumb_flush (agb) + 3;
              regs[15].blk -= 2;
              arm->cpsr_fast.thumb = true;
            } else {
              /* to arm7 mode, flush arm7 pipeline  */
              regs[15].blk = rm_value & ARM7_CODE_MASK;
              consume += arm7_flush (agb) + 3;
              arm->cpsr_fast.thumb = false;
            } 
          }
          break;

  #define ALU_BLOCK_NO_SIGN(throat_alu, OP_alu)\
        case 0x200 | (throat_alu) << 5 | 0:         \
        case 0x200 | (throat_alu) << 5 | 1:         \
        case 0x200 | (throat_alu) << 5 | 2:         \
        case 0x200 | (throat_alu) << 5 | 3:         \
        case 0x200 | (throat_alu) << 5 | 4:         \
        case 0x200 | (throat_alu) << 5 | 5:         \
        case 0x200 | (throat_alu) << 5 | 6:         \
        case 0x200 | (throat_alu) << 5 | 7:         \
        case 0x200 | (throat_alu) << 5 | 8:         \
        case 0x200 | (throat_alu) << 5 | 9:         \
        case 0x200 | (throat_alu) << 5 |10:         \
        case 0x200 | (throat_alu) << 5 |11:         \
        case 0x200 | (throat_alu) << 5 |12:         \
        case 0x200 | (throat_alu) << 5 |13:         \
        case 0x200 | (throat_alu) << 5 |14:         \
        case 0x200 | (throat_alu) << 5 |15:         \
          ISA_ARM7_ALU_NO_SIGN_ROT_IMM32 (OP_alu); \
          consume ++;                                   \
          break;                                        \
        case (throat_alu) << 5 | 0:                     \
        case (throat_alu) << 5 | 8: ISA_ARM7_ALU_NO_SIGN_SHIFT_IMM5 (OP_alu, LSL_IMM5_NO_SIGN);consume ++;break; \
        case (throat_alu) << 5 | 2:                     \
        case (throat_alu) << 5 |10: ISA_ARM7_ALU_NO_SIGN_SHIFT_IMM5 (OP_alu, LSR_IMM5_NO_SIGN);consume ++;break; \
        case (throat_alu) << 5 | 4:                     \
        case (throat_alu) << 5 |12: ISA_ARM7_ALU_NO_SIGN_SHIFT_IMM5 (OP_alu, ASR_IMM5_NO_SIGN);consume ++;break; \
        case (throat_alu) << 5 | 6:                     \
        case (throat_alu) << 5 |14: ISA_ARM7_ALU_NO_SIGN_SHIFT_IMM5 (OP_alu, ROR_IMM5_NO_SIGN);break;            \
        case (throat_alu) << 5 | 1: ISA_ARM7_ALU_NO_SIGN_SHIFT_RS (OP_alu, LSL_RS_NO_SIGN); consume ++; break;   \
        case (throat_alu) << 5 | 3: ISA_ARM7_ALU_NO_SIGN_SHIFT_RS (OP_alu, LSR_RS_NO_SIGN); consume ++; break;   \
        case (throat_alu) << 5 | 5: ISA_ARM7_ALU_NO_SIGN_SHIFT_RS (OP_alu, ASR_RS_NO_SIGN); consume ++; break;   \
        case (throat_alu) << 5 | 7: ISA_ARM7_ALU_NO_SIGN_SHIFT_RS (OP_alu, ROR_RS_NO_SIGN); consume ++; break; \

  #define ALU_BLOCK_SIGN(throat_alu, OP_alu, OP_w, OP_c)\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 0:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 1:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 2:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 3:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 4:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 5:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 6:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 7:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 8:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 | 9:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 |10:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 |11:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 |12:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 |13:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 |14:\
        case 0x200 | (throat_alu) << 5 | 1 << 4 |15:\
          ISA_ARM7_ALU_SIGN_ROT_IMM32 (OP_alu, OP_w, OP_c); \
          consume ++; \
          break;      \
        case (throat_alu) << 5 | 1 << 4 | 0: \
        case (throat_alu) << 5 | 1 << 4 | 8: ISA_ARM7_ALU_SIGN_SHIFT_IMM5 (OP_alu, LSL_IMM5_NO_SIGN, LSL_IMM5_SIGN, OP_w, OP_c);consume ++;break; \
        case (throat_alu) << 5 | 1 << 4 | 2: \
        case (throat_alu) << 5 | 1 << 4 |10: ISA_ARM7_ALU_SIGN_SHIFT_IMM5 (OP_alu, LSR_IMM5_NO_SIGN, LSR_IMM5_SIGN, OP_w, OP_c);consume ++;break; \
        case (throat_alu) << 5 | 1 << 4 | 4: \
        case (throat_alu) << 5 | 1 << 4 |12: ISA_ARM7_ALU_SIGN_SHIFT_IMM5 (OP_alu, ASR_IMM5_NO_SIGN, ASR_IMM5_SIGN, OP_w, OP_c);consume ++;break; \
        case (throat_alu) << 5 | 1 << 4 | 6: \
        case (throat_alu) << 5 | 1 << 4 |14: ISA_ARM7_ALU_SIGN_SHIFT_IMM5 (OP_alu, ROR_IMM5_NO_SIGN, ROR_IMM5_SIGN, OP_w, OP_c);break; \
        case (throat_alu) << 5 | 1 << 4 | 1: ISA_ARM7_ALU_SIGN_SHIFT_RS (OP_alu, LSL_RS_NO_SIGN, LSL_RS_SIGN, OP_w, OP_c); consume ++; break; \
        case (throat_alu) << 5 | 1 << 4 | 3: ISA_ARM7_ALU_SIGN_SHIFT_RS (OP_alu, LSR_RS_NO_SIGN, LSR_RS_SIGN, OP_w, OP_c); consume ++; break; \
        case (throat_alu) << 5 | 1 << 4 | 5: ISA_ARM7_ALU_SIGN_SHIFT_RS (OP_alu, ASR_RS_NO_SIGN, ASR_RS_SIGN, OP_w, OP_c); consume ++; break; \
        case (throat_alu) << 5 | 1 << 4 | 7: ISA_ARM7_ALU_SIGN_SHIFT_RS (OP_alu, ROR_RS_NO_SIGN, ROR_RS_SIGN, OP_w, OP_c); consume ++; break; 
        
        ALU_BLOCK_NO_SIGN (0, OP_AND)
        ALU_BLOCK_NO_SIGN (1, OP_EOR)
        ALU_BLOCK_NO_SIGN (2, OP_SUB)
        ALU_BLOCK_NO_SIGN (3, OP_RSB)
        ALU_BLOCK_NO_SIGN (4, OP_ADD)
        ALU_BLOCK_NO_SIGN (5, OP_ADC)
        ALU_BLOCK_NO_SIGN (6, OP_SBC)
        ALU_BLOCK_NO_SIGN (7, OP_RSC)
        ALU_BLOCK_NO_SIGN (12, OP_ORR)
        ALU_BLOCK_NO_SIGN (13, OP_MOV)
        ALU_BLOCK_NO_SIGN (14, OP_BIC)
        ALU_BLOCK_NO_SIGN (15, OP_MVN)
        
        ALU_BLOCK_SIGN (0, OP_ANDS, true, false)
        ALU_BLOCK_SIGN (1, OP_EORS, true, false)
        ALU_BLOCK_SIGN (2, OP_SUBS, true, true)
        ALU_BLOCK_SIGN (3, OP_RSBS, true, true)
        ALU_BLOCK_SIGN (4, OP_ADDS, true, true)
        ALU_BLOCK_SIGN (5, OP_ADCS, true, true)
        ALU_BLOCK_SIGN (6, OP_SBCS, true, true)
        ALU_BLOCK_SIGN (7, OP_RSCS, true, true)
        ALU_BLOCK_SIGN (8, OP_TSTS, false, false)
        ALU_BLOCK_SIGN (9, OP_TEQS, false, false)
        ALU_BLOCK_SIGN (10, OP_CMPS, false, true)
        ALU_BLOCK_SIGN (11, OP_CMNS, false, true)    
        ALU_BLOCK_SIGN (12, OP_ORRS, true, false)
        ALU_BLOCK_SIGN (13, OP_MOVS, true, false)
        ALU_BLOCK_SIGN (14, OP_BICS, true, false)
        ALU_BLOCK_SIGN (15, OP_MVNS, true, false)
          
        /* ARM7 LDRSB, LDRH, LDRSH, STRH */
  #define MAKE_SH(l, s, h) \
    (l << 4 | h << 1 | s << 2 | 9) 
  #define LDRSB_MASK MAKE_SH(1,1,0) 
  #define LDRH_MASK MAKE_SH(1,0,1)
  #define LDRSH_MASK MAKE_SH(1,1,1)
  #define STRH_MASK MAKE_SH(0,0,1)
    
  #define ARM7_JMEM_CASE(SH_mask, ARM7_MEM_ALL_B, ARM7_MEM_EL, pre_address, post, write_back)   \
    case SH_mask | 1 << 6 | write_back << 5 | pre_address << 8 | post << 7:                     \
      {                                                                                         \
        const uint32_t rd = OP_code >> 12 & 15;                                                 \
        const uint32_t rn = OP_code >> 16 & 15;                                                 \
        const uint32_t imm8 = OP_code & 0x0F | (OP_code >> 4 & 0xF0);                           \
        const uint32_t rn_w = pre_address != 0 ? write_back : 1;                                \
                                                                                                \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, imm8, rn_w);                    \
      }                                                                                         \
      break;                                                                                    \
    case SH_mask |          write_back << 5 | pre_address << 8 | post << 7:                     \
      {                                                                                         \
        const uint32_t rd = OP_code >> 12 & 15;                                                 \
        const uint32_t rn = OP_code >> 16 & 15;                                                 \
        const uint32_t rm_v = regs[OP_code & 15].blk;                                               \
        const uint32_t rn_w = pre_address != 0 ? write_back : 1;                                \
                                                                                                \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, rm_v, rn_w);                    \
      }                                                                                         \
      break;
   
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   0, 0, 0)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   0, 0, 1)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   0, 1, 0)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   0, 1, 1)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   1, 0, 0)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   1, 0, 1)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   1, 1, 0)
          ARM7_JMEM_CASE (LDRSB_MASK, ARM7_LDR_ALL_B, LDRSB,   1, 1, 1)

          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   0, 0, 0)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   0, 0, 1)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   0, 1, 0)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   0, 1, 1)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   1, 0, 0)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   1, 0, 1)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   1, 1, 0)
          ARM7_JMEM_CASE (LDRSH_MASK, ARM7_LDR_ALL_B, LDRSH,   1, 1, 1)

          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   0, 0, 0)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   0, 0, 1)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   0, 1, 0)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   0, 1, 1)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   1, 0, 0)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   1, 0, 1)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   1, 1, 0)
          ARM7_JMEM_CASE (LDRH_MASK, ARM7_LDR_ALL_B, LDRH,   1, 1, 1)

          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   0, 0, 0)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   0, 0, 1)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   0, 1, 0)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   0, 1, 1)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   1, 0, 0)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   1, 0, 1)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   1, 1, 0)
          ARM7_JMEM_CASE (STRH_MASK, ARM7_STR_ALL_B, STRH,   1, 1, 1)     
      
  #define MAKE_LB(load_bit, byte_bit)\
       (load_bit << 4 | byte_bit << 6)
  #define MARK_LB_IMM_OFFSET 1 << 10 
  #define MARK_LB_SCALED_OFFSET 1 << 10 | 1 << 9
  #define ARM7_MMEM_CASE(l_bit, b_bit, ARM7_MEM_ALL_B, ARM7_MEM_EL, pre_address, post, write_back)       \
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 0:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 1:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 2:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 3:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 4:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 5:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 6:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 7:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 8:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 9:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 10:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 11:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 12:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 13:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 14:\
    case MARK_LB_IMM_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 15:\
      {\
        const uint32_t rd = OP_code >> 12 & 15;\
        const uint32_t rn = OP_code >> 16 & 15;\
        const uint32_t imm12 = OP_code & 0xFFF;\
        \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, imm12, (pre_address != 0 ? write_back : 1));\
      }\
      break;\
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 0: \
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 8: \
      {                                                                       \
        const uint32_t rd = OP_code >> 12 & 15;                               \
        const uint32_t rn = OP_code >> 16 & 15;                               \
        const uint32_t rm_v = regs[OP_code >> 0 & 15].blk;                        \
        const uint32_t imm5 = OP_code >> 7 & 31;                              \
        uint32_t shift_v;                                                     \
        LSL_IMM5_NO_SIGN (rm_v, imm5, shift_v);                               \
                                                                              \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, shift_v, (pre_address != 0 ? write_back : 1)); \
      }                                                                       \
      break;\
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 2:\
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 10:\
      {                                                                       \
        const uint32_t rd = OP_code >> 12 & 15;                               \
        const uint32_t rn = OP_code >> 16 & 15;                               \
        const uint32_t rm_v = regs[OP_code >> 0 & 15].blk;                        \
        const uint32_t imm5 = OP_code >> 7 & 31;                              \
        uint32_t shift_v;                                                     \
        LSR_IMM5_NO_SIGN (rm_v, imm5, shift_v);                               \
                                                                              \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, shift_v, (pre_address != 0 ? write_back : 1)); \
      }                                                                       \
      break;\
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 4:\
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 12:\
      {                                                                       \
        const uint32_t rd = OP_code >> 12 & 15;                               \
        const uint32_t rn = OP_code >> 16 & 15;                               \
        const uint32_t rm_v = regs[OP_code >> 0 & 15].blk;                        \
        const uint32_t imm5 = OP_code >> 7 & 31;                              \
        uint32_t shift_v;                                                     \
        ASR_IMM5_NO_SIGN (rm_v, imm5, shift_v);                               \
                                                                              \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, shift_v, (pre_address != 0 ? write_back : 1)); \
      }                                                                       \
      break;\
      \
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 6:\
    case MARK_LB_SCALED_OFFSET | MAKE_LB(l_bit, b_bit) | write_back << 5 | pre_address << 8 | post << 7 | 14:\
      {                                                                       \
        const uint32_t rd = OP_code >> 12 & 15;                               \
        const uint32_t rn = OP_code >> 16 & 15;                               \
        const uint32_t rm_v = regs[OP_code >> 0 & 15].blk;                        \
        const uint32_t imm5 = OP_code >> 7 & 31;                              \
        uint32_t shift_v;                                                     \
        ROR_IMM5_NO_SIGN (rm_v, imm5, shift_v);                               \
                                                                              \
        ARM7_MEM_ALL_B (ARM7_MEM_EL, rd, rn, pre_address, post, shift_v, (pre_address != 0 ? write_back : 1)); \
      }                                                                       \
      break;
      
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   0, 0, 0)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   0, 0, 1)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   0, 1, 0)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   0, 1, 1)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   1, 0, 0)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   1, 0, 1)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   1, 1, 0)
          ARM7_MMEM_CASE (1, 1, ARM7_LDR_ALL_B, LDRB,   1, 1, 1)

          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   0, 0, 0)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   0, 0, 1)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   0, 1, 0)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   0, 1, 1)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   1, 0, 0)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   1, 0, 1)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   1, 1, 0)
          ARM7_MMEM_CASE (0, 1, ARM7_STR_ALL_B, STRB,   1, 1, 1)
    
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 0, 0, 0)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 0, 0, 1)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 0, 1, 0)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 0, 1, 1)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 1, 0, 0)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 1, 0, 1)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 1, 1, 0)
          ARM7_MMEM_CASE (1, 0, ARM7_LDR_ALL_B, LDR, 1, 1, 1)

          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 0, 0, 0)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 0, 0, 1)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 0, 1, 0)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 0, 1, 1)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 1, 0, 0)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 1, 0, 1)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 1, 1, 0)
          ARM7_MMEM_CASE (0, 0, ARM7_STR_ALL_B, STR, 1, 1, 1)  
          
  #define STM_BLOCK16(n)\
        case (0x800 | (n) << 5 | 0):\
        case (0x800 | (n) << 5 | 1):\
        case (0x800 | (n) << 5 | 2):\
        case (0x800 | (n) << 5 | 3):\
        case (0x800 | (n) << 5 | 4):\
        case (0x800 | (n) << 5 | 5):\
        case (0x800 | (n) << 5 | 6):\
        case (0x800 | (n) << 5 | 7):\
        case (0x800 | (n) << 5 | 8):\
        case (0x800 | (n) << 5 | 9):\
        case (0x800 | (n) << 5 |10):\
        case (0x800 | (n) << 5 |11):\
        case (0x800 | (n) << 5 |12):\
        case (0x800 | (n) << 5 |13):\
        case (0x800 | (n) << 5 |14):\
        case (0x800 | (n) << 5 |15):\
        {\
          const uint32_t p = n >> 3 & 1;\
          const uint32_t u = n >> 2 & 1;\
          const uint32_t s = n >> 1 & 1;\
          const uint32_t w = n >> 0 & 1;\
          \
          consume += isa_arm7_list_memory_access_store (agb, OP_code, p, u, s, w);\
        }\
        break;
          STM_BLOCK16 (0)
          STM_BLOCK16 (1)
          STM_BLOCK16 (2)
          STM_BLOCK16 (3)
          STM_BLOCK16 (4)
          STM_BLOCK16 (5)
          STM_BLOCK16 (6)
          STM_BLOCK16 (7)
          STM_BLOCK16 (8)
          STM_BLOCK16 (9)
          STM_BLOCK16 (10)
          STM_BLOCK16 (11)
          STM_BLOCK16 (12)
          STM_BLOCK16 (13)
          STM_BLOCK16 (14)
          STM_BLOCK16 (15)
          
  #define LDM_BLOCK16(n)\
        case (0x800 | (n) << 5 |16): \
        case (0x800 | (n) << 5 |17): \
        case (0x800 | (n) << 5 |18): \
        case (0x800 | (n) << 5 |19): \
        case (0x800 | (n) << 5 |20): \
        case (0x800 | (n) << 5 |21): \
        case (0x800 | (n) << 5 |22): \
        case (0x800 | (n) << 5 |23): \
        case (0x800 | (n) << 5 |24): \
        case (0x800 | (n) << 5 |25): \
        case (0x800 | (n) << 5 |26): \
        case (0x800 | (n) << 5 |27): \
        case (0x800 | (n) << 5 |28): \
        case (0x800 | (n) << 5 |29): \
        case (0x800 | (n) << 5 |30): \
        case (0x800 | (n) << 5 |31): \
        {\
          const uint32_t p = n >> 3 & 1;\
          const uint32_t u = n >> 2 & 1;\
          const uint32_t s = n >> 1 & 1;\
          const uint32_t w = n >> 0 & 1;\
          \
          consume += isa_arm7_list_memory_access_load (agb, OP_code, p, u, s, w);\
        }\
        break;
        
          LDM_BLOCK16 (0)
          LDM_BLOCK16 (1)
          LDM_BLOCK16 (2)
          LDM_BLOCK16 (3)
          LDM_BLOCK16 (4)
          LDM_BLOCK16 (5)
          LDM_BLOCK16 (6)
          LDM_BLOCK16 (7)
          LDM_BLOCK16 (8)
          LDM_BLOCK16 (9)
          LDM_BLOCK16 (10)
          LDM_BLOCK16 (11)
          LDM_BLOCK16 (12)
          LDM_BLOCK16 (13)
          LDM_BLOCK16 (14)
          LDM_BLOCK16 (15)

        default:
          assert (0);
        }
      }
      regs[15].blk += 4;
    }
    break;
  } while (0);
  
  return consume;
}
#endif 