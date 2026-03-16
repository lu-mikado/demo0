;**************************************************************************************************
;vemcpy.asm - simple vram copy rountine [RGB32 or RGB24]
;same as memcpy, but add pitch - offset and height [Y scanline] 
;
;Strategy:
; Small size copy 
;       - easy, no care 
; Source/target aligned or same address offset, num is align
;       - Simple, aligned copy or/corrent offset, copy it 
; Source/target aligned or same address offset, num is unalign
;       - Simple, rep movsb it 
;           [ 
;              Under certain conditions, Rep, movsd/movsb, 
;              Rep, and stdsd/stdsb copy speeds equal to movq/movaps in SSE/MMX
;
;              % Move the entire cache line at once
;              % source address and destination address are aligned by 8
;              % increment is forward (clear direction sign)
;              % counter (ECX) is greater than or equal to 64
;              % The difference between *EDI and ESI is numerically greater than or equal to 32
;              % source memory and destination memory must be write back or combination write mode
;           ]
;
;       - For misaligned addresses, we can always convert to\
;           target alignment and the source is misaligned.
;            Read the two MEM block, and use the palignr assembler instructions \
;                   [http://www.felixcloutier.com/x86/PALIGNR.html] \
;          to fit the source operands aligned to the write,   \
;            for boundary access, #GP (0) exception may be generated. Please note   -   moecmks
;**************************************************************************************************

    .386
    .mmx
    .xmm
    .model flat, c

;; save stack frame [use default TEB chunk]
ifdef __X86_32_
    STF_SAVE  equ   0700h 
elseifdef __X86_WOW64_ 
    STF_SAVE  equ   01ACh
elseifdef __X86_64_ 
    STF_SAVE  equ   02D0h 
else 
    STF_SAVE  equ   01ACh 
endif 
;;  for prg-code align
    ALIGN_Z   equ   align 16

    .code
vemcpy proc C
    option prologue:none, epilogue:none

    ;; esp + 4 <- target pointer 
    ;; esp + 8 <- target x 
    ;; esp + 12<- target y
    ;; esp + 16<- target pitch
    ;; esp + 20<- source pointer 
    ;; esp + 24<- source x 
    ;; esp + 28<- source y 
    ;; esp + 32<- source pitch
    ;; esp + 36<- copy's width 
    ;; esp + 40<- copy's height

    push  esi 
    push  edi 
    push  ebx 
    push  ebp     ;; save  context  register  
; save  esp 
  assume  fs:nothing
    mov   fs:[STF_SAVE], esp 
    cld 
    
    ;; reloc  target start address 
    mov   edi,  [esp+4 + 16] ;; edi <- target pointer 
    mov   ebx,  [esp+8 + 16] ;; ebx <- target x  
    mov   ecx,  [esp+12+ 16] ;; ecx <- target y 
    mov   edx,  [esp+16+ 16] ;; edx <- target pitch  
   
    test  edi,  edi          ;; null  pointer ?
    jz    V_EXIT
    
    shl   ebx,  2   
    add   edi,  ebx 
    imul  ecx,  edx 
    add   edi,  ecx          ;; edi <- target start pointer[save]
    
    ;; reloc  source start address 
    mov   esi,  [esp+20+ 16] ;; esi <- source pointer 
    mov   ebx,  [esp+24+ 16] ;; ebx <- source x  
    mov   ecx,  [esp+28+ 16] ;; ecx <- source y 
    mov   ebp,  [esp+32+ 16] ;; ebp <- source pitch  
   
    test  esi,  esi          ;; null  pointer ?
    jz    V_EXIT
    
    shl   ebx,  2   
    add   esi,  ebx 
    imul  ecx,  ebp 
    add   esi,  ecx          ;; esi <- source start pointer[save]
    
    ;; get width/height, pitch RVA.
    mov   ecx,  [esp+36 +16] ;; ecx <- width 
    mov   ebx,  [esp+40 +16] ;; ebx <- height 
    
    test  ecx,  ecx          ;; width is ZERO ?
    jz    V_EXIT
    
    test  ebx,  ebx          ;; height is ZERO ?
    jz    V_EXIT
    
    lea   eax,  [ecx*4] 
    nop                      ;; spare 
    test  edx,      15       ;; check pitch align 16??
    jne   unAlignPitchWind 
    test  ebp,      15       ;;  check pitch align 16??
    jne   unAlignPitchWind   
    sub   edx,  eax          ;; edx <- target pitch RVA
    sub   ebp,  eax          ;; ebp <- source pitch RVA 
    mov   esp,  ecx 
    
    ;; register dispatch 
    ;; 
    ;; eax <- none 
    ;; ebx <- height 
    ;; ecx <- width 
    ;; edx <- target pitch RVA
    ;; ebp <- source pitch RVA 
    ;; edi <- target pointer
    ;; esi <- source pointer  
    ;; esp <- width 
    
    ;  small size ???
    ;---------------------------------------------------------------------------------------------
        cmp   ecx,   31 
        ja    @F
        jmp   dword ptr smallCopyRoutine[ecx*4]
        
        ; check align
        ; ------------------------------------------------------------------------------
        ALIGN_Z
        @@:
          mov   eax,  edi 
          mov   esp,  esi 
          and   eax,  1100b
          and   esp,  1100b 
          shr   esp,    2 ;; DDSS
          or    eax,  esp 
          mov   esp,  ecx
          jmp   dword ptr chkAlignRoutine[eax*4]
    ;  small size ???
    ;---------------------------------------------------------------------------------------------
	unAlignPitchWind:
         sub   edx,  eax          ;; edx <- target pitch RVA
         sub   ebp,  eax          ;; ebp <- source pitch RVA 
         mov   esp,  ecx 
         cmp   ecx,   31 
         ja    @F
         jmp   dword ptr smallCopyRoutine[ecx*4]
        
        ; check align
        ; ------------------------------------------------------------------------------
        ALIGN_Z
        @@:
          mov   esp,  ecx
          jmp   cAR_SXXX
    ALIGN_Z  
cAR_S0D0: ;; aligned 16 (TODO:SSE Unwind!!!)
    test  ecx,  15
    jz    cAR_A16_pure
    sub   ecx,  16
    sub   esp,  16
    ALIGN_Z
cAR_A16_dirty:
    movdqa  xmm0,  [esi+000h]
    movdqa  xmm1,  [esi+010h]
    movdqa  xmm2,  [esi+020h]
    movdqa  xmm3,  [esi+030h]
    
    ;; maybe use movntdqa/prefetch ??
    movdqa  [edi+000h], xmm0
    movdqa  [edi+010h], xmm1
    movdqa  [edi+020h], xmm2
    movdqa  [edi+030h], xmm3

    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_A16_dirty  
    add   ecx,  16
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B    
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    jnz   cAR_A16_dirty
    jmp   V_EXIT
    
    ALIGN_Z
cAR_A16_pure:
    movdqa  xmm0,  [esi+000h]
    movdqa  xmm1,  [esi+010h]
    movdqa  xmm2,  [esi+020h]
    movdqa  xmm3,  [esi+030h]
    
    movdqa  [edi+000h], xmm0
    movdqa  [edi+010h], xmm1
    movdqa  [edi+020h], xmm2
    movdqa  [edi+030h], xmm3

    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_A16_pure      
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    jnz   cAR_A16_pure
    jmp   V_EXIT

    ALIGN_Z
cAR_S0D8: ;; aligned 8
cAR_S8D0: ;; aligned 8
cAR_S8D8: ;; aligned 8 
cAR_SXXX:
    test  ecx,  7
    jz    cAR_A8_pure
    sub   ecx,  8
    sub   esp,  8
    ALIGN_Z
cAR_A8_dirty:
    movq  mm0,  [esi+000h]
    movq  mm1,  [esi+008h]
    movq  mm2,  [esi+010h]
    movq  mm3,  [esi+018h]
    
    movq  [edi+000h], mm0
    movq  [edi+008h], mm1
    movq  [edi+010h], mm2
    movq  [edi+018h], mm3

    add   esi,  32
    add   edi,  32
    sub   ecx,   8
    jg    cAR_A8_dirty  
    add   ecx,   8
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B    
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    jnz   cAR_A8_dirty
    emms 
    jmp   V_EXIT
    
    ALIGN_Z
cAR_A8_pure:
    movq  mm0,  [esi+000h]
    movq  mm1,  [esi+008h]
    movq  mm2,  [esi+010h]
    movq  mm3,  [esi+018h]
    
    movq  [edi+000h], mm0
    movq  [edi+008h], mm1
    movq  [edi+010h], mm2
    movq  [edi+018h], mm3

    add   esi,  32
    add   edi,  32
    sub   ecx,   8
    jnz   cAR_A8_pure      
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    jnz   cAR_A8_pure
    emms
    jmp   V_EXIT

    ALIGN_Z
cAR_S4D4: ;; offset aligned one 
cAR_SCDC: ;; offset aligned one 
cAR_S4DC: ;; offset aligned one 
cAR_SCD4: ;; offset aligned one 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  1
    test  esp,  7  
    lea   esi,  [esi+4]
    lea   edi,  [edi+4]
    lea   edx,  [edx+4]
    lea   ebp,  [ebp+4]
    mov   ecx,  esp 
    jz    cAR_A8cc_pure
    sub   esp,  8 
    mov   ecx,  esp   
    ALIGN_Z
cAR_A8cc_dirty:
    movq  mm0,  [esi+000h]
    movq  mm1,  [esi+008h]
    movq  mm2,  [esi+010h]
    movq  mm3,  [esi+018h]
    
    movq  [edi+000h], mm0
    movq  [edi+008h], mm1
    movq  [edi+010h], mm2
    movq  [edi+018h], mm3

    add   esi,  32
    add   edi,  32
    sub   ecx,   8
    jg    cAR_A8cc_dirty 
    add   ecx,   8 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B    
    lea   eax,  [esp+8+1]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    add   esi,  ebp
    mov   [edi+eax*4],  ecx
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    jnz   cAR_A8cc_dirty
    emms 
    jmp   V_EXIT
   
    ALIGN_Z
cAR_A8cc_pure:
    movq  mm0,  [esi+000h]
    movq  mm1,  [esi+008h]
    movq  mm2,  [esi+010h]
    movq  mm3,  [esi+018h]
    
    movq  [edi+000h], mm0
    movq  [edi+008h], mm1
    movq  [edi+010h], mm2
    movq  [edi+018h], mm3

    add   esi,  32
    add   edi,  32
    sub   ecx,   8
    jnz   cAR_A8cc_pure  
    mov   ecx,  [esi+eax*4]    
    add   esi,  ebp
    mov   [edi+eax*4],  ecx
    mov   ecx,  esp
    add   edi,  edx 
    mov   eax,  eax  
    dec   ebx 
    jnz   cAR_A8cc_pure
    emms
    jmp   V_EXIT

    ALIGN_Z
cAR_S4D0: ;; ========================= target aligned 16- SYS-4 level %
    movdqa  xmm0,   [esi-04H]
    test  ecx,  15
    jz    cAR_S4D0_pure
    sub   ecx,  16
    sub   esp,  16
    ALIGN_Z
  cAR_S4D0_dirty: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h]  
     
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   4     ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   4     ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   4     ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   4     ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_S4D0_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B      
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    movdqa  xmm0,   [esi-04H]
    jnz   cAR_S4D0_dirty
    jmp   V_EXIT
      
   ;--------------------------------------------------------------------
    ALIGN_Z 
  cAR_S4D0_pure:   
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   4     ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   4     ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   4     ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   4     ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_S4D0_pure      
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    movdqa  xmm0,   [esi-04H]
    jnz   cAR_S4D0_pure
    jmp   V_EXIT

    ALIGN_Z    
