        ;; XXX: This macro branch is actually terrible to write.
        REGS_SET_OP macro link
        
          LOCAL stdPush 
          LOCAL fiqPush
          LOCAL usrPush 
          LOCAL usrPushFiq
          LOCAL usrPushFiq_R8_12
          LOCAL usrPushFiq_R13_14
          LOCAL usrPushFiq_next
          LOCAL usrPushFiq_next2       
          
          LOCAL stdPush_Q
          
          LOCAL usrPush_Q 
          LOCAL stdPush_Q2
          
          LOCAL usrPush_Q2 
          
          LOCAL outPush 
          
          LOCAL stdPop 
          LOCAL fiqPop
          LOCAL usrPop 
          LOCAL usrPopFiq
          LOCAL usrPopFiq_R8_12
          LOCAL usrPopFiq_R13_14
          LOCAL usrPopFiq_next
          LOCAL usrPopFiq_next2       
          LOCAL usrPopFiqFirst 
          LOCAL stdPop_Q
          
          LOCAL usrPop_Q 
          LOCAL stdPop_Q2
          LOCAL stdPop_HitR15
          
          LOCAL usrPop_Q2 
          
          LOCAL outPop 
          
          LOCAL stdPushFirst
          LOCAL usrPushFirst
          LOCAL usrPushFiqFirst
          LOCAL stdPopFirst
          LOCAL usrPopFirst      
          LOCAL usrPopFiqFirst
          
          
          LOCAL LDM_empty
          
          LOCAL STM_empty 
          
          
        z&link:
          ;; have to use variables.
        LDM_STACK_REQUIRE equ 128 
        RSO_L equ 1 
        RSO_W equ 2 
        RSO_S equ 4
        RSO_U equ 8 
        RSO_P equ 16 
        RnAddress equ 16
        RnIndex equ 20
        RnCount  equ 28
        Reglist equ 32
        RnWriteBack equ 36
        RnAdjust  equ 40
        RnFirst equ 44
        RnTemp equ 52
        RnTempi equ 56
           
          IF  (link and RSO_L) eq 0
            OUTd "STM"
          ELSEIF (link and RSO_L) ne 0
            OUTd "LDM"
            GamePAK_Prefetch 1 ;; I cycle before fetch new opcode.
          ENDIF 
          
          IF (link and RSO_U) eq 0 
            OUTd "D"
          ELSEIF (link and RSO_U) ne 0
            OUTd "I"
          ENDIF
          
          IF  (link and RSO_P)  eq 0 
            OUTd "A"
          ELSEIF  (link and RSO_P)  ne 0
            OUTd "B"
          ENDIF         
          
          OUTd " R%d", "SC_INVOL } 16 & 15"
          
          IF (link and RSO_W)  ne 0 
            OUTd "!"
          ENDIF
          
          OUTd ", "
          OUTd "[ R15-R12:%01X, R11-R8:%01X ", "SC_INVOL } 12 & 15", "SC_INVOL } 8 & 15"
          OUTd "  R7-R4:%01X, R3-R0:%01X ]", "SC_INVOL } 4 & 15", "SC_INVOL & 15"       
          
          IF (link and RSO_S) ne 0 
            OUTd "^"
          ENDIF
          
          sub esp, LDM_STACK_REQUIRE    
          xor eax, eax                  
          mov STA (RnAddress), eax          
          mov STA (RnIndex), eax 
          mov STA (RnAdjust), eax
          mov STA (RnFirst), eax
          mov STA (RnCount), eax 
          
          GetRFI SC_INVOL, 16, eax ;;Rn.
          mov ecx, SC_INVOL
          mov edx, ZRS (eax) ;;GetRn 
          and ecx, 0FFFFh
          mov STA (RnIndex), eax 
          mov STA (RnAddress), edx    
          IF (link and RSO_U) ne 0
            mov  SC_INVOL, 4
          ELSEIF (link and RSO_U) eq 0
            mov  SC_INVOL, -4
          ENDIF 
          mov STA (Reglist), ecx 
          mov eax, ecx 
          xor ecx, ecx 
          mov edx, 16 
          @@: 
          shr ax, 1
          adc cx, 0  
          dec dx
          jne @B 
          ;; TODO: assert 0 
          shl ecx, 2 
          mov edx, STA (RnAddress) 
        IF (link and RSO_U) ne 0
          add  edx, ecx
        ELSEIF (link and RSO_U) eq 0
          sub  edx, ecx
          lea  eax, [edx+4]
          mov  STA (RnAddress), eax 
        ENDIF 
          mov STA (RnWriteBack), edx     
        IF (link and RSO_P) ne 0
          add  STA (RnAddress), SC_INVOL
        ENDIF   
        mov SC_INVOL, STA (RnAddress)
        
      ;; STM-------------------------------------------------------
      ;; STM-------------------------------------------------------     
      ;; STM-------------------------------------------------------
      IF (link and RSO_L) eq 0
        test STA (Reglist), 0FFFFh 
        je STM_empty 
        IF (link and RSO_W) ne 0
          ;; Check Rn's adjust 
          mov ecx, STA (RnIndex) 
          bt STA (Reglist), ecx 
          jnc @F 
          sbb edx, edx   
          ;; Check is First Vailed bit 
          mov eax, edx 
          shl eax, cl 
          xor eax, edx 
          test STA (Reglist), eax 
          je @F ;; TODO: error.
          ;; Dirty register, write directly 
          mov eax, STA (RnIndex) 
          mov ecx, STA (RnWriteBack)
          mov ZRS (eax), ecx 
          @@:
        ENDIF      
        IF (link and RSO_S) ne 0
          ;; In User mode, Check Current is user/sys mode 
          ;; If hit, use std copy . (Because the user-mode cache register may not have been updated)
          mov eax, SC_CPSR 
          and eax, ARM7_MODE_GET_MASK
          cmp eax, ARM7_MODE_SYS_MASK 
          je stdPush
          cmp eax, ARM7_MODE_USER_MASK
          je stdPush 
          cmp eax, ARM7_MODE_FIQ_MASK
          je usrPushFiq 
          
          jmp usrPush               
        ELSEIF (link and RSO_S) eq 0
          jmp stdPush
        ENDIF 
        ;; STM  empty -------------------------------------------------------
        STM_empty: 
          mov edx, STA (RnIndex) 
          push STACK_PUSH_STATUS_PUSH
          push SZ_PC    
          mov eax, ZRS (SZ_PC)
          add eax, 4
          push eax
          push ZRS (edx)
        IF (link and RSO_W) ne 0
          IF (link and RSO_U) ne 0
            add ZRS (edx), 64 
          ELSE 
            sub ZRS (edx), 64 
          ENDIF
        ENDIF 
          and ZRS (edx), -4
          CallMemOrIOAddWaitState StackWriteNoSeq
          add esp, LDM_STACK_REQUIRE 
          mov [SC_ARM7].nextNoSeqFetch, 1
          ARM7_ExitAddPC 2 
        
        ;; STM  User Mode -------------------------------------------------------
        usrPush: 
          shr STA (Reglist), 1 
          jnc @F           
          inc SC_WAIT     ;; + 1 S or N cycle     
          mov eax, STA (RnCount)
          mov ecx, ZRS (eax)
          add eax, 1 
          and eax, 15 
          push STACK_PUSH_STATUS_PUSH
          push eax 
          cmp eax, 14
          jb usrPush_Q
          sub eax, 14
          mov ecx, [SC_ARM7].R1314_T[R1314b_SYSUSER+ eax*4] 
        usrPush_Q:
          test eax, eax 
          jne usrPush_Q2 
          add ecx, 4 
        usrPush_Q2:  
          push ecx 
          mov eax, SC_INVOL
          add SC_INVOL, 4
          ;;and eax, -4
          push eax 
          cmp STA (RnFirst+16), 0
          mov STA (RnFirst+16), 1
          je usrPushFirst
          CallMemOrIOAddWaitState StackWriteSeq        
          jmp @F
        usrPushFirst:
          CallMemOrIOAddWaitState StackWriteNoSeq          
        @@: 
          add STA (RnCount), 1            
          cmp STA (RnCount), 16
          jb usrPush 
          jmp outPush
          
        ;; STM  User Mode FIQ ----------------Will this happen to GBA?---------------------------------------
        usrPushFiq:
          shr STA (Reglist), 1 
          jnc @F           
          inc SC_WAIT     ;; + 1 S or N cycle     
          mov eax, STA (RnCount)
          mov ecx, ZRS (eax)
          add eax, 1 
          and eax, 15 
          push STACK_PUSH_STATUS_PUSH
          push eax 
          cmp eax, 9
          jb usrPushFiq_next
          cmp eax, 14  
          jb usrPushFiq_R8_12
        usrPushFiq_R8_12:
          sub eax, 9
          mov ecx, [SC_ARM7].R812_T[R812b_EXCEPT_FIQ+ eax*4] 
          jmp usrPushFiq_next2
        usrPushFiq_R13_14:
          sub eax, 14
          mov ecx, [SC_ARM7].R1314_T[R1314b_SYSUSER+ eax*4] 
          jmp usrPushFiq_next2
        usrPushFiq_next:
          test eax, eax 
          jne usrPushFiq_next2 
          add ecx, 4 
        usrPushFiq_next2:  
          push ecx 
          mov eax, SC_INVOL
          add SC_INVOL, 4
          ;;and eax, -4
          push eax 
          cmp STA (RnFirst+16), 0
          mov STA (RnFirst+16), 1
          je usrPushFiqFirst
          CallMemOrIOAddWaitState StackWriteSeq        
          jmp @F
        usrPushFiqFirst:
          CallMemOrIOAddWaitState StackWriteNoSeq          
        @@: 
          add STA (RnCount), 1            
          cmp STA (RnCount), 16
          jb usrPushFiq 
          jmp outPush 
          
        ;; STM  Std Mode --------------------------------------------------------
        stdPush: 
          shr STA (Reglist), 1 
          jnc @F            
          inc SC_WAIT     ;; + 1 S or N cycle   
          mov eax, STA (RnCount)
          push STACK_PUSH_STATUS_PUSH
          push eax 
          push ZRS (eax) 
          cmp eax, 15 
          jne stdPush_Q2
          add PTR32[esp], 4 
        stdPush_Q2:
          mov eax, SC_INVOL
          ;;and eax, -4
          push eax
          add SC_INVOL, 4
          cmp STA (RnFirst+16), 0
          mov STA (RnFirst+16), 1
          je stdPushFirst
          CallMemOrIOAddWaitState StackWriteSeq
          jmp @F
        stdPushFirst:
          CallMemOrIOAddWaitState StackWriteNoSeq    
        @@:  
          add STA (RnCount), 1            
          cmp STA (RnCount), 16
          jb  stdPush
        outPush:   
        IF (link and RSO_W) ne 0
          mov eax, STA (RnIndex) 
          mov ecx, STA (RnWriteBack)
          mov ZRS (eax), ecx  ;; TODO: RnCheck with Reglist 
        ENDIF  
        mov [SC_ARM7].nextNoSeqFetch, 1
        add esp, LDM_STACK_REQUIRE 
        ARM7_ExitAddPC 1 
      ELSEIF (link and RSO_L) ne 0
        ;; LDM ----------------------------------------------------------------------    
        ;; LDM ----------------------------------------------------------------------
        ;; LDM ----------------------------------------------------------------------       
        test STA (Reglist), 0FFFFh 
        je LDM_empty 
        ;; Check Rn's adjust 
        mov ecx, STA (RnIndex) 
        bt STA (Reglist), ecx 
        jnc @F 
        mov STA (RnAdjust), 1 
        @@:        
        IF (link and RSO_S) ne 0       
          test STA (Reglist), 08000h 
          jne stdPop
          mov eax, SC_CPSR 
          and eax, ARM7_MODE_GET_MASK
          cmp eax, ARM7_MODE_SYS_MASK 
          je stdPop
          cmp eax, ARM7_MODE_USER_MASK
          je stdPop 
          cmp eax, ARM7_MODE_FIQ_MASK
          je usrPopFiq 
          jmp usrPop               
        ELSEIF (link and RSO_S) eq 0 
          jmp stdPop
        ENDIF 
        ;; LDM  reglist empty -------------------------------------------------------
        LDM_empty:
          mov ecx, STA (RnIndex) 
          push ZRS(ecx) 
        IF (link and RSO_W) ne 0
          IF (link and RSO_U) ne 0
            add ZRS(ecx), 64
          ELSE 
            sub ZRS(ecx), 64
          ENDIF
        ENDIF 
          and ZRS(ecx), -4
          CallMemOrIOAddWaitState StackReadNoSeq 
          mov ZRS (SZ_PC), eax
          add esp, LDM_STACK_REQUIRE
          arm7_FlushPipeline 3 
          
        ;; LDM  User Mode -------------------------------------------------------
        usrPop: 
          shr STA (Reglist), 1 
          jnc @F            
          inc SC_WAIT     ;; + 1 S or N cycle   
          mov eax, STA (RnCount)
          lea SC_INVOL, ZRS (eax)
          add eax, 1 
          and eax, 15 
          cmp eax, 14
          jb @F
          sub eax, 14
          lea SC_INVOL, [SC_ARM7].R1314_T[R1314b_SYSUSER+ eax*4]          
        @@:
          mov eax, STA (RnAddress)
          add STA (RnAddress), 4
          ;;and eax, -4
          push eax 
          cmp STA (RnFirst+4), 0
          mov STA (RnFirst+4), 1
          je usrPopFirst
          CallMemOrIOAddWaitState StackReadSeq
          mov [SC_INVOL], eax
          jmp @F
        usrPopFirst:
          CallMemOrIOAddWaitState StackReadNoSeq 
          mov [SC_INVOL], eax          
        @@:       
          add STA (RnCount), 1            
          cmp STA (RnCount), 16
          jb usrPop 
          jmp outPop
          
        ;; LDM  FIQ Mode ----------------Will this happen to GBA?---------------------------------------
        usrPopFiq: 
          shr STA (Reglist), 1 
          jnc @F            
          inc SC_WAIT     ;; + 1 S or N cycle   
          mov eax, STA (RnCount)
          lea SC_INVOL, ZRS (eax)
          add eax, 1 
          and eax, 15 
          cmp eax, 14
          jb usrPopFiq_next
          sub eax, 14
          lea SC_INVOL, [SC_ARM7].R1314_T[R1314b_SYSUSER+ eax*4]   
          jmp usrPopFiq_next2     
        usrPopFiq_next:
          cmp eax, 9 
          jb usrPopFiq_next2 
          sub eax, 9
          lea SC_INVOL, [SC_ARM7].R812_T[R812b_EXCEPT_FIQ+ eax*4]       
        usrPopFiq_next2:
          mov eax, STA (RnAddress)
          add STA (RnAddress), 4
          ;;and eax, -4
          push eax 
          cmp STA (RnFirst+4), 0
          mov STA (RnFirst+4), 1
          je usrPopFiqFirst
          CallMemOrIOAddWaitState StackReadSeq
          mov [SC_INVOL], eax
          jmp @F
        usrPopFiqFirst:
          CallMemOrIOAddWaitState StackReadNoSeq   
          mov [SC_INVOL], eax
        @@:    
          add STA (RnCount), 1            
          cmp STA (RnCount), 16
          jb usrPopFiq 
          jmp outPop
 
        ;; LDM  Std Mode --------------------------------------------------------
        stdPop: 
          shr STA (Reglist), 1 
          jnc @F            
          inc SC_WAIT     ;; + 1 S or N cycle   
          mov eax, STA (RnCount)
          lea SC_INVOL, ZRS (eax)
          mov eax, STA (RnAddress)
          add STA (RnAddress), 4
          ;;and eax, -4
          push eax 
          cmp STA (RnFirst+4), 0
          mov STA (RnFirst+4), 1
          je stdPopFirst
          CallMemOrIOAddWaitState StackReadSeq
          mov [SC_INVOL], eax 
          ;; Check is finial?? 
          cmp STA (RnCount), 15 
          jne @F 
          jmp stdPop_HitR15
        stdPopFirst:
          CallMemOrIOAddWaitState StackReadNoSeq
          mov [SC_INVOL], eax 
          ;; Check is finial?? 
          cmp STA (RnCount), 15 
          jne @F 
        stdPop_HitR15:
          IF (link and RSO_W) ne 0
            cmp STA (RnAdjust), 1 
            je stdPop_Q
            mov eax, STA (RnIndex) 
            mov ecx, STA (RnWriteBack)
            mov ZRS (eax), ecx 
            stdPop_Q:
          ENDIF
          IF (link and RSO_S) ne 0
            SPSRToCPSR eax   
          ENDIF
          add esp, LDM_STACK_REQUIRE 
          test SC_CPSR, FLAG_THUMB
          jne stdPop_Q2 
          arm7_FlushPipeline 4
        stdPop_Q2:
          tb_FlushPipeline 4
        @@: 
          add STA (RnCount), 1            
          cmp STA (RnCount), 16
          jb  stdPop
        outPop:   
        IF (link and RSO_W) ne 0
          cmp STA (RnAdjust), 1 
          je @F
          mov eax, STA (RnIndex) 
          mov ecx, STA (RnWriteBack)
          mov ZRS (eax), ecx  
        @@:
        ENDIF  
        add esp, LDM_STACK_REQUIRE 
        ARM7_ExitAddPC 2    
      ENDIF 
      endm 
      ;; TODO: one unit for LDM/STM
      ;; TODO: other Bank Rn!^ ?
      ;; TODO: empty load/ store