
      fEQ:
        OUTd "Cond:EQ "
        test SC_CPSR, FLAG_Z
        jne fAL
      fNV:
        OUTd  ":Skip "
        ARM7_ExitAddPC 1
      fNE:  
        OUTd "Cond:NE "
        test SC_CPSR, FLAG_Z
        je fAL 
        ARM7_ExitAddPC 1    
      fCS: 
        OUTd "Cond:CS "
        test SC_CPSR, FLAG_C
        jne fAL
        jmp fNV
      fCC:
        OUTd "Cond:CC " 
        test SC_CPSR, FLAG_C
        je fAL
        jmp fNV
      fMI:      
        OUTd "Cond:MI "
        test SC_CPSR, FLAG_N
        jne fAL  
        jmp fNV       
      fPL:
        OUTd "Cond:PL "
        test SC_CPSR, FLAG_N
        je fAL
        jmp fNV
      fVS:
        OUTd "Cond:VS "
        test SC_CPSR, FLAG_V
        jne fAL
        jmp fNV
      fVC: 
        OUTd "Cond:VC "
        test SC_CPSR, FLAG_V
        je fAL 
        jmp fNV
      fHI: ;; C = 1 && Z = 0
        OUTd "Cond:HI "
        mov ecx, SC_CPSR
        and ecx, FLAG_CZ 
        cmp ecx, FLAG_C
        je fAL
        jmp fNV
      fLS: ;; C = 0 || Z = 1
        OUTd "Cond:LS "
        mov ecx, SC_CPSR
        and ecx, FLAG_CZ 
        ;; C0Z1 ^ C1Z0 -> C1Z1 
        ;; C1Z1 ^ C1Z0 -> C0Z1 
        ;; C0Z0 ^ C1Z0 -> C1Z0 
        ;; C1Z0 ^ C1Z0 -> ZERO.
        xor ecx, FLAG_C
        jne fAL
        jmp fNV
        
      fGE: ;; N = V 
        OUTd "Cond:GE "
        mov eax, SC_CPSR
        mov ecx, SC_CPSR
        shr eax, FLAG_V_TOLSB_BIT
        shr ecx, FLAG_N_TOLSB_BIT   
        and eax, 1
        and ecx, 1 
        xor eax, ecx 
        je fAL
        jmp fNV
      fLT:  ;; N != V 
        OUTd "Cond:LT "
        mov eax, SC_CPSR
        mov ecx, SC_CPSR
        shr eax, FLAG_V_TOLSB_BIT
        shr ecx, FLAG_N_TOLSB_BIT   
        and eax, 1
        and ecx, 1 
        xor eax, ecx 
        jne fAL
        jmp fNV  
      fGT: ;;    N = V  && Z= 0
        OUTd "Cond:GT "
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
        je fAL
        jmp fNV
      fLE: ;;  Z = 1 || N!=V
        OUTd "Cond:LE "
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
        je fNV  
      fAL: ;; 1110 
        mov ecx, SC_INVOL 
        mov eax, SC_INVOL
        mov edx, SC_INVOL 
        and edx, -1 
        rol ecx,  8
        nop
        rol cx, 4 
        nop 
        and ecx, 0FFh 
        nop 
        jmp jTAB[ecx*4]