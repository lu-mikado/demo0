

DEBUG_BREAK equ int 3

;; GBA Mmu Operate 
IF 1
CALLIt macro Symbol 
  call Symbol
endm 
ELSE 
CALLIt macro Symbol 
  call PTR32[Symbol]
endm
ENDIF

CallMemOrIO macro Symbol
  push [SC_ARM7].agb
  CALLIt Symbol
endm 
CallMemOrIOAddWaitState macro Symbol
  push [SC_ARM7].agb
  CALLIt Symbol
  add SC_WAIT, [SC_ARM7].waitState
endm
CallMemOrIOAddWaitState2 macro Symbol, Addr2
  push Addr2
  push [SC_ARM7].agb
  CALLIt Symbol
  add SC_WAIT, [SC_ARM7].waitState
endm

Call_IRQ_Hook macro
  push eax 
  push ecx 
  push edx 
  push [SC_ARM7].agb
  CALLIt AGB_IRQ_Hook
  pop edx 
  pop ecx 
  pop eax 
endm

Add_WaitStateClks equ add SC_WAIT, [SC_ARM7].waitState

GamePAK_Prefetch macro InternalCycle:REQ
  push  InternalCycle
  push  ZRS (SZ_PC)
  push  [SC_ARM7].agb 
  CALLIt AGB_GamePakPrefetch
endm 

STA macro offsets 
  EXITM <dword ptr[esp+offsets]>
endm 
ZRS macro offsets 
  EXITM <PTR32[SC_ARM7].Regs[(offsets)*4]>
endm



SetNZCV_A macro sub_bit:REQ ;; 1 indicate sub otherwise 0
  lahf 
  seto al 
  xor ah, sub_bit
  mov SC_CPSR16, ax 
endm 
SetNZC_A macro sub_bit ;; 1 indicate sub otherwise 0
  lahf 
  mov SC_CPSR16H, ah 
endm
SetC_A macro
  lahf 
  and SC_CPSR, not FLAG_C ;; c clear 
  and eax, FLAG_C ;; save c
  or SC_CPSR, eax ;;c reset  
endm 
SetNZ_A macro
  lahf 
  and SC_CPSR, not (FLAG_N or FLAG_Z) ;; nz clear 
  and eax, FLAG_NZ ;; save nz
  or SC_CPSR, eax ;;nz reset  
endm 
Set_NZmul32 macro out_reg
  and out_reg, out_reg 
  SetNZ_A
endm 
Set_NZmul64 macro out_lo, out_hi
  LOCAL SkipN
  LOCAL SkipZ
  
  and SC_CPSR, not (FLAG_N or FLAG_Z) ;; nz clear 
  bt out_hi, 31
  jnc SkipN
  or SC_CPSR, FLAG_N
  jmp SkipZ
SkipN: 
  or out_hi, out_lo
  jne SkipZ
  or SC_CPSR, FLAG_Z
SkipZ: 
endm  

;; ToStandPSR
;; destroy :  ecx, eax 
ToStandPSR macro Post, REC, epx, emx
  LOCAL @setZ
  LOCAL @setC
  LOCAL @setV
  LOCAL @setOver
  
  mov epx, Post
  mov emx, Post 
  rol epx, 8 
  and epx, 0000000FFh
  test emx, FLAG_N 
  je @setZ
  or epx, ARM7_STD_FLAGS_N 
@setZ:
  test emx,  FLAG_Z
  je @setC
  or epx, ARM7_STD_FLAGS_Z 
@setC:
  test emx,  FLAG_C
  je @setV
  or epx, ARM7_STD_FLAGS_C 
@setV:
  test emx,  FLAG_V
  je @setOver
  or epx, ARM7_STD_FLAGS_V 
@setOver:
  mov REC, epx
endm 

;; ToFastPSR
;; destroy :  ecx, eax 
ToFastPSR macro Post, REC, epx, emx

  LOCAL @setZ
  LOCAL @setC
  LOCAL @setV
  LOCAL @setOver
  
  mov epx, Post
  mov emx, Post 
  ror epx, 8 
  and epx, 0FF000000h
  test emx, ARM7_STD_FLAGS_N 
  je @setZ
  or epx, FLAG_N
@setZ:
  test emx, ARM7_STD_FLAGS_Z 
  je @setC
  or epx, FLAG_Z
