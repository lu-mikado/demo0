;;  simple bilinear interpolation for IDirect3DSurface9 - RGB32 (Alpha empty) MASM32 
;;     not mapper center point (SrcX=(dstX+0.5)* (srcWidth/dstWidth) -0.5 )  - moecmks . 
;;       FIXME: source code not debug in scale ratio. !!!
;;
;; settings .
;;  
MOD_SFT equ 12     ;;    n:= 12
MOD_MAP equ 4096    ;;   2^n := 4096
MOD_SFT_MUL equ MOD_MAP*MOD_MAP  ;; 4096 * 4096 := 16777216 := 0x0100 0000 suitfor pmulhuw
MOD_MASK equ 4095

;; 
;;
;; prototype 

;; void __cdecl bilinear (uint32_t *dstPtr, intptr_t dstPitch, 
;;                      uint32_t *srcPtr, intptr_t srcPitch,
;;               intptr_t dstW, intptr_t dstH,
;;               intptr_t srcW, intptr_t srcH) 


        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, c      ; 32 bit memory model
        option casemap :none      ; case sensitive

.code 
align 16
bilinear proc c 
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
  
        movd   mm0, esp 
        
        ;;   calc vptr and rva pitch 
        ;;   for target pitch, we can get rva 
        ;;   source pitch can't  
        
        mov    edi, stk_skip[esp+dstPtr]   ;; edi !! dstPtr 
        mov    ebx, stk_skip[esp+dstPitch]   
        mov    eax, stk_skip[esp+dstW]  ;; eax !! dstW
        mov    ecx, stk_skip[esp+dstH]  ;; ecx !! dstH
        
        cmp    eax, 0 
        jle    V_EXIT 
        cmp    ecx, 0 
        jle    V_EXIT        
        test   edi, edi 
        je     V_EXIT
        
        lea    edx, [eax*4] 
        sub    ebx, edx           ;;  ebx !! target pitch rva . 
        
        mov    edx, stk_skip[esp+srcH]  ;;  edx !!srcH
        mov    ebp, stk_skip[esp+srcW]  ;;  ebp !!srcW
        mov    esi, stk_skip[esp+srcPtr] ;; esi !!srcPtr

        cmp    edx, 0 
        jle    V_EXIT 
        cmp    ebp, 0 
        jle    V_EXIT        
        test   esi, esi 
        je     V_EXIT
   
        movd mm2, ebx 
        
        
        mov ebx, MOD_MAP
          
        cvtsi2ss  xmm7,  edx      ;;   srcH 
        cvtsi2ss  xmm5,  ebx      ;;   MOD_MAP 
        cvtsi2ss  xmm6,  ecx      ;;   dstH 
        divss xmm7, xmm6   ;; xmm7 <_ RT-H 
        mulss xmm7, xmm5   ;;  xmm7 !! rt_y unit 
        cvttps2dq xmm7, xmm7 ;; cast(int) 
        cvtsi2ss  xmm6,  ebp      ;;   srcW 
        cvtsi2ss  xmm0,  eax      ;;   dstW
        divss xmm6, xmm0   ;; xmm6 <_ RT-W
        mulss xmm6, xmm5   ;;  xmm6 <_ MOD_MAP * ( srcH/ dstH) 
        cvttps2dq xmm6, xmm6 ;; xmm6 !! rt_x unit 
        
        pshuflw xmm7, xmm7, 0 
        pshuflw xmm6, xmm6, 0  
        pshufd xmm7, xmm7, 0 
        pshufd xmm6, xmm6, 0      
        pxor xmm4, xmm4 
        
        xor ebx, ebx 
        mov ebp, stk_skip[esp+srcPitch]
        movd  esp,     xmm6   
        and esp, 0FFFFh 
        movd mm3, ebp 

        movdq2q mm7,   xmm7
        psrlq mm7, 48
        movd mm6, esi 
        pxor mm5, mm5 
        
        mov edx, MOD_MASK
        movd xmm3, edx 
        
        mov edx, esi
        pshuflw xmm3, xmm3, 0 
        movddup xmm7, xmm3 
        movdqa xmm5, xmm7 
        movd mm1, eax
        ;;    eax <_  dstW 
        ;;    ebx <_  cache-total-X   (only read, throw into mm cache)
        ;;    ecx <_  dstH 
        ;;    edx <_  
        ;;    ebp <_  srcPit 
        ;;    esp <_  rt_x unit 
        ;;    edi <_           
        ;;    esi <_  
        
        ;;    mm7 <_  h_base rt unit 
        ;;    mm6 <_  esi base 
        ;;    mm5 <_  y-cahce total 
        ;;    mm4 <_  cahce 
        ;;    mm3 <_  cache CsrcPitch 
        ;;    mm2 <_  RVA  
        ;;    mm1 <_  temp width
        ;;    mm0 <_  esp cache 
         
