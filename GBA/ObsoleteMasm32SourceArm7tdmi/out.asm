
        ARM7_bba1 macro 
          OUTd " #%08X (IMM8:%02X, ROR:%d)", "SC_INVOL & 255 ?~ SC_INVOL}8 & 15 { 1", "SC_INVOL & 255", "SC_INVOL}8 & 15 { 1"
        endm 
        
        ARM7_ALU_BASE_OUTd1 macro OPID
          IF OPID eq AND_OP
            OUTd "AND R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq EOR_OP 
            OUTd "EOR R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq ORR_OP 
            OUTd "ORR R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1  
          ELSEIF OPID eq SUB_OP 
            OUTd "SUB R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq ADD_OP
            OUTd "ADD R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1 
          ELSEIF OPID eq MOV_OP    
            OUTd "MOV R%d", "SC_INVOL}12 & 15"
            ARM7_bba1           
          ELSEIF OPID eq MVN_OP 
            OUTd "MVN R%d", "SC_INVOL}12 & 15"
            ARM7_bba1     
          ELSEIF OPID eq BIC_OP 
            OUTd "BIC R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1   
          ELSEIF OPID eq RSB_OP
            OUTd "RSB R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1 
          ELSEIF OPID eq SBC_OP 
            OUTd "SBC R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq RSC_OP  
            OUTd "RSC R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq ADC_OP  
            OUTd "ADC R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1 
          ELSEIF OPID eq TST_OP            
          ELSEIF OPID eq TEQ_OP      
          ELSEIF OPID eq CMP_OP              
          ELSEIF OPID eq CMN_OP             
          ELSE  
            ERRORS_ASSERT
          ENDIF 
        endm 
          
        ARM7_ALU_SIGN_BASE_OUTd1 macro OPID
          IF OPID eq AND_OP
            OUTd "ANDS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq EOR_OP 
            OUTd "EORS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq ORR_OP 
            OUTd "ORRS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1  
          ELSEIF OPID eq SUB_OP 
            OUTd "SUBS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq ADD_OP
            OUTd "ADDS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1 
          ELSEIF OPID eq MOV_OP    
            OUTd "MOVS R%d", "SC_INVOL}12 & 15"
            ARM7_bba1           
          ELSEIF OPID eq MVN_OP 
            OUTd "MVNS R%d", "SC_INVOL}12 & 15"
            ARM7_bba1     
          ELSEIF OPID eq BIC_OP 
            OUTd "BICS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1   
          ELSEIF OPID eq RSB_OP
            OUTd "RSBS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1 
          ELSEIF OPID eq SBC_OP 
            OUTd "SBCS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq RSC_OP  
            OUTd "RSCS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1
          ELSEIF OPID eq ADC_OP  
            OUTd "ADCS R%d, R%d", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15"
            ARM7_bba1       
          ELSEIF OPID eq TST_OP  
            OUTd "TST R%d", "SC_INVOL}16 & 15"
            ARM7_bba1          
          ELSEIF OPID eq TEQ_OP
            OUTd "TEQ R%d", "SC_INVOL}16 & 15"
            ARM7_bba1         
          ELSEIF OPID eq CMP_OP  
            OUTd "CMP R%d", "SC_INVOL}16 & 15"
            ARM7_bba1             
          ELSEIF OPID eq CMN_OP  
            OUTd "CMN R%d", "SC_INVOL}16 & 15"
            ARM7_bba1 
          ELSE 
            ERRORS_ASSERT
          ENDIF 
        endm 
        ARM7_ALU_BASE_OUTd2 macro OPID
          IF OPID eq AND_OP  ;; LSL #2
            OUTd "AND R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq EOR_OP 
            OUTd "EOR R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq ORR_OP 
            OUTd "ORR R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq SUB_OP 
            OUTd "SUB R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq ADD_OP
            OUTd "ADD R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15" 
          ELSEIF OPID eq MOV_OP    
            OUTd "MOV R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL & 15"           
          ELSEIF OPID eq MVN_OP 
            OUTd "MVN R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL & 15"  
          ELSEIF OPID eq BIC_OP 
            OUTd "BIC R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15" 
          ELSEIF OPID eq RSB_OP
            OUTd "RSB R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15" 
          ELSEIF OPID eq SBC_OP 
            OUTd "SBC R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq RSC_OP  
            OUTd "RSC R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq ADC_OP  
            OUTd "ADC R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq TST_OP      
            OUTd "TST R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"               
          ELSEIF OPID eq TEQ_OP  
            OUTd "TEQ R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"          
          ELSEIF OPID eq CMP_OP   
            OUTd "CMP R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"          
          ELSEIF OPID eq CMN_OP
            OUTd "CMN R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"              
          ELSE  
            ERRORS_ASSERT
          ENDIF 
        endm 
          
        ARM7_ALU_SIGN_BASE_OUTd2 macro OPID
          IF OPID eq AND_OP  ;; LSL #2
            OUTd "ANDS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq EOR_OP 
            OUTd "EORS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq ORR_OP 
            OUTd "ORRS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq SUB_OP 
            OUTd "SUBS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq ADD_OP
            OUTd "ADDS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15" 
          ELSEIF OPID eq MOV_OP    
            OUTd "MOVS R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL & 15"           
          ELSEIF OPID eq MVN_OP 
            OUTd "MVNS R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL & 15"  
          ELSEIF OPID eq BIC_OP 
            OUTd "BICS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15" 
          ELSEIF OPID eq RSB_OP
            OUTd "RSBS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15" 
          ELSEIF OPID eq SBC_OP 
            OUTd "SBCS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq RSC_OP  
            OUTd "RSCS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq ADC_OP  
            OUTd "ADCS R%d, R%d, R%d, ", "SC_INVOL}12 & 15", "SC_INVOL}16 & 15", "SC_INVOL & 15"
          ELSEIF OPID eq TST_OP      
            OUTd "TST R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"               
          ELSEIF OPID eq TEQ_OP  
            OUTd "TEQ R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"          
          ELSEIF OPID eq CMP_OP   
            OUTd "CMP R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"          
          ELSEIF OPID eq CMN_OP
            OUTd "CMN R%d, R%d, ", "SC_INVOL}16 & 15", "SC_INVOL & 15"          
          ELSE  
            ERRORS_ASSERT
          ENDIF  
        endm 