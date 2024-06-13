#include "prt_task_external.h"
#include "prt_typedef.h"
#include "os_attr_armv8_external.h"
#include "prt_asm_cpu_external.h"
#include "os_cpu_armv8_external.h"
#include "prt_amp_task_internal.h"

OS_SEC_BSS struct TagOsRunQue g_runQueue;  // 核的局部运行队列

/*
* 描述：将任务添加到就绪队列, 调用者确保不会换核，并锁上rq
*/
OS_SEC_L0_TEXT void OsTskReadyAdd(struct TagTskCb *task)
{
    struct TagOsRunQue *rq = &g_runQueue;
    TSK_STATUS_SET(task, OS_TSK_READY);

    OS_TSK_EN_QUE(rq, task, 0);
    OsTskHighestSet();

    return;
}

/*
* 描述：将任务从就绪队列中移除，关中断外部保证
*/
OS_SEC_L0_TEXT void OsTskReadyDel(struct TagTskCb *taskCb)
{
    struct TagOsRunQue *runQue = &g_runQueue;
    TSK_STATUS_CLEAR(taskCb, OS_TSK_READY);

    OS_TSK_DE_QUE(runQue, taskCb, 0);
    OsTskHighestSet();

    return;
}

// src/core/kernel/task/prt_task_del.c
/*
* 描述：任务结束退出
*/
OS_SEC_L4_TEXT void OsTaskExit(struct TagTskCb *tsk)
{

    uintptr_t intSave = OsIntLock();

    OsTskReadyDel(tsk);
    OsTskSchedule();

    OsIntRestore(intSave);

}

// src/core/kernel/task/prt_amp_task.c
/*
* 描述：如果快速切换后只有中断恢复，使用此接口
*/
OS_SEC_TEXT void OsTskScheduleFastPs(uintptr_t intSave)
{
    /* Find the highest task */
    OsTskHighestSet();

    /* In case that running is not highest then reschedule */
    if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        /* only if there is not HWI or TICK the trap */
        if (OS_INT_INACTIVE) {
            OsTaskTrapFastPs(intSave);
        }
    }
}

extern U32 PRT_Printf(const char *format, ...);
OS_SEC_TEXT void OsDisplayTasksInfo(void)
{
    struct TagTskCb *taskCb = NULL;
    U32 cnt = 0;

    PRT_Printf("\nPID\t\tPriority\tStack Size\n");
    // 遍历g_runQueue队列，查找优先级最高的任务
    LIST_FOR_EACH(taskCb, &g_runQueue, struct TagTskCb, pendList) {
        cnt++;
        PRT_Printf("%d\t\t%d\t\t%d\n", taskCb->taskPid, taskCb->priority, taskCb->stackSize);
    }
    PRT_Printf("Total %d tasks", cnt);

}