@setC:
  test emx, ARM7_STD_FLAGS_C 
  je @setV
  or epx, FLAG_C
@setV:
  test emx, ARM7_STD_FLAGS_V 
  je @setOver
  or epx, FLAG_V
@setOver:
  mov REC, epx
endm 

;; destroy eax, ecx 
Imm8BitmapSft_AC macro Sft4Imm8, REC 
  mov eax, Sft4Imm8
  mov ecx, Sft4Imm8 
  and eax, 0FFh 
  and ecx, 0F00h 
  shr ecx, 7 
  ror eax, cl 
  mov REC, eax 
endm 

Imm8BitmapSft_AC_SetC macro Sft4Imm8, REC  
  LOCAL @Skip
  
  mov eax, Sft4Imm8
  mov ecx, Sft4Imm8 
  and eax, 0FFh 
  and ecx, 0F00h 
  shr ecx, 7 
  je @Skip
  ror eax, cl 
  mov ecx, eax 
  SetC_A
  mov eax, ecx
@Skip:
  mov REC, eax 
endm 

C_op_Care equ 0
C_op_NOCare equ 1

;; XXX: better case
ShiftRegImm5 macro Post, REC, tr1c, tr2, C_op

  LOCAL  @END
  LOCAL @HASH 
  LOCAL @LSL 
  LOCAL  @LSR 
  LOCAL @ASR 
  LOCAL @ROR
  LOCAL @LSL_0
  LOCAL  @LSR_0 
  LOCAL @ASR_0 
  LOCAL @ROR_0

  mov tr1c, Post
  mov tr2,   Post
  shr tr1c, 5 
  and tr1c, 3
  jmp @HASH[tr1c*4]
  
  Si5SftOp macro X86Sft, lSym
    mov tr1c, tr2
    and tr2, 15 
    mov tr2, ZRS (tr2) ;; Get Rm. 
    shr tr1c, 7 
    and tr1c, 31
    je lSym
    and SC_CPSR, not FLAG_C
    X86Sft tr2, cl ;; check c 
    sbb tr1c, tr1c 
    and tr1c, FLAG_C 
    or SC_CPSR, tr1c
    mov REC, tr2
    jmp @END
  endm 

  @LSR:
    OUTd "LSR #%d ", "SC_INVOL}7 & 31"
    Si5SftOp shr, @LSR_0
  @LSR_0:
    and SC_CPSR, not FLAG_C
    xor tr1c, tr1c 
    bt tr2, 31 ;; check c[31]
    sbb tr2, tr2 
    and tr2, FLAG_C 
    or SC_CPSR, tr2
    mov REC, tr1c
    jmp @END
    
  @ASR:
    OUTd "ASR #%d ", "SC_INVOL}7 & 31"
    Si5SftOp sar, @ASR_0
  @ASR_0:
    and SC_CPSR, not FLAG_C
    bt tr2, 31 
    sbb tr1c, tr1c 
    mov tr2, tr1c 
    and tr1c, FLAG_C 
    or SC_CPSR, tr1c
    mov REC, tr2
    jmp @END
    
  @ROR:
    OUTd "ROR #%d ", "SC_INVOL}7 & 31"
    Si5SftOp ror, @ROR_0
  @ROR_0: ;; RRX 
    OUTd "...RRX "
    btr SC_CPSR, FLAG_CHECK_C_X86_BT
    rcr tr2,  1 ;; check c 
    sbb tr1c, tr1c 
    and tr1c,  FLAG_C 
    or SC_CPSR, tr1c
    mov REC, tr2
    jmp @END
  
  @ALIGN_Z
  @HASH  dd @LSL, @LSR, @ASR, @ROR
  @LSL: 
    OUTd "LSL #%d ", "SC_INVOL}7 & 31"
    Si5SftOp shl, @LSL_0
  @LSL_0:
    mov REC, tr2
  @END:
  OUTd "Sft Result:%08X  ", "&REC&"
endm 