cAR_S0DC: ;; unaligned ================================== SYS-4 level 1 BUG
    
    movdqa  xmm0,   [esi] 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  1
    test  esp,  15
    lea   esi,  [esi+4]
    lea   edi,  [edi+4]
    lea   edx,  [edx+4]
    lea   ebp,  [ebp+4]
    mov   ecx,  esp 
    jz    cAR_S0DC_pure
    sub   esp,  16 
    mov   ecx,  esp      
    ALIGN_Z 
  cAR_S0DC_dirty: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,    4    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,    4    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,    4    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,    4    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_S0DC_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B 
    lea   eax,  [esp+16+1]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    add   esi,  ebp
    mov   [edi+eax*4],  ecx
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-04H] 
    jnz   cAR_S0DC_dirty
    jmp   V_EXIT   
       
  cAR_S0DC_pure: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,    4    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,    4    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,    4    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,    4    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_S0DC_pure 
    mov   ecx,  [esi+eax*4]
    add   esi,  ebp
    mov   [edi+eax*4],  ecx
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-04H] 
    jnz   cAR_S0DC_pure
    jmp   V_EXIT  

cAR_SCD8: ;; unaligned ================================== SYS-4 level 2

    movdqa  xmm0,   [esi+4h] 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  2
    test  esp,  15
    lea   esi,  [esi+8]
    lea   edi,  [edi+8]
    lea   edx,  [edx+8]
    lea   ebp,  [ebp+8]
    mov   ecx,  esp 
    jz    cAR_SCD8_pure
    sub   esp,  16 
    mov   ecx,  esp      
    ALIGN_Z 
  cAR_SCD8_dirty: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,    4    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,    4    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,    4    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,    4    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_SCD8_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B 
    lea   eax,  [esp+16+2]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-04H] 
    jnz   cAR_SCD8_dirty
    jmp   V_EXIT   
       
  cAR_SCD8_pure: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,    4    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,    4    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,    4    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,    4    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_SCD8_pure 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-04H] 
    jnz   cAR_SCD8_pure
    jmp   V_EXIT  


