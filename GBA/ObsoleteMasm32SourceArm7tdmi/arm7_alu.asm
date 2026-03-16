        AND_OP equ 0 
        EOR_OP equ 1
        SUB_OP equ 2 ;; With C
        RSB_OP equ 3 ;; With C
        ADD_OP equ 4 ;; With C 
        ADC_OP equ 5 ;; With C 
        SBC_OP equ 6 ;; With C 
        RSC_OP equ 7 ;; With C 
        TST_OP equ 8 ;; NORMAL SET-
        TEQ_OP equ 9 ;; NORMAL SET- 
        CMP_OP equ 10 ;; With C 
        CMN_OP equ 11 ;; With C 
        ORR_OP equ 12 
        MOV_OP equ 13 
        BIC_OP equ 14 
        MVN_OP equ 15

        ARM7_ALU_BASE macro OPID, LhsOut, Rhs
          IF OPID eq AND_OP
            and LhsOut, Rhs 
          ELSEIF OPID eq EOR_OP 
            xor LhsOut, Rhs 
          ELSEIF OPID eq ORR_OP 
            or LhsOut, Rhs   
          ELSEIF OPID eq SUB_OP 
            sub LhsOut, Rhs
          ELSEIF OPID eq ADD_OP         
            add LhsOut, Rhs 
          ELSEIF OPID eq MOV_OP             
            mov LhsOut, Rhs            
          ELSEIF OPID eq MVN_OP           
            not Rhs           
            mov LhsOut, Rhs    
          ELSEIF OPID eq BIC_OP           
            not Rhs                       
            and LhsOut, Rhs   
          ELSEIF OPID eq RSB_OP
            sub Rhs, LhsOut
            mov LhsOut, Rhs 
          ELSEIF OPID eq SBC_OP  
            xor SC_CPSR, FLAG_C
            btc SC_CPSR, FLAG_CHECK_C_X86_BT            
            sbb LhsOut, Rhs
          ELSEIF OPID eq RSC_OP  
            xor SC_CPSR, FLAG_C
            btc SC_CPSR, FLAG_CHECK_C_X86_BT            
            sbb Rhs, LhsOut         
            mov LhsOut, Rhs  
          ELSEIF OPID eq ADC_OP  
            bt SC_CPSR, FLAG_CHECK_C_X86_BT            
            adc LhsOut, Rhs         
          ELSEIF OPID eq TST_OP  
            and LhsOut, Rhs
            SetNZ_A            
          ELSEIF OPID eq TEQ_OP         
            xor LhsOut, Rhs
            SetNZ_A         
          ELSEIF OPID eq CMP_OP  
            cmp LhsOut, Rhs
            SetNZCV_A 1             
          ELSEIF OPID eq CMN_OP  
            add LhsOut, Rhs
            SetNZCV_A 0
          ELSE 
            ;;  never reach here, make a simple syntax errors 
            ERRORS_ASSERT
          ENDIF 
        endm 
        ARM7_ALU_SIGN_BASE macro OPID, LhsOut, Rhs ;; LhsOut Must not eax!
          IF OPID eq AND_OP
            and LhsOut, Rhs 
            SetNZ_A
          ELSEIF OPID eq EOR_OP 
            xor LhsOut, Rhs 
            SetNZ_A
          ELSEIF OPID eq ORR_OP 
            or LhsOut, Rhs   
            SetNZ_A
          ELSEIF OPID eq SUB_OP 
            sub LhsOut, Rhs
            SetNZCV_A 1
          ELSEIF OPID eq ADD_OP            
            add LhsOut, Rhs 
            SetNZCV_A 0
          ELSEIF OPID eq MOV_OP              
            mov LhsOut, Rhs 
            test LhsOut, LhsOut              
            SetNZ_A              
          ELSEIF OPID eq MVN_OP  
            not Rhs           
            mov LhsOut, Rhs   
            test LhsOut, LhsOut 
            SetNZ_A              
          ELSEIF OPID eq BIC_OP  
            not Rhs                       
            and LhsOut, Rhs   
            SetNZ_A
          ELSEIF OPID eq RSB_OP
            sub Rhs, LhsOut
            mov LhsOut, Rhs 
            SetNZCV_A 1
          ELSEIF OPID eq SBC_OP  
            xor SC_CPSR, FLAG_C
            bt SC_CPSR, FLAG_CHECK_C_X86_BT            
            sbb LhsOut, Rhs
            SetNZCV_A 1
          ELSEIF OPID eq RSC_OP  
            xor SC_CPSR, FLAG_C
            bt SC_CPSR, FLAG_CHECK_C_X86_BT            
            sbb Rhs, LhsOut        
            mov LhsOut, Rhs 
            SetNZCV_A 1
          ELSEIF OPID eq ADC_OP  
            bt SC_CPSR, FLAG_CHECK_C_X86_BT            
            adc LhsOut, Rhs    
            SetNZCV_A 0             
          ELSE 
            ;;  never reach here, make a simple syntax errors 
            ERRORS_ASSERT
          ENDIF 
        endm   
        
        i8r4_ALU macro lSym, OPID
          @ALIGN_Z
          i8r4&lSym:    
          ;; Clks 1: ALU Operate, and MemAccess Seq Read Next Opcode, indicate next mREQ:0 Seq:0
          IF (OPID and 1100b) ne 1000b
            Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm   
          ELSEIF (OPID and 1100b) eq 1000b
            Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm  
          ENDIF
          GetRFIV SC_INVOL, 16, ecx        ;; Rn
          GetRFI SC_INVOL, 12, edx       ;; RdIs   
          ARM7_ALU_BASE_OUTd1 OPID      
          ARM7_ALU_BASE OPID, ecx, eax 
          IF (OPID and 1100b) ne 1000b    
            ;; Write Back, Check R15. 
            mov ZRS (edx), ecx
            cmp edx, 15 
            jne  @F 
            ;; dest := pc 
            ;; Clks 2: MemAccess, NoSeq Read. (because current is new pc read first), indicate next mREQ:0 Seq:1
            ;; Clks 3: MemAccess (new pc read, second), Seq Read.
            arm7_FlushPipeline 3
            @@:    
          ENDIF 
          ARM7_ExitAddPC 1
          IF (OPID and 1100b) ne 1000b 
            @ALIGN_Z
            i8r4&lSym&S:
            ;; Save Old CPSR 
            mov [SC_ARM7].calc, SC_CPSR 
            IF OPID eq AND_OP
              Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq EOR_OP  
              Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq ORR_OP  
              Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq SUB_OP ;; with C
              Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq ADD_OP ;; with C     
              Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm             
            ELSEIF OPID eq MOV_OP     
              Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm             
            ELSEIF OPID eq MVN_OP    
              Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm             
            ELSEIF OPID eq BIC_OP         
              Imm8BitmapSft_AC_SetC SC_INVOL, eax   ;; Rm             
            ELSEIF OPID eq RSB_OP ;; with C
              Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq SBC_OP ;; with C 
              Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq RSC_OP ;; with C  
              Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm   
            ELSEIF OPID eq ADC_OP ;; with C 
              Imm8BitmapSft_AC SC_INVOL, eax   ;; Rm          
            ELSE 
              ;;  never reach here, make a simple syntax errors 
              ERRORS_ASSERT
            ENDIF 
            GetRFIV SC_INVOL, 16, ecx        ;; Rn
            GetRFI SC_INVOL, 12, edx       ;; RdIs
            ARM7_ALU_SIGN_BASE_OUTd1 OPID
            ARM7_ALU_SIGN_BASE OPID, ecx, eax  
            mov ZRS (edx), ecx  
            cmp edx, 15 
            je @F 
            ARM7_ExitAddPC 1   
          @@:
            ;; R15 resume old CPSR.
            mov SC_CPSR, [SC_ARM7].calc 
            SPSRToCPSR edx
            test SC_CPSR, FLAG_THUMB 
            je @F 
            tb_FlushPipeline 3
          @@:
            arm7_FlushPipeline 3          
          ENDIF 
        endm 

        si5_ALU macro lSym, OPID
          si5&lSym:  
          ARM7_ALU_BASE_OUTd2 OPID
          IF (OPID and 1100b) ne 1000b
            ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax
          ELSEIF (OPID and 1100b) eq 1000b
            ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care
          ENDIF
          
          GetRFIV edx, 16, ecx        ;; Rn
          GetRFI edx, 12, edx       ;; RdIs   
          ARM7_ALU_BASE OPID, ecx, eax  
          IF (OPID and 1100b) ne 1000b 
            ;; Write Back, Check R15. 
            mov ZRS (edx), ecx
            cmp edx, 15 
            jne  @F 
            arm7_FlushPipeline 3;; +1S +1N     
            @@:    
          ENDIF 
          ARM7_ExitAddPC 1
          
          IF (OPID and 1100b) ne 1000b 
            si5&lSym&S:
            ;; Save Old CPSR 
            mov [SC_ARM7].calc, SC_CPSR 
            IF OPID eq AND_OP
              ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care   ;; Rm   
            ELSEIF OPID eq EOR_OP  
              ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care   ;; Rm   
            ELSEIF OPID eq ORR_OP  
              ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care   ;; Rm   
            ELSEIF OPID eq SUB_OP ;; with C
              ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax   ;; Rm   
            ELSEIF OPID eq ADD_OP ;; with C     
              ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax   ;; Rm             
            ELSEIF OPID eq MOV_OP     
              ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care   ;; Rm             
            ELSEIF OPID eq MVN_OP    
              ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care   ;; Rm             
            ELSEIF OPID eq BIC_OP         
              ShiftRegImm5 SC_INVOL, eax, ecx, eax, C_op_Care   ;; Rm             
            ELSEIF OPID eq RSB_OP ;; with C
              ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax   ;; Rm   
            ELSEIF OPID eq SBC_OP ;; with C 
              ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax   ;; Rm   
            ELSEIF OPID eq RSC_OP ;; with C  
              ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax   ;; Rm   
            ELSEIF OPID eq ADC_OP ;; with C 
              ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, eax   ;; Rm          
            ELSE 
              ;;  never reach here, make a simple syntax errors 
              ERRORS_ASSERT
            ENDIF 
            
            ARM7_ALU_SIGN_BASE_OUTd2 OPID         
            GetRFIV SC_INVOL, 16, ecx        ;; Rn
            GetRFI SC_INVOL, 12, edx       ;; RdIs
            ARM7_ALU_SIGN_BASE OPID, ecx, eax  
            mov ZRS (edx), ecx  
            cmp edx, 15 
            je @F 
            ARM7_ExitAddPC 1
          @@:
            ;; R15 resume old CPSR.
            mov SC_CPSR, [SC_ARM7].calc 
            SPSRToCPSR edx
            test SC_CPSR, FLAG_THUMB 
            je @F 
            tb_FlushPipeline 3
          @@:
            arm7_FlushPipeline 3       
          ENDIF 
        endm   

        rs_ALU macro lSym, OPID
          LOCAL locSym
          LOCAL locSym_S
          
          rs&lSym:     
          GamePAK_Prefetch 1
          IF (OPID and 1100b) ne 1000b
            ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare
          ELSEIF (OPID and 1100b) eq 1000b
            ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care
          ENDIF
          GetRFI SC_INVOL, 16, ecx        ;; RnI
          cmp ecx, SZ_PC 
          mov ecx, ZRS (ecx)
          jne locSym
          add ecx, 4
          locSym:
          GetRFI SC_INVOL, 12, edx       ;; RdIs 
          ARM7_ALU_BASE_OUTd2 OPID
          ARM7_ALU_BASE OPID, ecx, eax  
          ;; In Rs Shift. Add I (Internal) Clk
          ;; In AGB, I Clk can access GamePAK Cache. (I cycle does not access memory)
          IF (OPID and 1100b) ne 1000b 
            ;; Write Back, Check R15.  
            mov ZRS (edx), ecx
            cmp edx, 15 
            jne  @F 
            arm7_FlushPipeline 4
            @@:    
          ENDIF         
          ARM7_ExitAddPC 2
           
          IF (OPID and 1100b) ne 1000b 
            rs&lSym&S:
            GamePAK_Prefetch 1
            mov [SC_ARM7].calc, SC_CPSR 
            IF OPID eq AND_OP
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care   ;; Rm   
            ELSEIF OPID eq EOR_OP  
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care   ;; Rm   
            ELSEIF OPID eq ORR_OP  
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care   ;; Rm   
            ELSEIF OPID eq SUB_OP ;; with C
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare   ;; Rm   
            ELSEIF OPID eq ADD_OP ;; with C     
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare   ;; Rm             
            ELSEIF OPID eq MOV_OP     
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care   ;; Rm             
            ELSEIF OPID eq MVN_OP    
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care   ;; Rm             
            ELSEIF OPID eq BIC_OP         
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_Care   ;; Rm             
            ELSEIF OPID eq RSB_OP ;; with C
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare   ;; Rm   
            ELSEIF OPID eq SBC_OP ;; with C 
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare   ;; Rm   
            ELSEIF OPID eq RSC_OP ;; with C  
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare   ;; Rm   
            ELSEIF OPID eq ADC_OP ;; with C 
              ShiftRegRs SC_INVOL, eax, ecx, edx, C_op_NOCare   ;; Rm          
            ELSE 
              ;;  never reach here, make a simple syntax errors 
              ERRORS_ASSERT
            ENDIF 
            GetRFI SC_INVOL, 16, ecx        ;; RnI
            cmp ecx, SZ_PC 
            mov ecx, ZRS (ecx)
            jne locSym_S
            add ecx, 4
            locSym_S:
            GetRFI SC_INVOL, 12, edx       ;; RdIs
            ARM7_ALU_SIGN_BASE_OUTd2 OPID
            ARM7_ALU_SIGN_BASE OPID, ecx, eax  
            mov ZRS (edx), ecx  
            cmp edx, 15 
            je @F 
            ARM7_ExitAddPC 2
          @@:
            ;; R15 resume old CPSR.
            mov SC_CPSR, [SC_ARM7].calc 
            SPSRToCPSR edx 
            test SC_CPSR, FLAG_THUMB 
            je @F 
            tb_FlushPipeline 4 
          @@:
            arm7_FlushPipeline 4    
          ENDIF 
        endm 
        
        ;; ------------------------- ALU+Imm8+Sft             
        ;;  27 26 25 24 23 22 21 20   19 - 16     15 - 12       11 10 9 8  7 6 5 4  3 2 1 0 
        ;;   0  0  1    Opcode    S      Rn          Rd             Imm8+Sft
        ;;   0  0  1  1  0  R  1  0    fmask        SBO             Imm8+Sft  
        i8r4_ALU AND, AND_OP
        i8r4_ALU EOR, EOR_OP       
        i8r4_ALU SUB, SUB_OP    
        i8r4_ALU RSB, RSB_OP  
        i8r4_ALU ADD, ADD_OP
        i8r4_ALU ADC, ADC_OP       
        i8r4_ALU SBC, SBC_OP    
        i8r4_ALU RSC, RSC_OP
        i8r4_ALU TST, TST_OP
        i8r4_ALU TEQ, TEQ_OP       
        i8r4_ALU CMP, CMP_OP    
        i8r4_ALU CMN, CMN_OP  
        i8r4_ALU ORR, ORR_OP
        i8r4_ALU MOV, MOV_OP       
        i8r4_ALU BIC, BIC_OP    
        i8r4_ALU MVN, MVN_OP 
        mUB:   
        i8r4UB:  
          int 3   
          
        ;;  ------------------------- ALU+Imm5             
        ;;  27 26 25 24 23 22 21 20   19 - 16     15 - 12       11 10 9 8  7 6 5 4  3 2 1 0 
        ;;   0  0  0    Opcode    S      Rn          Rd             Imm5     T T 0    Rm 
        ;;   0  0  0  1  0  R  1  0    fmask        SBO             SBZ    0 0 0 0    Rm  MSR PSR,  Rm     
        si5_ALU AND, AND_OP
        si5_ALU EOR, EOR_OP       
        si5_ALU SUB, SUB_OP    
        si5_ALU RSB, RSB_OP  
        si5_ALU ADD, ADD_OP
        si5_ALU ADC, ADC_OP       
        si5_ALU SBC, SBC_OP    
        si5_ALU RSC, RSC_OP
        si5_ALU TST, TST_OP
        si5_ALU TEQ, TEQ_OP       
        si5_ALU CMP, CMP_OP    
        si5_ALU CMN, CMN_OP  
        si5_ALU ORR, ORR_OP
        si5_ALU MOV, MOV_OP       
        si5_ALU BIC, BIC_OP    
        si5_ALU MVN, MVN_OP
        
        ;;  ------------------------- ALU+RegSft             
        ;;  27 26 25 24 23 22 21 20   19 - 16     15 - 12       11 10 9 8  7 6 5 4  3 2 1 0 
        ;;   0  0  0    Opcode    S      Rn          Rd             Rs     0 T T 1    Rm     
        rs_ALU AND, AND_OP
        rs_ALU EOR, EOR_OP       
        rs_ALU SUB, SUB_OP    
        rs_ALU RSB, RSB_OP  
        rs_ALU ADD, ADD_OP
        rs_ALU ADC, ADC_OP       
        rs_ALU SBC, SBC_OP    
        rs_ALU RSC, RSC_OP
        rs_ALU TST, TST_OP
        rs_ALU TEQ, TEQ_OP       
        rs_ALU CMP, CMP_OP    
        rs_ALU CMN, CMN_OP  
        rs_ALU ORR, ORR_OP
        rs_ALU MOV, MOV_OP       
        rs_ALU BIC, BIC_OP    
        rs_ALU MVN, MVN_OP