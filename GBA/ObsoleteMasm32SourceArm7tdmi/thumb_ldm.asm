    ;; A7.1.57 - STMIA (INC, AFTER)
    ;;
    ;; Codec: 15 14 13 12 11    10 9 8      7 6 5 4 3 2 1 0
    ;;         1  1  1  0  0      Rn           Reglist. 
    ;;
    
    STMIA: 
        OUTd "T-STMIA [R%d]!, R7-R4:%01X R3-R0:%01X ", "SC_INVOL } 8 & 7", "SC_INVOL } 4 & 15", "SC_INVOL & 15"
        sub esp, LDM_STACK_REQUIRE 
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        and eax, 255 
        shr ecx, 8 
        and ecx, 7 
        ;; Get Rn's index and Rn's init value.
        mov edx, ZRS (ecx)
        mov STA (RnTempi), ecx 
        mov STA (RnTemp), edx 
        mov STA (RnFirst), 0
        mov STA (RnCount), 0
        ;; Check Reglist is empty ?
        ;; ================================= NO$GBA doc =================================
        ;; Strange Effects on Invalid Rlist's
        ;; Empty Rlist: R15 loaded/stored (ARMv4 only), and Rb=Rb+40h (ARMv4-v5).
        ;; Writeback with Rb included in Rlist: Store OLD base if Rb is FIRST entry in
        ;; Rlist, otherwise store NEW base (STM/ARMv4), always store OLD base (STM/ARMv5),
        ;; no writeback (LDM/ARMv4/ARMv5; at this point, THUMB opcodes work different than ARM opcodes).
        test eax, eax 
        jne @F
        push STACK_PUSH_STATUS_PUSH
        push SZ_PC    
        mov eax, ZRS (SZ_PC)
        add eax, 2
        push eax
        push edx 
        add ZRS (ecx), 64 
        and ZRS (ecx), -4
        CallMemOrIOAddWaitState StackWriteNoSeq
        add esp, LDM_STACK_REQUIRE 
        mov [SC_ARM7].nextNoSeqFetch, 1
        tb_ExitAddPC 2 
        ;; Check Rn with Reglist ?
      @@:
        bt eax, ecx 
        jnc @F  
        DEBUG_BREAK
        OUTd "R%d in reglist! FIRST-V:", "ecx"
        xor edx, edx   
        not edx 
        ;; Check First Vailed bit 
        ;; IF Done, write inital value to stack 
        ;; ELSE. write final Rn's value to stack (this is UB. but i test on AXD Debugger 1.2 is final Rn)
        ;; See ARM Developer Suite Assembler Guide:: Thumb Instruction Reference::5.1.5 LDMIA and STMIA
        ;; 
        shl edx, cl 
        not edx
        test edx, eax 
        je STMIA_k
        OUTd "NO"
        ;; Calc write back's Rn in end. 
      IF  1
        mov ecx, 8 
        xor edx, edx 
      STMIA0:
        shr eax, 1 
        adc edx, 0 
        dec ecx 
        jne STMIA0
      ELSE 
        and eax, 255
        popcnt edx, eax 
      ENDIF
        ;; Write back Rn.
        lea edx, [edx*4] 
        mov ecx, SC_INVOL 
        shr ecx, 8 
        and ecx, 7
        add ZRS (ecx), edx  
        and ZRS (ecx), -4
      STMIA_k:
        OUTd "YES"
      @@:
        shr SC_INVOL, 1
        jnc STMIA_c   
        inc SC_WAIT        
        mov eax, STA (RnCount)
        push STACK_PUSH_STATUS_PUSH
        push eax    
        push ZRS (eax)
        mov eax, STA (RnTemp+12)
        add STA (RnTemp+12), 4
        ;; and eax, -4 
        push eax 
        cmp STA (RnFirst+16), 0
        mov STA (RnFirst+16), 1
        je STMIA_first 
        CallMemOrIOAddWaitState StackWriteSeq
        jmp STMIA_c
      STMIA_first:
        CallMemOrIOAddWaitState StackWriteNoSeq
      STMIA_c: 
        add STA (RnCount), 1
        cmp STA (RnCount), 8
        jne @B 
        cmp STA (RnFirst), 0
        jne @F
      @@:
        mov ecx, STA (RnTemp) 
        mov eax, STA (RnTempi)
        mov ZRS (eax), ecx
        and ZRS (eax), -4
        add esp, LDM_STACK_REQUIRE 
        mov [SC_ARM7].nextNoSeqFetch, 1
        tb_ExitAddPC 1    
   
    ;; A7.1.27 LDMIA   
    ;;
    ;; Codec: 15 14 13 12 11    10 9 8      7 6 5 4 3 2 1 0
    ;;         1  1  1  0  1      Rn           Reglist. 
    ;;     
    
    LDMIA:
        GamePAK_Prefetch 1 
        OUTd "T-LDMIA [R%d]!, R7-R4:%01X R3-R0:%01X ", "SC_INVOL } 8 & 7", "SC_INVOL } 4 & 15", "SC_INVOL & 15"
        sub esp, LDM_STACK_REQUIRE 
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        and eax, 255 
        shr ecx, 8 
        and ecx, 7 
        ;; Get Rn's index and Rn's init value.
        mov edx, ZRS (ecx)
        mov STA (RnTempi), ecx 
        mov STA (RnTemp), edx 
        mov STA (RnFirst), 0
        mov STA (RnCount), 0
        mov STA (RnAdjust), 1  
        ;; Check empty reglist 
        test eax, eax 
        jne @F
        push edx 
        add ZRS (ecx), 64 
        and ZRS (ecx), -4
        CallMemOrIOAddWaitState StackReadNoSeq
        mov ZRS (SZ_PC), eax 
        add esp, LDM_STACK_REQUIRE 
        tb_FlushPipeline 3  
      @@:
        bt eax, ecx 
        jnc @F  
        ;; Rn with Reglist 
        ;; Rn WriteBack will miss in thumb-LDMIA.
        ;; DEBUG_BREAK
        OUTd "R%d in reglist!"
        mov STA (RnAdjust), 0
      @@:
        shr SC_INVOL, 1
        jnc LDMIA_c   
        inc SC_WAIT        
        mov eax, STA (RnTemp)
        add STA (RnTemp), 4
        ;; and eax, -4 
        push eax 
        cmp STA (RnFirst+4), 0
        mov STA (RnFirst+4), 1
        je LDMIA_first 
        CallMemOrIOAddWaitState StackReadSeq
        jmp LDMIA_a
      LDMIA_first:
        CallMemOrIOAddWaitState StackReadNoSeq 
      LDMIA_a:
        mov ecx, STA (RnCount)
        mov ZRS (ecx), eax         
      LDMIA_c: 
        add STA (RnCount), 1
        cmp STA (RnCount), 8
        jne @B 
        cmp STA (RnFirst), 0
        jne @F
        ;; FIXME: No Reglist mask exist stuff..
        DEBUG_BREAK
      @@:
        cmp STA (RnAdjust), 0 
        je @F 
        mov ecx, STA (RnTemp) 
        mov eax, STA (RnTempi) 
        mov ZRS (eax), ecx
        and ZRS (eax), -4
      @@:
        add esp, LDM_STACK_REQUIRE 
        tb_ExitAddPC 2 