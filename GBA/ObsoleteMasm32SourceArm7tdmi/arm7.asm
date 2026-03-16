;; arm7.asm
;; arm7 chip for gba 
;;
;; Copyripht 2019 moecmks (agalis01@outlook.com)
;; This file is part of ArchBoyAdvance.
;; 
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;; 
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;; 
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software Foundation,
;; Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

;; TODO: 
;; GamePak Prfetch Timing Problem.

  .686            
  .mmx 
  .xmm   
  .model flat, c     
  option casemap :none
  
;; mmu read/write 
;; internal will set arm7tdmi::waitState

extrn agb_mbus_rw_seq@8:proc  
extrn agb_mbus_ww_seq@12:proc 
extrn agb_code_rhw_seq@8:proc 
extrn agb_code_rw_seq@8:proc  
extrn agb_mbus_rb_noseq@8:proc 
extrn agb_mbus_rhw_noseq@8:proc  
extrn agb_mbus_rw_noseq@8:proc
extrn agb_mbus_wb_noseq@12:proc 
extrn agb_mbus_whw_noseq@12:proc 
extrn agb_mbus_ww_noseq@12:proc 
extrn agb_code_rhw_noseq@8:proc
extrn agb_code_rw_noseq@8:proc  
extrn agb_gamepak_prefetch@12:proc
extrn agb_irq_hook@4:proc
extrn agb_stack_ww_seq@20:proc 
extrn agb_stack_ww_noseq@20:proc 
extrn agb_stack_rw_seq@8:proc
extrn agb_stack_rw_noseq@8:proc  

STACK_PUSH_STATUS_INIT equ 0
STACK_PUSH_STATUS_PUSH equ 1

StackWriteSeq equ agb_stack_ww_seq@20
StackWriteNoSeq equ agb_stack_ww_noseq@20
StackReadSeq equ agb_stack_rw_seq@8
StackReadNoSeq equ agb_stack_rw_noseq@8
MmuReadWordSeq equ agb_mbus_rw_seq@8
MmuWriteWordSeq equ agb_mbus_ww_seq@12
MmuReadByteNoSeq equ agb_mbus_rb_noseq@8
MmuReadHalfWordNoSeq equ agb_mbus_rhw_noseq@8
MmuReadWordNoSeq equ agb_mbus_rw_noseq@8
MmuWriteByteNoSeq equ agb_mbus_wb_noseq@12
MmuWriteHalfWordNoSeq equ agb_mbus_whw_noseq@12
MmuWriteWordNoSeq equ agb_mbus_ww_noseq@12
tb_FetchSeq equ agb_code_rhw_seq@8
tb_FetchNoSeq equ agb_code_rhw_noseq@8
ARM7_FetchSeq equ agb_code_rw_seq@8
ARM7_FetchNoSeq equ agb_code_rw_noseq@8
AGB_GamePakPrefetch equ agb_gamepak_prefetch@12
AGB_IRQ_Hook equ agb_irq_hook@4

;;  XXX:memory order indep 
arm7tdmi struct

SZ_STACK equ 13
SZ_LRLINK equ 14
SZ_PC equ 15
SZ_CPSR equ 16 

  Regs dd 17 dup (?) 
  R812_T dd 10 dup (?) 
  R1314_T dd 12 dup (?)
  SPSR_T dd 6 dup (?) 
  Opcode dd 2 dup (?) 
  IME dd ?
  IFS dw ?
  IE dw ?
  waitState dd ?
  nextNoSeqFetch dd ?
  agb dd ?
  calc dd ?

arm7tdmi ends

include def.asm
include out.asm 
include debug.asm 
include helper.asm 

.code 
;;=====================================================================================
;; arm7tdmi_ticks c prototype 
;; uint16_t arm7tdmi_ticks (struct arm7tdmi *arm7) 
;; ret: real exec ticks.
arm7tdmi_ticks proc c 
             option prologue:none, epilogue:none

  push edi 
  push esi 
  push ebp 
  push ebx 
  
  STACK_PUSH equ 16 
    assume SC_ARM7:ptr arm7tdmi
      xor SC_WAIT, SC_WAIT
      mov SC_ARM7, STA (4+STACK_PUSH)  
      mov SC_CPSR, ZRS (SZ_CPSR) 
      ;; prefetcht0 [SC_ARM7]
      mov ax, [SC_ARM7].IFS 
      and ax, [SC_ARM7].IE 
      and eax, GBA_IE_IF_MASK
      bt dword ptr[SC_ARM7].IME, 0
      sbb edx, edx
      and eax, edx ;; GBA_IE_IF_MASK.
      bt SC_CPSR, IRQ_INHIBI_MASK_BIT
      sbb edx, edx 
      not edx 
      and eax, edx 
      je e_step  
