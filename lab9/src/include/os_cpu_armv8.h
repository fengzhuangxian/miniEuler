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

/*
* 任务上下文的结构体定义。
*/
struct TskContext {
    /* *< 当前物理寄存器R0-R12 */
    uintptr_t elr;               // 返回地址
    uintptr_t spsr;
    uintptr_t far;
    uintptr_t esr;
    uintptr_t xzr;
    uintptr_t x30;
    uintptr_t x29;
    uintptr_t x28;
    uintptr_t x27;
    uintptr_t x26;
    uintptr_t x25;
    uintptr_t x24;
    uintptr_t x23;
    uintptr_t x22;
    uintptr_t x21;
    uintptr_t x20;
    uintptr_t x19;
    uintptr_t x18;
    uintptr_t x17;
    uintptr_t x16;
    uintptr_t x15;
    uintptr_t x14;
    uintptr_t x13;
    uintptr_t x12;
    uintptr_t x11;
    uintptr_t x10;
    uintptr_t x09;
    uintptr_t x08;
    uintptr_t x07;
    uintptr_t x06;
    uintptr_t x05;
    uintptr_t x04;
    uintptr_t x03;
    uintptr_t x02;
    uintptr_t x01;
    uintptr_t x00;
};

#endif /* OS_CPU_ARMV8_H */