;; context_x86_32.asm
;;
;; Copyripht 2019 moecmks (agalis01@outlook.com)
;; This file is part of ArchBoyAdvance.
;; 
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;; 
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;; 
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software Foundation,
;; Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

.xmm    
.model flat, c      
option casemap :none
 
;; PUBLIC:
;; ------------------------------------------------------------------
;; int callc context_init (context_t *ctx);
;; void callc context_get (context_t ctx);
;; void callc context_set (context_t ctx);
;; void callc context_swap (context_t ctx);
;; void callc context_swap2 (context_t to, context_t recv);
;; void callc context_copy (context_t to, context_t from);
;; void callc context_setpc (context_t ctx, void *program_pointer);
;; void callc context_setstack (context_t ctx, uintptr_t stack);
;; void *callc context_getpc (context_t ctx);
;; uintptr_t callc context_getstack (context_t ctx);
;; void callc context_uninit (context_t *ctx);

.code

malloc PROTO C :DWORD
free PROTO C :DWORD

_fpu387 struct

  _387cw word ?
  _387sw word ?
  _387tw word ? 
  _387op word ?
  _387ip dword ? 
  _387cs word ?
  reserved_14_15 word ?
  _387dp dword ?
  _387ds word ?
  reserved_6_7_onbase16byte word ?
  _sse_csr dword ? 
  _sse_csr_mask dword ? 
  _mmx387_reg dd 32 dup (?)
  _sse_reg dd 32 dup (?)
  reserved_288 dd 56 dup (?)

_fpu387 ends

_cpu386 struct

   e_ip dword ?
   e_sp dword ?
   e_bp dword ?
   e_si dword ?
   
   e_di dword ?
   e_bx dword ?
   e_ps dword ?
   e_ax dword ? 
   
   e_cx dword ? 
   e_dx dword ?
   e_align dd 6 dup (?)
_cpu386 ends
  
context struct 
  r386 _cpu386 <> 
  r387 _fpu387 <>
  r386_oc _cpu386 <> 
  r387_oc _fpu387 <>
  src_pointer dword ?
context ends

;;------------------------------------------------------------------
;; int callc context_init (context_t *ctx);     
;;------------------------------------------------------------------
context_init proc C 
             option prologue:none, epilogue:none
             
  mov ecx, [esp+4] 
  mov eax, -1
  test ecx, ecx 
  je @F 
  invoke malloc, ((sizeof context) + 64)
assume eax:ptr context
  mov ecx, eax 
  add eax, 16 
  and eax, -16 
  mov [eax].src_pointer, ecx 
  mov ecx, [esp+4]
  mov [ecx], eax 
  xor eax, eax 
@@:
  ret 
context_init endp

;;------------------------------------------------------------------
;; void callc context_get (context_t ctx);       
;;------------------------------------------------------------------
context_get proc C 
             option prologue:none, epilogue:none
  push eax
  push edx 
  push ecx
  push  eax ; eax save.
  mov   eax, [esp+16+4] ;; get context pointer.
assume eax:ptr context
  mov [eax].r386.e_cx, ecx 
  mov [eax].r386.e_dx, edx
  pop ecx 
  mov [eax].r386.e_ax, ecx
  mov   ecx, [esp+12] ; get eip
  lea edx, [esp+16]
  mov [eax].r386.e_ip, ecx 
  mov [eax].r386.e_bx, ebx 
  mov [eax].r386.e_bp, ebp 
  mov [eax].r386.e_sp, edx
  mov [eax].r386.e_di, edi 
  mov [eax].r386.e_si, esi
  ;; Get FPU 
  fxsave [eax].r387
  ;; Get PSW.
  pushfd 
  mov ecx, [esp]
  popfd 
  mov [eax].r386.e_ps, ecx
  pop ecx 
  pop edx 
  pop eax 
  ret 
  
context_get endp

;;------------------------------------------------------------------
;; void callc context_set (context_t ctx);       
;;------------------------------------------------------------------
context_set proc C 
             option prologue:none, epilogue:none
             
  mov   eax, [esp+4] ;; get context pointer.
assume eax:ptr context
  ;; Resume FPU.
  fxrstor [eax].r387
  ;; Resume PSW.
  push [eax].r386.e_ps 
  popfd 
  ;; Resume X86.
  mov ebx, [eax].r386.e_bx 
  mov ebp, [eax].r386.e_bp 
  mov edi, [eax].r386.e_di 
  mov esi, [eax].r386.e_si 
  mov esp, [eax].r386.e_sp
  mov ecx, [eax].r386.e_cx
  mov edx, [eax].r386.e_dx
  push [eax].r386.e_ip
  mov eax, [eax].r386.e_ax 
  ret 
