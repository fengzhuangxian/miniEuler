#include "prt_typedef.h"
#include "os_attr_armv8_external.h"

#define OS_GIC_VER                 2

#define GIC_DIST_BASE              0x08000000
#define GIC_CPU_BASE               0x08010000

#define GICD_CTLR                  (GIC_DIST_BASE + 0x0000U)
#define GICD_TYPER                 (GIC_DIST_BASE + 0x0004U)
#define GICD_IIDR                  (GIC_DIST_BASE + 0x0008U)
#define GICD_IGROUPRn              (GIC_DIST_BASE + 0x0080U)
#define GICD_ISENABLERn            (GIC_DIST_BASE + 0x0100U)
#define GICD_ICENABLERn            (GIC_DIST_BASE + 0x0180U)
#define GICD_ISPENDRn              (GIC_DIST_BASE + 0x0200U)
#define GICD_ICPENDRn              (GIC_DIST_BASE + 0x0280U)
#define GICD_ISACTIVERn            (GIC_DIST_BASE + 0x0300U)
#define GICD_ICACTIVERn            (GIC_DIST_BASE + 0x0380U)
#define GICD_IPRIORITYn            (GIC_DIST_BASE + 0x0400U)
#define GICD_ICFGR                 (GIC_DIST_BASE + 0x0C00U)


#define GICD_CTLR_ENABLE                1  /* Enable GICD */
#define GICD_CTLR_DISABLE               0     /* Disable GICD */
#define GICD_ISENABLER_SIZE             32
#define GICD_ICENABLER_SIZE             32
#define GICD_ICPENDR_SIZE               32
#define GICD_IPRIORITY_SIZE             4
#define GICD_IPRIORITY_BITS             8
#define GICD_ICFGR_SIZE                 16
#define GICD_ICFGR_BITS                 2

#define GICC_CTLR                  (GIC_CPU_BASE  + 0x0000U)
#define GICC_PMR                   (GIC_CPU_BASE  + 0x0004U)
#define GICC_BPR                   (GIC_CPU_BASE  + 0x0008U)
#define IAR_MASK        0x3FFU
#define GICC_IAR            (GIC_CPU_BASE + 0xc)
#define GICC_EOIR           (GIC_CPU_BASE + 0x10)
#define     GICD_SGIR               (GIC_DIST_BASE + 0xf00)

#define BIT(n)                     (1 << (n))

#define GICC_CTLR_ENABLEGRP0       BIT(0)
#define GICC_CTLR_ENABLEGRP1       BIT(1)
#define GICC_CTLR_FIQBYPDISGRP0    BIT(5)
#define GICC_CTLR_IRQBYPDISGRP0    BIT(6)
#define GICC_CTLR_FIQBYPDISGRP1    BIT(7)
#define GICC_CTLR_IRQBYPDISGRP1    BIT(8)

#define GICC_CTLR_ENABLE_MASK      (GICC_CTLR_ENABLEGRP0 | \
                                    GICC_CTLR_ENABLEGRP1)

#define GICC_CTLR_BYPASS_MASK      (GICC_CTLR_FIQBYPDISGRP0 | \
                                    GICC_CTLR_IRQBYPDISGRP0 | \
                                    GICC_CTLR_FIQBYPDISGRP1 | \
                                    GICC_CTLR_IRQBYPDISGRP1)

#define GICC_CTLR_ENABLE            1
#define GICC_CTLR_DISABLE           0
// Priority Mask Register. interrupt priority filter, Higher priority corresponds to a lower Priority field value.
#define GICC_PMR_PRIO_LOW           0xff
// The register defines the point at which the priority value fields split into two parts,
// the group priority field and the subpriority field. The group priority field is used to
// determine interrupt preemption. NO GROUP.
#define GICC_BPR_NO_GROUP           0x00

#define GIC_REG_READ(addr)         (*(volatile U32 *)((uintptr_t)(addr)))
#define GIC_REG_WRITE(addr, data)  (*(volatile U32 *)((uintptr_t)(addr)) = (U32)(data))


void OsGicInitCpuInterface(void)
{
    // 初始化Gicv2的distributor和cpu interface
    // 禁用distributor和cpu interface后进行相应配置
    GIC_REG_WRITE(GICD_CTLR, GICD_CTLR_DISABLE);
    GIC_REG_WRITE(GICC_CTLR, GICC_CTLR_DISABLE);
    GIC_REG_WRITE(GICC_PMR, GICC_PMR_PRIO_LOW);
    GIC_REG_WRITE(GICC_BPR, GICC_BPR_NO_GROUP);


    // 启用distributor和cpu interface
    GIC_REG_WRITE(GICD_CTLR, GICD_CTLR_ENABLE);
    GIC_REG_WRITE(GICC_CTLR, GICC_CTLR_ENABLE);

}

