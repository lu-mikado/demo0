      ;; JMP Sign Offset 11bit. @!SMARK. 
      JMP11:
        and SC_INVOL, 07FFh 
        bt SC_INVOL, 10 
        sbb eax, eax 
        shl eax, 10 
        or SC_INVOL, eax 
        shl SC_INVOL, 1
        OUTd ":JMP 11 Offset %d-%08X ", "SC_INVOL", "SC_INVOL"
        add ZRS (SZ_PC), SC_INVOL 
        OUTd ":CurrentPC %d-%08X", "ZF", "ZF"
        tb_FlushPipeline 2 
IF 0
      ;; BL Ext 
      BLEXT:
        ;; Fetch Ext High   
        ;; int 3
        and SC_INVOL, 07FFh 
        bt SC_INVOL, 10 
        sbb eax, eax 
        shl eax, 10 
        or SC_INVOL, eax 
        shl SC_INVOL, 12
        add SC_INVOL, ZRS(SZ_PC)
        mov ZRS (SZ_LRLINK), SC_INVOL 
        ;; Fetch Next Opcode Must be BLSTD (TODO: Opcode Check) 
        mov eax, [SC_ARM7].Opcode[0]
        and eax, 07FFh
        shl eax, 1 
        add eax, ZRS (SZ_LRLINK) 
        OUTd ":BL Offset %d-%08X ", "ZE", "ZE"
        mov ecx, ZRS (SZ_PC) 
        mov ZRS (SZ_PC), eax 
        OUTd ":CurrentPC %d-%08X ", "ZF", "ZF"
        sub ecx, 0 ;; backup to next instruction base current instruction 
        or ecx, 1 ;; With Thumb flags, for BX 
        OUTd ":LINK %d-%08X ", "ecx", "ecx"
        mov ZRS (SZ_LRLINK) ,ecx 
        ;; prefetch opcode .
        tb_FlushPipeline 3
      BLSTD: ;;(in fact, with BLEXT).
ELSE
      ;; BL Ext 
      BLEXT:
        ;; Fetch Ext High   
        ;; int 3
        and SC_INVOL, 07FFh 
        bt SC_INVOL, 10 
        sbb eax, eax 
        shl eax, 10 
        or SC_INVOL, eax 
        shl SC_INVOL, 12
        add SC_INVOL, ZRS(SZ_PC)
        mov ZRS (SZ_LRLINK), SC_INVOL 
        tb_ExitAddPC 1
      BLSTD: ;;(in fact, with BLEXT).
        mov eax, SC_INVOL
        and eax, 07FFh
        shl eax, 1 
        add eax, ZRS (SZ_LRLINK) 
        OUTd ":BL Offset %d-%08X ", "ZE", "ZE"
        mov ecx, ZRS (SZ_PC) 
        mov ZRS (SZ_PC), eax 
        OUTd ":CurrentPC %d-%08X ", "ZF", "ZF"
        sub ecx, 2 ;; backup to next instruction base current instruction 
        or ecx, 1 ;; With Thumb flags, for BX 
        OUTd ":LINK %d-%08X ", "ecx", "ecx"
        mov ZRS (SZ_LRLINK) ,ecx 
        ;; prefetch opcode .
        tb_FlushPipeline 3
ENDIF 
   
      
      
      
      UNDEF:
        int 3 
      BX_TB:
        mov eax, SC_INVOL 
        and eax, 078h
        shr eax, 3 
        OUTd "BX R%d", "eax"
        mov ecx, ZRS (eax)
        test ecx, 1 
        je @F 
        or SC_CPSR, FLAG_THUMB
        and ecx, -2 
        mov ZRS (SZ_PC), ecx 
        tb_FlushPipeline 3
      @@:
        and SC_CPSR, not FLAG_THUMB
        and ecx, -4
        mov ZRS (SZ_PC), ecx 
        arm7_FlushPipeline 3