;; XXX: better case
ShiftRegImm5_WithOutC macro Post, REC, tr1c, tr2

  LOCAL @END
  LOCAL @HASH 
  LOCAL @LSL 
  LOCAL @LSR 
  LOCAL @ASR 
  LOCAL @ROR
  LOCAL @LSL_0
  LOCAL @LSR_0 
  LOCAL @ASR_0 
  LOCAL @ROR_0

  mov tr1c, Post
  mov tr2, Post
  shr tr1c, 5 
  and tr2, 15 
  mov tr2, ZRS (tr2) ;; Get Rm. 
  and tr1c, 3
  jmp @HASH[tr1c*4]
  
  Si5SftOp_WithOutC macro X86Sft, lSym 
    mov tr1c, Post
    shr tr1c, 7 
    and tr1c, 31
    je lSym
    X86Sft tr2, cl ;; check c 
    mov REC, tr2
    jmp @END
  endm 

  @LSR:
    OUTd "LSR #%d ", "SC_INVOL}7 & 31"
    Si5SftOp_WithOutC shr, @LSR_0
  @LSR_0:
    xor REC, REC
    jmp @END
    
  @ASR:
    OUTd "ASR #%d ", "SC_INVOL}7 & 31"
    Si5SftOp_WithOutC sar, @ASR_0
  @ASR_0:
    bt tr2, 31 
    sbb REC, REC 
    jmp @END
    
  @ROR:
    OUTd "ROR #%d ", "SC_INVOL}7 & 31"
    Si5SftOp_WithOutC ror, @ROR_0
  @ROR_0: ;; RRX 
    OUTd "...RRX "
    bt SC_CPSR, FLAG_CHECK_C_X86_BT
    rcr tr2,  1 ;; check c 
    mov REC, tr2
    jmp @END
  
  @ALIGN_Z
  @HASH  dd @LSL, @LSR, @ASR, @ROR
  
  @LSL:   
    OUTd "LSL #%d ", "SC_INVOL}7 & 31"
    Si5SftOp_WithOutC shl, @LSL_0
  @LSL_0:
    mov REC, tr2 
  @END:
  OUTd "Sft Result:%08X  ", "&REC&"
endm

;; XXX: better case, use inline label. 
ShiftRegRs macro Post, REC, tr1c, tr2, C_op

  LOCAL  @END
  LOCAL @HASH 
  LOCAL @LSL 
  LOCAL  @LSR 
  LOCAL @ASR 
  LOCAL @ROR
  LOCAL @ROR_ep
  
  mov tr1c, Post
  mov tr2,   Post
  shr tr1c, 5 
  and tr1c, 3
IF C_op eq C_op_NOCare 
  mov [esp-8], SC_CPSR
  IF ARM7_DEBUG ne 0
    sub esp, 32
  ENDIF
