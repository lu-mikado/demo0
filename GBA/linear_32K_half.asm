MOD_SFT equ 21     ;;    n:= 21
MOD_MAP equ 00200000h    ;;   2^21 
;; 
;;
;; prototype 

;; void __cdecl linear_x32K_half (uint16_t *dstPtr, intptr_t dstPitch, 
;;                      uint16_t *srcPtr, intptr_t srcPitch,
;;               intptr_t dstW, intptr_t dstH,
;;               intptr_t srcW, intptr_t srcH) 


        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, c      ; 32 bit memory model
        option casemap :none      ; case sensitive
 .data?
nPhase dd ? 
       
.code 
align 16
linear_x32K_half proc c 
 option prologue:none, epilogue:none
 
  ;;  stack arg base  offset typedef .
  dstPtr equ 4*1
  dstPitch equ 4*2
  srcPtr equ 4*3
  srcPitch equ 4*4
  dstW equ 4*5
  dstH equ 4*6
  srcW equ 4*7
  srcH equ 4*8
   
        ;;   first, save ctx  
        push   edi 
        push   esi 
        push   ebx 
        push   ebp ;; stack += 16 
        
  stk_skip equ 16 
  
        movd   xmm2, esp 
        
        ;;   calc vptr and rva pitch 
        ;;   for target pitch, we can get rva 
        ;;   source pitch can't  
        
        mov    edi, stk_skip[esp+dstPtr]   ;; edi !! dstPtr 
        mov    ebx, stk_skip[esp+dstPitch]   
        mov    eax, stk_skip[esp+dstW]  ;; eax !! dstW
        mov    ebp, stk_skip[esp+dstH]  ;; ebp !! dstH
        and    eax, -2 ;; @FIXME: For convenience, I may have lost one pixel of processing, 
                                ;;  but the problem should be minor.
        
        cmp    eax, 0 
        jle    V_EXIT 
        cmp    ebp, 0 
        jle    V_EXIT        
        test   edi, edi 
        je     V_EXIT
        
        lea    edx, [eax*2] 
        movd    mm0, ebx
        sub    ebx, edx           ;;  ebx !! target pitch rva . 
        
        mov    edx, stk_skip[esp+srcH]  ;;  edx !!srcH
        mov    ecx, stk_skip[esp+srcW]  ;;  ecx !!srcW
        mov    esi, stk_skip[esp+srcPtr] ;; esi !!srcPtr

        cmp    edx, 0 
        jle    V_EXIT 
        cmp    ecx, 0 
        jle    V_EXIT        
        test   esi, esi 
        je     V_EXIT
   
        ;;    eax <_  dstW        
        ;;    ebx <_  target pitch rva . 
        ;;    ecx <_  srcW
        ;;    edx <_  srcH       
        ;;    esi <_  source ptr poll
        ;;    edi <_  target ptr 
        ;;    ebp <_  dstH
        ;;    esp <_  temp 
        
        movd  mm4, dword ptr stk_skip[esp+srcPitch]  
        mov   esp,   MOD_MAP

        cvtsi2sd  xmm3,  edx      ;;   srcH 
        cvtsi2sd  xmm5,  esp      ;;   MOD_MAP 
        cvtsi2sd  xmm6,  ebp      ;;   dstH 
        divsd xmm3, xmm6          ;;   srcH/ dstH 
        mulsd xmm3, xmm5          ;;    * MOD_MAP
        cvtpd2pi mm3, xmm3      ;; cast(int)  rt_delta_y:DWORD 
        
        cvtsi2sd  xmm6,  ecx      ;;   srcW  -------------
        cvtsi2sd  xmm0,  eax      ;;   dstW
        divsd xmm6, xmm0          ;; srcW/ dstW
        mulsd xmm6, xmm5          ;;  * MOD_MAP
        cvtsd2si ecx, xmm6      ;; cast(int)  rt_delta_x:DWORD 
        
        pxor mm7, mm7  
        movd mm2, eax
        movd mm1, esi 
        movd mm5, edi

        ;;    eax <_  dstW        
        ;;    ebx <_  rt-total-X  
        ;;    ecx <_  rt_delta_x 
        ;;    edx <_  temp        
        ;;    esi <_  source ptr poll
        ;;    edi <_  target ptr 
        ;;    ebp <_  height count 
        ;;    esp <_  temp 
        ;; 
        ;;    mm7 <_ rt-total-Y .  
        ;;    mm6 <_ rt_delta_x_with rev 
        ;;    mm5 <_ free
        ;;    mm4 <_ srcPitch 
        ;;    mm3 <_ rt_delta_y 
        ;;    mm2 <_ dstW 
        ;;    mm1 <_ source ptr base 
        ;;    mm0 <_ Target RVA 
        
        mov esp, 03E07C1FH
        movd xmm6, esp 
        pshufd xmm6, xmm6, 0 
        
        mov esp, 0000001FH
        movd xmm7, esp 
        pshufd xmm7, xmm7, 0  
        
        lea esp, [ecx*2] 
        movd xmm4, esp 
        movlhps xmm4, xmm4
        
        pxor xmm5, xmm5 
        movd xmm0, ecx 
        movlhps xmm5, xmm0
        
        movdqa xmm3, xmm5 
        psrld xmm3, 16 
        pand xmm3, xmm7
        
        mov esp, ecx
        shr esp, MOD_SFT
        xor edx, edx 

        and nPhase, 1 
        xor nPhase, 1 
        jne doscanline2


