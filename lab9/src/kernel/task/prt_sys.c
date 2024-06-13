#include "prt_typedef.h"
#include "os_attr_armv8_external.h"
#include "prt_task.h"

OS_SEC_L4_BSS U32 g_threadNum;

/* Tick计数 */
// OS_SEC_BSS U64 g_uniTicks; // 把 lab5 中在 src/kernel/tick/prt_tick.c 定义的 g_uniTicks 移到此处则取消此行的注释

/* 系统状态标志位 */
OS_SEC_DATA U32 g_uniFlag = 0;

OS_SEC_DATA struct TagTskCb *g_runningTask = NULL;

// src/core/kernel/task/prt_task_global.c
OS_SEC_BSS TskEntryFunc g_tskIdleEntry;


OS_SEC_BSS U32 g_tskMaxNum;
OS_SEC_BSS struct TagTskCb *g_tskCbArray;
OS_SEC_BSS U32 g_tskBaseId;

OS_SEC_BSS TskHandle g_idleTaskId;
OS_SEC_BSS U16 g_uniTaskLock;
OS_SEC_BSS struct TagTskCb *g_highestTask;