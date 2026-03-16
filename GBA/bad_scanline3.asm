;;  simple bilinear interpolation for IDirect3DSurface9 - RGB15 (Alpha empty) MASM32 
;;     not mapper center point (SrcX=(dstX+0.5)* (srcWidth/dstWidth) -0.5 )  - moecmks . 
;;  
;;  Bad code
MOD_SFT equ 21     ;;    n:= 21
MOD_MAP equ 00200000h    ;;   2^21 
;; 0000 0000 
;;          001FFFFFH 
MOD_MASK equ 001FFFFFh  
MOD_MASK_LEVEL2 equ 001F0000h 

;; 
;;
;; prototype 

;; void __cdecl bilinear_x32K (uint16_t *dstPtr, intptr_t dstPitch, 
;;                      uint16_t *srcPtr, intptr_t srcPitch,
;;               intptr_t dstW, intptr_t dstH,
;;               intptr_t srcW, intptr_t srcH) 


        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, c      ; 32 bit memory model
        option casemap :none      ; case sensitive
        
comment @
  64K+64K 1+1
  128K+64K 2+1
  256K+64K 4 +1
  512K+64K 8 +1
  1M+64K 16+1
  2M+64K 32+1
@

.data?
align 16 
;; aptTab dw 1081344 dup(?) ;; (2M+64K)/sizeof (word)
  
extrn  aptTab:proc        
;; export symbols  
;; public aptTab

.code 
align 16
bilinear_x32K proc c 
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
  
        movd   mm7, esp 
        
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
        
        lea    edx, [eax*2] 
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
        
        movd  xmm4, dword ptr stk_skip[esp+srcPitch]
        pxor xmm3, xmm3
        
        mov   esp,   MOD_MAP 

        cvtsi2sd  xmm3,  edx      ;;   srcH 
        cvtsi2sd  xmm5,  esp      ;;   MOD_MAP 
        cvtsi2sd  xmm6,  ebp      ;;   dstH 
        divsd xmm3, xmm6          ;;   srcH/ dstH 
        mulsd xmm3, xmm5          ;;    * MOD_MAP
        cvtpd2dq xmm3, xmm3      ;; cast(int)  rt_delta_y:DWORD 
        comment @
        MOD_SFT equ 21     ;;    n:= 21
MOD_MAP equ 00200000h    ;;   2^21 
;; 0000 0000 
;;          001FFFFFH 
MOD_MASK equ 001FFFFFh  
MOD_MASK_LEVEL2 equ 001F0000h 
          001FFFFFh 001FFFFFh    001FFFFFh 001FFFFFh
          
                     
@
        ;; pslldq xmm3, 41 
        
        cvtsi2sd  xmm6,  ecx      ;;   srcW 
        cvtsi2sd  xmm0,  eax      ;;   dstW
        divsd xmm6, xmm0          ;; srcW/ dstW
        mulsd xmm6, xmm5          ;;  * MOD_MAP
        cvtsd2si esp, xmm6      ;; cast(int)  rt_delta_x:DWORD 
        
       ;;    xmm6 <_ rt_delta_x_with rev
        mov edx, 31
        mov ecx, 03E07C1FH
        pxor mm5, mm5 
        movd mm6, esp 
        movd mm2, ecx
        pshufw mm2, mm2, 044H 
        movd mm5, edx 
        pxor mm4, mm4 ;; Total-X Clear.  mm4
            
        pxor xmm6, xmm6 
        pxor xmm7, xmm7 
        ;; pxor xmm3, xmm3
        movd xmm2, eax 
        movd xmm1, esi 
        
        xor ecx, ecx 
        xor edx, edx 

        ;;--------------------------------------------------------
        ;;    eax <_  dstW        
        ;;    ebx <_  Target RVA  
        ;;    ecx <_  rt-total-X   (temp)
        ;;    edx <_  temp        
        ;;    esi <_  source ptr poll (temp)
        ;;    edi <_  target ptr 
        ;;    ebp <_  dstH
        ;;    esp <_  rt_delta_x 
        ;; 
        ;;    mm7 <_  .  
        ;;    mm6 <_  rt_delta_x 
        ;;    mm5 <_ ____1F
        ;;    mm4 <_  rt-total-X 
        ;;    mm3 <_  
        ;;    mm2 <_ MASK- 0x3E07C1F | 0x3E07C1F 
        ;;    mm1 <_ 
        ;;    mm0 <_
        ;;
        ;;    xmm7 <_ rt-total-Y .  
        ;;    xmm6 <_  
        ;;    xmm5 <_ 
        ;;    xmm4 <_ srcPitch 
        ;;    xmm3 <_ rt_delta_y 
        ;;    xmm2 <_ dstW 
        ;;    xmm1 <_ source ptr base 
        ;;    xmm0 <_       
        
