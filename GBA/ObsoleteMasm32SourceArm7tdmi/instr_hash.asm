.data
        i8r4TAB dd i8r4AND, i8r4ANDS, i8r4EOR, i8r4EORS
                  dd i8r4SUB, i8r4SUBS, i8r4RSB, i8r4RSBS 
                  dd i8r4ADD, i8r4ADDS, i8r4ADC, i8r4ADCS 
                  dd i8r4SBC, i8r4SBCS, i8r4RSC, i8r4RSCS 
                  dd i8r4UB, i8r4TST, i8r4ToCPSR, i8r4TEQ 
                  dd i8r4UB, i8r4CMP, i8r4ToSPSR, i8r4CMN   
                  dd i8r4ORR, i8r4ORRS, i8r4MOV, i8r4MOVS 
                  dd i8r4BIC, i8r4BICS, i8r4MVN, i8r4MVNS 
        si5TAB dd si5AND, si5ANDS, si5EOR, si5EORS
                  dd si5SUB, si5SUBS, si5RSB, si5RSBS 
                  dd si5ADD, si5ADDS, si5ADC, si5ADCS 
                  dd si5SBC, si5SBCS, si5RSC, si5RSCS 
                  dd si5PSR1, si5TST, si5ToCPSR, si5TEQ 
                  dd si5PSR3, si5CMP, si5PSR4, si5CMN   
                  dd si5ORR, si5ORRS, si5MOV, si5MOVS 
                  dd si5BIC, si5BICS, si5MVN, si5MVNS 
        rsTAB dd rsAND, rsANDS, rsEOR, rsEORS ;; 0  0  0  0  0
                  dd rsSUB, rsSUBS, rsRSB, rsRSBS ;; 0  0  1  0  0
                  dd rsADD, rsADDS, rsADC, rsADCS ;; 0  1  0  0  0
                  dd rsSBC, rsSBCS, rsRSC, rsRSCS ;; 0  1  1  0  0
                  dd rsUB, rsTST, rsBX, rsTEQ   ;; 1  0  0  0  0
                  dd rsUB, rsCMP, rsUB, rsCMN ;; 1  0  1  0  0 
                  dd rsORR, rsORRS, rsMOV, rsMOVS ;; 1  1  0  0  0
                  dd rsBIC, rsBICS, rsMVN, rsMVNS ;; 1  1  1  0  0    
                  ;; LDR/STR Word/Byte Imm12 
                                 ;; P  U  B  W  L            
        nTAB    dd n0, n1, n2, n3 ;; 0  0  0  0  0
                  dd n4, n5, n6, n7 ;; 0  0  1  0  0
                  dd n8, n9, n10, n11 ;; 0  1  0  0  0
                  dd n12, n13, n14, n15 ;; 0  1  1  0  0
                  dd n16, n17, n18, n19 ;; 1  0  0  0  0
                  dd n20, n21, n22, n23 ;; 1  0  1  0  0 
                  dd n24, n25, n26, n27 ;; 1  1  0  0  0
                  dd n28, n29, n30, n31 ;; 1  1  1  0  0     
                ;; LDR/STR Word/Byte Scaled                   
                                 ;; P  U  B  W  L            
        eTAB    dd e0, e1, e2, e3 ;; 0  0  0  0  0
                  dd e4, e5, e6, e7 ;; 0  0  1  0  0
                  dd e8, e9, e10, e11 ;; 0  1  0  0  0
                  dd e12, e13, e14, e15 ;; 0  1  1  0  0
                  dd e16, e17, e18, e19 ;; 1  0  0  0  0
                  dd e20, e21, e22, e23 ;; 1  0  1  0  0 
                  dd e24, e25, e26, e27 ;; 1  1  0  0  0
                  dd e28, e29, e30, e31 ;; 1  1  1  0  0  
                  
                  ;; LDR/STR HalfWord/SByte Imm8 Hash 
                              ;; P  U  X1 W X2      
        cTAB    dd c0, c1, c2, c3 ;; 0  0  0  0  0
                  dd c4, c5, c6, c7 ;; 0  0  1  0  0
                  dd c8, c9, c10, c11 ;; 0  1  0  0  0
                  dd c12, c13, c14, c15 ;; 0  1  1  0  0
                  dd c16, c17, c18, c19 ;; 1  0  0  0  0
                  dd c20, c21, c22, c23 ;; 1  0  1  0  0 
                  dd c24, c25, c26, c27 ;; 1  1  0  0  0
                  dd c28, c29, c30, c31 ;; 1  1  1  0  0 
                    ;; LDR/STR HalfWord/SByte  Rm, Rn Hash
                                                   ;; P  U  X1 W X2            
        pTAB    dd p0, p1, p2, p3 ;; 0  0  0  0  0
                  dd p4, p5, p6, p7 ;; 0  0  1  0  0
                  dd p8, p9, p10, p11 ;; 0  1  0  0  0
                  dd p12, p13, p14, p15 ;; 0  1  1  0  0
                  dd p16, p17, p18, p19 ;; 1  0  0  0  0
                  dd p20, p21, p22, p23 ;; 1  0  1  0  0 
                  dd p24, p25, p26, p27 ;; 1  1  0  0  0
                  dd p28, p29, p30, p31 ;; 1  1  1  0  0 
                                                  ;; P  U  X1 W X2  
                                                   ;; X1 X2     3:= LDRSH (7)
                                                   ;;        2:= STRHW (1)  
                                                   ;;        1:= LDRSB (6) 
                                                   ;;        0:= LDRUH (5)  
                                 ;; LDM/STM Hash 
                                 ;; P  U  S  W  L            
        zTAB    dd z0, z1, z2, z3 ;; 0  0  0  0  0
                  dd z4, z5, z6, z7 ;; 0  0  1  0  0
                  dd z8, z9, z10, z11 ;; 0  1  0  0  0
                  dd z12, z13, z14, z15 ;; 0  1  1  0  0
                  dd z16, z17, z18, z19 ;; 1  0  0  0  0
                  dd z20, z21, z22, z23 ;; 1  0  1  0  0 
                  dd z24, z25, z26, z27 ;; 1  1  0  0  0
                  dd z28, z29, z30, z31 ;; 1  1  1  0  0 
          fTAB dd fEQ, fNE, fCS, fCC 
                   dd fMI, fPL, fVS, fVC 
                  dd fHI, fLS, fGE, fLT
                  dd fGT, fLE, fAL, fNV 

        pcMTAB dd 000h, 100b ;; 0 0 0
                   dd 000b, 000b ;; 0 1 0
                    dd 000b, 000b ;; 0 0 0
                   dd 0001b, 0101b ;; 0 1 0  
                   
        ARM7_MUL_TAB dd mMUL, mMULS, mMLA, mMLAS 
                     dd mUB, mUB, mUB, mUB 
                     dd mUMUL64, mUMUL64S, mUMLA64, mUMLA64S 
                     dd mSMUL64, mSMUL64S, mSMLA64, mSMLA64S
                     ;; thumb alu 
         taTAB dd taAND, taEOR, taLSL, taLSR 
              dd taASR, taADC, taSBC, taROR
              dd taTST, taNEG, taCMP, taCMN
              dd taORR, taMUL, taBIC, taMVN
              
      ;; ALU Shift Imm5 / Shift Rs / MUL / MISC                  |                           |