ENDIF
  jmp @HASH[tr1c*4]
  
  Rs_SftOp macro X86Sft
    and SC_CPSR, not FLAG_C
    X86Sft tr2, cl ;; check c 
    sbb tr1c, tr1c
    and tr1c,  FLAG_C 
    or SC_CPSR, tr1c
    mov REC, tr2
    jmp @END
  endm  
  Rs_Stiff macro 
    LOCAL locSym
    mov tr1c, tr2
    and tr2, 15 
    cmp tr2, SZ_PC 
    mov tr2, ZRS (tr2) ;; Get Rm. 
    jne locSym
    add tr2, 4 ;; PC + 12, if rm in rorate shift.
  locSym:
    shr tr1c, 8 
    and tr1c, 15
    mov tr1c, ZRS (tr1c) ;; Get Rs
    and tr1c, 255
    ;; TODO:better case ?
    cmp tr1c, 32 
    jl @F
  endm 

  @LSL: 
    OUTd "LSL R%d ", "SC_INVOL}8 & 15"
    Rs_Stiff
    ;; Rs >= 32
    ;; ;; DEBUG_BREAK    
    sete cl 
    and ecx, 1 
    and SC_CPSR, not FLAG_C  
    and tr1c, tr2 
    shl tr1c, FLAG_CHECK_C_X86_BT 
    or SC_CPSR, tr1c
    xor REC, REC
    jmp @END
  @@:
    test tr1c, tr1c
    jne @F
    ;; Rs := 0 
    mov REC, tr2
    jmp @END
  @@:
    Rs_SftOp shl 
    
  @LSR:
    OUTd "LSR R%d ", "SC_INVOL}8 & 15"
    Rs_Stiff
    ;; Rs >= 32 
    ;; DEBUG_BREAK
    sete cl 
    and ecx, 1 
    and SC_CPSR, not FLAG_C 
    bt tr2, 31 
    sbb tr2, tr2      
    and tr1c, tr2 
    shl tr1c, FLAG_CHECK_C_X86_BT 
    or SC_CPSR, tr1c
    xor REC, REC
    jmp @END
  @@:
    test tr1c, tr1c
    jne @F
    ;; Rs := 0 
    mov REC, tr2
    jmp @END
  @@:
    Rs_SftOp shr   
     
  @ASR:
    OUTd "ASR R%d ", "SC_INVOL}8 & 15"
    Rs_Stiff
    ;; Rs >= 32 
    ;; DEBUG_BREAK
    and SC_CPSR, not FLAG_C
    shl tr2, 1
    sbb tr2, tr2 
    mov tr1c, tr2
    and tr2, 1
    shl tr2, FLAG_CHECK_C_X86_BT
    or SC_CPSR, tr2
    mov REC, tr1c
    jmp @END   
  @@:
    test tr1c, tr1c
    jne @F
    ;; Rs := 0 
    mov REC, tr2
    jmp @END
  @@:
    Rs_SftOp sar  
    
  @ROR:
    OUTd "ROR R%d ", "SC_INVOL}8 & 15"
    mov tr1c, tr2
    and tr2, 15 
    cmp tr2, SZ_PC 
    mov tr2, ZRS (tr2) ;; Get Rm. 
    jne @ROR_ep
    add tr2, 4  
  @ROR_ep:
    shr tr1c, 8 
    and tr1c, 15
    mov tr1c, ZRS (tr1c) ;; Get Rs
    and tr1c, 255
    jne @F
    ;; Rs := 0 
    mov REC, tr2
    jmp @END 
  @@:
    and tr1c, 31 
    jne @F
    and SC_CPSR, not FLAG_C
    bt tr2, 31
    sbb tr1c, tr1c 
    and tr1c, FLAG_C
    or SC_CPSR, tr1c
    mov REC, tr2
    jmp @END 
  @@:
    Rs_SftOp ror  
    
  @HASH  dd @LSL, @LSR, @ASR, @ROR
  @END:
IF C_op eq C_op_NOCare 
  IF ARM7_DEBUG ne 0
    add esp, 32
  ENDIF
  mov SC_CPSR, [esp-8]
ENDIF
  OUTd "Sft Result:%08X  ", "&REC&"
endm 

;; XXX: better case, use inline label. 
ShiftRegRs_WithOutC macro Post, REC, tr1c, tr2, C_op

  LOCAL  @END
  LOCAL @HASH 
  LOCAL @LSL 
  LOCAL  @LSR 
  LOCAL @ASR 
  LOCAL @ROR
  LOCAL @ZERO 
  
  mov tr1c, Post
  mov tr2,   Post
  shr tr1c, 5 
  and tr1c, 3
  jmp @HASH[tr1c*4]
  
  Rs_SftOp macro X86Sft
    X86Sft tr2, cl ;; check c 
    mov REC, tr2
    jmp @END
  endm  
  Rs_Stiff macro 
    mov tr1c, tr2
    and tr2, 15 
    mov tr2, ZRS (tr2) ;; Get Rm. 
    shr tr1c, 8 
    and tr1c, 15
    mov tr1c, ZRS (tr1c) ;; Get Rs
    and tr1c, 255
    ;; TODO:better case ?
    cmp tr1c, 32 
    jl @F
  endm 

  @LSR:
    OUTd "LSR R%d ", "SC_INVOL}8 & 15"
    Rs_Stiff
    ;; Rs >= 32 
    ;; ;; DEBUG_BREAK
    xor REC, REC
    jmp @END
  @@:
    test tr1c, tr1c
    je @ZERO
    Rs_SftOp shr   
     
  @ASR:
    OUTd "ASR R%d ", "SC_INVOL}8 & 15"
    Rs_Stiff
    ;; Rs >= 32
    ;; DEBUG_BREAK
    shl tr2, 1
    sbb REC, REC 
    jmp @END   
  @@:
    test tr1c, tr1c
    je @ZERO
    Rs_SftOp sar  
    
  @ROR:
    OUTd "ROR R%d ", "SC_INVOL}8 & 15"
    mov tr1c, Post
    shr tr1c, 8 
    and tr1c, 15
    mov tr1c, ZRS (tr1c)
    and tr1c, 31
    je @ZERO
    Rs_SftOp ror  
    
  @HASH  dd @LSL, @LSR, @ASR, @ROR
  
  @LSL: 
    OUTd "LSL R%d ", "SC_INVOL}8 & 15"
    Rs_Stiff
    ;; Rs >= 32 
    ;; DEBUG_BREAK
    xor REC, REC
    jmp @END
  @@:
    test tr1c, tr1c
    je @ZERO
    shl tr2, cl ;; check c 
  @ZERO:
    mov REC, tr2
  @END:
  OUTd "Sft Result:%08X  ", "&REC&"
