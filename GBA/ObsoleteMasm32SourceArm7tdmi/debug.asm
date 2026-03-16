
.data 

;; TODO: Bad Code .
;; 
;; Macros assembled by MASM are difficult to debug in VS2012's embedded environment, 
;; especially because ehco macros do not work.

@ModeUser@ db 'MODE:USER', 0
@ModeSys@ db 'MODE:SYS', 0
@ModeUdef@ db 'MODE:Undef', 0
@ModeABT@ db 'MODE:Abort', 0
@ModeIRQ@ db 'MODE:IRQ', 0
@ModeFIQ@ db 'MODE:FIQ', 0
@ModeMGR@ db 'MODE:MGR', 0

@ModeTabs       dd @ModeUser@, @ModeFIQ@, @ModeIRQ@, @ModeMGR@
                dd @ModeUser@, @ModeUser@, @ModeUser@, @ModeABT@
                dd @ModeUser@, @ModeUser@, @ModeUser@, @ModeUdef@ 
                dd @ModeUser@, @ModeUser@, @ModeUser@, @ModeSys@
               
printf PROTO C :DWORD,:VARARG
DEBUG_OUT equ printf


;; input : "content"
rdata_mak macro format 

 LOCAL mID  ;; loop's iterator
 LOCAL mLEN
 LOCAL mCHSET ; eg. "Hello World", 00Ah, "Goddbyte word", 0
 LOCAL mN ;; Pos init left.
 LOCAL mTAIL_COMP ;; :BOOL init tail comp , 
 LOCAL mT1 ;; fro calc temp 
 LOCAL mT2  ;; fro calc temp 
 
 mN = 1
 mID = 0
 mTAIL_COMP = 0
 mLEN = @SizeStr (format)
 mCHSET TEXTEQU <> 
 ;; mCHSET CATSTR mCHSET, <ccc db >

 ;; echo format
 FORC CHS, <format>
   IFIDN <CHS>, <\> 
     IF mID ne (mLEN-2)
       CHR_N TEXTEQU @SubStr (format, mID+2, 1)
       %FORC CHS_N, CHR_N
         IFIDN <CHS_N>, <n> 
           IF mID eq (mLEN-3)
             mTAIL_COMP = 1
           ENDIF  
           IF mID ne 1 
             IF (mID-mN) ne 0
               mT1 = mN+1
               mT2 = mID - mN
               mCHSET CATSTR mCHSET,  <!">, @SubStr (format, mT1, mT2), <!"!,>
             ENDIF          
           ENDIF   
           mN = mID + 2           
           mCHSET CATSTR mCHSET, <00Ah>, <!,>
         ENDIF 
       ENDM 
     ENDIF
   ENDIF  
   mID = mID + 1
 ENDM 
 ;; add tail 
 IF mTAIL_COMP ne 1
;; echo <mTAIL_INCOMP>
           mLEN = mLEN - mN - 1
           mN = mN + 1
           mCHSET CATSTR mCHSET, <!">, @SubStr (format, mN, mLEN), <!"!, 0>
 ELSEIF mTAIL_COMP eq 1 
  ;; echo <mTAIL_COMP>
           mCHSET CATSTR mCHSET, <0>
 ENDIF 
 ;; %echo mCHSET
 EXITM <mCHSET>
endm 

tem_rev_arg macro arg1,  arglist:vararg
  LOCAL mCHSET
  mCHSET TEXTEQU <arg1>

  FOR ARG, <arglist>
  mCHSET CATSTR <ARG>, <!,>, mCHSET
  ENDM

  EXITM <mCHSET>
endm 

tem_alphatoint macro arg 

    IFIDNI <arg>, <0>
      EXITM <0>
    ELSEIFIDNI <arg>, <1> 
      EXITM <1>
    ELSEIFIDNI <arg>, <2>  
      EXITM <2>
    ELSEIFIDNI <arg>, <3> 
      EXITM <3>
    ELSEIFIDNI <arg>, <4>  
      EXITM <4>
     ELSEIFIDNI <arg>, <5> 
      EXITM <5>
    ELSEIFIDNI <arg>, <6> 
      EXITM <6>
    ELSEIFIDNI <arg>, <7> 
      EXITM <7>
    ELSEIFIDNI <arg>, <8>
      EXITM <8>
    ELSEIFIDNI <arg>, <9> 
      EXITM <9>
    ELSEIFIDNI <arg>, <A>  
      EXITM <10>
    ELSEIFIDNI <arg>, <B> 
      EXITM <11>
    ELSEIFIDNI <arg>, <C>  
      EXITM <12>
     ELSEIFIDNI <arg>, <D> 
      EXITM <13>
    ELSEIFIDNI <arg>, <E> 
      EXITM <14>
    ELSEIFIDNI <arg>, <F> 
      EXITM <15>
    ELSE 
      ERRORS_ASSERT
    ENDIF
