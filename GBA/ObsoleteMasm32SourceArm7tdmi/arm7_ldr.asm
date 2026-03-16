        LDR_SET macro ldrtype, pre, link  
        
          LDR_L equ 1 
          LDR_W equ 2 
          LDR_B equ 4
          LDR_U equ 8 
          LDR_P equ 16       
          
          LDR_STD_IMM12 equ 0 ;;->
          LDR_STD_SCALED equ 1  ;;->
          LDR_EXT_LOHI8BIT equ 2 ;;->
          LDR_EXT_RNMD equ 3 ;; :: ldrtype
        
          pre&link:
          
          IF ldrtype eq LDR_STD_IMM12 or (ldrtype eq LDR_STD_SCALED)
            IF (link and LDR_L) eq 0
              OUTd "STR"         
            ELSEIF (link and LDR_L) ne 0
              OUTd "LDR"
            ENDIF
            IF (link and LDR_B) ne 0
              OUTd "B"
            ENDIF 
            IF  ((link and LDR_W) ne 0) and ((link and LDR_P) eq 0)
              OUTd "T"
            ENDIF   
          ELSEIF ldrtype eq LDR_EXT_LOHI8BIT or (ldrtype eq LDR_EXT_RNMD)
            IF (link and 5) eq 4 
              OUTd "STRH"
            ELSEIF  (link and 5) ne 4 
              OUTd "LDR"
              IF (link and 5) eq 0 
                OUTd "H"
              ELSEIF  (link and 5) eq 1 
                OUTd "SB"              
              ELSEIF  (link and 5) eq 5 
                OUTd "SH"
              ELSE 
                ERRORS_ASSERT
              ENDIF   
            ELSE 
              ERRORS_ASSERT
            ENDIF    
          ELSE 
            ERRORS_ASSERT
          ENDIF    
          
          IF  (link and LDR_P) ne 0 
            OUTd " R%d, [R%d, ", "SC_INVOL } 12 & 15", "SC_INVOL } 16 & 15"          
          ELSEIF (link and LDR_P) eq 0 
            OUTd " R%d, [R%d], ", "SC_INVOL } 12 & 15", "SC_INVOL } 16 & 15"         
          ENDIF   
          
          IF (link and LDR_U) eq 0
            OUTd "-"
          ENDIF   
          ;; Fetch Source .
                                                  ;; P  U  X1 W X2  
                                                   ;; X1 X2     3:= LDRSH (7)
                                                   ;;        2:= STRHW (1)  
                                                   ;;        1:= LDRSB (6) 
                                                   ;;        0:= LDRUH (5)
          ;; eax:temp value, ecx:rn                                      
          IF ldrtype eq LDR_STD_IMM12
            mov eax, SC_INVOL
            and eax, 0FFFh ;; GetImm12 
            OUTd "#%03X", "eax"
          ELSEIF ldrtype eq LDR_STD_SCALED
            OUTd "R%d, ", "SC_INVOL & 15"
            ShiftRegImm5_WithOutC SC_INVOL, eax, ecx, edx ;; Rm Sft.
          ELSEIF ldrtype eq LDR_EXT_LOHI8BIT
            mov eax, SC_INVOL 
            shl al, 4 
            shr eax, 4 
            and eax, 255                   ;; LO|HI 
            OUTd "#%02X", "eax"    
          ELSEIF ldrtype eq LDR_EXT_RNMD       
            GetRFIV SC_INVOL,  0, eax ;; GetRm
            OUTd "R%d", "SC_INVOL & 15"
          ELSE 
            ERRORS_ASSERT
          ENDIF 
          
          IF  (link and LDR_P) ne 0 
            OUTd "]"          
            IF  (link and LDR_W) ne 0 
              OUTd "!" 
            ENDIF       
          ENDIF 
          
          GetRFI SC_INVOL,  16, ecx ;; GetRn
          GetRFI SC_INVOL,  12, SC_INVOL ;; GetRd
          
          IF (link and LDR_U) eq 0 
            neg eax 
          ENDIF 
           
          ;; Post-Index?? 
          IF (link and LDR_P) eq 0
            mov edx, ZRS (ecx) 
            add eax, edx 
          ELSEIF (link and LDR_P) ne 0 
            mov edx, eax 
            add edx, ZRS (ecx)
            mov eax, edx 
          ENDIF 
          
          IF (((link and LDR_P) eq 0) or ((link and LDR_W) ne 0))
            mov ZRS (ecx), eax 
          ENDIF 
          
          ;; Check Access type 
          IF ldrtype eq LDR_STD_IMM12 or (ldrtype eq LDR_STD_SCALED)
            IF (link and LDR_L) eq 0 
              ;; STR. STRB
              push ZRS (SC_INVOL)
              push edx 
              push [SC_ARM7].agb 
              IF (link and LDR_B) ne 0              
                CALLIt MmuWriteByteNoSeq             
              ELSEIF  (link and LDR_B) eq 0 
                ;; Is R15?? + PCSTOREOFFSET - 8
                cmp SC_INVOL, 15 
                jne @F 
                add PTR32[esp+8], 4
              @@:
                CALLIt MmuWriteWordNoSeq
              ENDIF  
              mov [SC_ARM7].nextNoSeqFetch, 1
              Add_WaitStateClks
              ARM7_ExitAddPC 2 ;; 2 noSEQ clks. 
            ELSEIF  (link and LDR_L) ne 0 
              ;; LDR. LDRB 
              GamePAK_Prefetch 1
              push edx 
              push [SC_ARM7].agb 
              IF (link and LDR_B) ne 0   
                CALLIt MmuReadByteNoSeq  
                movzx eax, al 
              ELSEIF  (link and LDR_B) eq 0 
                mov [SC_ARM7].calc, edx 
                CALLIt MmuReadWordNoSeq
                mov ecx, [SC_ARM7].calc
                and ecx, 3
                shl ecx, 3
                ror eax, cl
              ENDIF  
              Add_WaitStateClks
              mov ZRS (SC_INVOL), eax
              cmp SC_INVOL, 15 
              je @F 
              
              ARM7_ExitAddPC 3 ;; S+N+1 3Clks. 
            @@: 
              and ZRS (SC_INVOL), -4
              arm7_FlushPipeline 5      
            ENDIF 
          ELSEIF ldrtype eq LDR_EXT_LOHI8BIT or (ldrtype eq LDR_EXT_RNMD)
            IF (link and 5) eq 4 
              push ZRS (SC_INVOL)
              push edx 
              CallMemOrIOAddWaitState MmuWriteHalfWordNoSeq 
              mov [SC_ARM7].nextNoSeqFetch, 1
              ARM7_ExitAddPC 2 ;; 2 noSEQ clks. 
            ELSEIF  (link and 5) ne 4 
              push edx 
              push [SC_ARM7].agb
              GamePAK_Prefetch 1
              IF (link and 5) eq 0 
                mov [SC_ARM7].calc, edx 
                CALLIt MmuReadHalfWordNoSeq
                mov ecx, [SC_ARM7].calc
                and eax, 0FFFFh
                and ecx, 1 
                shl ecx, 3
                ror eax, cl 
              ELSEIF  (link and 5) eq 1 
                CALLIt MmuReadByteNoSeq
                movsx eax, al              
              ELSEIF  (link and 5) eq 5 
                mov [SC_ARM7].calc, edx
                CALLIt MmuReadHalfWordNoSeq
                mov ecx, [SC_ARM7].calc  
                mov edx, eax 
                movsx eax, ax 
                and ecx, 1      
                je @F 
                movsx eax, dh 
              @@:       
              ELSE 
                ERRORS_ASSERT
              ENDIF 
              Add_WaitStateClks
              mov ZRS (SC_INVOL), eax
              cmp SC_INVOL, 15 
              je @F 
              ARM7_ExitAddPC 3 ;; S+N+1 3Clks. 
            @@: 
              and ZRS (SC_INVOL), -4
              arm7_FlushPipeline 5  
            ENDIF    
          ELSE 
            ERRORS_ASSERT
          ENDIF      
        endm 