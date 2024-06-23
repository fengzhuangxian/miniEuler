#include "prt_typedef.h"
#include "prt_tick.h"

extern U32 PRT_Printf(const char *format, ...);
extern void PRT_UartInit(void);
extern void CoreTimerInit(void);
extern U32 OsHwiInit(void);

U64 delay_time = 10000;

S32 main(void)
{
    // 初始化GIC
    OsHwiInit();
    // 启用Timer
    CoreTimerInit();

    PRT_UartInit();

    PRT_Printf("\n");
    PRT_Printf("███╗   ███╗██╗███╗   ██╗██╗███████╗██╗   ██╗██╗     ███████╗██████╗     ██████╗ ██╗   ██╗    ██╗  ██╗███╗   ██╗██╗   ██╗███████╗██████╗\n");
    PRT_Printf("████╗ ████║██║████╗  ██║██║██╔════╝██║   ██║██║     ██╔════╝██╔══██╗    ██╔══██╗╚██╗ ██╔╝    ██║  ██║████╗  ██║██║   ██║██╔════╝██╔══██╗\n");
    PRT_Printf("██╔████╔██║██║██╔██╗ ██║██║█████╗  ██║   ██║██║     █████╗  ██████╔╝    ██████╔╝ ╚████╔╝     ███████║██╔██╗ ██║██║   ██║█████╗  ██████╔╝\n");
    PRT_Printf("██║╚██╔╝██║██║██║╚██╗██║██║██╔══╝  ██║   ██║██║     ██╔══╝  ██╔══██╗    ██╔══██╗  ╚██╔╝      ██╔══██║██║╚██╗██║██║   ██║██╔══╝  ██╔══██╗\n");
    PRT_Printf("██║ ╚═╝ ██║██║██║ ╚████║██║███████╗╚██████╔╝███████╗███████╗██║  ██║    ██████╔╝   ██║       ██║  ██║██║ ╚████║╚██████╔╝███████╗██║  ██║\n");
    PRT_Printf("╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝    ╚═════╝    ╚═╝       ╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝\n");

    PRT_Printf("ctr-a h: print help of qemu emulator. ctr-a x: quit emulator.\n\n");

    for(int i = 0; i < 10; i++)
    {

        U32 tick = PRT_TickGetCount();
        PRT_Printf("[%d] current tick: %u\n", i, tick);

        //delay
        int delay_time = 10000000;  // 根据自己机器计算能力不同调整该值
        while(delay_time>0){
            PRT_TickGetCount();  //消耗时间，防止延时代码被编译器优化
            delay_time--;
        }

    }
    
    while(1);
    return 0;
}

 





                                                                                                                                        
