        ;; PSR MASK In V4T Version , UseMask 0xF0000000, PrivMask 0x0000000F, State Mask 0x00000020
        ;; This means that in any case you can't control the interrupt shielding bit field through MSR.
        ;; That sounds really incredible!
        ;; Many books and software examples show that it can be controlled by MSR.
        ;; I really don't know who to trust.
        ;; See ARM Architecture Reference Manual::A4.1.39 MSR   
        
        IFDEF STRICT_MODE_PSR
          ARM7_PSR_CTL_MASK equ ARM7_PSR_MODE4BIT_MASK
        ELSE 
          ARM7_PSR_CTL_MASK equ ARM7_PSR_MODE8BIT_MASK
        ENDIF
        i8r4ToCPSR:      
          Imm8BitmapSft_AC SC_INVOL, eax 
          OUTd "MSR #%08X(org)", "eax"
          jmp @F
        si5ToCPSR: ;; 0   1 MSR Rs  CPSR  TODO:Check d4-d7 := 0
          OUTd "MSR R%d to cpsr_", "SC_INVOL & 15"
          GetRFIV edx, 0, eax   
         @@: 
          ToFastPSR eax, eax, eax,  ecx
          ;; Limit MASK, in fact write will only affect flag and cpu mode's low 4bit in std arm7.
          OUTd " #%08X(fast) to spsr_", "eax"
          mov [SC_ARM7].calc, SC_CPSR ;;;; FIXME:
          mov ecx, SC_CPSR
          and ecx, ARM7_MODE_GET_MASK
          cmp ecx, ARM7_MODE_USER_MASK 
          jne @Privilege_MSR   
          ;; USER Mode MSR.
          ;; Only Affect Flag Bit.
        @ChkSetFLags:
          test edx, FLAG_MSR_FLAGS
          je @F   
          and SC_CPSR, not ARM7_PSR_FLAG_MASK
          and eax, ARM7_PSR_FLAG_MASK
          or SC_CPSR, eax 
        @@:
          test SC_CPSR, FLAG_THUMB
          je @F
          int 3 
        @@:          
          ARM7_ExitAddPC 1
        @Privilege_MSR:  
          test edx, FLAG_MSR_CTL
          je @ChkSetFLags 
      IF 1
         ;; PriMode MSR.
          test edx, FLAG_MSR_FLAGS
          jne @F 
          mov edx, SC_CPSR         
          and edx, not ARM7_PSR_CTL_MASK
          and eax, ARM7_PSR_CTL_MASK
          or eax, edx 
          or eax, ARM7_MODE_MSB_BIT_MASK
          SwitchMode eax, 1
          test SC_CPSR, FLAG_THUMB
          je @Skip
          int 3 
        @Skip:          
          ARM7_ExitAddPC 1
        @@:
          mov SC_INVOL, eax 
          mov edx, SC_CPSR         
          and edx, not ARM7_PSR_CTL_MASK
          and eax, ARM7_PSR_CTL_MASK
          or eax, edx 
          or eax, ARM7_MODE_MSB_BIT_MASK
          SwitchMode eax, 1
          and SC_CPSR, not ARM7_PSR_FLAG_MASK
          and SC_INVOL, ARM7_PSR_FLAG_MASK
          or SC_CPSR, SC_INVOL 
          test SC_CPSR, FLAG_THUMB
          je @F
          int 3 
        @@:          
          ARM7_ExitAddPC 1 
      ELSE 
      ENDIF
        ;; TODO: assert mode, flags
        si5PSR4: ;; 1   1 MSR Rs  SPSR
          OUTd "MSR R%d to spsr_", "SC_INVOL & 15"
          GetRFIV edx, 0, eax 
          ToFastPSR eax, eax, eax,  ecx 
          jmp toSPSR_main
        i8r4ToSPSR:
          Imm8BitmapSft_AC SC_INVOL, eax 
          OUTd "MSR #%08X(org)", "eax"
          ToFastPSR eax, eax, eax,  ecx 
          OUTd " #%08X(fast) to spsr_", "eax"
        toSPSR_main:
          GetCurSPSRP ecx
          test edx, FLAG_MSR_FLAGS
          je @F 
          mov SC_INVOL, eax 
          and PTR32[ecx], not ARM7_PSR_FLAG_MASK
          and SC_INVOL, ARM7_PSR_FLAG_MASK
          or [ecx], SC_INVOL 
          OUTd "f"
          @@:          
          test edx, FLAG_MSR_CTL
          je @F  
          OUTd "c"
          and PTR32[ecx], not (ARM7_PSR_CTL_MASK or ARM7_PSR_STATE_MASK)
          and eax, (ARM7_PSR_CTL_MASK or ARM7_PSR_STATE_MASK)
          or [ecx], eax
          or PTR32[ecx], ARM7_MODE_MSB_BIT_MASK
          @@:
          ARM7_ExitAddPC 1  
          
        si5PSR1: ;; 0   0 MRS Rs  CPSR 
          OUTd "MRS R%d, CPSR", "SC_INVOL}12 & 15"
          ToStandPSR SC_CPSR, ecx, eax,  ecx 
          jmp @F
          
        si5PSR3: ;; 1   0 MRS Rs  SPSR, TODO: Check SBO/SBZ 
          OUTd "MRS R%d, SPSR", "SC_INVOL}12 & 15"
          GetCurSPSRP eax
          mov eax, [eax]
          ToStandPSR eax, ecx, eax,  ecx 
        @@:
          GetRFI edx, 12, eax 
          and ecx, 00F00000FFh
          or ecx, 010h
          mov ZRS (eax), ecx       
          ARM7_ExitAddPC 1  