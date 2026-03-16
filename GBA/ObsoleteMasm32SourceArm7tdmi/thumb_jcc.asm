      ;; Thumb JCC -----------------------------------------------------------------
      JCCEQ:
        OUTd "JCC:EQ "
        test SC_CPSR, FLAG_Z
        jne JCCAL
      JCCNV:
        OUTd  ":Skip "
        tb_ExitAddPC 1
      JCCNE:  
        OUTd "JCC:NE "
        test SC_CPSR, FLAG_Z
        je JCCAL 
        tb_ExitAddPC 1    
      JCCCS: 
        OUTd "JCC:CS "
        test SC_CPSR, FLAG_C
        jne JCCAL
        jmp JCCNV
      JCCCC:
        OUTd "JCC:CC " 
        test SC_CPSR, FLAG_C
        je JCCAL
        jmp JCCNV
      JCCMI:      
        OUTd "JCC:MI "
        test SC_CPSR, FLAG_N
        jne JCCAL  
        jmp JCCNV       
      JCCPL:
        OUTd "JCC:PL "
        test SC_CPSR, FLAG_N
        je JCCAL
        jmp JCCNV
      JCCVS:
        OUTd "JCC:VS "
        test SC_CPSR, FLAG_V
        jne JCCAL
        jmp JCCNV
      JCCVC: 
        OUTd "JCC:VC "
        test SC_CPSR, FLAG_V
        je JCCAL 
        jmp JCCNV
      JCCHI: ;; C = 1 && Z = 0
        OUTd "JCC:HI "
        mov ecx, SC_CPSR
        and ecx, FLAG_CZ 
        cmp ecx, FLAG_C
        je JCCAL
        jmp JCCNV
      JCCLS: ;; C = 0 || Z = 1
        OUTd "JCC:LS "
        mov ecx, SC_CPSR
        and ecx, FLAG_CZ 
        ;; C0Z1 ^ C1Z0 -> C1Z1 
        ;; C1Z1 ^ C1Z0 -> C0Z1 
        ;; C0Z0 ^ C1Z0 -> C1Z0 
        ;; C1Z0 ^ C1Z0 -> ZERO.
        xor ecx, FLAG_C
        jne JCCAL
        jmp JCCNV
        
      JCCGE: ;; N = V 
        OUTd "JCC:GE "
        mov eax, SC_CPSR
        mov ecx, SC_CPSR
        shr eax, FLAG_V_TOLSB_BIT
        shr ecx, FLAG_N_TOLSB_BIT   
        and eax, 1
        and ecx, 1 
        xor eax, ecx 
        je JCCAL
        jmp JCCNV
      JCCLT:  ;; N != V 
        OUTd "JCC:LT "
        mov eax, SC_CPSR
        mov ecx, SC_CPSR
        shr eax, FLAG_V_TOLSB_BIT
        shr ecx, FLAG_N_TOLSB_BIT   
        and eax, 1
        and ecx, 1 
        xor eax, ecx 
        jne JCCAL
        jmp JCCNV  
      JCCGT: ;;    N = V  && Z= 0
        OUTd "JCC:GT "
        mov eax, SC_CPSR
        mov ecx, SC_CPSR
        shr eax, FLAG_V_TOLSB_BIT
        shr ecx, FLAG_N_TOLSB_BIT   
        and eax, 1
        and ecx, 1 
        xor eax, ecx 
        mov ecx, SC_CPSR
        shr ecx, FLAG_Z_TOLSB_BIT
        and ecx, 1 
        or eax, ecx 
        je JCCAL
        jmp JCCNV
      JCCLE: ;;  Z = 1 || N!=V
        OUTd "JCC:LE "
        mov eax, SC_CPSR
        mov ecx, SC_CPSR
        shr eax, FLAG_V_TOLSB_BIT
        shr ecx, FLAG_N_TOLSB_BIT   
        and eax, 1
        and ecx, 1 
        xor eax, ecx 
        mov ecx, SC_CPSR
        shr ecx, FLAG_Z_TOLSB_BIT
        and ecx, 1 
        or eax, ecx 
        je JCCNV 
      JCCAL: ;; 1110 
        OUTd "Exec It.:"
        mov eax, SC_INVOL
        and eax, 255 
        mov [esp-8], eax 
        movsx eax, byte ptr [esp-8]
        OUTd ":Offset %d-%08X ", "eax", "eax"
        shl eax, 1 
        add ZRS (SZ_PC), eax
        OUTd ":CurrentPC %d-%08X", "ZF", "ZF"
        tb_FlushPipeline 2    