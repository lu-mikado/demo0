      ALUOP:
        SRC_cx equ ecx
        DST_i equ edx
        DST equ ZRS (DST_i)
        TMPR equ eax 
        TMPR8 equ al
        ;; ALU Unwind ------------------------
        GetRFI_T SC_INVOL, 0, DST_i  ;;Rd's Index . 
        GetRFIV_T SC_INVOL, 3, SRC_cx  ;;Rs/Rm/Rn
        mov TMPR, SC_INVOL
        shr TMPR, 6 
        and TMPR, 15 
        jmp taTAB[TMPR*4]
      taAND:
        OUTd "AND R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        and DST, SRC_cx 
        SetNZ_A 
        tb_ExitAddPC 1
      taEOR:
        OUTd "EOR R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        xor DST, SRC_cx 
        SetNZ_A 
        tb_ExitAddPC 1    
      taORR:
        OUTd "ORR R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        or DST, SRC_cx 
        SetNZ_A 
        tb_ExitAddPC 1    
      taADC:
        OUTd "ADC R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        bt SC_CPSR, FLAG_CHECK_C_X86_BT 
        adc DST, SRC_cx 
        SetNZCV_A 0 
        tb_ExitAddPC 1
      taSBC:
        OUTd "SBC R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        xor SC_CPSR, FLAG_C
        bt SC_CPSR, FLAG_CHECK_C_X86_BT 
        sbb DST, SRC_cx 
        SetNZCV_A 1 
        tb_ExitAddPC 1
      taCMP: 
        OUTd "CMP R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        cmp DST, SRC_cx
        SetNZCV_A 1 
        tb_ExitAddPC 1  
      taCMN:
        OUTd "CMN R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        add SRC_cx, DST
        SetNZCV_A 0
        tb_ExitAddPC 1 
      taTST:
        OUTd "TST R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        test DST, SRC_cx
        SetNZ_A
        tb_ExitAddPC 1
      taNEG:
        mov DST, 0
        sub DST, SRC_cx 
        SetNZCV_A 1 
        tb_ExitAddPC 1  
      taBIC:
        OUTd "BIC R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        not SRC_cx 
        and DST, SRC_cx
        SetNZ_A 
        tb_ExitAddPC 1      
      taMVN:
        OUTd "MVN R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        xor SRC_cx, -1
        mov DST, SRC_cx
        SetNZ_A  
        tb_ExitAddPC 1              
      taMUL:
        OUTd "MUL R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        movd xmm0, SRC_cx 
        movd xmm1, DST 
        pmuludq xmm0, xmm1 
        movd TMPR, xmm0 
        mov DST, TMPR 
        or TMPR, TMPR 
        SetNZ_A 
        mulClks SRC_cx, DST_i, TMPR ;; @FIXME
        lea SC_INVOL, [TMPR+1]
        GamePAK_Prefetch TMPR
        tb_ExitAddPC SC_INVOL 
      taLSL:
        OUTd "LSL R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        and SRC_cx, 255 
        jne @F 
        ;; ZERO. 
        mov TMPR, DST
        or TMPR, TMPR 
        SetNZ_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@: 
        xor TMPR, TMPR 
        cmp SRC_cx, 32 
        jb @F 
        ;; >= 32      
        mov SRC_cx, DST
        mov DST, TMPR ;; Rd:= 0
        sete TMPR8 ;; 32 1
        and SC_CPSR, not (FLAG_C or FLAG_N)
        and SRC_cx, 1
        and SRC_cx, TMPR ;; and it.  
        shl SRC_cx, FLAG_CHECK_C_X86_BT
        or SC_CPSR, SRC_cx 
        or SC_CPSR, FLAG_Z 
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@:
        shl DST, cl 
        SetNZC_A 
        GamePAK_Prefetch 1
        tb_ExitAddPC 2 
      taLSR:
        OUTd "LSR R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        and SRC_cx, 255 
        jne @F 
        ;; ZERO. 
        mov TMPR, DST
        or TMPR, TMPR 
        SetNZ_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@: 
        xor TMPR, TMPR 
        cmp SRC_cx, 32 
        jb @F 
        ;; >= 32      
        mov SRC_cx, DST
        mov DST, TMPR ;; Rd:= 0
        sete TMPR8 ;; 32 1
        and SC_CPSR, not (FLAG_C or FLAG_N)
        shr SRC_cx, 31
        and SRC_cx, TMPR ;; and it.  
        shl SRC_cx, FLAG_CHECK_C_X86_BT
        or SC_CPSR, SRC_cx 
        or SC_CPSR, FLAG_Z 
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@:
        shr DST, cl 
        SetNZC_A 
        GamePAK_Prefetch 1
        tb_ExitAddPC 2       
      taASR:
        OUTd "ASR R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        and SRC_cx, 255 
        jne @F 
        ;; ZERO. 
        mov TMPR, DST
        or TMPR, TMPR 
        SetNZ_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@: 
        cmp SRC_cx, 32 
        jb @F 
        bt DST, 31 
        sbb SRC_cx, SRC_cx  
        mov DST, SRC_cx 
        SetNZC_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@:
        sar DST, cl 
        SetNZC_A 
        GamePAK_Prefetch 1
        tb_ExitAddPC 2 
      taROR:
        OUTd "ROR R%d, R%d", "SC_INVOL & 7", "SC_INVOL } 3 & 7"
        and SRC_cx, 255 
        jne @F 
        ;; ZERO. 
        mov TMPR, DST
        or TMPR, TMPR 
        SetNZ_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@: 
        test SRC_cx, 31 
        jne @F 
        mov SC_INVOL, DST
        or SC_INVOL, SC_INVOL
        rol SC_INVOL, 1 
        SetNZC_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2
      @@:
        and SRC_cx, 31 
        ror DST, cl 
        SetC_A 
        mov eax, DST
        or eax, eax 
        SetNZ_A
        GamePAK_Prefetch 1
        tb_ExitAddPC 2 