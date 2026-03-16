          MUL_OP equ 0 
          MLA_OP equ 1
          UMUL64_OP equ 2 
          UMLA64_OP equ 3  
          SMUL64_OP equ 4 
          SMLA64_OP equ 5 

          mulClks macro Post, TReg, Recv ;; XXX: TReg ^  Post ^ Recv
                                         ;; XXX: better case
          
            ;; multiplier's clks, m
            ;; see ARM7TDMI Technical Reference Manual's 6.20 Instruction speed summary
            ;; m is: 
            ;; 1 if bits [31:8] of the multiplier operand (Rs) are all zero or one, else 
            ;; 2 if bits [31:16] of the multiplier operand (Rs) are all zero or one, else 
            ;; 3 if bits [31:24] of the multiplier operand (Rs) are all zero or all one, else 
            ;; 4.
            
            bt Post, 31 
            sbb TReg, TReg 
            xor TReg, Post      
            movd xmm0, TReg 
            pxor xmm1, xmm1 
            pcmpeqb xmm1, xmm0 
            pmovmskb TReg, xmm1 
            shl TReg, 29  
            sbb Recv, Recv 
            shl TReg, 1 
            sbb Post, Post
            shl TReg, 1           
            sbb TReg, TReg
            and Post, Recv 
            and TReg, Post 
            add Recv, TReg 
            lea Recv, [Recv+Post+4]
          endm 
          
          mul_interp_b macro OPID
            GetRFIV SC_INVOL, 0, eax ;; Rm.      
            GetRFIV SC_INVOL, 8, ecx ;; Rs.                  
            IF OPID eq MUL_OP 
              OUTd "MUL R%d, R%d, R%d", "SC_INVOL}16 & 15", "SC_INVOL & 15", "SC_INVOL}8 & 15" 
              mul ecx 
              GetRFI SC_INVOL, 16, edx ;; Rd
              mov ZRS (edx), eax  
            ELSEIF  OPID eq MLA_OP
              OUTd "MLA R%d, R%d, R%d, R%d", "SC_INVOL}16 & 15", "SC_INVOL & 15", "SC_INVOL}8 & 15",  "SC_INVOL}12 & 15"
              mul ecx        
              GetRFIV SC_INVOL, 12, edx ;; Rn 
              add eax, edx 
              GetRFI SC_INVOL, 16, edx ;; Rd
              mov ZRS (edx), eax               
            ELSEIF  OPID eq UMUL64_OP 
              OUTd "UMULL R%d, R%d, R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15",  "SC_INVOL}8 & 15"
              movd xmm0, ecx 
              mul ecx 
              GetRFI SC_INVOL, 12, ecx ;; RdLO 
              GetRFI SC_INVOL, 16, SC_INVOL ;; RdHI           
              mov ZRS (ecx), eax        
              mov ZRS (SC_INVOL), edx
              movd ecx, xmm0
            ELSEIF  OPID eq UMLA64_OP 
              OUTd "UMLAL R%d, R%d, R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15",  "SC_INVOL}8 & 15"
              movd xmm0, ecx 
              mul ecx 
              GetRFI SC_INVOL, 12, ecx ;; RdLO 
              GetRFI SC_INVOL, 16, SC_INVOL ;; RdHI           
              add eax, ZRS (ecx)
              adc edx, ZRS (SC_INVOL)
              mov ZRS (ecx), eax        
              mov ZRS (SC_INVOL), edx
              movd ecx, xmm0            
            ELSEIF  OPID eq SMUL64_OP  
              OUTd "SMULL R%d, R%d, R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15",  "SC_INVOL}8 & 15"            
              movd xmm0, ecx 
              imul ecx 
              GetRFI SC_INVOL, 12, ecx ;; RdLO 
              GetRFI SC_INVOL, 16, SC_INVOL ;; RdHI           
              mov ZRS (ecx), eax        
              mov ZRS (SC_INVOL), edx
              movd ecx, xmm0         
            ELSEIF  OPID eq SMLA64_OP 
              OUTd "SMLAL R%d, R%d, R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15",  "SC_INVOL}8 & 15"
              movd xmm0, ecx 
              imul ecx 
              GetRFI SC_INVOL, 12, ecx ;; RdLO 
              GetRFI SC_INVOL, 16, SC_INVOL ;; RdHI           
              add eax, ZRS (ecx)
              adc edx, ZRS (SC_INVOL)
              mov ZRS (ecx), eax        
              mov ZRS (SC_INVOL), edx
              movd ecx, xmm0             
            ELSE 
              ERRORS_ASSERT
            ENDIF     
          endm 
          mul_interp macro lSym, OPID   
           ;;LOCAL StdLink 
           ;;LOCAL StdLinkWithSymbol 
           LOCAL SetMClks 
           
           m&lSym:
             OUTd "NOSET_SIGN "
             mul_interp_b  OPID 
             jmp SetMClks
           m&lSym&S:
            OUTd "SET_SIGN "
            mul_interp_b OPID              
            IF OPID eq MUL_OP
              Set_NZmul32 eax              
            ELSEIF  OPID eq MLA_OP
              Set_NZmul32 eax                    
            ELSEIF  OPID eq UMUL64_OP 
              Set_NZmul64 eax, edx 
            ELSEIF  OPID eq UMLA64_OP 
              Set_NZmul64 eax, edx            
            ELSEIF  OPID eq SMUL64_OP   
              Set_NZmul64 eax, edx   
            ELSEIF  OPID eq SMLA64_OP 
              Set_NZmul64 eax, edx            
            ELSE 
              ERRORS_ASSERT
            ENDIF    
          SetMClks:
            mulClks ecx, edx, eax 
            
            IF OPID eq MUL_OP 
              ;; MUL := M I Clks.  
            ELSEIF  OPID eq MLA_OP
              ;; MLA := M+1 I Clks
              add eax, 1              
            ELSEIF  OPID eq UMUL64_OP 
              ;; UMULL := M+1 I Clks 
              add eax, 1  
            ELSEIF  OPID eq UMLA64_OP 
              ;; UMLAL := M+2 I Clks    
              add eax, 2              
            ELSEIF  OPID eq SMUL64_OP   
              ;; SMULL := M+1 I Clks  
              add eax, 1                
            ELSEIF  OPID eq SMLA64_OP 
              ;; SMLAL := M+2 I Clks   
              add eax, 2               
            ELSE 
              ERRORS_ASSERT
            ENDIF     
            add SC_WAIT, eax 
            GamePAK_Prefetch eax
            ARM7_ExitAddPC 1 
          endm   
        @ALIGN_Z                                                  
 
        mul_interp MUL, MUL_OP
        mul_interp MLA, MLA_OP      
        mul_interp UMUL64, UMUL64_OP       
        mul_interp UMLA64, UMLA64_OP
        mul_interp SMUL64, SMUL64_OP      
        mul_interp SMLA64, SMLA64_OP