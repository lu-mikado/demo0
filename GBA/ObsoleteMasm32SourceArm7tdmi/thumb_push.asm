    PUSHR: ;; PUSH is DB stack opreate type, if R spec, Push LRLINK to stack. 
        OUTd "PUSH R:%d R7-R4:%01X R3-R0:%01X", "SC_INVOL } 8 & 1", "SC_INVOL } 4 & 15", "SC_INVOL & 15"  
        sub esp, LDM_STACK_REQUIRE 
        mov STA (RnFirst), 0
        sub ZRS (SZ_STACK), 4
        mov STA (RnCount), 8
        shl SC_INVOL, 23 
      @@:
        shl SC_INVOL, 1
        jnc PUSHR_c
        add SC_WAIT, 1
        mov eax, STA (RnCount) 
        bt eax, 3  ;; Check R Spec 
        sbb ecx, ecx 
        and ecx, SZ_LRLINK 
        or eax, ecx ;; XXX: LRLINK must be 14  
        push STACK_PUSH_STATUS_PUSH
        push eax       
        push ZRS (eax) 
        push ZRS (SZ_STACK)
        sub ZRS (SZ_STACK), 4
        cmp STA (RnFirst+16), 0
        mov STA (RnFirst+16), 1
        je PUSHR_first
        CallMemOrIOAddWaitState StackWriteSeq
        jmp PUSHR_c
      PUSHR_first:
        CallMemOrIOAddWaitState StackWriteNoSeq
      PUSHR_c: 
        sub STA (RnCount), 1
        jge @B 
        add ZRS (SZ_STACK), 4 
        add esp, LDM_STACK_REQUIRE 
        mov [SC_ARM7].nextNoSeqFetch, 1
        tb_ExitAddPC 1 
        
    POPRP:  ;; POP is IA stack opreate type, if R spec, Pop PC from stack. 
        OUTd "POP R:%d R7-R4:%01X R3-R0:%01X", "SC_INVOL } 8 & 1", "SC_INVOL } 4 & 15", "SC_INVOL & 15"
        GamePAK_Prefetch 1
        xor eax, eax
        sub esp, LDM_STACK_REQUIRE 
        mov STA (RnFirst), eax
        mov STA (RnCount), eax
      @@:
        shr SC_INVOL, 1
        jnc POPRP_c   
        inc SC_WAIT
        push ZRS (SZ_STACK)
        cmp STA (RnFirst+4), 0
        mov STA (RnFirst+4), 1
        je POPRP_first
        CallMemOrIOAddWaitState StackReadSeq
        jmp POPRP_continue
      POPRP_first:
        CallMemOrIOAddWaitState StackReadNoSeq
      POPRP_continue:        
        mov edx, STA (RnCount) 
        bt edx, 3 ;; Check R Spec 
        sbb ecx, ecx 
        and ecx, SZ_PC 
        or edx, ecx ;; XXX: PC must be 15   
        mov ZRS (edx), eax    
        add ZRS (SZ_STACK), 4
        cmp edx, SZ_PC ;; PC Flush? Flush Pipeline : nodone. 
        je @F
      POPRP_c: 
        add STA (RnCount), 1
        cmp STA (RnCount), 9
        jne @B 
        add esp, LDM_STACK_REQUIRE 
        tb_ExitAddPC 2
      @@: 
        add esp, LDM_STACK_REQUIRE  
        tb_FlushPipeline 4 