IF 0
      Call_IRQ_Hook
ENDIF
      mov eax, ZRS (SZ_PC)
      mov ecx, SC_CPSR 
      and ecx, ARM7_MODE_CLR_MASK 
      or ecx, ARM7_MODE_IRQ_MASK  ;; Set IRQ mode. 
      and ecx, not FLAG_THUMB ;; Clear Thumb exec flag 
      or ecx, IRQ_INHIBI_MASK ;; Set IRQ inhibit mask 
      ;; PC to IRQ's LR, LR:= Current Instruction +4
      bt SC_CPSR, ARM7_THUMB_BIT 
      sbb edx, edx 
      xor edx, -1
      and edx, -4 
      add edx, eax  
      mov [SC_ARM7].R1314_T[R1314b_IRQ +4], edx 
      ;; Adjust PC Pointer to IRQ Interrupt vector address 
      mov ZRS (SZ_PC), ARM7_VECTOR_IRQ 
      mov SC_INVOL, SC_CPSR 
      and SC_INVOL, ARM7_MODE_GET_MASK 
      cmp SC_INVOL, ARM7_MODE_IRQ_MASK 
      je @Nest_IRQ
      ;; switch Mode 
      SwitchMode ecx, 0
      ;; Prefetch Opcode pipeline 
      arm7_FlushPipeline 5 ;; minimum interrupt cycle delay 
  @Nest_IRQ:
      mov ZRS (SZ_LRLINK), edx
      mov [SC_ARM7].SPSR_T[SPSRb_IRQ], SC_CPSR
      mov eax, ZRS (SZ_STACK)
      mov [SC_ARM7].R1314_T[R1314b_IRQ], eax 
      mov eax, ZRS (SZ_LRLINK)
      mov [SC_ARM7].R1314_T[R1314b_IRQ+4], eax 
      mov SC_CPSR, ecx 
      ;; Prefetch Opcode pipeline 
      arm7_FlushPipeline 5
        
      ;; CPU simulation.
      ;; According to the order section of the official manual DDI0210B.pdf of ARM7TDMI,
      ;; Because of the pipeline relationship, the PC pointer always points to the two instructions 
      ;; after the current execution address before each instruction is executed by SC_ARM7.
      ;; This implicitly points out the following points.
      ;; 
      ;; (1) Instruction execution is always in pipeline cycle >= 2 execution, which means
      ;; (2) Previous and at least two Fetch scripts (this operation may be included with specific implementation instructions)
      ;; (3) The first instruction code has been decoded, the second instruction code has been pushed to the first instruction code
    @ALIGN_Z
DEBUG_TICK macro Address
   local dm
   push eax 
   mov eax, ZRS(SZ_PC)
   cmp eax, Address+8 
   jne dm 
   int 3
dm:
   pop eax
endm 
DEBUG_TICK2 macro Address
   local dm
   push eax 
   mov eax, ZRS(SZ_PC)
   cmp eax, Address+4
   jne dm 
   int 3
dm:
   pop eax
