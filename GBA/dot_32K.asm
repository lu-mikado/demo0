
MOD_SFT equ 21     ;;    n:= 21
MOD_MAP equ 00200000h    ;;   2^21 
;;          001FFFFFH 
MOD_MASK equ 001FFFFFh  
MOD_MASK_LEVEL2 equ 001F0000h 

;; 
;;
;; prototype 

;; void __cdecl dot_32K (uint16_t *dstPtr, intptr_t dstPitch, 
;;                      uint16_t *srcPtr, intptr_t srcPitch,
;;               intptr_t dstW, intptr_t dstH,
;;               intptr_t srcW, intptr_t srcH) 


        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, c      ; 32 bit memory model
        option casemap :none      ; case sensitive
        
.code 
align 16
dot_32K proc c 
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
  
        movd   xmm7, esp 
        
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
        
        lea    ebx, [ebx*2]

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
        movd  mm5, edi 
        mov   esp,   MOD_MAP 

        cvtsi2sd  xmm3,  edx      ;;   srcH 
        cvtsi2sd  xmm5,  esp      ;;   MOD_MAP 
        cvtsi2sd  xmm6,  ebp      ;;   dstH 
        divsd xmm3, xmm6          ;;   srcH/ dstH 
        mulsd xmm3, xmm5          ;;    * MOD_MAP
        cvtpd2pi mm3, xmm3      ;; cast(int)  rt_delta_y:DWORD 
        pslld mm3, 1

        cvtsi2sd  xmm6,  ecx      ;;   srcW 
        cvtsi2sd  xmm0,  eax      ;;   dstW
        divsd xmm6, xmm0          ;; srcW/ dstW
        mulsd xmm6, xmm5          ;;  * MOD_MAP
        cvtsd2si esp, xmm6      ;; cast(int)  rt_delta_x:DWORD 
        
       ;;    xmm6 <_ rt_delta_x_with rev
        pxor mm7, mm7 
        movd mm2, eax
        movd mm1, esi 
        movd mm0, ebx
        mov ecx, ebx 
        mov ebx, 00000000h  
        xor edx, edx 
        xor ebx, ebx 
        ;;    eax <_  dstW        
        ;;    ebx <_  rt-total-X  
        ;;    ecx <_  target RVA  
        ;;    edx <_  temp        
        ;;    esi <_  source ptr poll
        ;;    edi <_  target ptr 
        ;;    ebp <_  dstH
        ;;    esp <_  rt_delta_x 
        ;; 
        ;;    mm7 <_ rt-total-Y .  
        ;;    mm6 <_ rt_delta_x_with rev 
        ;;    mm5 <_ 
        ;;    mm4 <_ srcPitch 
        ;;    mm3 <_ rt_delta_y 
        ;;    mm2 <_ dstW 
        ;;    mm1 <_ source ptr base 
        ;;    mm0 <_ temp 
align 16
doscanline:
        mov dx, [esi+edx*2]
        mov [edi], dx 
        ;; lea ebx, [ebx+esp*2]
        add ebx, esp 
        add ebx, esp
        ;; add ebx, esp                    ;; MORE Bearr code
        mov edx, ebx 
        shr edx, MOD_SFT
        add edi, 4
        sub eax, 2
        jg doscanline
        ;;   reset inline doscanline data/status, outloop check set 
        ;;   calc target pointer. 
        
        paddd mm7, mm3 
        pshufw mm0, mm7, 0FDH 
        psrld mm0, 5 
        pmuludq mm0, mm4
        paddd mm0, mm1 
        movd esi, mm0 
        movd eax, mm2 
        movd edi, mm5 
        add edi, ecx
        
        movd mm5, edi 
        xor edx, edx
        xor ebx, ebx 
        sub ebp, 2
        jg doscanline
V_EXIT: 
        movd    esp, xmm7
        emms 
        pop    ebp 
        pop    ebx 
        pop    esi 
        pop    edi    
        ret           
        
dot_32K endp 
end

