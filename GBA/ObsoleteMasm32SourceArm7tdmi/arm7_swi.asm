        SWINT: ;; ARM7 Software Interrupt 
          OUTd "SWI-ARM7 FUNC:%02X R0 %08X R1 %08X R2 %08X\n", "SC_INVOL & 0FFFFFFh", "Z0", "Z1", "Z2"
          mov eax, ZRS (SZ_PC) 
          mov ecx, SC_CPSR 
          and ecx, ARM7_MODE_CLR_MASK 
          or ecx, ARM7_MODE_MGR_MASK  ;; Set SVC mode. 
          and ecx, not FLAG_THUMB ;; Clear Thumb exec flag 
          or ecx, IRQ_INHIBI_MASK ;; Set IRQ inhibit mask 
          ;; PC to SVC's LR, LR:= Current Instruction +4 
          lea edx, [eax-4] 
          mov [SC_ARM7].R1314_T[R1314b_MGR+4], edx  
          ;; Adjust PC Pointer to IRQ Interrupt vector address 
          mov ZRS (SZ_PC), ARM7_VECTOR_SOFTWARE
          mov SC_INVOL, SC_CPSR 
          and SC_INVOL, ARM7_MODE_GET_MASK 
          cmp SC_INVOL, ARM7_MODE_MGR_MASK 
          je @NestSwi_ARM7
          ;; switch Mode 
          SwitchMode ecx, 0
          ;; Prefetch Opcode pipeline 
          arm7_FlushPipeline 3
      @NestSwi_ARM7:
          mov ZRS (SZ_LRLINK), edx
          mov [SC_ARM7].SPSR_T[SPSRb_MGR], SC_CPSR
          mov eax, ZRS (SZ_STACK)
          mov [SC_ARM7].R1314_T[R1314b_MGR], eax 
          mov eax, ZRS (SZ_LRLINK)
          mov [SC_ARM7].R1314_T[R1314b_MGR+4], eax 
          mov SC_CPSR, ecx 
          ;; Prefetch Opcode pipeline 
          arm7_FlushPipeline 3
            