endm 
    e_step:
      xor SC_WAIT, SC_WAIT
      mov eax, [SC_ARM7].nextNoSeqFetch
      mov [SC_ARM7].nextNoSeqFetch, SC_WAIT
      test SC_CPSR, FLAG_THUMB
      jne @F
      ;; ARM7 Code, do next instruction's first seq/noseq read cycle
      push ZRS (SZ_PC)
      test eax, eax 
      je aaf7
      CallMemOrIOAddWaitState ARM7_FetchNoSeq 
      jmp nn
      aaf7:
      CallMemOrIOAddWaitState ARM7_FetchSeq 
    nn:
      mov SC_INVOL, [SC_ARM7].Opcode[0]
      mov ecx, [SC_ARM7].Opcode[4] 
      mov [SC_ARM7].Opcode[4], eax 
      mov [SC_ARM7].Opcode[0], ecx 
      mov eax, SC_INVOL
      OUTd "\n\nPC:%08X-OP:%08X-SP:%08X LR:%08X  -M:%s-\n", "ZF-8", "SC_INVOL", "ZD", "ZE", "#"
      OUTd "R0:%08X R1:%08X R2:%08X R3:%08X \n", "Z0", "Z1", "Z2", "Z3"
      OUTd "R4:%08X R5:%08X R6:%08X R7:%08X \n", "Z4", "Z5", "Z6", "Z7"
      OUTd "R8:%08X R9:%08X R10:%08X R11:%08X R12:%08X \n", "Z8", "Z9", "ZA", "ZB", "ZC"
      OUTd "IRQ:%d N:%d Z:%d C:%d V:%d\n", "SC_CPSR}31 & 1", "SC_CPSR}15 & 1", "SC_CPSR}14 & 1", "SC_CPSR}8 & 1", "SC_CPSR & 1"
      shr eax, 28  
      jmp fTAB[eax*4]
    @@:
      ;; Thumb Code, do next instruction's first seq/noseq read cycle
      push ZRS (SZ_PC)
      test eax, eax 
      je uaf7
      CallMemOrIOAddWaitState tb_FetchNoSeq 
      jmp cc
      uaf7:
      CallMemOrIOAddWaitState tb_FetchSeq 
    cc:
      mov SC_INVOL, [SC_ARM7].Opcode[0]
      mov ecx, [SC_ARM7].Opcode[4] 
      mov [SC_ARM7].Opcode[4], eax 
      mov [SC_ARM7].Opcode[0], ecx 
      OUTd "\n\nPC:%08X-OP:%04X-SP:%08X LR:%08X  -M:%s-\n", "ZF-4", "SC_INVOL & 0FFFFh","ZD", "ZE" , "#"
      OUTd "R0:%08X R1:%08X R2:%08X R3:%08X \n", "Z0", "Z1", "Z2", "Z3"
      OUTd "R4:%08X R5:%08X R6:%08X R7:%08X \n", "Z4", "Z5", "Z6", "Z7"
      OUTd "R8:%08X R9:%08X R10:%08X R11:%08X R12:%08X \n", "Z8", "Z9", "ZA", "ZB", "ZC"
      OUTd "IRQ:%d N:%d Z:%d C:%d V:%d\n", "SC_CPSR}31 & 1", "SC_CPSR}15 & 1", "SC_CPSR}14 & 1", "SC_CPSR}8 & 1", "SC_CPSR & 1"
      and SC_INVOL, 0FFFFh 
      mov eax, SC_INVOL
      mov edx, SC_INVOL
      mov ecx, SC_INVOL
      shr ecx, 8 
      and ecx, 255
      jmp sTAB[ecx*4]  

        ;; ARM7 Instruction Entry -----------------------------------------------------------------------------------
      AS_I8: 
        OUTd "I:AS_I8;;*** "
        mov eax, edx 
        and eax, 001F00000h
        shr eax, 20
        jmp i8r4TAB[eax*4]  
      SF_RS:
        OUTd "I:SF_RS;;*** "
        mov eax, edx 
        and eax, 001F00000h
        shr eax, 20  
        jmp rsTAB[eax*4]
      SF_I5:
        OUTd "I:SF_I5;;*** "
        mov eax, edx 
        and eax, 001F00000h
        shr eax, 20   
        jmp si5TAB[eax*4]
      A7MUL:
        OUTd "I:A7MUL;;*** "
        mov eax, edx 
        and eax, 000F00000h
        shr eax, 20
        jmp ARM7_MUL_TAB[eax*4]        
      LDI12:    
        OUTd "I:LDI12;;*** "
        mov eax, edx 
        and eax, 001F00000h
        shr eax, 20    
        jmp nTAB[eax*4]  
      LDIRS:
        OUTd "I:LDIRS;;*** "
        mov eax, edx 
        and eax, 001F00000h
        shr eax, 20
        jmp eTAB[eax*4]          
      LDRHW: ;; FIXME
        OUTd "I:LDRHW;;*** "
        mov eax, edx 
        mov ecx, edx 
        and ecx, 060h 
        and eax, 0100000h 
        shr eax, 18
        shr ecx, 5
        or ecx,  eax ;; 
        and ecx, 7
        mov eax, SC_INVOL
        and eax, 01a00000h
        shr eax, 20
        or eax, pcMTAB[ecx*4] 
        test edx, 0400000h 
        je @F    
        jmp cTAB[eax*4]    
     @@:jmp pTAB[eax*4]    
      RGSET:
        OUTd "I:RGSET;;*** "
        mov eax,  SC_INVOL
        shr eax, 20 
        and eax, 31 
        jmp zTAB[eax*4]  
        
      ;; ARM7 Instruction Entry -----------------------------------------------------------------------------------  
        include arm7_cond.asm
        include arm7_alu.asm 
        include arm7_branch.asm 
        include arm7_ldr.asm 
        include arm7_swi.asm 
        include arm7_psr.asm 
        include arm7_ldm.asm 
        include arm7_atomic.asm
        include arm7_mul.asm 
        include arm7_misc_stuff.asm
      ;; Thumb Instruction Entry -----------------------------------------------------------------------------------  
        include thumb_alu.asm
        include thumb_alu_ext.asm  
        include thumb_swi.asm 
        include thumb_push.asm        
        include thumb_ldr.asm         
        include thumb_ldm.asm    
        include thumb_branch.asm   
        include thumb_jcc.asm       
      ; Instruction Hash Entry ----------------------------------------------------------------------------------- 
        include instr_hash.asm 
        
arm7tdmi_ticks endp 
   
                  
end 


end 
  