jTAB  dd  SF_I5, SF_RS, SF_I5, SF_RS, SF_I5, SF_RS, SF_I5, SF_RS, SF_I5, A7MUL, SF_I5, LDRHW, SF_I5, LDRHW, SF_I5, LDRHW
      dd  SF_I5, SF_RS, SF_I5, SF_RS, SF_I5, SF_RS, SF_I5, SF_RS, SF_I5, A7SWP, SF_I5, LDRHW, SF_I5, LDRHW, SF_I5, LDRHW
      ;; ALU Imm8 Bitmap + 4bit Even Shift 
      dd  AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8
      dd  AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8, AS_I8  
      ;; LDR/ STR Imm
      dd  LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12 ;;post index 
      dd  LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12, LDI12 ;;preindex or std index 
      ;; LDR/ STR Shift Rm 
      dd  LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI
      dd  LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI, LDIRS, UDEFI
      
      ;; Negtive --------------------|---------------------------|---------------------------|---------------------------------------------------
      
      ;; Load/Stroe Multi
      dd  RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET
      dd  RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET, RGSET
      ;; Branch Sign Imm24 Without LRLINK 
      dd  JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24, JMP24
      ;; Branch Sign Imm24 With LRLINK 
      dd  JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR, JMPLR
      ;; FPU, GBA doesn't have to deal with these things, For Simple, Link it to int 3 
      dd  COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT
      dd  COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT, COPDT
      dd  COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO, COPMO
      ;; GBA Bios Software Interrupt.
      dd  SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT, SWINT

