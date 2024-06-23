#ifndef OS_CPU_ARMV8_H
#define OS_CPU_ARMV8_H

#include "prt_typedef.h"

// CurrentEl等级
#define CURRENT_EL_2       0x8
#define CURRENT_EL_1       0x4
#define CURRENT_EL_0       0x0

#define DAIF_DBG_BIT      (1U << 3)
#define DAIF_ABT_BIT      (1U << 2)
#define DAIF_IRQ_BIT      (1U << 1)
#define DAIF_FIQ_BIT      (1U << 0)

#define INT_MASK          (1U << 7)

#define PRT_DSB() OS_EMBED_ASM("DSB sy" : : : "memory")
#define PRT_DMB() OS_EMBED_ASM("DMB sy" : : : "memory")
#define PRT_ISB() OS_EMBED_ASM("ISB" : : : "memory")

#endif /* OS_CPU_ARMV8_H */