cAR_S8D4: ;; unaligned ================================== SYS-4 level 3
    movdqa  xmm0,   [esi+8h] 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  3
    test  esp,  15
    lea   esi,  [esi+12]
    lea   edi,  [edi+12]
    lea   edx,  [edx+12]
    lea   ebp,  [ebp+12]
    mov   ecx,  esp 
    jz    cAR_S8D4_pure
    sub   esp,  16 
    mov   ecx,  esp      
    ALIGN_Z 
  cAR_S8D4_dirty: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,    4    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,    4    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,    4    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,    4    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_S8D4_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B 
    lea   eax,  [esp+16+3]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    mov   ecx,  [esi+eax*4+8]
    mov   [edi+eax*4+8],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-04H] 
    jnz   cAR_S8D4_dirty
    jmp   V_EXIT   
       
  cAR_S8D4_pure: 
    movdqa  xmm1,   [esi-04H+010h] 
    movdqa  xmm2,   [esi-04H+020h] 
    movdqa  xmm3,   [esi-04H+030h]    
    movdqa  xmm4,   [esi-04H+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,    4    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,    4    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,    4    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,    4    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_S8D4_pure 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    mov   ecx,  [esi+eax*4+8]
    mov   [edi+eax*4+8],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-04H] 
    jnz   cAR_S8D4_pure
    jmp   V_EXIT  