sTAB dd  INLSL, INLSL, INLSL, INLSL, INLSL, INLSL, INLSL, INLSL, INLSR, INLSR, INLSR, INLSR, INLSR, INLSR, INLSR, INLSR
      dd  INASR, INASR, INASR, INASR, INASR, INASR, INASR, INASR, ADDRT, ADDRT, SUBRT, SUBRT, ADDI3, ADDI3, SUBI3, SUBI3
      dd  MOVI8, MOVI8, MOVI8, MOVI8, MOVI8, MOVI8, MOVI8, MOVI8, CMPI8, CMPI8, CMPI8, CMPI8, CMPI8, CMPI8, CMPI8, CMPI8   
      dd  ADDI8, ADDI8, ADDI8, ADDI8, ADDI8, ADDI8, ADDI8, ADDI8, SUBI8, SUBI8, SUBI8, SUBI8, SUBI8, SUBI8, SUBI8, SUBI8    
      dd  ALUOP, ALUOP, ALUOP, ALUOP, ADD16, CMP16, MOV16, BX_TB, LDRPC, LDRPC, LDRPC, LDRPC, LDRPC, LDRPC, LDRPC, LDRPC
      dd  STRWD, STRWD, STRHW, STRHW, STRUB, STRUB, LDRSB, LDRSB, LDRWD, LDRWD, LDHW2, LDHW2, LDRUB, LDRUB, LDRSW, LDRSW
      dd  STRW5, STRW5, STRW5, STRW5, STRW5, STRW5, STRW5, STRW5, LDRW5, LDRW5, LDRW5, LDRW5, LDRW5, LDRW5, LDRW5, LDRW5
      dd  STRB5, STRB5, STRB5, STRB5, STRB5, STRB5, STRB5, STRB5, LDRB5, LDRB5, LDRB5, LDRB5, LDRB5, LDRB5, LDRB5, LDRB5
      
      ;; Negtive --------------------------------------------------------------------------------------------------------------------------------
      dd  STRH5, STRH5, STRH5, STRH5, STRH5, STRH5, STRH5, STRH5, LDRH5, LDRH5, LDRH5, LDRH5, LDRH5, LDRH5, LDRH5, LDRH5   
      dd  STRSP, STRSP, STRSP, STRSP, STRSP, STRSP, STRSP, STRSP, LDRSP, LDRSP, LDRSP, LDRSP, LDRSP, LDRSP, LDRSP, LDRSP
      dd  PCI08, PCI08, PCI08, PCI08, PCI08, PCI08, PCI08, PCI08, SPI08, SPI08, SPI08, SPI08, SPI08, SPI08, SPI08, SPI08
      dd  STKS7, UNDEF, UNDEF, UNDEF, PUSHR, PUSHR, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, POPRP, POPRP, UNDEF, UNDEF      
      dd  STMIA, STMIA, STMIA, STMIA, STMIA, STMIA, STMIA, STMIA, LDMIA, LDMIA, LDMIA, LDMIA, LDMIA, LDMIA, LDMIA, LDMIA
      dd  JCCEQ, JCCNE, JCCCS, JCCCC, JCCMI, JCCPL, JCCVS, JCCVC, JCCHI, JCCLS, JCCGE, JCCLT, JCCGT, JCCLE, JCCAL, SWI08  
      dd  JMP11, JMP11, JMP11, JMP11, JMP11, JMP11, JMP11, JMP11, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF
      dd  BLEXT, BLEXT, BLEXT, BLEXT, BLEXT, BLEXT, BLEXT, BLEXT, BLSTD, BLSTD, BLSTD, BLSTD, BLSTD, BLSTD, BLSTD, BLSTD
      
      .code