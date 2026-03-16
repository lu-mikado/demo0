        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, stdcall      ; 32 bit memory model
        option casemap :none      ; case sensitive

;=============================================================================
;     int blend (void *  _d_vptr, int _d_dword_byte, 
;               int dst_x, int dst_y,  void *  _s_vptr, int _s_dword_byte, 
;               int src_x, int src_y, 
;                                               int w, int h, 
;                   BYTE alpha_dst,  BYTE alpha_src  );
;
;    @TODO: SSSE4 vector 127 opt. better register mgr. align 
;=============================================================================   
           
          
    .code

align 16 
blend proc C
             option prologue:none, epilogue:none
             
    push  edi    ;- U save ctx. 
    push  esi    ;- V save ctx 
    push  ebx    ;- U save ctx 
    push  ebp    ;- V save ctx  
STACK_PUSH_CC equ 32

    mov   edi,   [esp+4+STACK_PUSH_CC] ; -U  load _d_vptr .
    mov   ecx,     [esp+8+STACK_PUSH_CC] ; -V  load _d_dword_byte.
    mov   eax,      [esp+12+STACK_PUSH_CC] ; -U   load  dst_x
    mov   edx,    [esp+16+STACK_PUSH_CC] ; -V load dst_y
    imul edx, ecx   ;-N tempA := _d_dword_byte * dst_y
    add   edi,   edx  ; -N  repos dst_y 
    lea  edi, [edi+eax*4];-N  repos dst_x  dst vptr repos over. 
    mov   esi,   [esp+20+STACK_PUSH_CC] ; -N load _s_vptr .
    mov   edx,     [esp+24+STACK_PUSH_CC] ; -N  load _s_dword_byte.
    mov   eax,      [esp+28+STACK_PUSH_CC] ; -N   load  src_x
    mov   ebx,    [esp+32+STACK_PUSH_CC] ; -N load src_y
    imul ebx, edx   ;-N tempA := _d_dword_byte * src_y
    add   esi,   ebx  ; -N  repos src_y 
    lea  esi, [edi+eax*4];-N  repos src_x  src vptr repos over. 
    mov    eax, [esp+36+STACK_PUSH_CC] ; -N W
    lea    ebx, [eax *4]
    sub    edx,  ebx 
    sub    ecx,  ebx  
    mov    ebx, [esp+40+STACK_PUSH_CC] ; - N H
    mov  ebp,  eax 
    cmp   eax,   0
    jle        V_EXIT_FAILED
    cmp   edx,   0
    jle     V_EXIT_FAILED
    
    pxor xmm7,  xmm7 
    movss xmm6, dword ptr[esp+44+STACK_PUSH_CC] ; load alpha dst .
    movss xmm5, dword ptr[esp+48+STACK_PUSH_CC] ; load alpha src . 
    pshufb xmm6,  xmm7    ; fill all alpha - dst.
    pshufb xmm5,  xmm7    ; fill all alpha - src.
    psllw  xmm6,  8         
    psllw  xmm5,  8       
    ;;  xmm7 <- ZERO
    ;;  xmm6 <- alpha dst high . 
    ;;  xmm5 <- alpha src high . 
    cmp   eax,   8  
    jge   _vecBlock_predo
    
    ;;
    ;; make alpha marco .
    ;; 
    mixer_make_base macro mixer_out, base_mix
      movdqa  xmm4,  mixer_out   ;; save cache .
      punpcklbw   mixer_out,   xmm7
      pmulhuw   mixer_out,   base_mix
      punpckhbw   xmm4,   xmm7
      pmulhuw   xmm4,   base_mix   
      packuswb    mixer_out,  xmm4
    endm ;;; !! mixer_make_base
    
    mixer_make_src macro mixer_out
      mixer_make_base mixer_out, xmm5
    endm ;;; !! mixer_make_src
  
    mixer_make_dst macro mixer_out
      mixer_make_base mixer_out, xmm6
    endm ;;; !! mixer_make_dst
    
    align 16
    _badLoop:
      movd   xmm0, dword ptr [esi]     ;; fetch S_P0
      movd   xmm2, dword ptr [edi]     ;; fetch D_P0    
      mixer_make_src xmm0  ;; 
      mixer_make_dst xmm2  ;;  MIX out .           
      paddusb xmm0, xmm2  ;; MIX add 
      movd   dword ptr [edi], xmm0    ;; write D_P0  
      add edi, 4
      add esi, 4
      dec eax
      jne _badLoop 
      add  edi, ecx 
      add esi, edx 
      dec  edx 
      mov  eax, ebp 
      nop 
      jne _badLoop 
      jmp V_EXIT_SUCCESS
      
    align 16 
    _vecBlock_predo:
      test  eax,  15
      je   _betterLoop 
      and   eax, -8
    align 16
    _StdLoop:
      movdqu   xmm0, xmmword ptr [esi+000h]     ;; fetch S_P0
      movdqu   xmm1, xmmword ptr [esi+010h]     ;; fetch S_P1
      movdqu   xmm2, xmmword ptr [edi+000h]     ;; fetch D_P0
      movdqu   xmm3, xmmword ptr [edi+010h]     ;; fetch D_P1       
      mixer_make_src xmm0 
      mixer_make_src xmm1
      mixer_make_dst xmm2
      mixer_make_dst xmm3  ;;  MIX out .   
      paddusb xmm0, xmm2
      paddusb xmm1, xmm3   ;; MIX add   
      movdqu   xmmword ptr [edi+000h], xmm0    ;; write D_P0
      movdqu   xmmword ptr [edi+010h], xmm1     ;; write D_P1   
      add  edi,  32
      add  esi,  32
      sub  eax,  8   
      jne  _StdLoop 
      mov  eax,  ebp 
      and  eax,  7
    @@:
      movd   xmm0, dword ptr [esi]     ;; fetch S_P0
      movd   xmm2, dword ptr [edi]     ;; fetch D_P0    
      mixer_make_src xmm0  ;; 
      mixer_make_dst xmm2  ;;  MIX out .           
      paddusb xmm0, xmm2  ;; MIX add 
      movd   dword ptr [edi], xmm0    ;; write D_P0   
      add edi, 4
      add esi, 4   
      dec eax
      jne @B 
      add  edi, ecx 
      mov  eax,  ebp
      add  esi, edx 
      and   eax, -8 
      dec  edx 
      jne _StdLoop 
      jmp V_EXIT_SUCCESS
      
    align 16
    _betterLoop:
      movdqu   xmm0, xmmword ptr [esi+000h]     ;; fetch S_P0
      movdqu   xmm1, xmmword ptr [esi+010h]     ;; fetch S_P1
      movdqu   xmm2, xmmword ptr [edi+000h]     ;; fetch D_P0
      movdqu   xmm3, xmmword ptr [edi+010h]     ;; fetch D_P1    
      mixer_make_src xmm0   
      mixer_make_src xmm1
      mixer_make_dst xmm2
      mixer_make_dst xmm3  ;;  MIX out .
      paddusb xmm0, xmm2
      paddusb xmm1, xmm3    ;; MIX add 
      movdqu   xmmword ptr [edi+000h], xmm0    ;; write D_P0
      movdqu   xmmword ptr [edi+010h], xmm1     ;; write D_P1  
      add  edi,  32
      add  esi,  32 
      sub  eax,  8 
      jne _betterLoop  
      add  edi, ecx 
      add  esi, edx
      mov  eax, ebp 
      mov  ecx, ecx 
      dec  edx 
      jne _betterLoop
      
  V_EXIT_SUCCESS:
    pop  ebp
    pop  ebx 
    pop  esi 
    pop  edi 
    xor  eax,  eax 
    ret   
  V_EXIT_FAILED:
    pop  ebp 
    pop  ebx 
    pop  esi 
    pop  edi 
    mov  eax,  -1 
    ret  

blend endp
 
    end
