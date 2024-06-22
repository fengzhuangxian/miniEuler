#include "prt_typedef.h"

extern U32 PRT_Printf(const char *format, ...);
extern void PRT_UartInit(void);

S32 main(void)
{
    const char Test_SVC_str[] = "Hello, my first system call!";
	
    PRT_UartInit();

    PRT_Printf("\n");
    PRT_Printf("███╗   ███╗██╗███╗   ██╗██╗███████╗██╗   ██╗██╗     ███████╗██████╗     ██████╗ ██╗   ██╗    ██╗  ██╗███╗   ██╗██╗   ██╗███████╗██████╗\n");
    PRT_Printf("████╗ ████║██║████╗  ██║██║██╔════╝██║   ██║██║     ██╔════╝██╔══██╗    ██╔══██╗╚██╗ ██╔╝    ██║  ██║████╗  ██║██║   ██║██╔════╝██╔══██╗\n");
    PRT_Printf("██╔████╔██║██║██╔██╗ ██║██║█████╗  ██║   ██║██║     █████╗  ██████╔╝    ██████╔╝ ╚████╔╝     ███████║██╔██╗ ██║██║   ██║█████╗  ██████╔╝\n");
    PRT_Printf("██║╚██╔╝██║██║██║╚██╗██║██║██╔══╝  ██║   ██║██║     ██╔══╝  ██╔══██╗    ██╔══██╗  ╚██╔╝      ██╔══██║██║╚██╗██║██║   ██║██╔══╝  ██╔══██╗\n");
    PRT_Printf("██║ ╚═╝ ██║██║██║ ╚████║██║███████╗╚██████╔╝███████╗███████╗██║  ██║    ██████╔╝   ██║       ██║  ██║██║ ╚████║╚██████╔╝███████╗██║  ██║\n");
    PRT_Printf("╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝    ╚═════╝    ╚═╝       ╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝\n");

    PRT_Printf("ctr-a h: print help of qemu emulator. ctr-a x: quit emulator.\n\n");



    // 回到异常 EL 0级别，模拟系统调用，查看异常的处理，了解系统调用实现机制。
    // 《Bare-metal Boot Code for ARMv8-A Processors》
    OS_EMBED_ASM(
        "MOV    X1, #0b00000\n" // Determine the EL0 Execution state.
        "MSR    SPSR_EL1, X1\n"
        "ADR    x1, EL0Entry\n" // Points to the first instruction of EL0 code
        " MSR    ELR_EL1, X1\n"
        "eret\n"  // 返回到 EL 0 级别
        "EL0Entry: \n"
        "MOV x0, %0 \n" //参数1
        "MOV x8, #1\n" //在linux中,用x8传递 syscall number，保持一致。
        "SVC 0\n"    // 系统调用
        "B .\n" // 死循环，以上代码只用于演示，EL0级别的栈未正确设置
        ::"r"(&Test_SVC_str[0])
    );


    // 在 EL1 级别上模拟系统调用
    // OS_EMBED_ASM("SVC 0");
    return 0;
}

 





                                                                                                                                        
