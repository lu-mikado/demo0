      TB_LDR equ 0 
      TB_LDRH equ 1 
      TB_LDRSH equ 2 
      TB_RESET equ 7

      STRsc macro Routine 
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        mov edx, SC_INVOL
        and eax, 7 
        shr ecx, 3 
        and ecx, 7 
        shr edx, 6 
        and edx, 7 
        OUTd "R%d, [R%d, R%d]", "eax", "ecx", "edx"
        mov ecx, ZRS (ecx)
        mov eax, ZRS (eax)      
        add ecx, ZRS (edx)  
        push eax 
        push ecx 
        CallMemOrIOAddWaitState Routine 
        mov [SC_ARM7].nextNoSeqFetch, 1 ;; STR always NoSeq fetch in next fetch.
        tb_ExitAddPC 2
      endm 
      LDRsc macro MemAccessType, Routine, MovSt, rhs, alignBlk
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        mov edx, SC_INVOL
        and eax, 7 
        shr ecx, 3 
        and ecx, 7 
        shr edx, 6 
        and edx, 7 
        OUTd "R%d, [R%d, R%d]", "eax", "ecx", "edx"
        mov ecx, ZRS (ecx)
        mov SC_INVOL, eax     
        add ecx, ZRS (edx)  
        mov [SC_ARM7].calc, ecx 
        ;; and ecx, alignBlk     
        push ecx   
        CallMemOrIOAddWaitState Routine 
        mov ecx, [SC_ARM7].calc
        
        IF MemAccessType eq TB_RESET
          MovSt eax, rhs
        ELSEIF MemAccessType eq TB_LDR
          and ecx, 3
          shl ecx, 3 
          ror eax, cl 
        ELSEIF MemAccessType eq TB_LDRH  
          and eax, 0FFFFh
          and ecx, 1
          shl ecx, 3 
          ror eax, cl
        ELSEIF MemAccessType eq TB_LDRSH 
          mov edx, eax 
          movsx eax, ax 
          and ecx, 1
          je @F 
          movsx eax, dh 
        @@:
        ELSE 
          ERRORS_ASSERT
        ENDIF
        mov ZRS (SC_INVOL), eax 
        GamePAK_Prefetch 1
        tb_ExitAddPC 3 
      endm 
      
      STRWD:   
        OUTd "STR "
        STRsc MmuWriteWordNoSeq
      STRHW:
        OUTd "STRH "
        STRsc MmuWriteHalfWordNoSeq  
      STRUB: 
        OUTd "STRB "
        STRsc MmuWriteByteNoSeq 
      LDRWD: 
        OUTd "LDR "
        LDRsc TB_LDR, MmuReadWordNoSeq, mov, eax, 0FFFFFFFCh
      LDHW2: 
        OUTd "LDRH "
        LDRsc TB_LDRH, MmuReadHalfWordNoSeq, movzx, ax, 0FFFFFFFEh 
      LDRSW: 
        OUTd "LDRSH "
        LDRsc TB_LDRSH, MmuReadHalfWordNoSeq, movsx, ax, 0FFFFFFFEh 
      LDRUB:
        OUTd "LDRB "
        LDRsc TB_RESET, MmuReadByteNoSeq, movzx, al, 0FFFFFFFFh
      LDRSB: 
        OUTd "LDRSB "
        LDRsc TB_RESET, MmuReadByteNoSeq, movsx, al, 0FFFFFFFFh
       
      ;; encode: d0-d2 rd, d3-d5 rn, r6-r10 imm5*sizeof(bitdepth/8)      
      ADR5 macro SftBit ;; eax:rdi, edx:address
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        mov edx, SC_INVOL
        and eax, 7 ;; Rd
        shr ecx, 3 
        and ecx, 7 ;; Rn 
        shr edx, 6
        and edx, 31 ;; Imm5  
        shl edx, SftBit    
        OUTd "R%d, [R%d, %d]", "eax", "ecx", "edx"
        add edx, ZRS (ecx)     
        mov [SC_ARM7].calc, edx 
      endm 
      STR5 macro SftBit, CallRoutine 
        ADR5 SftBit
        push ZRS (eax) 
        push edx 
        CallMemOrIOAddWaitState CallRoutine
        mov [SC_ARM7].nextNoSeqFetch, 1
        tb_ExitAddPC 2
      endm 
      LDR5 macro MemAccessType, SftBit, BitLimit, CallRoutine 
        ADR5 SftBit 
        mov SC_INVOL, eax
        push edx 
        CallMemOrIOAddWaitState CallRoutine
        mov ecx, [SC_ARM7].calc
        IF MemAccessType eq TB_LDR 
          and ecx, 3
          shl ecx, 3 
          ror eax, cl
        ELSEIF MemAccessType eq TB_LDRH
          and eax, 0FFFFh
          and ecx, 1
          shl ecx, 3 
          ror eax, cl 
        ELSEIF MemAccessType eq TB_RESET 
          movzx eax, al         
        ENDIF 
        mov ZRS (SC_INVOL), eax 
        GamePAK_Prefetch 1
        tb_ExitAddPC 3
      endm 

      LDSTR1315 macro lSym
       lSym:
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        mov edx, SC_INVOL
        and eax, 255 ;; Imm8 
        shr ecx, 8
        and ecx, 7 ;; Rd 
        shl eax, 2 ;;;;4 address temp 
      endm 
      
      STRW5: 
        OUTd "STR "
        STR5 2, MmuWriteWordNoSeq
      STRH5: 
        OUTd "STRH "
        STR5 1, MmuWriteHalfWordNoSeq
      STRB5: 
        OUTd "STRB "
        STR5 0, MmuWriteByteNoSeq
      LDRW5: ;;=======================DEBUG
        OUTd "LDR "
        LDR5 TB_LDR, 2, 0FFFFFFFFh, MmuReadWordNoSeq
      LDRH5: 
        OUTd "LDRH "
        LDR5 TB_LDRH, 1, 0FFFFh, MmuReadHalfWordNoSeq
      LDRB5: 
        OUTd "LDRB "
        LDR5 TB_RESET, 0, 0FFh, MmuReadByteNoSeq
        
      LDSTR1315 LDRPC
        OUTd "LDR R%d, [PC, #%d-%03X]", "ecx", "eax", "eax"    
        mov edx, ZRS (SZ_PC) 
        and edx, -4 
        add eax, edx 
        mov SC_INVOL, ecx 
        push eax 
        mov [SC_ARM7].calc, eax
        CallMemOrIOAddWaitState MmuReadWordNoSeq
        mov ecx, [SC_ARM7].calc 
        and ecx, 3 
        shl ecx, 3 
        ror eax, cl 
        mov ZRS (SC_INVOL), eax 
        GamePAK_Prefetch 1
        tb_ExitAddPC 3 
      LDSTR1315 LDRSP
        OUTd "LDR R%d, [SP, #%d-%03X]", "ecx", "eax", "eax"
        add eax, ZRS (SZ_STACK) 
        mov SC_INVOL, ecx  
        push eax 
        mov [SC_ARM7].calc, eax
        CallMemOrIOAddWaitState MmuReadWordNoSeq
        mov ecx, [SC_ARM7].calc 
        and ecx, 3 
        shl ecx, 3 
        ror eax, cl 
        mov ZRS (SC_INVOL), eax 
        GamePAK_Prefetch 1
        tb_ExitAddPC 3           
      LDSTR1315 STRSP
        OUTd "STR R%d, [SP, #%d-%03X]", "ecx", "eax", "eax"
        add eax, ZRS (SZ_STACK)  
        push ZRS (ecx)
        push eax 
        CallMemOrIOAddWaitState MmuWriteWordNoSeq
        mov [SC_ARM7].nextNoSeqFetch, 1
        tb_ExitAddPC 2 