endm

;; Resume CPSR from current mode's SPSR.
;; destroy: eax, ecx, edx 
SPSRToCPSR macro tr
  ;; Copy SPSR to CPSR. 
  GetCurSPSRP tr
  mov tr, [tr]
  ;; Switch CPU mode. 
  SwitchMode tr, 1
endm 

SC_CPSR8 equ bl
SC_CPSR16 equ bx 
SC_CPSR16H equ bh
SC_CPSR equ ebx
SC_ARM7 equ esi
SC_INVOL equ edi 
SC_WAIT equ ebp

;; FETCH --------------------------------------------------------------- 
ARM7_ExitAddPC macro Clks:REQ 
  mov ecx, ZRS (SZ_PC)
  mov ZRS (SZ_CPSR), SC_CPSR
  add ecx, 4
  and ecx,-4
  lea eax, [SC_WAIT+Clks]
  mov ZRS (SZ_PC), ecx
  pop ebx 
  pop ebp 
  pop esi 
  pop edi 
  ret 
endm 
tb_ExitAddPC macro Clks:REQ
  mov ecx, ZRS (SZ_PC)
  mov ZRS (SZ_CPSR), SC_CPSR
  add ecx, 2
  and ecx,-2
  lea eax, [SC_WAIT+Clks]
  mov ZRS (SZ_PC), ecx
  pop ebx 
  pop ebp 
  pop esi 
  pop edi 
  ret 
endm

arm7_FlushPipeline macro Clks:REQ
  mov SC_INVOL, ZRS (SZ_PC)
  push SC_INVOL
  CallMemOrIOAddWaitState ARM7_FetchNoSeq
  add SC_INVOL, 4 ;; ++PC
  mov [SC_ARM7].Opcode[0], eax 
  push SC_INVOL
  CallMemOrIOAddWaitState ARM7_FetchSeq
  add SC_INVOL, 4 ;; ++PC
  and SC_INVOL, -4
  mov [SC_ARM7].Opcode[4], eax          
  mov ZRS (SZ_PC), SC_INVOL
  mov ZRS (SZ_CPSR), SC_CPSR
  lea eax, [SC_WAIT+Clks]
  pop ebx 
  pop ebp 
  pop esi 
  pop edi 
  ret 
endm 
tb_FlushPipeline macro Clks:REQ
  mov SC_INVOL, ZRS (SZ_PC)
  push SC_INVOL
  CallMemOrIOAddWaitState tb_FetchNoSeq
  add SC_INVOL, 2 ;; ++PC
  mov [SC_ARM7].Opcode[0], eax 
  push SC_INVOL
  CallMemOrIOAddWaitState tb_FetchSeq
  add SC_INVOL, 2 ;; ++PC
  and SC_INVOL, -2
  mov [SC_ARM7].Opcode[4], eax          
  mov ZRS (SZ_PC), SC_INVOL
  mov ZRS (SZ_CPSR), SC_CPSR
  lea eax, [SC_WAIT+Clks]
  pop ebx 
  pop ebp 
  pop esi 
  pop edi 
  ret 
endm 

GetRFI macro Post, BitScanStart, REC
  mov REC, Post
  shr REC, BitScanStart
  and REC, 15  
endm 
GetRFIV macro Post, BitScanStart, REC
  GetRFI  Post, BitScanStart, REC
  mov REC, ZRS (REC)
endm
GetRFI_T macro Post, BitScanStart, REC
  mov REC, Post
  shr REC, BitScanStart
  and REC, 7  
endm 
GetRFIV_T macro Post, BitScanStart, REC
  GetRFI_T  Post, BitScanStart, REC
  mov REC, ZRS (REC)
endm

