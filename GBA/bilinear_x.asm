;;  simple bilinear interpolation for IDirect3DSurface9 - RGB32 (Alpha empty) MASM32 
;;     not mapper center point (SrcX=(dstX+0.5)* (srcWidth/dstWidth) -0.5 )  - moecmks . 
;;   TODO: more opt. In fact, the code is not very good.
;; settings .
;;  
MOD_SFT equ 16     ;;    n:= 16
MOD_MAP equ 00010000h    ;;   2^16 
MOD_MASK equ 0000FFFFh  

;; 
;;
;; prototype 

;; void __cdecl bilinear_x (uint32_t *dstPtr, intptr_t dstPitch, 
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
bilinear_x proc c 
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
        mov    ebp, stk_skip[esp+dstH]  ;; ebp !! dstH
        
        cmp    eax, 0 
        jle    V_EXIT 
        cmp    ebp, 0 
        jle    V_EXIT        
        test   edi, edi 
        je     V_EXIT
        
        lea    edx, [eax*4] 
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
        
        movss xmm4, dword ptr stk_skip[esp+srcPitch]
        mov   esp,   MOD_MAP 

        cvtsi2sd  xmm3,  edx      ;;   srcH 
        cvtsi2sd  xmm5,  esp      ;;   MOD_MAP 
        cvtsi2sd  xmm6,  ebp      ;;   dstH 
        divsd xmm3, xmm6          ;;   srcH/ dstH 
        mulsd xmm3, xmm5          ;;    * MOD_MAP
        cvttpd2dq xmm3, xmm3      ;; cast(int)  rt_delta_y:DWORD 
        
        cvtsi2sd  xmm6,  ecx      ;;   srcW 
        cvtsi2sd  xmm0,  eax      ;;   dstW
        divsd xmm6, xmm0          ;; srcW/ dstW
        mulsd xmm6, xmm5          ;;  * MOD_MAP
        cvttpd2dq xmm6, xmm6      ;; cast(int)  rt_delta_x:DWORD 
        
       ;;    xmm6 <_ rt_delta_x_with rev
        pxor xmm1, xmm1 
        pxor xmm7, xmm7 
        pcmpeqd xmm0, xmm0 
        movq xmm1, xmm0 
        movlhps xmm7, xmm0 
        pshuflw xmm2, xmm6, 0 
        pshufd xmm2, xmm2, 0 
        movd esp, xmm6 
        movdqa xmm6, xmm2
        movd xmm2, eax
        movd xmm1, esi 
        movd xmm0, ebx
        pxor xmm6, xmm7
        pshufd xmm6, xmm6, 00AH
        movdqa xmm7, xmm7 
        movlhps xmm0, xmm0
        pcmpeqq xmm5, xmm5 
        psrlw xmm5, 15
        movhlps xmm5, xmm1
        pshufd xmm5, xmm5, 00AH
        paddw xmm6, xmm5
        
        xor ebx, ebx 
        xor ecx, ecx 
        xor edx, edx 

        ;;    eax <_  dstW        
        ;;    ebx <_  rt-total-X  
        ;;    ecx <_  rt-total-Y 
        ;;    edx <_  spare        
        ;;    esi <_  source ptr poll
        ;;    edi <_  target ptr 
        ;;    ebp <_  dstH
        ;;    esp <_  rt_delta_x 
        ;; 
        ;;    xmm7 <_ rt_poll .  
        ;;    xmm6 <_ rt_delta_x_with rev 
        ;;    xmm5 <_ temp use
        ;;    xmm4 <_ srcPitch 
        ;;    xmm3 <_ rt_delta_y 
        ;;    xmm2 <_ dstW
        ;;    xmm1 <_ source ptr base 
        ;;    xmm0 <_ Target RVA  | LO-QWORD : temp use 
align 16
doscanline:
        pmovzxbw xmm5, [esi+edx*4] 
        pmulhuw xmm5, xmm7
        ;;  00 A1 00 R1 00 G1 00 B1 | 00 A0 00 R0 00 G0 00 B0       
        movhlps xmm0, xmm5 
        paddw xmm5, xmm0
        packuswb xmm5, xmm5 
        movd dword ptr[edi], xmm5 
        paddw xmm7, xmm6
        add ebx, esp   
        mov edx, ebx 
        shr edx, 16
        add edi, 4
        dec eax 
        jne doscanline
        ;;   reset inline doscanline data/status, outloop check set 
        movd eax, xmm3 
        movd ebx, xmm1 
        movd edx, xmm4
        
        pcmpeqd xmm7, xmm7
        pxor xmm5, xmm5 
        movhlps xmm7, xmm5 
        ;;
        ;;
        add ecx, eax 
        mov eax, ecx 
        shr eax, 16  
        imul eax, edx
        lea esi, [ebx+eax] 
        ;;
        ;;
        movhlps xmm5, xmm0
        movd ebx, xmm5 
        movd eax, xmm2
        
        add edi, ebx 
        xor ebx, ebx 
        xor edx, edx 
        
        dec ebp 
        jne doscanline
V_EXIT: 
        movd    esp, mm0
        emms 
        pop    ebp 
        pop    ebx 
        pop    esi 
        pop    edi    
        ret           
        
bilinear_x endp 
end