align 16
doscanline:     
        ;;    xmm7: 0000001FH | 0000001FH | 0000001FH | 0000001FH 
        ;;    xmm6: 03E07C1FH | 03E07C1FH | 03E07C1FH | 03E07C1FH 
        ;;    xmm5: base - init- 0, 1
        ;;    xmm4: poll - poll double ryt.
        ;;    xmm3: temp write. 
        movss xmm0, dword ptr[esi+edx*2] ;; current one.
        movss xmm1, dword ptr[esi+esp] ;; next it 
        
        punpcklwd xmm0, xmm0 
        punpcklwd xmm1, xmm1     
        
        ;; xmm0 <- a1 a1 a0 a0 
        ;; xmm1 <- b1 b1 b0 b0 
        
        movlhps xmm0, xmm1 ;; pad it.
        paddd xmm5, xmm4
        
        pand xmm0, xmm6 ;; pure mask+ rgb pes.. 
        
        ;; xmm0 <- b1 b1 b0 b0  | a1 a1 a0 a0 | 
        pshufd xmm1, xmm0,        010110001b                ;;  2   3    0   1
        ;; xmm1 <- b0 b0 b1 b1  | a0 a0 a1 a1 | 
        
        phsubd xmm1, xmm1 
        punpckldq xmm1, xmm1
 
        pmuludq xmm1, xmm3
        movdqa xmm3, xmm5
        
        psrld xmm1, 5
        
        paddd xmm0, xmm1
        psrld xmm3, 16
        
        pand xmm0, xmm6 ;; pure mask+ rgb pes.. 
        phaddw xmm0, xmm0 
        
        pand xmm3, xmm7
        pshuflw xmm0, xmm0,  8 
     IF 0
        movss dword ptr[edi], xmm0 
     ELSE 
        movd edx, xmm0 
        movnti [edi], edx 
     ENDIF 
        
        ;; make 
        lea edx, [ebx+ecx*2]  
        lea esp, [edx+ecx]
        
        mov ebx, edx 
        shr edx, MOD_SFT 
        
        shr esp, MOD_SFT 
        add edi, 4 
        shl esp, 1 
        
        sub eax, 2
        jne doscanline

        ;;    reset inline doscanline data/status, outloop check set 
        ;;    calc target pointer. 
        ;;
        ;;    mm7 <_ rt-total-Y .  
        ;;    mm6 <_ free
        ;;    mm5 <_ edi++
        ;;    mm4 <_ srcPitch 
        ;;    mm3 <_ rt_delta_y 
        ;;    mm2 <_ dstW 
        ;;    mm1 <_ source ptr base 
        ;;    mm0 <_ Target Pitch 
    loopout_stuff:
        paddd mm7, mm3 
        paddd mm5, mm0 
        movq mm6, mm7 
        psrld mm6, MOD_SFT
        pmuludq mm6, mm4
        paddd mm6, mm1 
        
        movd esi, mm6 
        movd edi, mm5 
        movd eax, mm2 ;; Te,pWidth
        
        pxor xmm5, xmm5 
        movd xmm0, ecx 
        movlhps xmm5, xmm0
        
        movdqa xmm3, xmm5 
        psrld xmm3, 16 
        pand xmm3, xmm7

        xor edx, edx  
        xor ebx, ebx 
        mov esp, ecx 
        shr esp, MOD_SFT
        shl esp, 1
        dec ebp 
        je V_EXIT
        test ebp, 1
        jne doscanline
        jmp loopout_stuff
        