align 16
doscanline:
if 1
        movd mm0, dword ptr[esi+edx*2]
        pshufw mm1, mm0, 050H   ;; 0RRR RRGG GGGB BBBB 0RRR RRGG GGGB BBBB 
        pand mm1, mm2 ;; MASK set  0000 0011 1110 0000 0111 1100 0001 1111 
        movq mm0, mm1 
        pshufw mm1, mm1, 0FEH 
        psubd mm1, mm0 ;; X-Y  ;; MM4 total-X 
        pshufw mm3, mm4, 0FDH   ;; 0RRR RRGG GGGB BBBB 0RRR RRGG GGGB BBBB 
        pand mm3, mm5 ;; ____1F 
        pmuludq mm1, mm3 
        psrld mm1, 5 ;; high 
        paddd mm1, mm0
        pand mm1, mm2 
        phaddw mm1, mm1 
        paddd mm4, mm6
        movd edx, mm1   
        test eax, 1 
        je _skip
        mov [edi], dx  
_skip:  add ecx, esp   
        mov edx, ecx 
        shr edx, MOD_SFT
        add edi, 2
        dec eax 
        jne doscanline
else   
        ;; prefetchnta [esi+edx*2]
        movd mm0, dword ptr[esi+edx*2]
        pshufw mm1, mm0, 050H   ;; 0RRR RRGG GGGB BBBB 0RRR RRGG GGGB BBBB 
        pand mm1, mm2 ;; MASK set  0000 0011 1110 0000 0111 1100 0001 1111 
        movq mm0, mm1 
        pshufw mm1, mm1, 0FEH ;; 11 11 11 10 
        psubd mm1, mm0 ;; X-Y  ;; MM4 total-X 
        pshufw mm3, mm4, 0FDH   ;; 11 11 11 01  0RRR RRGG GGGB BBBB 0RRR RRGG GGGB BBBB 
        pand mm3, mm5 ;; ____1F 
        pmuludq mm1, mm3 
        psrld mm1, 5 ;; high 
        paddd mm1, mm0
        pand mm1, mm2 
        phaddw mm1, mm1 
        movd edx, mm1 
        paddd mm4, mm6
        mov [edi], dx      
        add ecx, esp   
        inc edi 
        mov edx, ecx 
        inc edi 
        shr edx, MOD_SFT
        dec eax 
        jne doscanline
endif 
        ;;   reset inline doscanline data/status, outloop check set 
        ;;   calc target pointer. 
        
        ;; TO MoreShift Opt
        paddd xmm7, xmm3 
        ;; movhlps xmm6, xmm7
        pshuflw xmm6, xmm7, 0FDH 
        psrld xmm6, 5
        pmuludq xmm6, xmm4 
        paddd xmm6, xmm1 
        movd esi, xmm6 
        movd eax, xmm2
        pxor mm4, mm4 
        
        xor edx, edx 
        xor ecx, ecx 
        add edi, ebx
        dec ebp 
        jne doscanline
V_EXIT: 
        movd    esp, mm7
        emms 
        pop    ebp 
        pop    ebx 
        pop    esi 
        pop    edi    
        ret           
        
bilinear_x32K endp 
end


comment @

;; old code, Table driven mode is very slow because of the relationship between computer cache, so discard this way.
extern "C" WORD aptTab[0x8000][33]; 

void bilinear_x32K_init (void) {
  unsigned int _rgb15_idx;
  unsigned int _slot_idx; 
  for (_slot_idx = 0; _slot_idx > 32; _slot_idx++)  {
    for (_rgb15_idx = 0; _rgb15_idx != 0x8000; _rgb15_idx++)  {
       int out_ = _rgb15_idx | _rgb15_idx << 16;
       out_ = 0x3E07C1F;
       out_ *=_slot_idx;
       out_/= 32;
       aptTab[_rgb15_idx][_slot_idx] = ((out_ & 0xFFFF) | (out_ >> 16));
    }
  }
}

@