#include "prt_typedef.h"
#include "os_exc_armv8.h"
#include "os_attr_armv8_external.h"
#include "os_cpu_armv8.h"

extern U32 PRT_Printf(const char *format, ...);

// ExcRegInfo 格式与 OsExcDispatch 中寄存器存储顺序对应
void OsExcHandleEntry(U32 excType, struct ExcRegInfo *excRegs)
{
    PRT_Printf("Catch a exception.\n");
}

extern void TryPutc(unsigned char ch);

void MyFirstSyscall(char *str)
{
    while (*str != '\0') {
        TryPutc(*str);
        str++;
    }
}

// ExcRegInfo 格式与 OsExcDispatch 中寄存器存储顺序对应
void OsExcHandleFromLowElEntry(U32 excType, struct ExcRegInfo *excRegs)
{
    int ExcClass = (excRegs->esr&0xfc000000)>>26;
    if (ExcClass == 0x15){ //SVC instruction execution in AArch64 state.
        PRT_Printf("Catch a SVC call.\n");
        // syscall number存在x8寄存器中, x0为参数1
        int syscall_num = excRegs->xregs[(XREGS_NUM - 1)- 8]; //uniproton存储的顺序x0在高，x30在低
        uintptr_t param0 = excRegs->xregs[(XREGS_NUM - 1)- 0];
        PRT_Printf("syscall number: %d, param 0: 0x%x\n", syscall_num, param0);

        switch(syscall_num){
            case 1:
                MyFirstSyscall((void *)param0);
                break;
            default:
                PRT_Printf("Unimplemented syscall.\n");
        }
    }else{
        PRT_Printf("Catch a exception.\n");

    }
}

extern void OsTickDispatcher(void);
extern void OsUartRxHandle(void);
OS_SEC_ALW_INLINE INLINE void OsHwiHandleActive(U32 irqNum)
{
    switch(irqNum){
        case 30:
            OsTickDispatcher();
            // PRT_Printf(".");
            break;
        case 33:
            OsUartRxHandle();
        default:
            break;
    }
}

extern  U32 OsGicIntAcknowledge(void);
extern void OsGicIntClear(U32 value);
// src/arch/cpu/armv8/common/hwi/prt_hwi.c  OsHwiDispatch(),OsHwiDispatchHandle()
/*
* 描述: 中断处理入口, 调用处外部已关中断
*/
OS_SEC_L0_TEXT void OsHwiDispatch( U32 excType, struct ExcRegInfo *excRegs) //src/arch/cpu/armv8/common/hwi/prt_hwi.c
{
    // 中断确认，相当于OsHwiNumGet()
    U32 value = OsGicIntAcknowledge();
    U32 irq_num = value & 0x1ff;
    U32 core_num = value & 0xe00;

    OsHwiHandleActive(irq_num);

    // 清除中断，相当于 OsHwiClear(hwiNum);
    OsGicIntClear(irq_num|core_num);

    
}

/*
* 描述: 开启全局可屏蔽中断。
*/
OS_SEC_L0_TEXT uintptr_t PRT_HwiUnLock(void) //src/arch/cpu/armv8/common/hwi/prt_hwi.c
{
    uintptr_t state = 0;

    OS_EMBED_ASM(
        "mrs %0, DAIF      \n"
        "msr DAIFClr, %1   \n"
        : "=r"(state)
        : "i"(DAIF_IRQ_BIT)
        : "memory", "cc");

    return state & INT_MASK;
}

/*
* 描述: 关闭全局可屏蔽中断。
*/
OS_SEC_L0_TEXT uintptr_t PRT_HwiLock(void) //src/arch/cpu/armv8/common/hwi/prt_hwi.c
{
    uintptr_t state = 0;
    OS_EMBED_ASM(
        "mrs %0, DAIF      \n"
        "msr DAIFSet, %1   \n"
        : "=r"(state)
        : "i"(DAIF_IRQ_BIT)
        : "memory", "cc");
    return state & INT_MASK;
}

/*
* 描述: 恢复原中断状态寄存器。
*/
OS_SEC_L0_TEXT void PRT_HwiRestore(uintptr_t intSave) //src/arch/cpu/armv8/common/hwi/prt_hwi.c
{
    if ((intSave & INT_MASK) == 0) {
        OS_EMBED_ASM(
            "msr DAIFClr, %0\n"
            :
            : "i"(DAIF_IRQ_BIT)
            : "memory", "cc");
    } else {
        OS_EMBED_ASM(
            "msr DAIFSet, %0\n"
            :
            : "i"(DAIF_IRQ_BIT)
            : "memory", "cc");
    }
    return;
}