GetCurSPSRP macro REC

  LOCAL @FIQ
  LOCAL @IRQ 
  LOCAL @UDEF 
  LOCAL @USYS 
  LOCAL @UNUSED
  LOCAL @SVC
  LOCAL @ABT 
  LOCAL @END 
  LOCAL @HASH 
  
  mov REC, SC_CPSR
  and REC, 00F000000h
  shr REC, 24 
  jmp PTR32 @HASH [REC*4]
  
  ;; ---------------------------------------------------------------------------------
  @USYS:  
    lea REC, [SC_ARM7].SPSR_T[SPSRb_SYSUSER]
    jmp @END
  @IRQ:
    lea REC, [SC_ARM7].SPSR_T[SPSRb_IRQ]
    jmp @END
  .data 
  @ALIGN_Z
  @HASH        dd @USYS, @FIQ, @IRQ, @SVC
                    dd @UNUSED, @UNUSED, @UNUSED, @ABT
                    dd @UNUSED, @UNUSED, @UNUSED, @UDEF
                    dd @UNUSED, @UNUSED, @UNUSED, @USYS
  .code 
  @FIQ:
    lea REC, [SC_ARM7].SPSR_T[SPSRb_FIQ]
    jmp @END
  @UDEF:
  @ABT:
  @UNUSED:
    int 3
  @SVC:
    lea REC, [SC_ARM7].SPSR_T[SPSRb_MGR]
  @END:
endm

