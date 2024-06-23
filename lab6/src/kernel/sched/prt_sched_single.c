#include "prt_task_external.h"
#include "os_attr_armv8_external.h"
#include "prt_asm_cpu_external.h"
#include "os_cpu_armv8_external.h"

/*
* 描述：任务调度，切换到最高优先级任务
*/
OS_SEC_TEXT void OsTskSchedule(void)
{
    /* 外层已经关中断 */
    /* Find the highest task */
    OsTskHighestSet();

    /* In case that running is not highest then reschedule */
    if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        /* only if there is not HWI or TICK the trap */
        if (OS_INT_INACTIVE) { // 不在中断上下文中，否则应该在中断返回时切换
            OsTaskTrap();
            return;
        }
    }

    return;
}

/*
* 描述: 调度的主入口
* 备注: NA
*/
OS_SEC_L0_TEXT void OsMainSchedule(void)
{
    struct TagTskCb *prevTsk;
    if ((UNI_FLAG & OS_FLG_TSK_REQ) != 0) {
        prevTsk = RUNNING_TASK;

        /* 清除OS_FLG_TSK_REQ标记位 */
        UNI_FLAG &= ~OS_FLG_TSK_REQ;

        RUNNING_TASK->taskStatus &= ~OS_TSK_RUNNING;
        g_highestTask->taskStatus |= OS_TSK_RUNNING;

        RUNNING_TASK = g_highestTask;
    }
    // 如果中断没有驱动一个任务ready，直接回到被打断的任务
    OsTskContextLoad((uintptr_t)RUNNING_TASK);
}

/*
* 描述: 系统启动时的首次任务调度
* 备注: NA
*/
OS_SEC_L4_TEXT void OsFirstTimeSwitch(void)
{
    OsTskHighestSet();
    RUNNING_TASK = g_highestTask;
    TSK_STATUS_SET(RUNNING_TASK, OS_TSK_RUNNING);
    OsTskContextLoad((uintptr_t)RUNNING_TASK);
    // never get here
    return;
}