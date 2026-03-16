
    (* 访存指令 LDR/STR/LDM/STM/SWP
    (*----------*_*)
      LDR:         从内存中读word或者 halfword,byte,无符号/符号扩展成word
      LDM:         指定地址多次连续读取进指定寄存器集, 寄存器入栈
      STM:         指定寄存器集连续写进内存, 寄存器出栈
      SWP:         寄存器值和指定内存地址的值交换
      
      LDR/STR就不用多说了, 读写内存是最基本的内存访问手段
      
      LDM/STM有两个用处,  1是保持指定的上下文寄存器,
                           2是单指令拷存(memcpy)减少代码尺寸.
                           
      SWP, 类似与XCHG, 相似的是, 这条指令也是一条原子指令,
                   提供对高级同步原语的底层支持 
                   
        (* 标准访存指令 LDR/STR/LDRB/STRB
        (*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*_*)        
          LDR,      读内存, 读得时候有五种类型 Word, 无符号 Halfword/byte, 后者会无符号扩展为Word放入Rd寄存器
                                                       有符号 Halfword/byte, 会符号扩展为Word放入Rd寄存器
          STR,      写内存, 这就简单多了, Word/HalfWord/Byte 三种写入形式
                    机器码编码把LDR Word/Byte STR Word/Byte编码成一类, 其余的另外自成一码.寻址方式也是有部分不同.
                    
          STR/LDR唯一区别是 Rd, LDR计算好地址把数据读到Rd, STR则是把Rd的数据写进内存       
                   
          寻址计算有两种方式:
          
          <立即数寻址>       三个操作数 Rd, Rn, -/+Imm12
                                         Rn+正负12位立即数地址范围 例子 LDRB Rd, [Rn, #960], 取Rn+960的一个字节
                                                                    例子 LDREQ Rd, [Rn, #4],l 上次执行结果测试是否相等, 相等取Rn+4的一个Word
                                         
          <寄存器移位>       五个操作数 Rd/Rn/Rm/ShiftImm5/Shift 移位类型
          
                              对于寄存器移位 有如下公式 
                              
                              地址:= Rn+Index
                              Index:= -/+ Rm 移位类型 5位移位立即数
                              
                              移位类型有五种
                              
                              LSL              逻辑左移
                              LSR              逻辑右移
                              ASR              算术右移
                              ROR              循环右移
                              RRX              逻辑右移1位, C位进入 MSB(第31位), 此时没有立即数操作数, 实际机器码编码中
                                                                                   RRX是 ROR Imm5等于0的情况下发生的特殊指令
          关于移位类型和Imm5移位操作数
          
                              当Imm5等于0的时候, 会发生很多诡异的移位行为
                              想想也情有可原, Imm5是固定编码, 即-程序员是知道自己写了Imm5移位量的这个滑稽的无意义操作的.
                              但是谁没事写移位量为0的移位给自己添堵呢?有啥用呢, 所以ARM就充分利用了这部分编码
                              
                              对于 LSL Rm还是Rm, 形如 Op Rd, Rn, Rm这样指令是没有额外的编码空间的, 所以把它编码成 Op Rd, Rn, Rm, LSL #0的形式
                              对于 LSR Rm此时为0
                              对于 ASR 测试Rm的第31位, 如果为1那么Rm:= 0xFFFFFFFF否则为0
                              对于 ROR 此时执行RRX指令
                   
          寻址方式 <后索引 | 前索引>
          
                              后索引寻址: 直接用Rn作为访问地址, 访存完成后照寻址计算规则执行指令操作, 把计算的得到地址写回Rn
                              前索引寻址: 照寻址计算规则执行指令操作获得地址, 访存完成后可决定地址是否写回Rn
                              
                              前索引和后索引类似于++i和i++的区别
                              前索引寻址表示形式就是通常写法, 后面加个感叹号表示写回基地址Rn
                              
                              后索引寻址表示形式 LDR Rd, [Rn], #
                              前索引寻址表示形式 LDR Rd, [Rn, #]
                              
                              前索引的几个例子:
                              
                              LDRNEB R1, [R0, #2]!
                              LDR R8, [R0, R2,LSL #2]!
                              STREQB R3, [R0]!
                              后索引的几个例子:
                              LDRB R3, [R0]
                              STRHSB R3, [R0], #-0x17
                              LDREQB R3, [R0], R2,RRX      
                   
          T后缀声言:          指示访存是在用户模式下,即使是特权模式
                              注意, 不能和前索引配合使用
                              这个后缀主要用来验权, 通常是不使用的
                              比如你在用户模式触发了非法访存(可能是内存页保护),
                              当你在异常处修复了这个错误
                              LDRT用于模拟在用户模式下验证访存是否正常
                              
        (* LDR/STR/LDRB/STRB 机器码
        (****************************************************************************************_*)     
          27 26 25 24 23 22 21 20    19 - 16     15 - 12           11 10 9 8 7 6 5 4 3 2 1 0
           0  1  I  P  U  B  W  L       Rn         Rd                   Offset (d11-d0)
           
          Rd:LDR读取加载到Rd, STR从Rd写入
          Rn:索引地址
          I ? 寄存器移位: 12位立即数
          U ? 正偏移 :负偏移
          B ? byte : word
          L ? 读LDR :写STR
          
          W和P联合指示
          
          W:0 P:0 后索引寻址写回
          W:0 P:1 前索引寻址不写回
          W:1 P:0 后索引寻址写回+T后缀声言
          W:1 P:1 前索引寻址写回         
                   
          <寄存器移位>
          d11-d7: 移位立即数 (5bit 0-31)
          d6-d5:移位类型 0:LSL 1:LSR 2:ASR 3:ROR RRX:ROR移位量为0
          d4:0
          d3-d0:Rm
          
          立即数寻址都是常规套路, 不写了.
          
          LSL-                      常规操作
          LSR-
                                    #Imm5bit = 0 索引为0
                                    #Imm5bit!= 0 常规操作
          ASR-
                                    #Imm5bit = 0 && Rm[31] = 1, 索引为0xFFFFFFFF
                                    #Imm5bit = 0 && Rm[31]!= 1, 索引为0
                                    #Imm5bit!= 0 常规操作
          ROR-
                                    #Imm5bit = 0 RRX
                                    #Imm5bit!= 0 常规操作    
                   
        (* 标准访存指令 LDRH/STRH/LDRSB/LDRSH
        (*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*_*)    
        
          跟上面差不多, 12位立即数变成8位
          没有寄存器移位, 只有简单的Rn+/-Rm
          其余的都是一样的. 哦,还一点不一样,不能用T声言后缀     
          
        (* LDRH/STRH/LDRSB/LDRSH 机器码 1,用于寻址立即数.
        (****************************************************************************************_*)    
         27 26 25 24 23 22 21 20        19 - 16      15 - 12    11 10 9 8   7 6 5 4   3 2 1 0
          0  0  0  P  U  1  W  L           Rn           Rd       Offset1    1 S H 1   Offset2 (d22位那是数字1不是字母I)

          L:读写控制位?LDR:STR
          S:符号控制位?有符号:无符号
          H:half控制位?half:byte
          这几个标志位组合很混乱,在ARM7TDMI中只有如下几种情况
          (注意!!! S|H绝对不会为0,这也是这条指令可以被无二义识别的关键因素)
          
          L = 0, S = 0, H = 1 STRH halfword
          L = 1, S = 0, H = 1 LDRH
          L = 1, S = 1, H = 0 LDRSB
          L = 1, S = 1, H = 1 LDRSH
          
          halfword/byte会根据有无符号按movzx/movsx方式扩展成32位
          其余的ldr也是一样
          P 同上
          U 同上
          W: 当P= 0, 必须为0, 因为此模式无T后缀声言
          当P= 1, 同上
          Offset1, Offset2组成一个八位的偏移, 配合U位, 可寻址+/-256范围内的数据
          Offset1 <- HI
          Offset2 <- LO
          
        (* LDRH/STRH/LDRSB/LDRSH 机器码 2,用于寻址寄存器
        (****************************************************************************************_*)  
          27 26 25 24 23 22 21 20    19 - 16        15 - 12   11 10 9 8 7 6 5 4    3 2 1 0
           0  0  0  P  U  0  W  L       Rn             Rd      0  0 0 0 1 S H 1       Rm 
          标志位同上, Rm是32位 全偏移
          
        (* 栈操作访存指令 LDM/STM
        (*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*_*)  
          LDM LDR Multi 的缩写, STM同理
          一个寄存器集出栈, 一个入栈
          不像X86的PUSH/ POP ..
          LDM/STM有着更加丰富的栈操作玩法
          
          栈增长方式可以往下, 也就是进栈一个寄存器, 地址-4(一个寄存器四个字节(Word))
          也可以往上, 也就是进栈一个寄存器, 地址+4(一个寄存器四个字节(Word))
          
          对于提交的基地址Rn,
          LDM/STM操作的时候也可以选择包括这一个地址或者在这个地址+/-4的位置开始栈操作
          基地址偏移选择和栈增长方式都有各种两种操作, 组合起来就能有四种操作方式
          
          1. 栈方向往下- 基址包括 DA ED (指令声言)
          2. 栈方向往下- 基址排除 DB FD (指令声言)
          3. 栈方向往上- 基址包括 IA EA (指令声言)
          4. 栈方向往上- 基址排除 IB FA (指令声言)
          
          这四种方式STM/ LDM都能用, 不怕没有你想用的.
          栈方向排斥的一对, 和基址排斥的一对可以组成一对 PUSH/ POP来保持上下文
          栈方向和基址相同的, 可以组成拷存例程减少代码尺寸, 十分灵活.
          
          有人可能会问了? 栈操作方向相反, 为啥基址也要排斥呢?
          举个基地址操作相同的例子 LDM使用类型1, STM使用类型3, 栈方向相反, 皆为基址包括
          SP:=1000 STM你推一个寄存器进去 基址包括, 写进1000, SP+=4
          SP:=1004
          你要拿出来, 你用类型3,LDM去取, 基址包括, 取1004, SP-=4
          SP:=1000, 很明显取错地方了, 用基址排除, 先减-4, 1000,取1000这次终于正常啦
          基地址操作相同, 取/写总会有一个单元的偏移, 所以要基址排斥
          
          四种类型有着更利于人理解的名字
          STMIB 地址先增而后完成操作 STMFA 满递增堆栈
          STMIA 完成操作而后地址递增 STMEA 空递增堆栈
          STMDB 地址先减而后完成操作 STMFD 满递减堆栈
          STMDA 完成操作而后地址递减 STMED 空递减堆栈
          
          同行的表示同一种操作方式, 名字不同而已
          
          LDM/STM 语法
          
          ACCESS:= LDM | STM
          ADDRM:= DA|DB|IA|IB|ED|FD|EA|FA
          WB:= !(WriteBack Rn) | null
          AC:= ^(Same as User Mode)
          #:=Literal link
          ACCESS#Cond#ADDRM Rn WB, {寄存器集合表达式} AC
          
          Rn提供基地址
          ^表示操作的寄存器集都是用的用户模式下的寄存器集合
          !感叹号表示用于栈计算的地址最后会被写回Rn
          寄存器集合表达式怎么写呢?
          简单地一个可以直接这么写 {R3}, 例子 LDMEQIA R0, {R3}
          多个可以这么写Rstart-Rend, 多个非连续段寄存器用,链接起来
          例子, LDMEQIA R0, {R3-R15}
          STMEQED R0!, {R0, R2-R3, R4-R10, R12, R13, R14-R15}^
          
        (* LDM/STM 机器码
        (****************************************************************************************_*) 
          27 26 25 24 23 22 21 20    19 - 16        15 - 12 11 10 9 8 7 6 5 4 3 2 1 0
           1  0  0  P  U  S  W  L       Rn               Register List (d15-d0) 寄存器栈操作
           
          L?LDM(POP):STM(PUSH)
          Rn:出栈/入栈用的基址, 不一定要是r13 (sp).
          U?升栈:降栈PUSH/POP 升PUSH/POP由低变高, 降PUSH/POP地址由高变低
          Register List:寄存器列表, 按d15指示r15-> d0指示r0的序列排列, 1指示当前位置的寄存器被PUSH/POP
          W?更新基址Rn:不更新
          P?不包括当前地址, 会按照当前栈方向先上升一个单元:包括
          S?对于LDM&&当前Register List包含r15, SPSR拷贝进CPSR
          S?对于STM || (LDM&&当前Register List不包含r15), 拷贝进用户模式下的通用寄存器
          无论何种方式,
          寄存器集在内存低到高总是按照R0-R15增长方向排列的 (此测试基于CodeWarrior for ARM Developer Suite IDE)
          
          读取PC会对齐一个Word地址 (&-4)
          当Rn不对齐时, 也会强制对齐一个Word,
          如果此时再次写回Rn, 这个写回的地址却不会是对齐的 (此测试基于CodeWarrior for ARM Developer Suite IDE)
          Reglist 与写回Base 寄存器Rn 冲突时的UB行为
          对于LDM 写回不发生, 此测试基于 CodeWarrior IDE
          对于STM, 如果是最低有效位, 则写入Rn的原始值, 否则会写回计算完成的Rn地址, 此测试基于 CodeWarrior IDE
          
          <STM && R15>
          当STM中写入R15-PC时, 会轧入R15+12(这个R15是当前这条指令的地址)的值,
          此测试基于CodeWarrior 4.25, STR也是如此
          基于 Keil MDK-Lite 4.70.0 会轧入0
          
        (* 原子交换 SWP.
        (*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*_*)  
          语法:SWP[B] Cond Rd Rm [Rn]
          Rn:地址
          Rm:将要写进内存的值
          Rd:读出的值写进Rd
          
        (* SWP 机器码
        (****************************************************************************************_*)
          
          27 26 25 24 23 22 21 20   19 - 16    15 - 12    11 10 9 8 7 6 5 4   3 2 1 0
            0 0  0  1  0  B  0  0      Rn         Rd       0  0 0 0 1 0 0 1     Rm 数据交换
            
          B:1?交换8bit:交换32bit
          Rd:内存区域加载到此寄存器
          Rn:内存地址
          Rm:写入内存的字节/32bit
          
          对齐测试:对于读:=LDR, 对于写:=STR, 8位写入的时候只写入八位数据, 读取的时候会8bit零扩展为32位
          上述所有访存指令都不影响标志位
          访存指令在修改PC的时候会刷新当前CPU流水线, 至此, 标准ARM7的访存指令解释结束
            
    (* 标准ALU指令 ADD/SUB/ADC/SBC/RSB/RSC /CMP/CMN/TST/TEQ/MOV/MVN /BIC/AND/ORR/EOR
    (*----------*_*)
      ADD/SUB 正常加减 举个例子 ADD Rd, Rn, Rm (Rm+Rn放进Rd寄存器中)
      ADC/SBC 带进位加减 SBC是结果进位C清零
      RSB/RSC 反序减法(RSC带进位) RSB Rd, Rn, Rm (Rm-Rn放进Rd寄存器中)
      CMP/CMN 行为类似ADD/SUB, CMP对应ADD, CMN对应SUB.但是不保持结果, 用于测试执行结果
      AND 操作数相与
      ORR 操作数相或
      EOR 操作数异或
      TST 操作数相与不保持结果, 用于测试执行结果
      TEQ 操作数异或不保持结果, 用于测试执行结果
      BIC 操作数与第二个操作数取反相与
      MOV 寄存器之间互相拷贝
      MVN 寄存器取反拷贝
      除了CMP/CMN/TST/TEQ外都能设置是否影响标志位, 而这几个都会影响标志位, 无论是否设置.
      
      
      这些ALU指令都有一个统称的Oprand2, 一共12位
      这个Oprand2根据不同需求会被分别解释成三种类型的寻址计算
      
      1. 8位立即数+4位偶数移位数 扩展成32位立即数,            注意,这个移位是循环右移, 并且也可能因为设置影响C位 
      
      2. 5位移位立即数+移位类型+Rm,                            这个跟上面LDR用的移位是一样的, 不同的是某些指令设置影响标志位时, 
                                                                移位进位会进入C位 (Rs移位也如此)
                                                                这个C位是否影响标志位取决于所执行的指令是否影响C位, 
                                                                如果本身也影响将会忽略移位的C位进位
                                                                当然ALU指令没有设置影响标志也不会测试C位, Rs移位也是一样
      3. Rs移位寄存器+移位类型+Rm
                                                                差不多, 跟上面一样, 移位量取最低的一个字节, 多了对>=32移位量的判断, 
                                                                使用Rs移位会增加一个I周期
      具体情况如下, 只写特殊情况了, 常规操作不写了,大家应该都懂的:
      对于所有Rs移位 Rs:=0, 都是 Operand2 = Rm, C 不变
      
      LSL-#Imm5bit
                               #Imm5bit = 0 Operand2 = Rm, 不影响C位
      LSL-Rs
                                Rs = 32 Operand2 = 0, C:= Rm bit0
                                Rs > 32 Operand2 = 0, C := 0
      LSR-#Imm5bit
                               #Imm5bit = 0 Operand2 = 0, C:= Rm bit31
      LSR-Rs
                                Rs = 32 Operand2 = 0, C:= Rm bit31
                                Rs > 32 Operand2 = 0, C := 0
      ASR-#Imm5bit
                               #Imm5bit = 0 Operand2 = (Rm bit31)? 0 :0xFFFFFFFF:, C:= Rm bit31
      ASR-Rs
                                Rs = 32 Operand2 = 0, C:= Rm bit31
                                Rs > 32 Operand2 = (Rm bit31)? 0 :0xFFFFFFFF:, C:= Rm bit31
      ROR-#Imm5bit
                               #Imm5bit = 0 RRX
      ROR-Rs
                              if Rslo5bit = 0 Operand2 = Rm, C:= Rm bit31
                               else 基本操作, 此模式只使用低5位.
    
      Operand2 3种形式格式
      
      1. 八位循环移位+ROR Operand2:= Imm32:= Imm8 >> (Shift*2)
      2. Shift+Imm5 Operand2:= Rm ShiftType ShiftImm5bit
      3. Shift+Rs Operand2:= Rm ShiftType Rs's Byte
    
      各种形式都举几个例子哈!, Oprand2具体形式位域编码见后面机器码表
      
      1. 八位移位+ROR         MOV R1, #0x80000000
      2. Shift+Imm5            MOV R1, R0, LSL #2
                                SBC R1, R0, RRX
      2. Shift+Rs             MOV R1, R0, LSL R2
                              MOVS R1, R0, LSL R2 ;; MOVS, S表示同时设置标志位
                              
      ``````````````` ADD/SUB/ADC/SBC/RSB/RSC/CMP/CMN
      这几个操作数都是相加,相减
      对于 保持结果到Rd的指令有 Rd, Rn, Operand2 形式的操作
      对于 比较不保持结果的指令 不使用Rd位域, 即使他是有位域编码存在, 只有 Rn, Operand2 形式的操作
      对于 比较不保持结果的指令设置S与否始终会设置标志位 .
      这些指令都会影响NZCV标志位 (如果设置了S标志位)
      对于带进位减法指令, 当C位为0, 会额外减去一个1, 跟带进位加法相反!, thumb指令的alu也是如此
      
      ``````````````` AND/ORR/EOR/TST/TEQ
      这几个操作数都是逻辑操作
      对于 保持结果到Rd的指令有 Rd, Rn, Operand2 形式的操作
      对于 比较不保持结果的指令 不使用Rd位域, 即使他是有位域编码存在, 只有 Rn, Operand2 形式的操作
      对于 比较不保持结果的指令设置S与否始终会设置标志位 .
      这些指令都会影响NZ标志位 (如果设置了S标志位, C位看移位类型了, 否则不影响)
      
      ``````````````` MOV/MVN/BIC
      这几个操作数都是寄存器之间传输操作
      都是 Rd, Rn, Operand2 形式的操作
      这些指令都会影响NZ标志位 (如果设置了S标志位, C位看移位类型了, 否则不影响)
      
      
      (* 主ALU 机器码
      (****************************************************************************************_*)
       27 26 25   24 23 22 21   20     19 - 16     15 - 12       11 10 9 8 7 6 5 4 3 2 1 0
        0  0  I      Opcode      S        Rn          Rd                 Operand2
        
      oprand2: 编码1 (I =1) : 4位移位+8位数据 d0-d7 8位立即数, d8-d11 4位偶数移位数
      编码2 (I =0 && d4=0) : 寄存器移位 5位移位量(d11-d7)+2位移位类型(d6-d5)+4位寄存器索引(d3-d0)
      编码3 (I =0 && d4=1 && d7 = 0) :
      寄存器RS移位 RS寄存器索引(d11-d8)+2位移位类型(d6-d5)+4位寄存器索引(d3-d0)
      
      Opcode指示具体的 ALU/逻辑处理指令 - 操作模式
      
      0: AND Rd,Rn,Operand2
      1: EOR Rd,Rn,Operand2
      2: SUB Rd,Rn,Operand2
      3: RSB 反序 Sub Rd,Rn,Operand2
      4: ADD Rd,Rn,Operand2
      5: ADC Rd,Rn,Operand2
      6: SBC Rd,Rn,Operand2
      7: RSC 反序 Sbc Rd,Rn,Operand2
      8: TST Rn,Operand2 (S位域必为1, 同时如果是Rs/Imm5移位寻址会测试移位C来设置CPSR的进位状态位)
      9: TEQ XOR不保持结果 Rn,Operand  (S位域必为1, 同时如果是Rs/Imm5移位寻址会测试移位C来设置CPSR的进位状态位)
      A: CMP Rn,Operand2 (S位域必为1)
      B: CMN A+B不保持结果 Rn,Operand2 (S位域必为1)
      C: ORR OR Rd,Rn,Operand2
      D: MOV Rd,Operand2
      E: BIC 位清理 Rd,Rn,Operand2, rd:= rd & ~rn
      F: MVN 取反MOV Rd,Operand2 rd := ~rm 
      
      SUB/ADD/ADC/SBC/CMP/CMN/RSB/RSC 设置NZCV
      EOR/ORR/AND/TST/TEQ/BIC/MVN/MOV 设置NZC-(C由移位Operand2获得)
      
      S表示是否设置CPSR标志位, 1设置0不设置
      
      对于ALU操作中具有Rd参数且设置S标志的指令且Rd=PC的情况下
      会把当前模式下的SPSR缓存PSR拷贝到CPSR切换模式, 此细节一般用于从中断返回.
      举几个中断返回的例子,
      未定义指令: MOVS PC, LR
      预取异常: SUBS PC, LR, #4 (中断时候加上的微小的, 无用的偏移量, 需要减去,才能返回到正确的地址)
      访存异常: SUBS PC, LR, #8 (中断时候加上的微小的, 无用的偏移量, 需要减去,才能返回到正确的地址)
      
    (* 乘法/乘加/长乘/长乘加法/符号长乘/符号长乘加法 及机器码
    (*----**********************************************_*)
    
      MUL语法: Rd, Rm, Rs (乘法)
      MLA语法: Rd, Rm, Rs, Rn (乘加)
      UMULL语法 RdLo, RdHi, Rm, Rs (长乘)
      UMLAL语法 RdLo, RdHi, Rm, Rs (长乘加法)
      SMULL语法 RdLo, RdHi, Rm, Rs (符号长乘)
      SMLAL语法 RdLo, RdHi, Rm, Rs (符号长乘加法)
      
      乘法很简单, 直接放机器码了
      27 26 25 24 23 22 21 20 19 - 16           15 - 12     11 10 9 8    7 6 5 4      3 2 1 0
       0  0  0  0  0  0  A  S    Rd               Rn            Rs       1 0 0 1        Rm       乘法
       0  0  0  0  1  U  A  S    RdHi             RdLo          Rs       1 0 0 1        Rm       长乘
      
      各乘法- 简要算法:
      UMULL <- RdHi:RdLo := Rm*Rs
      USMULA <- RdHi: Lo += Rm*Rs
      SMULL <- RdHi:RdLo := Rm*Rs
      SMULA <- RdHi:RdLo += Rm*Rs
      
    (* 指令跳转 B/BL/BX 及机器码
    (*----**********************************************_*)
      B/BL语法: BL标号
      BX语法: BX寄存器
      编码1
      27 26 25 24           23 22 21 20 19 - 16 15 - 12 11 10 9 8 7 6 5 4 3 2 1 0
       1  0  1  L                            Offset (d23-d0)                            B/BL 分支指令
       
      L:?下一条指令的PC(在V4版本芯片里这个地址为PC-4)会驻留在r14中:
      Offset是24位有符号偏移, 这个偏移需要符号扩展成30位再乘以4才是正在的偏移,
      PC在加上这段偏移,才是实际的跳转地址, (注意这里的PC,是当前执行指令处+8的PC)
      
      编码2
      27 26 25 24 23 22 21 20 19 -  16  15 -  12    11 10 9 8 7 6 5 4   3 2 1 0
       0  0  0  1  0  0  1  0  1 1 1 1   1 1 1 1     1  1 1 1 0 0 0 1     Rn            BX分支交换指令
       
      Rn[0]进Thumb标志位
      PC := Rn[0] & 0xFFFFFFFE
      
    (* CPSR/SPSR读写/SWI软件中断指令 及机器码
    (*----**********************************************_*)  
      读写CPSR/SPSR
      
      MRS是读PSR指令, 读指令只能读到寄存器里面
      MSR是写PSR指令, 写指令可以把立即数/寄存器的值写入PSR寄存器
      
      MRS语法:
      PSR:= CPSR|SPSR
      MRS#Cond Rd, PSR
      
      例子:
      MRSEQ R0, CPSR
      
      MSR语法
      PSR:= CPSR|SPSR
      Field:= c|x|s|f (field的声言可以不重复叠加, 并且顺序无关)
      SRC:=8位位图+偶数移位立即数|寄存器
      MSR#Cond PSR#_#Field, SRC
      
      例子:
      MSR CPSR_xfsc, R0
      MSR CPSR_xfcs, R0
      MSR CPSR_x, R0
      MSR SPSR_x, R0
      
      掩码x:扩展位
      掩码c:控制位
      掩码s:状态位
      掩码f:标志位
      
      对于写CPSR, 在用户模式只能修改标志位(掩码f), 在特权模式, 除了thumb标识无法修改外, 其余全可以根据标识需求符修改指定的位域.
      对于写SPSR, 无论何种模式, 所有位域皆可以修改.
      对于读SPSR/CPSR, 则是正常读取完整的Block块, 
      (其实对于写CPSR, 标准ARM7TDMI芯片是不允许修改中断抑制标志的, GBA大概是改了一下吧. 详见 ARM手册 4.1.39 MSR指令的详细解释)
      CPSR/SPSR 的d4在操作过后总是会自动设置为1 .
      对于MSR写CPSR引起的切换模式, 会交换当前模式寄存器, 但不会发生当前CPSR保存到切换到的模式SPSR中. (此细节待测试).
      
      MRS机器码
      -**************************************************************************************
      27 26 25 24 23 22 21 20 19 - 16 15 - 12 11 10 9 8 7 6 5 4 3 2 1 0
       0  0  0  1  0  R  0  0   1111    Rd     0  0 0 0 0 0 0 0 0 0 0 0
      
      MSR立即数机器码
      -**************************************************************************************
      27 26 25 24 23 22 21 20 19 - 16 15 - 12 11 10 9 8 7 6 5 4 3 2 1 0
       0  0  1  1  0  R  1  0   MASK     1111 rotate_imm 8_bit_immediate
      MSR寄存器机器码
      -**************************************************************************************
      27 26 25 24 23 22 21 20 19 - 16 15 - 12 11 10 9 8 7 6 5 4   3 2 1 0
       0  0  0  1  0  R  1  0   MASK     1111  0  0 0 0 0 0 0 0      Rm
      
      I对于PSR传输指示: 0->寄存器模式 1->立即数模式 (对于MRS总是为0,
      因为在ARM7中不存在 mrs imm8, cpsr/spsr这样的指令).
      d22指示加载/读取 0? CPSR:SPSR
      d21指示 0? MRS : MSR, 对于MSR写PSR寄存器,只能以位域掩码的形式写入,
      读取则是完整的读取整个PSR寄存器
      对于MRS, d12-d15指示了PSR加载到哪个寄存器
      对于MSR 有两种模式-I=0寄存器模式 d3-d0指示要写入CPSR/SPSR的源寄存器
      I=1立即数模式 d7-d0指示八位位图立即数, d8-d11指示供移动偶数位的4位移位量
      
      可供MSR使用的掩码, 可组和使用:
      
      d19 标志位 Bit 31-24
      d18 状态位 Bit 23-16           (你不能动这个位域, 实质性的写入不会改变位域的内容,
                                       ARM7这个位域不用.., >=V6版本包含Simd浮点条件标记等..)
      d17 扩展位 Bit 15-8            (你不能动这个位域, 实质性的写入不会改变位域的内容,
                                       ARM7这个位域不用.., >=V6版本包含条件状态/大小端控制等..)
      d16 控制位 Bit 7-0             
      
      软件中断, 在GBA, 这用于调用BIOS提供的软件中断
      语法 SWI Imm24
      27 26 25 24          23 22 21 20 19 - 16 15 - 12 11 10 9 8 7 6 5 4 3 2 1 0
       1  1  1  1                    SWI Number (d23-d0) 软件中断
       
      具体的中断功能将在后面章节讲解
      至此所有标准的ARM7指令讲解完毕
      
{ ======  THUMB 指令 详细解释 ======== }
  (* 访存指令 LDR/STR/PUSH/POP/STMIA/LDMIA
  (*----**********************************************_*)  
    访存指令 LDR/STR/PUSH/POP/STMIA/LDMIA
    LDR/STR上面讲过了
    PUSH是DB类型栈操作
    POP则是IA类型栈操作
    STMIA/LDMIA 是用来拷存的.
    (* ------- LDR/STR ----- *)
      ``````````````` LDR/STR
      语法1:     LDR Rd, [Rn, #imm5*4]
                 LDRH Rd, [Rn, #imm5*2]
                 LDRB Rd, [Rn, #imm5*1]
                 STR Rd, [Rn, #imm5*4]
                 STRH Rd, [Rn, #imm5*2]
                 STRB Rd, [Rn, #imm5*1]
                 
      对于imm5*n的立即数偏移,必须对齐当前尺寸, 大部分编译器/汇编器也会限制这些语法(-4/-2/-1)
      
      语法2:     OP Rd, [Rn, Rm]
                 OP := LDR|STR|LDRH|LDRSH|STRH|LDRB|LDRSB|STRB
      
      语法3:     LDR Rd, [pc, #imm8*4]
                 LDR Rd, label (这根LDR Rd, [pc, #imm8*4]一样的,只是语法不同, 其实暗示了此指令只能是正偏移, 只能加载指令后面的标号, 而且要对齐一个Word)
                 LDR Rd, [sp, #imm8*4]
                 STR Rd, [sp, #imm8*4]

      LDR/STR imm5机器码
      15 14 13 12 11    10 9 8 7 6    5 4 3   2 1 0
       0  1  1  B  L     Offset5        Rn     Rd 读写立即数偏移
       1  0  0  0  L     Offset5        Rn     Rd 读写halfword
       
      L?LDR:STR
      B?指示字节:指示字
      
      LDR/STR Rn, Rm 机器码
      15 14 13 12 11 10 9    8 7 6    5 4 3    2 1 0
       0  1  0  1  L  B 0     Rm        Rn      Rd 读写相对+偏移
       0  1  0  1  H  S 1     Rm        Rn      Rd 读写符号字节/halfword
       
      L?LDR:STR
      B?字节:Word
      
      H=0 S=0 STRH
      H=0 S=1 LDRSB
      H=1 S=0 LDRH
      H=1 S=1 LDRSH
      
      LDR/STR SP/PC, #imm8*4 机器码
      15 14 13 12 11     10 9      8 7 6 5 4 3 2 1 0
       0  1  0  0  1       Rd            Offset8 读PC相对偏移
      15 14 13 12 11     10 9      8 7 6 5 4 3 2 1 0
       1  0  0  1  L       Rd            Offset8 读写SP偏移
      L?LDR:STR
    (* ------- PUSH/POP ----- *)
      15 14 13 12 11 10 9 8        7 6 5 4 3 2 1 0
       1  0  1  1  L  1 0 R             Rlist              Push/Pop 寄存器
      L?POP:PUSH
      Rlist:寄存器列表, 跟ARM7的一样, 不同的是只有r0-r7可以入栈
      R要配合L使用, 对于PUSH, R:=1会额外PUSH LR寄存器, 对于POP, R:=1会额外POP到 PC里
      计算完成的地址会写回SP (废话....)
    (* ------- LDMIA/STMIA ----- *)
      15 14 13 12 11   10 9 8    7 6 5 4 3 2 1 0
       1  1  0  0  L     Rn           Rlist         寄存器栈操作
      L?LDMIA:STMIA
      注意计算完成获得的地址一定会写回Rn
      
  (* THUMB 主ALU指令, 机器码没有S标志位, 但是一定会设置标志...
  (*----**********************************************_*)
      15 14 13 12 11 10      9 8 7 6      5 4 3         2 1 0
       0  1  0  0  0  0        Op(4bit)   Rm/Rs         Rd/Rn
       
      Op: 0000<- AND Rd, Rm NZ-
          0001<- EOR Rd, Rm NZ-
          0010<- LSL Rd, Rs NZC-
          0011<- LSR Rd, Rs NZC-
          0100<- ASR Rd, Rs NZC-
          0101<- ADC Rd, Rm NZCV
          0110<- SBC Rd, Rm NZCV
          0111<- ROR Rd, Rs NZC-
          1000<- TST Rn, Rm NZ-
          1001<- NEG Rd, Rm NZCV, 等同于 rd:= 0-rm
          1010<- CMP Rn, Rm NZCV
          1011<- CMN Rn, Rm NZCV
          1100<- ORR Rd, Rm NZ-
          1101<- MUL Rd, Rm NZ-
          1110<- BIC Rd, Rm NZ-
          1111<- MVN Rd, Rm NZ-
          
  (* THUMB 扩展ALU指令
  (*----**********************************************_*)       
    机器码1
    15 14 13    12 11   10 9 8 7 6   5 4 3   2 1 0
      0 0  0      Op      Offset5     Rm      Rd
      
    Op: 00 <- LSL
        01 <- LSR
        10 <- ASR
        11 <- NULL, 没有,或者是另外的指令
    影响 NZC-
    
    机器码2
    15 14 13 12 11 10     9      8 7 6             5 4 3     2 1 0
     0  0  0  1  1  I    Op      Rm/offset3          Rn       Rd
     
    I?3位立即数:寄存器
    OP?Sub:Add
    影响 NZCV
          
    机器码3
    15 14 13    12 11   10 9 8   7 6 5 4 3 2 1 0
     0  0  1      Op     Rd/Rn    Offset8 (d7-d0)
     
    Op:00<- MOV NZ-
       01<- CMP NZCV
       10<- ADD NZCV
       11<- SUB NZCV  
       
    机器码4
    15 14 13 12 11 10   9 8   7  6   5 4 3    2 1 0
     0  1  0  0  0  1   Op    H1 H2    Rm     Rd/Rn
     
    Op: 00 <- ADD Rd, Rm - 不影响标志位
        01 <- CMP Rn, Rm - NZCV
        10 <- MOV Rd, Rm - 不影响标志位
        11 <- BX Rm
        
    H1是Rn的高位, 和3位Rn变成4位寻址r0-r15
    H2是Rm的高位, 和3位Rm变成4位寻址r0-r15
       
    机器码5 ADD Rd, SP/PC*Imm8*4
    15 14 13 12 11    10 9 8    7 6 5 4 3 2 1 0
     1  0  1  0 SP       Rd        Imm8
    SP?:SP:PC
    
    机器码6 ADD/SUB SP, Imm7*4
    15 14 13 12 11 10 9 8 7   6 5 4 3 2 1 0
     1  0  1  1  0  0 0 0 S         Imm7
    S?:SUB:ADD
    
  (* THUMB 条件分支
  (*----**********************************************_*)   
    15 14 13 12          11 10 9 8      7 6 5 4 3 2 1 0
     1  1  0  1          Cond(4bit)         SImm8                 B Cond 条件分支
    寻址计算 PC:=PC+ movsx(Imm8)<<1
    
    15 14 13 12 11          10 9 8 7 6 5 4 3 2 1 0
     1  1  1  0  0                  Offset11                      B无条件分支
    寻址计算 PC:=PC+ movsx(Imm11)<<1
    
    15 14 13 12 11                    10 9 8 7 6 5 4 3 2 1 0
     1  1  1  1  H                           Imm11                BL长偏移分支
     
    长偏移被编码成两条指令
    H=0是第一条指令
    LR = PC + (movsx(SImm11) << 12)
    
    H=1是第二条指令
    PC = LR + (Imm11 << 1)
    LR = 下一条指令地址 | 1
    
    15 14 13 12 11 10 9 8  7  6        5 4 3          2 1 0 BX Rm
     0  1  0  0  0  1 1 1 H1 H2          Rm            Rd/Rn
     
    实际上扩展ALU指令机器码4 OP:=11情况
    Rm注意是Rm不是Rn, Rm的0位进T标志位域
    然后PC:= Rm & -2
    
{ ====== 指令周期概览 ======== }
    ALU指令 S +I for SHIFT(Rs) +S + N if R15 written
    MSR, MRS S
    LDR S+N+I +S +N if R15 loaded
    STR 2N
    LDM nS+N+I +S +N if R15 loaded
    STM (n-1)S+2N
    SWP S+2N+I
    B,BL 2S+N
    SWI, trap 2S+N
    MUL S+mI
    MLA S+(m+1)I
    MULL S+(m+1)I
    MLAL S+(m+2)I
    S:连续周期
    N:非连续周期
    I:内部周期
    
    乘法的m周期计算:
    对于乘法实际就是移位相加的组合
    乘法周期的计算主要依赖于Rs移位寄存器的内部字节信息.
    按照 d31 -> d0的方向 拆分成 四个字节
    
    Block3 - Block2 - Block1 - Block0
    
    if ((int32_t) Rs < 0)
    Rs = ~Rs;
    Block 3- 1全为0 则 m:= 1
    Block 3- 2全为0 则 m:= 2
    Block 3为0 则 m:=3
    全部不为0, 那么m:= 4
    
    对于取指, 实际需要的时间与当前设备的cache状态有关
    缓存线内部有相应的cache数据, 则不需要额外的访存时间,
    缓存脱靶时需要的时间 (WaitState) 依赖GBA内部IO指定的WaitState和当前周期类型
    注意这个缓存只适用于取指(指令cache)
    
    对于STR/STM, 他的最后一个周期总是指示下一条指令的预取是N周期.
    
{ ====== 未对齐的读写访问UB行为 ======== }
    一般情况下都是下降对齐地址在取值, 当然, 也会有特殊情况
    
    对于 未对齐的LDR:来说,先下降内存对齐4字节, 然后取出的值:= 值 ROR循环移位 (address & 3) * 8 位
    对于 未对齐的LDRH:来说,先下降内存对齐2字节, 然后取出的值:= 值 ROR循环移位  8 位 (例子 r0 := r1 ror 8)
    对于 未对齐的LDRSH:来说,取出的值:= 取未对齐的原始地址单字节有符号扩展为32位 
    SWP[B]指令的未对齐行为, 同LDR[B]/ STR[B]
    
    第一条引用于ARM手册 arm_arm.pdf :: A4.1.23 LDR Alignment 章节解释
    剩下的引用于GBA模拟器no cashGBA的GBATEK的ARM章节. (因为在官方手册中没找到记载, 直接用UB行为敷衍了事了)
{ ====== 资料参考. =========== }
    ARM Developer Suite Assembler Guide (DUI0068.pdf, ARM通用汇编编程手册, 提供快速的指令参考, 用户层面的参考书)
    ARM7TDMI Technical Reference Manual (DDI0210B.pdf, 此文档包含详细的ARM7TDMI指令周期时序, 和简略的操作码表)
    ARM Architecture Reference Manual (arm_arm.pdf, 这个是最全的芯片细节解释, 提供最详细的操作码机器码解码流程)
    上海交大 ARM7TDMI总线接口 (不错的中文教程).
    NO CASH GBA模拟器文档 GBATEK (关于未对齐的指令解释很详细)