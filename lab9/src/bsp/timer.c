#include "prt_typedef.h"
#include "prt_config.h"
#include "os_cpu_armv8.h"

U64 g_timerFrequency;
extern void OsGicIntSetConfig(uint32_t interrupt, uint32_t config);
extern void OsGicIntSetPriority(uint32_t interrupt, uint32_t priority);
extern void OsGicEnableInt(U32 intId);
extern void OsGicClearInt(uint32_t interrupt);

void CoreTimerInit(void)
{
    // 配置中断控制器
    OsGicIntSetConfig(30, 0); // 配置为电平触发
    OsGicIntSetPriority(30, 0); // 优先级为0
    OsGicClearInt(30); // 清除中断
    OsGicEnableInt(30); // 启用中断

    // 配置定时器
    OS_EMBED_ASM("MRS %0, CNTFRQ_EL0" : "=r"(g_timerFrequency) : : "memory", "cc"); //读取时钟频率

    U32 cfgMask = 0x0;
    U64 cycle = g_timerFrequency / OS_TICK_PER_SECOND;

    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("MSR CNTP_TVAL_EL0, %0" : : "r"(cycle) : "memory", "cc"); //设置中断周期

    cfgMask = 0x1;
    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(cfgMask) : "memory"); //启用定时器 enable=1, imask=0, istatus= 0,
    OS_EMBED_ASM("MSR DAIFCLR, #2");
}