cAR_SCD0: ;; target aligned 16 ========================== SYS-C level %
    movdqa  xmm0,   [esi-0CH]
    test  ecx,  15
    jz    cAR_SCD0_pure  
    sub   ecx,  16 
    mov   esp,  ecx 
    ALIGN_Z 
  cAR_SCD0_dirty:
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7       

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_SCD0_dirty
    add   ecx,  16
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B     
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    movdqa  xmm0,   [esi-0CH]
    jnz   cAR_SCD0_dirty
    jmp   V_EXIT
   ;--------------------------------------------------------------------
    ALIGN_Z 
  cAR_SCD0_pure:  
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4   
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_SCD0_pure      
    add   esi,  ebp
    add   edi,  edx   
    mov   ecx,  esp 
    dec   ebx 
    movdqa  xmm0,   [esi-0CH]
    jnz   cAR_SCD0_pure
    jmp   V_EXIT


cAR_S8DC: ;; unaligned ================================== SYS-C level 1 BUG
    movdqa  xmm0,   [esi-08h] 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  1
    test  esp,  15
    lea   esi,  [esi+4]
    lea   edi,  [edi+4]
    lea   edx,  [edx+4]
    lea   ebp,  [ebp+4]
    mov   ecx,  esp 
    jz    cAR_S8DC_pure
    sub   esp,  16 
    mov   ecx,  esp      
    ALIGN_Z 
  cAR_S8DC_dirty: 
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_S8DC_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B 
    lea   eax,  [esp+16+1]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    add   esi,  ebp
    mov   [edi+eax*4],  ecx
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-0CH] 
    jnz   cAR_S8DC_dirty
    jmp   V_EXIT   
       
  cAR_S8DC_pure: 
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_S8DC_pure 
    mov   ecx,  [esi+eax*4]
    add   esi,  ebp
    mov   [edi+eax*4],  ecx
    mov   ecx,  esp
    add   edi,  edx  
    mov   eax,  eax ;; spare  
    dec   ebx 
    movdqa  xmm0,   [esi-0CH] 
    jnz   cAR_S8DC_pure
    jmp   V_EXIT

