;; 
;;
;; prototype 

;; void __cdecl gray_32K (  uint16_t *dsPtr, intptr_t dsPitch, 
;;                          intptr_t dsW, intptr_t dsH, 
;;                         int mem_order == 0 ? D0-D15  ->R5G5BX1  : D0-D15  ->B5G5RX1  ) 


        .686                      ; create 32 bit code
        .mmx
        .xmm                     
        .model flat, c      ; 32 bit memory model
        option casemap :none      ; case sensitive

comment @



Gray = R*0.299 + G*0.587 + B*0.114
;;   G- 19234.812500000000
Gray = (R*39190 + G*76939 + B*14943) >> 17
 
 
-0000 0000 -00RR RRR0 --0000 0000 --0GGG GG00 --0000 0000 -000B BBBB
;; Type 1
GP-0 :  --0000 0000 -000B BBBB - B         /1
GP-1 : --0000 00GG -GGG0 0000 - G         /32
GP-2 :  --0000 0000 -0RRR RR00 - R           /4
GP-3 : N/A 
;; Type2
GP-0 : --0000 0000 -000R RRRR - R             /1
GP-1 : --0000 00GG -GGG0 0000 - G               /32
GP-2 : --0000 0000 -0BBB BB00 - B                     /4
GP-3 : N/A 

0RRR RRGG GGGB BBBB 

@ 

.data
align 16
__lsbtomsb_bgr_ dw  4 dup (14943, 2405, 9798, 0)   ;; lsb -> msb b g r
align 16
__lsbtomsb_rgb_ dw  4 dup (39190, 2405, 3736, 0)   ;; lsb -> msb r g b      
align 16 
__pmaskli dw 4 dup (31, 03E0H, 07CH, 0) 
align 16 
__pshufb_32K_ db  16 dup (0, -1, 0, 1, 1, -1, -1, -1)   
      
.code 
align 16
gray_32K proc c 
 option prologue:none, epilogue:none
 
  ;;  stack arg base  offset typedef .
  dsPtr equ 4*1
  dsPitch equ 4*2
  dsW equ 4*3
  dsH equ 4*4
  mem_order equ 4*5
   
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
        
        mov    edi, stk_skip[esp+dsPtr]   ;; edi !! dstPtr 
        mov    ebx, stk_skip[esp+dsPitch]   
        mov    eax, stk_skip[esp+dsW]  ;; eax !! dstW
        mov    ebp, stk_skip[esp+dsH]  ;; ebp !! dstH
        
        cmp    eax, 0 
        jle    V_EXIT 
        cmp    ebp, 0 
        jle    V_EXIT        
        test   edi, edi 
        je     V_EXIT
        
        lea    edx, [eax*2] 
        sub    ebx, edx           ;;  ebx !! target pitch rva . 
        
        mov    edx, eax 
        xor    esi, esi 
        
        movq mm7, mmword ptr[__lsbtomsb_rgb_]
        
        cmp    dword ptr[esp+mem_order], 0 
        je     __skip  
        
        movq mm7, mmword ptr[__lsbtomsb_bgr_]
__skip: movq mm6, mmword ptr[__pmaskli] ;; mask 
        movq mm5, mmword ptr[__pshufb_32K_]
        
        ;;    eax <_  dstW        
        ;;    ebx <_  target pitch rva . 
        ;;    ecx <_  
        ;;    edx <_  dstW cache   
        ;;    esi <_  
        ;;    edi <_  target ptr 
        ;;    ebp <_  dstH
        ;;    esp <_   
  
align 16
doscanline:
        movzx ecx, word ptr[edi]
        movd mm0, ecx
        pshufb mm0, mm5      ;;    00
        pand mm0, mm6
        pmaddwd mm0, mm7 
        phaddd mm0, mm0 
        psrld mm0, 17 
        movd ecx, mm0 
        and ecx, 31
        mov esp, ecx 
        mov esi, ecx 
        shl esp, 5
        shl esi, 10
        or ecx, esp
        or ecx, esi   ;;  TODO: MORE Better Code. 
        mov word ptr[edi], cx
        add edi, 2
        dec eax 
        jne doscanline
        ;;   reset inline doscanline data/status, outloop check set 
        ;;   calc target pointer. 
        
        add edi, ebx 
        dec ebp 
        mov eax, edx 
        jne doscanline
V_EXIT: 
        movd    esp, xmm7
        emms 
        pop    ebp 
        pop    ebx 
        pop    esi 
        pop    edi    
        ret           
        
gray_32K endp 
end

