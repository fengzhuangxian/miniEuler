.global OsExcHandleEntry
.type   OsExcHandleEntry, function

.macro SAVE_EXC_REGS  // 保存通用寄存器的值到栈中
    stp    x1, x0, [sp,#-16]!
    stp    x3, x2, [sp,#-16]!
    stp    x5, x4, [sp,#-16]!
    stp    x7, x6, [sp,#-16]!
    stp    x9, x8, [sp,#-16]!
    stp    x11, x10, [sp,#-16]!
    stp    x13, x12, [sp,#-16]!
    stp    x15, x14, [sp,#-16]!
    stp    x17, x16, [sp,#-16]!
    stp    x19, x18, [sp,#-16]!
    stp    x21, x20, [sp,#-16]!
    stp    x23, x22, [sp,#-16]!
    stp    x25, x24, [sp,#-16]!
    stp    x27, x26, [sp,#-16]!
    stp    x29, x28, [sp,#-16]!
    stp    xzr, x30, [sp,#-16]!
.endm

.macro RESTORE_EXC_REGS  // 从栈中恢复通用寄存器的值
    ldp    xzr, x30, [sp],#16
    ldp    x29, x28, [sp],#16
    ldp    x27, x26, [sp],#16
    ldp    x25, x24, [sp],#16
    ldp    x23, x22, [sp],#16
    ldp    x21, x20, [sp],#16
    ldp    x19, x18, [sp],#16
    ldp    x17, x16, [sp],#16
    ldp    x15, x14, [sp],#16
    ldp    x13, x12, [sp],#16
    ldp    x11, x10, [sp],#16
    ldp    x9, x8, [sp],#16
    ldp    x7, x6, [sp],#16
    ldp    x5, x4, [sp],#16
    ldp    x3, x2, [sp],#16
    ldp    x1, x0, [sp],#16
.endm

.macro EXC_HANDLE vecId handler
    SAVE_EXC_REGS // 保存寄存器宏

    mov x1, #\vecId // x1 记录异常类型
    b   \handler // 跳转到异常处理
.endm

    .section .os.vector.text, "ax"

    .global  OsVectorTable
    .type  OsVectorTable,function

    .align 13

OsVectorTable:
.set    VBAR, OsVectorTable
.org VBAR                                // Synchronous, Current EL with SP_EL0
    EXC_HANDLE  0 OsExcDispatch

.org (VBAR + 0x80)                       // IRQ/vIRQ, Current EL with SP_EL0
    EXC_HANDLE  1 OsExcDispatch

.org (VBAR + 0x100)                      // FIQ/vFIQ, Current EL with SP_EL0
    EXC_HANDLE  2 OsExcDispatch

.org (VBAR + 0x180)                      // SERROR, Current EL with SP_EL0
    EXC_HANDLE  3 OsExcDispatch

.org (VBAR + 0x200)                      // Synchronous, Current EL with SP_ELx
    EXC_HANDLE  4 OsExcDispatch

.org (VBAR + 0x280)                      // IRQ/vIRQ, Current EL with SP_ELx
    EXC_HANDLE  5 OsHwiDispatcher

.org (VBAR + 0x300)                      // FIQ/vFIQ, Current EL with SP_ELx
    EXC_HANDLE  6 OsExcDispatch

.org (VBAR + 0x380)                      // SERROR, Current EL with SP_ELx
    EXC_HANDLE  7 OsExcDispatch

.org (VBAR + 0x400)                      // Synchronous, EL changes and the target EL is using AArch64
    EXC_HANDLE  8 OsExcDispatchFromLowEl

.org (VBAR + 0x480)                      // IRQ/vIRQ, EL changes and the target EL is using AArch64
    EXC_HANDLE  9 OsExcDispatch

.org (VBAR + 0x500)                      // FIQ/vFIQ, EL changes and the target EL is using AArch64
    EXC_HANDLE  10 OsExcDispatch

.org (VBAR + 0x580)                      // SERROR, EL changes and the target EL is using AArch64
    EXC_HANDLE  11 OsExcDispatch

.org (VBAR + 0x600)                      // Synchronous, L changes and the target EL is using AArch32
    EXC_HANDLE  12 OsExcDispatch

.org (VBAR + 0x680)                      // IRQ/vIRQ, EL changes and the target EL is using AArch32
    EXC_HANDLE  13 OsExcDispatch

.org (VBAR + 0x700)                      // FIQ/vFIQ, EL changes and the target EL is using AArch32
    EXC_HANDLE  14 OsExcDispatch

.org (VBAR + 0x780)                      // SERROR, EL changes and the target EL is using AArch32
    EXC_HANDLE  15 OsExcDispatch

    .text
    
       .global OsExcHandleEntry
    .type   OsExcHandleEntry, function

    .global OsExcHandleEntryFromLowEl
    .type   OsExcHandleEntryFromLowEl, function


    .section .os.init.text, "ax"
    .globl OsExcDispatch
    .type OsExcDispatch, @function
    .align 4
OsExcDispatch:
    mrs    x5, esr_el1
    mrs    x4, far_el1
    mrs    x3, spsr_el1
    mrs    x2, elr_el1
    stp    x4, x5, [sp,#-16]!
    stp    x2, x3, [sp,#-16]!

    mov    x0, x1  // x0： 异常类型
    mov    x1, sp  // x1: 栈指针
    bl     OsExcHandleEntry  // 跳转到实际的 C 处理函数， x0, x1分别为该函数的第1，2个参数。

    ldp    x2, x3, [sp],#16
    add    sp, sp, #16        // 跳过far, esr, HCR_EL2.TRVM==1的时候，EL1不能写far, esr
    msr    spsr_el1, x3
    msr    elr_el1, x2
    dsb    sy
    isb

    RESTORE_EXC_REGS // 恢复上下文

    eret //从异常返回


    .globl OsExcDispatchFromLowEl
    .type OsExcDispatchFromLowEl, @function
    .align 4
OsExcDispatchFromLowEl:
    mrs    x5, esr_el1
    mrs    x4, far_el1
    mrs    x3, spsr_el1
    mrs    x2, elr_el1
    stp    x4, x5, [sp,#-16]!
    stp    x2, x3, [sp,#-16]!

    mov    x0, x1
    mov    x1, sp
    bl     OsExcHandleFromLowElEntry

    ldp    x2, x3, [sp],#16
    add    sp, sp, #16        // 跳过far, esr, HCR_EL2.TRVM==1的时候，EL1不能写far, esr
    msr    spsr_el1, x3
    msr    elr_el1, x2
    dsb    sy
    isb

    RESTORE_EXC_REGS // 恢复上下文

    eret //从异常返回


    .globl OsHwiDispatcher
    .type OsHwiDispatcher, @function
    .align 4
OsHwiDispatcher:
    mrs    x5, esr_el1
    mrs    x4, far_el1
    mrs    x3, spsr_el1
    mrs    x2, elr_el1
    stp    x4, x5, [sp,#-16]!
    stp    x2, x3, [sp,#-16]!

    mov    x0, x1  // 异常类型0~15，参见异常向量表
    mov    x1, sp  // 异常时寄存器信息，通过栈及其sp指针提供
    bl     OsHwiDispatch

    ldp    x2, x3, [sp],#16
    add    sp, sp, #16        // 跳过far, esr, HCR_EL2.TRVM==1的时候，EL1不能写far, esr
    msr    spsr_el1, x3
    msr    elr_el1, x2
    dsb    sy
    isb

    RESTORE_EXC_REGS // 恢复上下文

    eret //从异常返回


/*
* 描述: void OsTskContextLoad(uintptr_t stackPointer)
*/
    .globl OsTskContextLoad
    .type OsTskContextLoad, @function
    .align 4
OsTskContextLoad:
    ldr    X0, [X0]
    mov    SP, X0            // X0 is stackPointer

OsContextLoad:
    ldp    x2, x3, [sp],#16
    add    sp, sp, #16        // 跳过far, esr, HCR_EL2.TRVM==1的时候，EL1不能写far, esr
    msr    spsr_el1, x3
    msr    elr_el1, x2
    dsb    sy
    isb

    RESTORE_EXC_REGS // 恢复上下文

    eret //从异常返回


/*
* 描述: Task调度处理函数。 X0 is g_runningTask
*/
    .globl OsTaskTrap
    .type OsTaskTrap, @function
    .align 4

OsTaskTrap:
    LDR    x1, =g_runningTask /* OsTaskTrap是函数调用过来，x0 x1寄存器是caller save，此处能直接使用 */
    LDR    x0, [x1] /* x0 is the &g_pRunningTask->sp */

    SAVE_EXC_REGS

    /* TskTrap需要保存CPSR，由于不能直接访问，需要拼接获取当前CPSR入栈 */
    mrs    x3, DAIF /* CPSR：DAIF 4种事件的mask, bits[9:6] */
    mrs    x2, NZCV /* NZCV：Condition flags, bits[31:28] */
    orr    x3, x3, x2
    orr    x3, x3, #(0x1U << 2) /* 当前的 exception level,bits[3:2] 00:EL0,01:El1,10:El2,11:EL3 */
    orr    x3, x3, #(0x1U) /* 当前栈的选择,bits[0] 0:SP_EL0,1:SP_ELX */

    mov    x2, x30    // 用返回地址x30作为现场恢复点
    sub    sp, sp, #16  // 跳过esr_el1, far_el1, 异常时才有用
    stp    x2, x3, [sp,#-16]!

    // 存入SP指针到g_pRunningTask->sp
    mov    x1, sp
    str    x1, [x0]   // x0 is the &g_pRunningTask->sp

    B      OsMainSchedule
loop1:
    B      loop1