cAR_S4D8: ;; unaligned ================================== SYS-C level 2

    movdqa  xmm0,   [esi-04H] 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  2
    test  esp,  15
    lea   esi,  [esi+8]
    lea   edi,  [edi+8]
    lea   ebp,  [ebp+8]
    lea   edx,  [edx+8]
    mov   ecx,  esp 
    jz    cAR_S4D8_pure
    sub   esp,  16 
    mov   ecx,  esp      
    ALIGN_Z 
  cAR_S4D8_dirty: 
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_S4D8_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B 
    lea   eax,  [esp+16+2]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx   
    dec   ebx 
    movdqa  xmm0,   [esi-0CH] 
    jnz   cAR_S4D8_dirty
    jmp   V_EXIT   
      
  cAR_S4D8_pure: 
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_S4D8_pure 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx   
    dec   ebx 
    movdqa  xmm0,   [esi-0CH] 
    jnz   cAR_S4D8_pure
    jmp   V_EXIT 

cAR_S0D4: ;; unaligned ================================== SYS-C level 3

    movdqa  xmm0,   [esi] 
    lea   eax,  [esp]
    neg   eax
    sub   esp,  3
    test  esp,  15
    lea   esi,  [esi+12]
    lea   edi,  [edi+12]
    lea   ebp,  [ebp+12]
    lea   edx,  [edx+12]
    mov   ecx,  esp 
    jz    cAR_S0D4_pure
    sub   esp,  16 
    mov   ecx,  esp      
    ALIGN_Z 
  cAR_S0D4_dirty: 
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jg    cAR_S0D4_dirty
    add   ecx,  16 
  @@:
    mov   eax,  [esi]
    lea   esi,  [esi+4]
    mov   [edi],  eax 
    dec   ecx 
    lea   edi,  [edi+4]  
    jnz   @B 
    lea   eax,  [esp+16+3]
    neg   eax 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    mov   ecx,  [esi+eax*4+8]
    mov   [edi+eax*4+8],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx   
    dec   ebx 
    movdqa  xmm0,   [esi-0CH] 
    jnz   cAR_S0D4_dirty
    jmp   V_EXIT   
      
  cAR_S0D4_pure: 
    movdqa  xmm1,   [esi-0CH+010h] 
    movdqa  xmm2,   [esi-0CH+020h] 
    movdqa  xmm3,   [esi-0CH+030h]    
    movdqa  xmm4,   [esi-0CH+040h] 
    
    movdqa  xmm7,   xmm4 
    palignr xmm4,   xmm3,   12    ;; fit, <-xmm4 <-target[3] 
    palignr xmm3,   xmm2,   12    ;; fit, <-xmm3 <-target[2] 
    palignr xmm2,   xmm1,   12    ;; fit, <-xmm2 <-target[1]  
    palignr xmm1,   xmm0,   12    ;; fit, <-xmm1 <-target[0]  
    movdqa  xmm0,   xmm7      

    ;; write back buffer 
    movdqa  [edi+000h],  xmm1
    movdqa  [edi+010h],  xmm2
    movdqa  [edi+020h],  xmm3
    movdqa  [edi+030h],  xmm4    
    
    add   esi,  64
    add   edi,  64
    sub   ecx,  16
    jnz   cAR_S0D4_pure 
    mov   ecx,  [esi+eax*4]
    mov   [edi+eax*4],  ecx
    mov   ecx,  [esi+eax*4+4]
    mov   [edi+eax*4+4],  ecx
    mov   ecx,  [esi+eax*4+8]
    mov   [edi+eax*4+8],  ecx
    add   esi,  ebp
    mov   ecx,  esp
    add   edi,  edx   
    dec   ebx 
    movdqa  xmm0,   [esi-0CH] 
    jnz   cAR_S0D4_pure
    jmp   V_EXIT 

    ALIGN_Z 
