    .global   OsEnterMain
    .extern __os_sys_sp_end

    .type     start, function
    .section  .text.bspinit, "ax"
    .balign   4

    .global OsElxState
    .type   OsElxState, @function
OsElxState:
    MRS    x6, CurrentEL // 把系统寄存器 CurrentEL 的值读入到通用寄存器 x6 中
    MOV    x2, #0x4 // CurrentEL EL1： bits [3:2] = 0b01
    CMP    w6, w2

    BEQ Start // 若 CurrentEl 为 EL1 级别，跳转到 Start 处执行，否则死循环。

OsEl2Entry:
    B OsEl2Entry

Start:
    LDR    x1, =__os_sys_sp_end // ld文件中定义，堆栈设置
    BIC    sp, x1, #0xf

//参考： https://developer.arm.com/documentation/den0024/a/Memory-Ordering/Barriers/ISB-in-more-detail

Enable_FPU:
    MRS X1, CPACR_EL1
    ORR X1, X1, #(0x3 << 20)
    MSR CPACR_EL1, X1
    ISB

// 启用 MMU
    BL     mmu_init
    
// 进入 main 函数    
    B      OsEnterMain

OsEnterReset:
    B      OsEnterReset
