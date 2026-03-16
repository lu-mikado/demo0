      A7SWP: ;; SWAP. S+2N+I
        GetRFIV SC_INVOL, 16, eax ;; Rn:= Address 
        GetRFIV SC_INVOL, 0, ecx ;; Rm:     
        mov [SC_ARM7].calc, eax         
        push ecx 
        push eax
        push [SC_ARM7].agb 
        push eax
        push [SC_ARM7].agb 
        test SC_INVOL, 00400000h
        je @F 
        
        A7SWP_INLINE macro rd, wr, msk
          CALLIt rd
          add SC_WAIT, [SC_ARM7].waitState  
          GetRFI SC_INVOL, 12, SC_INVOL ;; Rd:= Rd.
          and eax, msk
          if msk eq 0FFFFFFFFh
            ;; test align rot. 
            mov ecx, [SC_ARM7].calc
            and ecx, 3 
            shl ecx, 3 
            ror eax, cl 
          endif 
          mov ZRS (SC_INVOL), eax                  
          CALLIt wr     
          add SC_WAIT, [SC_ARM7].waitState
          GamePAK_Prefetch 1
          ARM7_ExitAddPC 4      
        endm 
        
        OUTd "SWPB R%d, R%d, [R%d]", "SC_INVOL}12 & 15", "SC_INVOL & 15", "SC_INVOL}16 & 15"
        A7SWP_INLINE MmuReadByteNoSeq, MmuWriteByteNoSeq, 0FFh
      @@:
        OUTd "SWP R%d, R%d, [R%d]", "SC_INVOL}12 & 15", "SC_INVOL & 15", "SC_INVOL}16 & 15"
        A7SWP_INLINE MmuReadWordNoSeq, MmuWriteWordNoSeq, 0FFFFFFFFh