V_EXIT:
    mov   esp,  fs:[STF_SAVE]
    pop   ebp 
    pop   ebx
    pop   edi 
    pop   esi 
    ret 
    
    ALIGN_Z 
;---------------------------------------------------------------------------------------------
chkAlignRoutine dd cAR_S0D0, cAR_S4D0, cAR_S8D0, cAR_SCD0 ;; 00xx
                dd cAR_S0D4, cAR_S4D4, cAR_S8D4, cAR_SCD4 ;; 01xx
                dd cAR_S0D8, cAR_S4D8, cAR_S8D8, cAR_SCD8 ;; 10xx
                dd cAR_S0DC, cAR_S4DC, cAR_S8DC, cAR_SCDC ;; 11xx 
chkUnAlignRoutine dd cAR_S0D0, cAR_S4D0, cAR_S8D0, cAR_SCD0 ;; 00xx
                dd cAR_S0D4, cAR_S4D4, cAR_S8D4, cAR_SCD4 ;; 01xx
                dd cAR_S0D8, cAR_S4D8, cAR_S8D8, cAR_SCD8 ;; 10xx
                dd cAR_S0DC, cAR_S4DC, cAR_S8DC, cAR_SCDC ;; 11xx 
;---------------------------------------------------------------------------------------------
; smallCopyRoutine MACRO 
;---------------------------------------------------------------------------------------------
    sCR_start macro s
        ALIGN_Z 
      sCR&s&:
      
      endm 
      
    sCR_end macro s
        lea   esi,  [esi+ebp+0&s&h*4]
        lea   edi,  [edi+edx+0&s&h*4]
        dec   ebx 
        jnz   sCR&s& 
        jmp   V_EXIT
         
      endm 
    ; ----------------------------------------------------
    ; step 
    ; ----------------------------------------------------
    sCRc1 macro s 
        mov   eax,  [esi+s]
        mov   [edi+s],  eax 
        
      endm 
      
    sCRc2 macro s 
        mov   eax,  [esi+s]
        mov   ecx,  [esi+s+4]
        mov   [edi+s],  eax 
        mov   [edi+s+4],ecx     
      endm 
      
    sCRc4 macro s 
        sCRc2 s 
        sCRc2 s + 08h
      endm 

    sCRc8 macro s 
        sCRc4 s 
        sCRc4 s + 10h  
      endm 
      
    sCRc16 macro s 
        sCRc8 s 
        sCRc8 s + 20h     
      endm 
      
    sCR_start 01
    sCRc1 00h
    sCR_end 01 ;; ----------------- width 1 

    sCR_start 02
    sCRc2 00h
    sCR_end 02 ;; ----------------- width 2   
    
    sCR_start 03
    sCRc2 00h
    sCRc1 08h
    sCR_end 03 ;; ----------------- width 3
    
    sCR_start 04
    sCRc4 00h
    sCR_end 04 ;; ----------------- width 4
    
    sCR_start 05
    sCRc4 00h
    sCRc1 10h
    sCR_end 05 ;; ----------------- width 5

    sCR_start 06
    sCRc4 00h
    sCRc2 10h
    sCR_end 06 ;; ----------------- width 6 
    
    sCR_start 07
    sCRc4 00h
    sCRc2 10h
    sCRc1 18h
    sCR_end 07 ;; ----------------- width 7
    
    sCR_start 08
    sCRc8 00h    
    sCR_end 08 ;; ----------------- width 8 
    
    sCR_start 09
    sCRc8 00h    
    sCRc1 20h
    sCR_end 09 ;; ----------------- width 9 

    sCR_start 0A
    sCRc8 00h    
    sCRc2 20h 
    sCR_end 0A ;; ----------------- width 10   
    
    sCR_start 0B
    sCRc8 00h    
    sCRc2 20h 
    sCRc1 28h
    sCR_end 0B ;; ----------------- width 11
    
    sCR_start 0C 
    sCRc8 00h     
    sCRc4 20h
    sCR_end 0C ;; ----------------- width 12
    
    sCR_start 0D
    sCRc8 00h     
    sCRc4 20h
    sCRc1 30h
    sCR_end 0D ;; ----------------- width 13

    sCR_start 0E
    sCRc8 00h     
    sCRc4 20h
    sCRc2 30h
    sCR_end 0E ;; ----------------- width 14    
    
    sCR_start 0F
    sCRc8 00h    
    sCRc4 20h 
    sCRc2 30h
    sCRc1 38h
    sCR_end 0F ;; ----------------- width 15
    
    sCR_start 10
    sCRc16 00h
    sCR_end 10 ;; ----------------- width 16 
    
    sCR_start 11
    sCRc16 00h
    sCRc1 40h
    sCR_end 11 ;; ----------------- width 17 

    sCR_start 12
    sCRc16 00h
    sCRc2 40h
    sCR_end 12 ;; ----------------- width 18    
    
    sCR_start 13
    sCRc16 00h
    sCRc2 40h
    sCRc1 48h
    sCR_end 13 ;; ----------------- width 19
    
    sCR_start 14 
    sCRc16 00h
    sCRc4 40h
    sCR_end 14 ;; ----------------- width 20 
    
    sCR_start 15
    sCRc16 00h
    sCRc4 40h
    sCRc1 50h
    sCR_end 15 ;; ----------------- width 21

    sCR_start 16
    sCRc16 00h
    sCRc4 40h
    sCRc2 50h
    sCR_end 16 ;; ----------------- width 22    
    
    sCR_start 17
    sCRc16 00h
    sCRc4 40h
    sCRc2 50h
    sCRc1 58h
    sCR_end 17 ;; ----------------- width 23
    
    sCR_start 18 
    sCRc16 00h
    sCRc8 40h
    sCR_end 18 ;; ----------------- width 24 
    
    sCR_start 19
    sCRc16 00h
    sCRc8 40h
    sCRc1 60h
    sCR_end 19 ;; ----------------- width 25

    sCR_start 1A
    sCRc16 00h
    sCRc8 40h
    sCRc2 60h
    sCR_end 1A ;; ----------------- width 26  
    
    sCR_start 1B
    sCRc16 00h
    sCRc8 40h
    sCRc2 60h
    sCRc1 68h   
    sCR_end 1B ;; ----------------- width 27
    
    sCR_start 1C
    sCRc16 00h
    sCRc8 40h
    sCRc4 60h
    sCR_end 1C ;; ----------------- width 28
    
    sCR_start 1D
    sCRc16 00h
    sCRc8 40h
    sCRc4 60h
    sCRc1 70h
    sCR_end 1D ;; ----------------- width 29

    sCR_start 1E
    sCRc16 00h
    sCRc8 40h
    sCRc4 60h
    sCRc2 70h 
    sCR_end 1E ;; ----------------- width 30 
    
    sCR_start 1F
    sCRc16 00h
    sCRc8 40h
    sCRc4 60h
    sCRc2 70h
    sCRc1 78h
    sCR_end 1F ;; ----------------- width 31
    
    ALIGN_Z 
    
;---------------------------------------------------------------------------------------------
smallCopyRoutine dd sCR1F, sCR01, sCR02, sCR03, sCR04, sCR05, sCR06, sCR07 
                 dd sCR08, sCR09, sCR0A, sCR0B, sCR0C, sCR0D, sCR0E, sCR0F 
                 dd sCR10, sCR11, sCR12, sCR13, sCR14, sCR15, sCR16, sCR17  
                 dd sCR18, sCR19, sCR1A, sCR1B, sCR1C, sCR1D, sCR1E, sCR1F 
        
vemcpy endp 
  end 
