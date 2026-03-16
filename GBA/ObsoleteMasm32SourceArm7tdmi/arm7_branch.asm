        ;;  ---------------------------------------------------------------------------------------------        
        ;;  27 26 25 24 23 22 21 20   19 - 16     15 - 12       11 10 9 8  7 6 5 4  3 2 1 0     
        ;;   0  0  0  1  0  0  1  0   1 1 1 1     1 1 1 1        1  1 1 1  0 0 0 1    Rn   branch exchange
        @ALIGN_Z 
        rsBX: ;; BX Rn     
          GetRFIV SC_INVOL, 0, eax
          OUTd "BX R%d:%08X- PC:%08X  \n", "SC_INVOL & 15", "eax","ZF-8"
          test eax, 1 
          jne @F 
          and eax, -4
          mov ZRS (SZ_PC), eax
          arm7_FlushPipeline 3
        @@:
          and eax, -2 
          or SC_CPSR, FLAG_THUMB ;; set thumb flag 
          and eax, -2
          mov ZRS (SZ_PC), eax
          tb_FlushPipeline 3 
        rsUB:   
          int 3  
        ;;  ------------------------- branch clks:3 2Seq + 1N Cycles 
        ;;  27 26 25 24 23 22 21 20   19 - 16     15 - 12       11 10 9 8  7 6 5 4  3 2 1 0 
        ;;   1  0  1  L          Sign Offset 24  
        JMP24: ;; without LR
          and edx, 0FFFFFFh
          bt edx, 23
          sbb ecx, ecx 
          and ecx, 03F000000h  ;; save 30 bit 
          or ecx, edx
          shl ecx, 2 ;; shift 2  
          OUTd "B %06X- PC:%08X LR:%08X \n", "ZF+ecx",      "ZF-8" , "ZE"
          add ZRS (SZ_PC), ecx      
          arm7_FlushPipeline 3      
        JMPLR: ;; with LR 
          mov SC_INVOL, ZRS (SZ_PC)
          and edx, 0FFFFFFh
          bt edx, 23
          sbb ecx, ecx 
          and ecx, 03F000000h  ;; save 30 bit 
          or ecx, edx
          shl ecx, 2 ;; shift 2  
          OUTd "BL %06X- PC:%08X LR:%08X ", "ZF+ecx",      "ZF-8" , "ZE"
          add ZRS (SZ_PC), ecx   
          lea eax, [SC_INVOL-4]
          OUTd "NEW-LR:%08X \n", "eax"
          mov ZRS (SZ_LRLINK), eax ;; next pc save to LR. 
          arm7_FlushPipeline 3