// src/arch/drv/gic/prt_gic_init.c
/*
* 描述: 去使能（禁用）指定中断
*/
OS_SEC_L4_TEXT void OsGicDisableInt(U32 intId)
{
    // Interrupt Clear-Enable Registers
    uint32_t shift = intId % GICD_ICENABLER_SIZE; //中断号对应的比特位
    volatile uint32_t* addr = ((volatile U32 *)(uintptr_t)(GICD_ICENABLERn + (intId / GICD_ICENABLER_SIZE) * sizeof(U32))) ;//中断号对应的GICD_ISENABLERn寄存器地址
    uint32_t value = GIC_REG_READ(addr);
    value |= (0x1 << shift); // 每个中断占1位，所以掩码为 0x1，将对应位置为1
    GIC_REG_WRITE(addr, value);
}

/*
* 描述: 使能指定中断
*/
OS_SEC_L4_TEXT void OsGicEnableInt(U32 intId)
{
    // Interrupt Set-Enable Registers
    uint32_t shift = intId % GICD_ISENABLER_SIZE; //中断号对应的比特位
    volatile uint32_t* addr = ((volatile U32 *)(uintptr_t)(GICD_ISENABLERn + (intId / GICD_ISENABLER_SIZE) * sizeof(U32))) ;//中断号对应的GICD_ISENABLERn寄存器地址
    uint32_t value = GIC_REG_READ(addr);
    value |= (0x1 << shift); // 每个中断占1位，所以掩码为 0x1，将对应位置为1
    GIC_REG_WRITE(addr, value);
}

OS_SEC_L4_TEXT void OsGicClearInt(uint32_t interrupt)
{
    GIC_REG_WRITE(GICD_ICPENDRn + (interrupt / GICD_ICPENDR_SIZE)*sizeof(U32), 1 << (interrupt % GICD_ICPENDR_SIZE));
}

// 设置中断号为interrupt的中断的优先级为priority
OS_SEC_L4_TEXT void OsGicIntSetPriority(uint32_t interrupt, uint32_t priority) {
    uint32_t shift = (interrupt % GICD_IPRIORITY_SIZE) * GICD_IPRIORITY_BITS;
    volatile uint32_t* addr = ((volatile U32 *)(uintptr_t)(GICD_IPRIORITYn + (interrupt / GICD_IPRIORITY_SIZE) * sizeof(U32))) ;
    uint32_t value = GIC_REG_READ(addr);
    value &= ~(0xff << shift); // 每个中断占8位，所以掩码为 0xFF
    value |= priority << shift;
    GIC_REG_WRITE(addr, value);
}

// 设置中断号为interrupt的中断的属性为config
OS_SEC_L4_TEXT void OsGicIntSetConfig(uint32_t interrupt, uint32_t config) {
    uint32_t shift = (interrupt % GICD_ICFGR_SIZE) * GICD_ICFGR_BITS;
    volatile uint32_t* addr = ((volatile U32 *)(uintptr_t)(GICD_ICFGR + (interrupt / GICD_ICFGR_SIZE)*sizeof(U32)));
    uint32_t value = GIC_REG_READ(addr);
    value &= ~(0x03 << shift);
    value |= config << shift;
    GIC_REG_WRITE(addr, value);
}

/*
* 描述: 中断确认
*/
OS_SEC_L4_TEXT U32 OsGicIntAcknowledge(void)
{
    // reads this register to obtain the interrupt ID of the signaled interrupt.
    // This read acts as an acknowledge for the interrupt.
    U32 value = GIC_REG_READ(GICC_IAR);
    return value;
}

/*
* 描述: 标记中断完成，清除相应中断位
*/
OS_SEC_L4_TEXT void OsGicIntClear(U32 value)
{
    // A processor writes to this register to inform the CPU interface either:
    // • that it has completed the processing of the specified interrupt
    // • in a GICv2 implementation, when the appropriate GICC_CTLR.EOImode bit is set to 1, to indicate that the interface should perform priority drop for the specified interrupt.
    GIC_REG_WRITE(GICC_EOIR, value);
}

U32 OsHwiInit(void)
{

    OsGicInitCpuInterface();

    return OS_OK;
}