context_set endp

;;------------------------------------------------------------------
;; void callc context_uninit (context_t *ctx);       
;;------------------------------------------------------------------
context_uninit proc C 
             option prologue:none, epilogue:none
             
  mov ecx, [esp+4] 
  test ecx, ecx 
  je @F 
  xor eax, eax
  mov edx, [ecx]
assume edx:ptr context
  mov edx, [edx].src_pointer
  mov [ecx], eax
  invoke free, edx 
@@:
  ret  
context_uninit endp

;;------------------------------------------------------------------
;int callc context_swap (context_t ctx);       
;------------------------------------------------------------------
context_swap proc C 
             option prologue:none, epilogue:none
             
  push eax ;;1
  push ecx ;;2
  push edx ;;3
  push esi ;;4
  push edi ;;5
  push ebp ;;6
  push ebx ;;7-------- old flags. 
  
  pushfd 
  mov ebx, [esp] 
  popfd 

  mov   esi, [esp+4*7+4] ;; get context pointer.
  lea   edi, [esi + context.r386_oc]
  mov ecx, (sizeof _fpu387 + sizeof _cpu386)
  
  ;; -------- copy context to temp.--------
align 16
@@:
  test ecx, 15
  je @F
  mov dl, [esi]
  mov [edi], dl 
  dec ecx 
  jne @B 
  jmp continue
align 16
@@:
  mov edx, [esi]
  mov ebp, [esi+4]
  movnti [edi], edx 
  movnti [edi+4], ebp 
  mov edx, [esi+8]
  mov ebp, [esi+12]
  movnti [edi+8], edx 
  movnti [edi+12], ebp 
  add edi, 16 
  add esi, 16 
  sub ecx, 16
  jne @B 
continue:
  ;; Resume old PSW adn common registers ...
  push ebx 
  popfd
  
  pop ebx ;;7-------- old flags. 
  pop ebp ;;6
  pop edi ;;5
  pop esi ;;4
  pop edx ;;3
  pop ecx ;;2
  pop eax ;;1
  
  ;; -------- current context to context.--------
  push  eax ; eax save.
  mov   eax, [esp+8] ;; get context pointer.
assume eax:ptr context
  mov [eax].r386.e_cx, ecx 
  mov [eax].r386.e_dx, edx
  pop ecx 
  mov [eax].r386.e_ax, ecx
  mov   ecx, [esp] ; get eip
  lea edx, [esp+4]
  mov [eax].r386.e_ip, ecx 
  mov [eax].r386.e_bx, ebx 
  mov [eax].r386.e_bp, ebp 
  mov [eax].r386.e_sp, edx
  mov [eax].r386.e_di, edi 
  mov [eax].r386.e_si, esi
  ;; Get FPU 
  fxsave [eax].r387
  ;; Get PSW.
  pushfd 
  mov ecx, [esp]
  popfd 
  mov [eax].r386.e_ps, ecx
  
  ;; -------- context to current.--------
  lea eax, [eax + (sizeof _fpu387 + sizeof _cpu386)]
  ;; Resume FPU.
  fxrstor [eax].r387
  ;; Resume PSW.
  push [eax].r386.e_ps 
  popfd 
  ;; Resume X86.
  mov ebx, [eax].r386.e_bx 
  mov ebp, [eax].r386.e_bp 
  mov edx, [eax].r386.e_dx
  mov edi, [eax].r386.e_di 
  mov esi, [eax].r386.e_si 
  mov esp, [eax].r386.e_sp
  mov ecx, [eax].r386.e_cx
  push [eax].r386.e_ip
  mov eax, [eax].r386.e_ax 
  
  ret   
context_swap endp

;;------------------------------------------------------------------
;; void callc context_swap2 (context_t to, context_t recv);
;------------------------------------------------------------------
context_swap2 proc C 
             option prologue:none, epilogue:none
             
  push eax ;;1
  push ecx ;;2
  push edx ;;3
  push esi ;;4
  push edi ;;5
  push ebp ;;6
  push ebx ;;7-------- old flags. 
  
  pushfd 
  mov ebx, [esp] 
  popfd 

  mov   esi, [esp+4*7+4] ;; get context pointer.
  lea   edi, [esi + context.r386_oc]
  mov ecx, (sizeof _fpu387 + sizeof _cpu386)
  
  ;; -------- copy context to temp.--------
