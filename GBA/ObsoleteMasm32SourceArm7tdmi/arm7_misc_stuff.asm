        IRL_PUSH_b macro num 
          LDR_SET LDR_STD_IMM12, n, num
          LDR_SET LDR_STD_SCALED, e, num 
          LDR_SET LDR_EXT_LOHI8BIT, c, num 
          LDR_SET LDR_EXT_RNMD, p, num
          REGS_SET_OP num 
        endm           
        
        IRL_PUSH macro 
          ID = 0
          WHILE ID ne 32
            IRL_PUSH_b %ID
            ID = ID + 1
          ENDM
        endm 
       
       IRL_PUSH
        int 3
                COPMO:
        COPDT:
        UDEFI: ;; undef opcode abnormal 
         int 3 