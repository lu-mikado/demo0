      INLSL:
        OUTd "LSL R%d, R%d, #%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 31"
        mov ecx, SC_INVOL
        GetRFIV_T SC_INVOL, 3, edx  ;;Rm.
        GetRFI_T SC_INVOL, 0,  eax  ;;Rd's Index . 
        shr ecx, 6
        and ecx, 31  
        jne @F
        or edx, edx
        mov ZRS (eax), edx 
        SetNZ_A
        tb_ExitAddPC 1
      @@: 
        shl edx, cl
        mov ZRS (eax), edx             
        SetNZC_A  
        tb_ExitAddPC 1      
      INLSR:
        OUTd "LSR R%d, R%d, #%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 31"
        mov ecx, SC_INVOL
        GetRFIV_T SC_INVOL, 3, edx  ;;Rm.
        GetRFI_T SC_INVOL, 0,  eax  ;;Rd's Index . 
        shr ecx, 6
        and ecx, 31  
        jne @F
        and SC_CPSR, not (FLAG_Z or FLAG_N or FLAG_C)
        or SC_CPSR, FLAG_Z
        shl edx, 1
        mov ZRS (eax), 0 
        SetC_A
        tb_ExitAddPC 1
      @@: 
        shr edx, cl
        mov ZRS (eax), edx             
        SetNZC_A  
        tb_ExitAddPC 1     
      INASR:
        OUTd "ASR R%d, R%d, #%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 31"
        mov ecx, SC_INVOL
        GetRFIV_T SC_INVOL, 3, edx  ;;Rm.
        GetRFI_T SC_INVOL, 0,  eax  ;;Rd's Index . 
        shr ecx, 6
        and ecx, 31  
        jne @F
        shl edx, 1 
        sbb ecx, ecx  ;; 0 or -1 
        mov ZRS (eax), ecx           
        SetNZC_A
        tb_ExitAddPC 1
      @@: 
        sar edx, cl
        mov ZRS (eax), edx             
        SetNZC_A  
        tb_ExitAddPC 1
      ADDI3:
        OUTd "ADD R%d, R%d, #%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 7"
        GetRFIV_T SC_INVOL, 3, edx  ;;Rn.
        GetRFI_T SC_INVOL, 0, eax  ;;Rd's Index .  
        GetRFI_T SC_INVOL, 6, ecx  ;;Imm3 
        add edx, ecx  
        mov ZRS (eax), edx 
        SetNZCV_A 0
        tb_ExitAddPC 1
      SUBI3:
        OUTd "SUB R%d, R%d, #%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 7"
        GetRFIV_T SC_INVOL, 3, edx  ;;Rn.
        GetRFI_T SC_INVOL, 0, eax  ;;Rd's Index .  
        GetRFI_T SC_INVOL, 6, ecx  ;;Imm3 
        sub edx, ecx  
        mov ZRS (eax), edx 
        SetNZCV_A 1
        tb_ExitAddPC 1    
      ADDRT:
        OUTd "ADD R%d, R%d, R%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 7"
        GetRFIV_T SC_INVOL, 3, edx  ;;Rn.
        GetRFI_T SC_INVOL, 0, eax  ;;Rd's Index .  
        GetRFIV_T SC_INVOL, 6, ecx  ;;Rm 
        add edx, ecx  
        mov ZRS (eax), edx 
        SetNZCV_A 0
        tb_ExitAddPC 1
      SUBRT:
        OUTd "SUB R%d, R%d, R%d", "SC_INVOL & 7", "SC_INVOL }3 & 7", "SC_INVOL } 6 & 7"
        GetRFIV_T SC_INVOL, 3, edx  ;;Rn.
        GetRFI_T SC_INVOL, 0, eax  ;;Rd's Index .  
        GetRFIV_T SC_INVOL, 6, ecx  ;;Rm 
        sub edx, ecx  
        mov ZRS (eax), edx 
        SetNZCV_A 1
        tb_ExitAddPC 1     
      MOVI8:
        OUTd "MOV R%d, #%d", "SC_INVOL }8 & 7", "SC_INVOL & 255"
        GetRFI_T SC_INVOL, 8, ecx  ;;Rd's Index .  
        and SC_INVOL, 255 
        mov ZRS (ecx), SC_INVOL
        SetNZ_A 
        tb_ExitAddPC 1    
      CMPI8:
        OUTd "CMP R%d, #%d", "SC_INVOL }8 & 7", "SC_INVOL & 255" 
        GetRFIV_T SC_INVOL, 8, ecx  ;;Rn's Value .  
        and SC_INVOL, 255 
        cmp ecx, SC_INVOL
        SetNZCV_A 1 
        tb_ExitAddPC 1
      ADDI8:
        OUTd "ADD R%d, #%d", "SC_INVOL }8 & 7", "SC_INVOL & 255"
        GetRFI_T SC_INVOL, 8, ecx  ;;Rd's Index .  
        and SC_INVOL, 255 
        add ZRS (ecx), SC_INVOL
        SetNZCV_A 0 
        tb_ExitAddPC 1      
      SUBI8:
        OUTd "SUB R%d, #%d", "SC_INVOL }8 & 7", "SC_INVOL & 255"
        GetRFI_T SC_INVOL, 8, ecx  ;;Rd's Index .  
        and SC_INVOL, 255 
        sub ZRS (ecx), SC_INVOL
        SetNZCV_A 1
        tb_ExitAddPC 1 
        
      ih1h2Get macro Post, RdRev, RmRev 
        mov RmRev, Post
        bt Post, 7
        sbb RdRev, RdRev 
        and RdRev, 8
        and RmRev, 7 
        or RdRev, RmRev
        mov RmRev, Post
        and RmRev, 078h
        shr RmRev, 3  ;; TODO: Check LO-LO Trans UB.
      endm 
      
      ADD16:
        ih1h2Get SC_INVOL, ecx, edx 
        OUTd "ADD R%d, R%d", "ecx", "edx"
        mov edx, ZRS (edx) 
        add ZRS (ecx), edx         
        cmp ecx, 15 
        jne @F
        and ZRS (ecx), -2 
        tb_FlushPipeline 3 
      @@:        
        tb_ExitAddPC 1
      MOV16:
        ih1h2Get SC_INVOL, ecx, edx 
        OUTd "MOV R%d, R%d", "ecx", "edx"
        mov edx, ZRS (edx) 
        mov ZRS (ecx), edx   
        cmp ecx, 15 
        jne @F 
        and ZRS (ecx), -2 
        tb_FlushPipeline 3
      @@:        
        tb_ExitAddPC 1 
      CMP16:
        ih1h2Get SC_INVOL, ecx, edx 
        OUTd "CMP R%d, R%d", "ecx", "edx"
        mov edx, ZRS (edx) 
        cmp ZRS (ecx), edx       
        SetNZCV_A 1        
        tb_ExitAddPC 1

      SIM08 macro lSym, SZ_BODY
       lSym:
        mov eax, SC_INVOL
        mov ecx, SC_INVOL
        mov edx, ZRS (SZ_BODY) ;; edx:PC or SP 
        and eax, 255 ;; Imm8 
        shr ecx, 8
        and ecx, 7 ;; ecx: RdI
        shl eax, 2 ;; eax: address temp  
        IF SZ_BODY eq SZ_STACK 
          OUTd "ADD R%d, SP, %d-%03X", "ecx", "eax", "eax"
        ELSEIF SZ_BODY eq SZ_PC 
          OUTd "ADD R%d, PC, %d-%03X", "ecx", "eax", "eax"
        ELSE 
          ERRORS_ASSERT
        ENDIF
      endm 
      
      SIM08 PCI08, SZ_PC
        and edx, -4 
        add edx, eax 
        mov ZRS (ecx), edx 
        tb_ExitAddPC 1
      SIM08 SPI08, SZ_STACK
        add edx, eax 
        mov ZRS (ecx), edx 
        tb_ExitAddPC 1     
      STKS7:
        test SC_INVOL, 080h 
        je @F 
        and SC_INVOL, 07Fh 
        shl SC_INVOL, 2 
        OUTd "SUB SP, #%d", "SC_INVOL"
        sub ZRS (SZ_STACK), SC_INVOL     
        tb_ExitAddPC 1
      @@:
        and SC_INVOL, 07Fh 
        shl SC_INVOL, 2 
        OUTd "ADD SP, #%d", "SC_INVOL"
        add ZRS (SZ_STACK), SC_INVOL
        tb_ExitAddPC 1