align 16
@@:
  test ecx, 15
  je @F
  mov dl, [esi]
  mov [edi], dl 
  dec ecx 
  jne @B 
  jmp continue
align 16
@@:
  mov edx, [esi]
  mov ebp, [esi+4]
  movnti [edi], edx 
  movnti [edi+4], ebp 
  mov edx, [esi+8]
  mov ebp, [esi+12]
  movnti [edi+8], edx 
  movnti [edi+12], ebp 
  add edi, 16 
  add esi, 16 
  sub ecx, 16
  jne @B 
continue:
  ;; Resume old PSW adn common registers ...
  push ebx 
  popfd
  
  pop ebx ;;7-------- old flags. 
  pop ebp ;;6
  pop edi ;;5
  pop esi ;;4
  pop edx ;;3
  pop ecx ;;2
  pop eax ;;1
  
  ;; -------- current context to context.--------
  push  eax ; eax save.
  mov   eax, [esp+8+4] ;; get context recv pointer.
assume eax:ptr context
  mov [eax].r386.e_cx, ecx 
  mov [eax].r386.e_dx, edx
  pop ecx 
  mov [eax].r386.e_ax, ecx
  mov   ecx, [esp] ; get eip
  lea edx, [esp+4]
  mov [eax].r386.e_ip, ecx 
  mov [eax].r386.e_bx, ebx 
  mov [eax].r386.e_bp, ebp 
  mov [eax].r386.e_sp, edx
  mov [eax].r386.e_di, edi 
  mov [eax].r386.e_si, esi
  ;; Get FPU 
  fxsave [eax].r387
  ;; Get PSW.
  pushfd 
  mov ecx, [esp]
  popfd 
  mov [eax].r386.e_ps, ecx
  
  ;; -------- context to current.--------
  mov   eax, [esp+4] ;; get context set pointer. 
  lea eax, [eax + (sizeof _fpu387 + sizeof _cpu386)]
  ;; Resume FPU.
  fxrstor [eax].r387
  ;; Resume PSW.
  push [eax].r386.e_ps 
  popfd 
  ;; Resume X86.
  mov ebx, [eax].r386.e_bx 
  mov ebp, [eax].r386.e_bp 
  mov edx, [eax].r386.e_dx
  mov edi, [eax].r386.e_di 
  mov esi, [eax].r386.e_si 
  mov esp, [eax].r386.e_sp
  mov ecx, [eax].r386.e_cx
  push [eax].r386.e_ip
  mov eax, [eax].r386.e_ax 
  
  ret    
context_swap2 endp

;;------------------------------------------------------------------
;; void callc context_setpc (context_t ctx, void *program_pointer);       
;;------------------------------------------------------------------
context_setpc proc C 
             option prologue:none, epilogue:none
             
  mov eax, [esp+4]
  mov edx, [esp+8]
  mov [eax].r386.e_ip, edx 
  ret 
context_setpc endp

;;------------------------------------------------------------------
;; void *callc context_getpc (context_t ctx);       
;;------------------------------------------------------------------
context_getpc proc C 
             option prologue:none, epilogue:none
             
  mov eax, [esp+4]
  mov eax, [eax].r386.e_ip
  ret 
context_getpc endp

;;------------------------------------------------------------------
;; void callc context_setstack (context_t ctx, uintptr_t program_pointer);       
;;------------------------------------------------------------------
context_setstack proc C 
             option prologue:none, epilogue:none
             
  mov eax, [esp+4]
  mov edx, [esp+8]
  mov [eax].r386.e_sp, edx 
  ret 
context_setstack endp

;;------------------------------------------------------------------
;; uintptr_t callc context_getstack (context_t ctx);       
;;------------------------------------------------------------------
context_getstack proc C 
             option prologue:none, epilogue:none
             
  mov eax, [esp+4]
  mov eax, [eax].r386.e_sp
  ret 
context_getstack endp

;;------------------------------------------------------------------
;; void callc context_copy (context_t to, context_t from);
;;------------------------------------------------------------------
context_copy proc C 
             option prologue:none, epilogue:none
  cld
             
  push edi 
  push esi 
  push ecx 
  
  mov esi, [esp+8+12]
  mov edi, [esp+4+12] 
  mov ecx, (sizeof context -4)
  
  rep movsb
  pop ecx 
  pop esi 
  pop edi 
  ret 
context_copy endp

    end