endm 

tem_push_arg macro arglist0:vararg

  ;; e.g. "eax>7&00Ah", "ecx<5&ecx"
  ;; op } shift right 0
  ;; op { shift left 1
  ;; op + add 2
  ;; op - sub 3
  ;; op & and 4
  ;; op ^ xor 5
  ;; op | or 6
  ;; op ~ ror right 7
  ;; op ? 8 LhsOut rhs link sym, after ?, must offer a op symbol 
  
  ;; src/dst : reg or imm (e.g. 0A00h, 1, 2, 3)
  ;;
  ;; Z0-ZF Mapper register 0-15 (current)
  ;; U0-UF Mapper register 0-15 (sys/user) 
  ;; Q0-QF Mapper register 0-15 (irq) F:=SPSR
  ;; M0-MF Mapper register 0-15 (mgr) F:=SPSR
  ;; O0-O1 pipeline opcode
  ;;
  ;; $alu, get value register index, outval 
  
  LOCAL mRSC_POS  ;; LhsOut's end pos 
  LOCAL mLEN 
  LOCAL mID 
  LOCAL mOP
  LOCAL mT 
  LOCAL mT2
  LOCAL mT3
  LOCAL mSET_TYPE ;; 
  LOCAL mTEMP 
  LOCAL mID2
  LOCAL mTEMP2 
  LOCAL mSIGJ 
  LOCAL mACRG 
  LOCAL mCC
  LOCAL mDIS
  LOCAL mLINK 
  LOCAL mAGQ 
  
  FOR arg, <arglist0>

    mRSC_POS = 1
    mLEN  = @SizeStr (arg) 
    mID = 0
    mOP = -1
    mSIGJ = 0
    mACRG = 0
    mDIS = 0
    mLINK = 0
    mAGQ TEXTEQU <!?>
    
    FORC CHS, <arg>
      IFIDN <CHS>, <!#> 
        mDIS =1
      ENDIF 
    ENDM 
    IF mDIS eq 1 
      GOTO IBGS
    ENDIF 

    ;; %echo arg
    FORC CHS, <arg>
     ; echo CHS
      mSET_TYPE = -1
      mT = 0
      mT2 = 0
      mACRG = 0
      IF mLINK ne 0
        mLINK = mLINK - 1 ;; skip n.
        GOTO IBGA
      ENDIF 
      IFIDN <CHS>, <!"> 
        IF mID eq (mLEN-1)
          mSET_TYPE = 0      
        ENDIF 
      ELSEIFIDN <CHS>, < > 
        ;; Skip it.   
      ELSEIFIDN <CHS>, <!?> 
        ;; adjust pos, to rhs
        mT =  mID + 2
        mAGQ SUBSTR <arg>, mT, 1
        mACRG = 1
        mSET_TYPE = 0
      ELSEIFIDN <CHS>, <!$> ;; set reg
        mSIGJ = 1  
        mRSC_POS = mRSC_POS + 1               
      ELSEIFIDN <CHS>, <!{> 
        mSET_TYPE = 0
      ELSEIFIDN <CHS>, <!}>   
        mSET_TYPE = 1
      ELSEIFIDN <CHS>, <!+>  
        mSET_TYPE = 2     
      ELSEIFIDN <CHS>, <!->   
        mSET_TYPE = 3     
      ELSEIFIDN <CHS>, <!&>   
        mSET_TYPE = 4      
      ELSEIFIDN <CHS>, <!^>  
        mSET_TYPE = 5    
      ELSEIFIDN <CHS>, <!|>     
        mSET_TYPE = 6  
      ELSEIFIDN <CHS>, <!~>     
        mSET_TYPE = 7        
      ENDIF 

      IF mSET_TYPE ne -1
        ;; Check MAPPER 
        mT =  mRSC_POS + 1
        mT2 = mID - 1 - mRSC_POS + 1
        mTEMP TEXTEQU @SubStr (<arg>, mT, mT2) 
        mT = 0
        mT2= 1
        mID2 = 0
        %FORC CHS_N, mTEMP
          IFIDNI <CHS_N>, <Z>   
            mT = mID2+2
            mCC SubStr mTEMP, mT, 1
            mTEMP2 CATSTR <![SC_ARM7!].Regs![0>, mCC, <h*4!]>  
            mT2 = 0
          ELSEIFIDNI <CHS_N>, <O> 
            mT = mID2+2
            mCC SubStr mTEMP, mT, 1
            mTEMP2 CATSTR <![SC_ARM7!].Opcode![>, <0>, mCC, <h*4!]>     
            mT2 = 0
          ELSEIFIDNI <CHS_N>, <U> ;;sys/user 
            mT = mID2+2          
            mCC SubStr mTEMP, mT, 1
            mT3 = tem_alphatoint (%mCC)
            mT2 = 0
            mT3 = mT3 + 1  
            mT3 = mT3 and 15     
            mTEMP2 CATSTR <![SC_ARM7!].Regs![>, <0>, mCC, <h*4!]>               
            IF mT3 GE 14 
              mTEMP2 TEXTEQU <>
              mTEMP2 CATSTR <![SC_ARM7!].R1314_T![R1314b_SYSUSER>, <!(0>, mCC, <h-13!)*4!]>  
            ENDIF
          ELSEIFIDNI <CHS_N>, <Q> ;;IRQ  
            mT = mID2+2 
            mCC SubStr mTEMP, mT, 1
            mT3 = tem_alphatoint (%mCC)
            mT3 = mT3 + 1  
            mT3 = mT3 and 15     
            mT2 = 0
            mTEMP2 CATSTR <![SC_ARM7!].Regs![>, <0>, mCC, <h*4!]>               
            IF mT3 GE 14
              mTEMP2 CATSTR <![SC_ARM7!].R1314_T![R1314b_IRQ>, <!(0>, mCC, <h-13!)*4!]>  
            ELSEIF mT3 EQ 0 
              mTEMP2 TEXTEQU <>
              mTEMP2 CATSTR <![SC_ARM7!].SPSR_T![SPSRb_IRQ]>       
            ENDIF        
          ELSEIFIDNI <CHS_N>, <M> ;;MGR  
            mT = mID2+2
            mCC SubStr mTEMP, mT, 1
            mT3 = tem_alphatoint (%mCC)
            mT3 = mT3 + 1  
            mT2 = 0
            mT3 = mT3 and 15     
            mTEMP2 CATSTR <![SC_ARM7!].Regs![>, <0>, mCC, <h*4!]>               
            IF mT3 GE 14
              mTEMP2 TEXTEQU <>
              mTEMP2 CATSTR <![SC_ARM7!].R1314_T![R1314b_MGR>, <!(0>, mCC, <h-13!)*4!]>  
            ELSEIF mT3 EQ 0 
              mTEMP2 TEXTEQU <>
              mTEMP2 CATSTR <![SC_ARM7!].SPSR_T![SPSRb_MGR]>       
            ENDIF
          ENDIF 
          mID2 = mID2 +1
        ENDM 
        IF mT2 eq 0
          mTEMP TEXTEQU  mTEMP2        
        ENDIF 
        
        ;; Compile code
        movd xmm7, eax 
        mov eax, mTEMP
        movd xmm1, eax
        movd eax, xmm7
        
        IF mOP eq -1 ;; mem/imm/reg.
          movdqa xmm0, xmm1
        ELSEIF mOP eq 0 
          movd xmm7, eax 
          movd xmm6, ecx 
          movd ecx, xmm1 
          movd eax, xmm0 
          shl eax, cl 
          movd xmm0, eax 
          movd eax, xmm7 
          movd ecx, xmm6 
        ELSEIF mOP eq 1 
          movd xmm7, eax 
          movd xmm6, ecx 
          movd ecx, xmm1 
          movd eax, xmm0 
          shr eax, cl 
          movd xmm0, eax 
          movd eax, xmm7 
          movd ecx, xmm6      
        ELSEIF mOP eq 2 ;; +  mem/imm/reg.  
          paddd  xmm0, xmm1           
        ELSEIF mOP eq 3
          psubd  xmm0, xmm1     
        ELSEIF mOP eq 4 ;; & 
          pand  xmm0, xmm1   
        ELSEIF mOP eq 5
          pxor  xmm0, xmm1  
        ELSEIF mOP eq 6
          por  xmm0, xmm1   
        ELSEIF mOP eq 7 ;; ror it. 
          movd xmm7, eax 
          movd xmm6, ecx 
          movd ecx, xmm1 
          movd eax, xmm0 
          ror eax, cl 
          movd xmm0, eax 
          movd eax, xmm7 
          movd ecx, xmm6
        ELSE 
          ERRORS_ASSERT        
        ENDIF  
        mOP = mSET_TYPE
        mRSC_POS = mID+1
        
        IF mACRG ne 0
          mACRG = 0
          mLINK = 1
          mOP = -1
          mRSC_POS = mID + 2
          mSIGJ = 0
          movdqa xmm5, xmm0 
        ENDIF
      ENDIF 
      :IBGA
      mID = mID + 1
    ENDM 
    
    %FORC CHS_N, mAGQ
      IFIDNI <CHS_N>, <!?> 
        IF mSIGJ eq 1
            movd xmm1, eax 
            movd eax, xmm0
            and eax, 15
            push PTR32[SC_ARM7].Regs[eax*4]
            movd eax, xmm1
        ELSE
            movd xmm1, eax 
            movd eax, xmm0
            push eax
            movd eax, xmm1
        ENDIF   
      ELSE  
        IFIDN <CHS_N>, <!{> 
          movd xmm7, eax 
          movd xmm6, ecx 
          movd ecx, xmm0
          movd eax, xmm5
          shl eax, cl 
          movd xmm5, eax 
          movd eax, xmm7 
          movd ecx, xmm6
        ELSEIFIDN <CHS_N>, <!}>   
          movd xmm7, eax 
          movd xmm6, ecx 
          movd ecx, xmm0
          movd eax, xmm5
          shr eax, cl 
          movd xmm5, eax 
          movd eax, xmm7 
          movd ecx, xmm6
        ELSEIFIDN <CHS_N>, <!+>  
          paddd xmm5, xmm0  
        ELSEIFIDN <CHS_N>, <!->   
          psubd xmm5, xmm0       
        ELSEIFIDN <CHS_N>, <!&>   
          pand xmm5, xmm0      
        ELSEIFIDN <CHS_N>, <!^>  
          pxor xmm5, xmm0     
        ELSEIFIDN <CHS_N>, <!|>     
          por xmm5, xmm0 
        ELSEIFIDN <CHS_N>, <!~>     
          movd xmm7, eax 
          movd xmm6, ecx 
          movd ecx, xmm0
          movd eax, xmm5
          ror eax, cl 
          movd xmm5, eax 
          movd eax, xmm7 
          movd ecx, xmm6
        ELSE       
        ENDIF 
        movd xmm7, eax 
        movd eax, xmm5 
        push eax 
        movd eax, xmm7      
      ENDIF 
    ENDM 
    
    :IBGS
    IF mDIS eq 1 
      ;; MAKE DISPLAY. 
      movd xmm0, eax 
      movd xmm1, ebx 
      mov eax, SC_CPSR
      shr eax, ARM7_MODE_SFT_BIT 
      and eax, 01Fh 
      sub eax, 16
      push PTR32 [@ModeTabs+eax*4] 
      movd eax, xmm0
      movd ebx, xmm1
      .code
    ENDIF 
    
    :IBGC
  ENDM
endm


tem_print macro format, arglist:vararg 
  LOCAL cstr
  ARGLIST_EMPTY = 0
  FOR ARG,<arglist>
      ARGLIST_EMPTY = ARGLIST_EMPTY + 4
  ENDM 
  
  .data
  cstr db rdata_mak (format)
	.code 
  push eax 
  push edx 
  push ecx
  
  %tem_push_arg tem_rev_arg (arglist) 
  
  push offset cstr
  CALLIt DEBUG_OUT 
  add esp, 4 
  add esp, ARGLIST_EMPTY
  
  pop ecx 
  pop edx 
  pop eax 
endm

IF ARM7_DEBUG ne 0
OUTd equ tem_print

ELSE 
;; non-debug mode 
OUTd macro arglist:vararg
endm 
ENDIF
OUTc equ tem_print


