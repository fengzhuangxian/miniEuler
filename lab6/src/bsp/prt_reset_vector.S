DAIF_MASK = 0x1C0       // disable SError Abort, IRQ, FIQ

    .global  OsVectorTable
    .global  OsEnterMain

    .section .text.startup, "ax"
OsEnterMain:
OsVectTblInit: // 设置 EL1 级别的异常向量表
    LDR x0, =OsVectorTable
    MSR VBAR_EL1, X0
    
    BL      main

    MOV     x2, DAIF_MASK // bits [9:6] disable SError Abort, IRQ, FIQ
    MSR     DAIF, x2 // 把通用寄存器 x2 的值写入系统寄存器 DAIF 中

EXITLOOP:
    B EXITLOOP