;; switch ARM7 mode 
;; destroy: eax, ecx, edx 
SwitchMode macro SG_PSR, CPSR_Copy_dis:=<0>

  LOCAL @FIQ
  LOCAL @IRQ 
  LOCAL @UDEF 
  LOCAL @USER 
  LOCAL @SVC
  LOCAL @ABT 
  LOCAL @UNUSED
  
  LOCAL @SPSR_TABLE
  
  LOCAL @FIQ2 
  LOCAL @IRQ2  
  LOCAL @UDEF2   
  LOCAL @USER2  
  LOCAL @SVC2 
  LOCAL @ABT2 
  LOCAL @UNUSED2

  LOCAL @SPSR2_TABLE
  
  LOCAL @END
  
  push SC_WAIT  
  mov eax, SG_PSR 
  mov edx, SC_CPSR
  xor ecx, ecx
  mov [SC_ARM7].calc, SC_CPSR ;; save old CPSR
  mov SC_CPSR, eax
  and eax, ARM7_MODE_GET_MASK
  and edx, ARM7_MODE_GET_MASK 
  ;; Check FIQ r8_r12 bank store
  cmp eax, ARM7_MODE_FIQ_MASK 
  sete cl
  cmp edx, ARM7_MODE_FIQ_MASK 
  sete ch
  or cl, ch 
  movzx SC_WAIT, cl
  xor ecx, ecx 
  cmp eax, ARM7_MODE_USER_MASK
  sete ch
  cmp eax, ARM7_MODE_SYS_MASK
  sete cl 
  or cl, ch  
  shl ecx, 24
  cmp edx, ARM7_MODE_USER_MASK
  sete ch
  cmp edx, ARM7_MODE_SYS_MASK
  sete cl
  or cl, ch
  rol ecx, 8 
  and ch, cl 
  jne @END
  mov ecx, [SC_ARM7].calc
  shr eax, ARM7_MODE_SFT_BIT 
  shr edx, ARM7_MODE_SFT_BIT 
  sub eax, ARM7_MODE_BLOCK_SUB_TOF
  sub edx, ARM7_MODE_BLOCK_SUB_TOF 
  jmp PTR32 @SPSR_TABLE[edx*4]
   
  saveR8_12 macro R812Bank, Post
    mov Post, ZRS (8)
    mov [SC_ARM7].R812_T[R812Bank], Post 
    mov Post, ZRS (9)
    mov [SC_ARM7].R812_T[R812Bank+4], Post 
    mov Post, ZRS (10)
    mov [SC_ARM7].R812_T[R812Bank+8], Post 
    mov Post, ZRS (11)
    mov [SC_ARM7].R812_T[R812Bank+12], Post 
    mov Post, ZRS (12)
    mov [SC_ARM7].R812_T[R812Bank+16], Post   
  endm  
  
  loadR8_12 macro R812Bank, Post
    mov Post,  [SC_ARM7].R812_T[R812Bank]
    mov ZRS (8), Post 
    mov Post,  [SC_ARM7].R812_T[R812Bank+4]
    mov ZRS (9), Post 
    mov Post,  [SC_ARM7].R812_T[R812Bank+8]
    mov ZRS (10), Post 
    mov Post,  [SC_ARM7].R812_T[R812Bank+12]
    mov ZRS (11), Post 
    mov Post,  [SC_ARM7].R812_T[R812Bank+16]
    mov ZRS (12), Post 
  endm  
  
  saveR13_14 macro R1314Bank, Post
    mov Post, ZRS (13)
    mov [SC_ARM7].R1314_T[R1314Bank], Post 
    mov Post, ZRS (14)
    mov [SC_ARM7].R1314_T[R1314Bank+4], Post 
  endm  
  
  loadR13_14 macro R1314Bank, Post
    mov Post,  [SC_ARM7].R1314_T[R1314Bank]
    mov ZRS (13), Post 
    mov Post,  [SC_ARM7].R1314_T[R1314Bank+4]
    mov ZRS (14), Post 
  endm  
  
  markBankSave macro lSym, R812Bank, R1314Bank
   LOCAL locSym 
   lSym:
     test SC_WAIT, SC_WAIT 
     je locSym
     saveR8_12 R812Bank, edx 
   locSym:
     saveR13_14 R1314Bank, edx 
     jmp PTR32 @SPSR2_TABLE[eax*4]
  endm 
  
  ;; -------- Mode Current Check Save to bank --------
  markBankSave @USER, R812b_EXCEPT_FIQ, R1314b_SYSUSER
  markBankSave @FIQ, R812b_FIQ, R1314b_FIQ  
  markBankSave @IRQ, R812b_EXCEPT_FIQ, R1314b_IRQ
  markBankSave @SVC, R812b_EXCEPT_FIQ, R1314b_MGR 
  markBankSave @ABT, R812b_EXCEPT_FIQ, R1314b_ABT
  markBankSave @UDEF, R812b_EXCEPT_FIQ, R1314b_UDEF  
  
  markBankLoad macro lSym, R812Bank, R1314Bank, SPSRBank
   LOCAL locSym 
   lSym:
    test SC_WAIT, SC_WAIT 
    je locSym
    loadR8_12 R812Bank, eax 
   locSym:
    loadR13_14 R1314Bank, eax 
    IF R1314Bank ne R1314b_SYSUSER
      IF CPSR_Copy_dis eq 0
        mov [SC_ARM7].SPSR_T[SPSRBank], ecx 
      ENDIF
    ENDIF
    jmp @END
  endm

  ;; -------- Mode New Check Store to runtime regs group --------  
  markBankLoad @USER2, R812b_EXCEPT_FIQ, R1314b_SYSUSER, SPSRb_SYSUSER
  markBankLoad @FIQ2, R812b_FIQ, R1314b_FIQ, SPSRb_FIQ
  markBankLoad @IRQ2, R812b_EXCEPT_FIQ, R1314b_IRQ, SPSRb_IRQ
  markBankLoad @SVC2, R812b_EXCEPT_FIQ, R1314b_MGR, SPSRb_MGR
  markBankLoad @ABT2, R812b_EXCEPT_FIQ, R1314b_ABT, SPSRb_ABT
  markBankLoad @UDEF2, R812b_EXCEPT_FIQ, R1314b_UDEF, SPSRb_UDEF

  @UNUSED:
  @UNUSED2:
    int 3
  .data 
  @ALIGN_Z
  @SPSR_TABLE       dd @USER, @FIQ, @IRQ, @SVC
                    dd @UNUSED, @UNUSED, @UNUSED, @ABT
                    dd @UNUSED, @UNUSED, @UNUSED, @UDEF
                    dd @UNUSED, @UNUSED, @UNUSED, @USER
  @SPSR2_TABLE       dd @USER2, @FIQ2, @IRQ2, @SVC2
                    dd @UNUSED2, @UNUSED2, @UNUSED2, @ABT2
                    dd @UNUSED2, @UNUSED2, @UNUSED2, @UDEF2
                    dd @UNUSED2, @UNUSED2, @UNUSED2, @USER2                  
  .code 
  @END:
  pop SC_WAIT
endm 