align 16
doscanline:
        ;;   edx <_ cahce source ptr. 
        

        ;;   fetch pixel 
        
        ;;   -----------------------------------
        ;;   top-left     |  top-right
        ;;                |
        ;;   bottom-left  |  bottom- right 
        ;;   -----------------------------------
        
        ;;    xmm3 <_  mod map mask 2^n-1 in lo ddword 
        ;;    xmm4 <_  x rt  
        ;;    xmm5 <_  y rt   bottom 
        ;;    xmm6 <_  w_base rt unit 
        ;;    xmm7 <_  mod map mask 2^n-1 in all ddword 
        
        movdqa  xmm0, xmm4 ;; xmm0<_ x rt cache 
        pxor xmm0, xmm3 ;; xmm0<_ apRight | apLeft   LO->HI (temp!save )
        movdqa xmm1, xmm0 ;; cache 
        pmulhuw xmm1, xmm5 ;; LO->HI apRight* apBottom | apLeft * apBottom
        pmovzxbw xmm2, [edx]  ;;  fetch cross- top pixel 
        psllw xmm2, 8
        pmulhuw xmm2, xmm1  ;; xmm2 <_ pixel 
        ;; xmm0, xmm1 free .. 
        movdqa xmm1, xmm5 
        pxor xmm1, xmm7 ;; all is top  
        pmulhuw xmm1, xmm0  ;; xmm0 is free 
        pxor xmm0, xmm0 
        pmovzxbw xmm0, [edx+ebp]  ;;  fetch cross- top pixel 
        psllw xmm0, 8
        pmulhuw xmm0, xmm1  ;; xmm0 <_ pixel 
        paddusb xmm0, xmm2  
        packuswb xmm0, xmm0 
        pshufd xmm1, xmm0, 1 ;; shift 
        paddusb xmm0, xmm1 
        movd dword ptr[edi], xmm0 
        
        ;; add x - rt .
        paddw xmm4, xmm6 
        pand xmm4, xmm7
              
        add ebx, esp
        mov edx, ebx 
        shr edx, MOD_SFT 
        lea edx, [esi+edx*4]     
        add edi, 4 
        dec eax 
        jne doscanline
        
        ;; add y - rt .
        pxor xmm5, xmm7
        movq2dq xmm0, mm7 
        movlhps xmm0, xmm0 
        paddw xmm5, xmm0 
        pshuflw xmm5, xmm5, 0 
        pshufd xmm5, xmm5, 0 
        pand xmm5, xmm7
        pxor xmm4, xmm4 
        pxor xmm5, xmm7
        movd eax, mm1

        ;; acquire next esi cache 
        paddd  mm5, mm7 
        movq mm4, mm5 
        psrld mm4, MOD_SFT
        pmuludq mm4, mm3 
        paddd mm4, mm6 
        movd esi, mm4 
        movd edx, mm2 
        
        add edi, edx 
        mov edx, esi 
        xor ebx, ebx 
        dec ecx 
        jne doscanline  
V_EXIT: 
        movd    esp, mm0
        emms  
    
        pop    ebp 
        pop    ebx 
        pop    esi 
        pop    edi    
        ret           
        
bilinear endp 
end