V_EXIT: 
        movd    esp, xmm2
        emms 
        pop    ebp 
        pop    ebx 
        pop    esi 
        pop    edi    
        ret     
align 16
doscanline2:     
        ;;    xmm7: 0000001FH | 0000001FH | 0000001FH | 0000001FH 
        ;;    xmm6: 03E07C1FH | 03E07C1FH | 03E07C1FH | 03E07C1FH 
        ;;    xmm5: base - init- 0, 1
        ;;    xmm4: poll - poll double ryt.
        ;;    xmm3: temp write. 
        movss xmm0, dword ptr[esi+edx*2] ;; current one.
        movss xmm1, dword ptr[esi+esp] ;; next it 
        
        punpcklwd xmm0, xmm0 
        punpcklwd xmm1, xmm1     
        
        ;; xmm0 <- a1 a1 a0 a0 
        ;; xmm1 <- b1 b1 b0 b0 
        
        movlhps xmm0, xmm1 ;; pad it.
        paddd xmm5, xmm4
        
        pand xmm0, xmm6 ;; pure mask+ rgb pes.. 
        
        ;; xmm0 <- b1 b1 b0 b0  | a1 a1 a0 a0 | 
        pshufd xmm1, xmm0,        010110001b                ;;  2   3    0   1
        ;; xmm1 <- b0 b0 b1 b1  | a0 a0 a1 a1 | 
        
        phsubd xmm1, xmm1 
        punpckldq xmm1, xmm1
 
        pmuludq xmm1, xmm3
        movdqa xmm3, xmm5
        
        psrld xmm1, 5
        
        paddd xmm0, xmm1
        psrld xmm3, 16
        
        pand xmm0, xmm6 ;; pure mask+ rgb pes.. 
        phaddw xmm0, xmm0 
        
        pand xmm3, xmm7
        pshuflw xmm0, xmm0,  8 
     IF 0
        movss dword ptr[edi], xmm0 
     ELSE 
        movd edx, xmm0 
        movnti [edi], edx 
     ENDIF 
        
        ;; make 
        lea edx, [ebx+ecx*2]  
        lea esp, [edx+ecx]
        
        mov ebx, edx 
        shr edx, MOD_SFT 
        
        shr esp, MOD_SFT 
        add edi, 4 
        shl esp, 1 
        
        sub eax, 2
        jne doscanline2

        ;;    reset inline doscanline data/status, outloop check set 
        ;;    calc target pointer. 
        ;;
        ;;    mm7 <_ rt-total-Y .  
        ;;    mm6 <_ free
        ;;    mm5 <_ edi++
        ;;    mm4 <_ srcPitch 
        ;;    mm3 <_ rt_delta_y 
        ;;    mm2 <_ dstW 
        ;;    mm1 <_ source ptr base 
        ;;    mm0 <_ Target Pitch 
    loopout_stuff2:
        paddd mm7, mm3 
        paddd mm5, mm0 
        movq mm6, mm7 
        psrld mm6, MOD_SFT
        pmuludq mm6, mm4
        paddd mm6, mm1 
        
        movd esi, mm6 
        movd edi, mm5 
        movd eax, mm2 ;; Te,pWidth
        
        pxor xmm5, xmm5 
        movd xmm0, ecx 
        movlhps xmm5, xmm0
        
        movdqa xmm3, xmm5 
        psrld xmm3, 16 
        pand xmm3, xmm7

        xor edx, edx  
        xor ebx, ebx 
        mov esp, ecx 
        shr esp, MOD_SFT
        shl esp, 1
        dec ebp 
        je V_EXIT
        test ebp, 1
        je doscanline2
        jmp loopout_stuff2
        
linear_x32K_half endp 
end