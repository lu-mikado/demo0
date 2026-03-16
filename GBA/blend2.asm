        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, stdcall      ; 32 bit memory model
        option casemap :none      ; case sensitive

;=============================================================================
;     int blend2 (void *  _vptr, int pitch, 
;               int dst_x, int dst_y, 
;                                               int dst_w, int dst_h, 
;               COLORREF col,
;                   BYTE alpha_dst,  BYTE alpha_src  );
;
;    @TODO: SSSE4 vector 127 opt. better register mgr. align 
;=============================================================================   
           
          
    .code

align 16 
blend2 proc C
             option prologue:none, epilogue:none
             
    push  edi    ;- U save ctx. 
    push  esi    ;- V save ctx 
STACK_PUSH_CC equ 8

    mov   edi,   [esp+4+STACK_PUSH_CC] ; -U  load _vptr .
    mov   ecx,     [esp+8+STACK_PUSH_CC] ; -V  load pitch .
    mov   eax,      [esp+12+STACK_PUSH_CC] ; -U pos X 
    mov   edx,    [esp+16+STACK_PUSH_CC] ; -V pos Y
    imul edx, ecx   ;-N mul it. 
    add   edi,   edx  ; -N 
    lea  edi, [edi+eax*4];-N
    mov    eax, [esp+20+STACK_PUSH_CC] ; -N W
    xor    esi,  esi 
    mov    edx, [esp+24+STACK_PUSH_CC] ; - N H
    cmp   eax,   esi
    jle        V_EXIT_FAILED
    cmp   edx,   esi
    jle     V_EXIT_FAILED
    lea    esi,  [eax *4]
    sub      ecx,    esi 
    mov  esi,  eax 
    
    pxor xmm7,  xmm7 
    movss xmm6, dword ptr[esp+28+STACK_PUSH_CC] ; load source . 
    movss xmm5, dword ptr[esp+32+STACK_PUSH_CC] ; load alpha dst . 
    movss xmm0, dword ptr[esp+36+STACK_PUSH_CC] ; load alpha src . 
    pshufb xmm0,  xmm7    ; fill all alpha - src.
    psllw  xmm0,  8       ;  save word's hi bit alpha value  
    pshufb xmm5,  xmm7    ; fill all alpha - dst.
    psllw  xmm5,  8       ;  save word's hi bit alpha value  
    punpcklbw   xmm6,   xmm7    
    pmulhuw   xmm6,  xmm0 
    packuswb       xmm6,   xmm6
    pshufd  xmm6, xmm6,  0
    ;;  xmm7 <- ZERO
    ;;  xmm6 <- source fixed pixel . 
    ;;  xmm5 <- ahpla group. 
    cmp   eax,   16  
    jge   _vecBlock_predo
    ;;
    ;; make alpha marco .
    ;; 
    mixer_make macro mixer_out
      movdqa  xmm4,  mixer_out   ;; save cache .
      punpcklbw   mixer_out,   xmm7
      pmulhuw   mixer_out,   xmm5
      punpckhbw   xmm4,   xmm7
      pmulhuw   xmm4,   xmm5   
      packuswb    mixer_out,  xmm4
    endm ;;; !! mixer_make
    
    align 16
    _badLoop:
      movd   xmm0, dword ptr [edi]     ;; fetch dword 
      mixer_make xmm0 
      paddusb xmm0, xmm6     
      movd   dword ptr [edi], xmm0
      add edi, 4
      dec eax
      jne _badLoop 
      add  edi, ecx 
      dec  edx 
      mov  eax, esi 
      jne _badLoop 
      jmp V_EXIT_SUCCESS
      
    align 16 
    _vecBlock_predo:
      test  eax,  15
      je   _betterLoop 
      and   eax, -16
    align 16
    _StdLoop:
      movdqu   xmm0, xmmword ptr [edi+000h]     ;; fetch P0
      movdqu   xmm1, xmmword ptr [edi+010h]     ;; fetch P1
      movdqu  xmm2, xmmword ptr [edi+020h]     ;; fetch P2
      movdqu  xmm3, xmmword ptr [edi+030h]     ;; fetch P3
    
      mixer_make xmm0 
      mixer_make xmm1
      mixer_make xmm2
      mixer_make xmm3
      
      paddusb xmm0, xmm6
      paddusb xmm1, xmm6    
      paddusb xmm2, xmm6
      paddusb xmm3, xmm6      

      movdqu   xmmword ptr [edi+000h], xmm0 ;; write P0
      movdqu  xmmword ptr [edi+010h], xmm1 ;; write P1
      movdqu  xmmword ptr [edi+020h], xmm2 ;; write P2
      movdqu  xmmword ptr [edi+030h], xmm3 ;; write P3  

      add  edi,  64
      sub  eax,  16 
      mov  esi,  esi  
      jne  _StdLoop 
      mov  eax,  esi 
      and  eax,  15
    @@:
      movd   xmm0, dword ptr [edi]     ;; fetch dword 
      mixer_make xmm0 
      paddusb xmm0, xmm6     
      movd   dword ptr [edi], xmm0
      add edi, 4
      dec eax
      jne @B 
      add  edi, ecx 
      mov  eax,  esi
      and   eax, -16 
      dec  edx 
      jne _StdLoop 
      jmp V_EXIT_SUCCESS
      
    align 16
    _betterLoop:
      movdqu   xmm0, xmmword ptr [edi+000h]     ;; fetch P0
      movdqu   xmm1, xmmword ptr [edi+010h]     ;; fetch P1
      movdqu  xmm2, xmmword ptr [edi+020h]     ;; fetch P2
      movdqu  xmm3, xmmword ptr [edi+030h]     ;; fetch P3
    
      mixer_make xmm0 
      mixer_make xmm1
      mixer_make xmm2
      mixer_make xmm3
      
      paddusb xmm0, xmm6
      paddusb xmm1, xmm6    
      paddusb xmm2, xmm6
      paddusb xmm3, xmm6      

      movdqu   xmmword ptr [edi+000h], xmm0 ;; write P0
      movdqu  xmmword ptr [edi+010h], xmm1 ;; write P1
      movdqu  xmmword ptr [edi+020h], xmm2 ;; write P2
      movdqu  xmmword ptr [edi+030h], xmm3 ;; write P3 
      
      add  edi,  64
      mov  esi,  esi  
      sub  eax,  16 
      jne _betterLoop 
       
      add  edi, ecx 
      mov  eax,  esi 
      dec  edx 
      jne _betterLoop
      
  V_EXIT_SUCCESS:
    pop  esi 
    pop  edi 
    xor  eax,  eax 
    ret   
  V_EXIT_FAILED:
    pop  esi 
    pop  edi 
    mov  eax,  -1 
    ret  

blend2 endp
 
    end
