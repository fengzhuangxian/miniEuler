#include "prt_typedef.h"
#include "os_exc_armv8.h"

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
