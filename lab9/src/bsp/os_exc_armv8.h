#ifndef ARMV8_EXC_H
#define ARMV8_EXC_H

#include "prt_typedef.h"

#define XREGS_NUM       31

struct ExcRegInfo {
    // 以下字段的内存布局与TskContext保持一致
    uintptr_t elr;                  // 返回地址
    uintptr_t spsr;
    uintptr_t far;
    uintptr_t esr;
    uintptr_t xzr;
    uintptr_t xregs[XREGS_NUM];     // 0~30 : x30~x0
};

#endif /* ARMV8_EXC_H */