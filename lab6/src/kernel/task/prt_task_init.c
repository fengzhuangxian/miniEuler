#include "list_types.h"
#include "os_attr_armv8_external.h"
#include "prt_list_external.h"
#include "prt_task.h"
#include "prt_task_external.h"
#include "prt_asm_cpu_external.h"
#include "os_cpu_armv8_external.h"
#include "prt_config.h"


/* Unused TCBs and ECBs that can be allocated. */
OS_SEC_DATA struct TagListObject g_tskCbFreeList = LIST_OBJECT_INIT(g_tskCbFreeList);

extern U32 OsTskAMPInit(void);
extern U32 OsIdleTskAMPCreate(void);
extern void OsFirstTimeSwitch(void);



//简单实现OsMemAllocAlign
/*
* 描述：分配任务栈空间
* 仅支持4K大小，最多256字节对齐空间的分配
*/
uint8_t stackMem[20][4096] __attribute__((aligned(256))); // 256 字节对齐，20 个 4K 大小的空间
uint8_t stackMemUsed[20] = {0}; // 记录对应 4K 空间是否已被分配
OS_SEC_TEXT void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, U8 alignPow)
{
    // 最多支持256字节对齐
    if (alignPow > 8)
        return NULL;
    if (size != 4096)
        return NULL;
    for(int i = 0; i < 20; i++){
        if (stackMemUsed[i] == 0){
            stackMemUsed[i] = 1; // 记录对应 4K 空间已被分配
            return &(stackMem[i][0]); // 返回 4K 空间起始地址
        }
    }
    return NULL;
}

/*
* 描述：分配任务栈空间
*/
OS_SEC_L4_TEXT void *OsTskMemAlloc(U32 size)
{
    void *stackAddr = NULL;
        stackAddr = OsMemAllocAlign((U32)OS_MID_TSK, (U8)0, size,
                                /* 内存已按16字节大小对齐 */
                                OS_TSK_STACK_SIZE_ALLOC_ALIGN);
    return stackAddr;
}


/*
* 描述: 初始化任务栈的上下文
*/
void *OsTskContextInit(U32 taskID, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
{
    (void)taskID;
    struct TskContext *stack = (struct TskContext *)((uintptr_t)topStack + stackSize);

    stack -= 1; // 指针减，减去一个TskContext大小

    stack->x00 = 0;
    stack->x01 = 0x01010101;
    stack->x02 = 0x02020202;
    stack->x03 = 0x03030303;
    stack->x04 = 0x04040404;
    stack->x05 = 0x05050505;
    stack->x06 = 0x06060606;
    stack->x07 = 0x07070707;
    stack->x08 = 0x08080808;
    stack->x09 = 0x09090909;
    stack->x10 = 0x10101010;
    stack->x11 = 0x11111111;
    stack->x12 = 0x12121212;
    stack->x13 = 0x13131313;
    stack->x14 = 0x14141414;
    stack->x15 = 0x15151515;
    stack->x16 = 0x16161616;
    stack->x17 = 0x17171717;
    stack->x18 = 0x18181818;
    stack->x19 = 0x19191919;
    stack->x20 = 0x20202020;
    stack->x21 = 0x21212121;
    stack->x22 = 0x22222222;
    stack->x23 = 0x23232323;
    stack->x24 = 0x24242424;
    stack->x25 = 0x25252525;
    stack->x26 = 0x26262626;
    stack->x27 = 0x27272727;
    stack->x28 = 0x28282828;
    stack->x29 = 0x29292929;
    stack->x30 = funcTskEntry;   // x30： lr(link register)
    stack->xzr = 0;

    stack->elr = funcTskEntry;
    stack->esr = 0;
    stack->far = 0;
    stack->spsr = 0x305;    // EL1_SP1 | D | A | I | F
    return stack;
}


/*
* 描述：所有任务入口
*/
OS_SEC_L4_TEXT void OsTskEntry(TskHandle taskId)
{
    struct TagTskCb *taskCb;
    uintptr_t intSave;

    (void)taskId;

    taskCb = RUNNING_TASK;

    taskCb->taskEntry(taskCb->args[OS_TSK_PARA_0], taskCb->args[OS_TSK_PARA_1], taskCb->args[OS_TSK_PARA_2],
                    taskCb->args[OS_TSK_PARA_3]);

    // 调度结束后会开中断，所以不需要自己添加开中断
    intSave = OsIntLock();

    OS_TASK_LOCK_DATA = 0;

    /* PRT_TaskDelete不能关中断操作，否则可能会导致它核发SGI等待本核响应时死等 */
    OsIntRestore(intSave);

    OsTaskExit(taskCb);
}



// src/core/kernel/task/prt_task_internal.h
OS_SEC_ALW_INLINE INLINE U32 OsTaskCreateChkAndGetTcb(struct TagTskCb **taskCb)
{
    if (ListEmpty(&g_tskCbFreeList)) {
        return OS_ERRNO_TSK_TCB_UNAVAILABLE;
    }

    // 先获取到该控制块
    *taskCb = GET_TCB_PEND(OS_LIST_FIRST(&g_tskCbFreeList));
    // 成功，从空闲列表中移除
    ListDelete(OS_LIST_FIRST(&g_tskCbFreeList));

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE bool OsCheckAddrOffsetOverflow(uintptr_t base, size_t size)
{
    return (base + size) < base;
}

OS_SEC_L4_TEXT U32 OsTaskCreateRsrcInit(U32 taskId, struct TskInitParam *initParam, struct TagTskCb *taskCb,
                                                uintptr_t **topStackOut, uintptr_t *curStackSize)
{
    U32 ret = OS_OK;
    uintptr_t *topStack = NULL;

    /* 查看用户是否配置了任务栈，如没有，则进行内存申请，并标记为系统配置，如有，则标记为用户配置。 */
    if (initParam->stackAddr != 0) {
        topStack = (void *)(initParam->stackAddr);
        taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_USER;
    } else {
        topStack = OsTskMemAlloc(initParam->stackSize);
        if (topStack == NULL) {
            ret = OS_ERRNO_TSK_NO_MEMORY;
        } else {
            taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_SYS;
        }
    }
    *curStackSize = initParam->stackSize;
    if (ret != OS_OK) {
        return ret;
    }

    *topStackOut = topStack;
    return OS_OK;
}

OS_SEC_L4_TEXT void OsTskCreateTcbInit(uintptr_t stackPtr, struct TskInitParam *initParam,
    uintptr_t topStackAddr, uintptr_t curStackSize, struct TagTskCb *taskCb)
{
    /* Initialize the task's stack */
    taskCb->stackPointer = (void *)stackPtr;
    taskCb->args[OS_TSK_PARA_0] = (uintptr_t)initParam->args[OS_TSK_PARA_0];
    taskCb->args[OS_TSK_PARA_1] = (uintptr_t)initParam->args[OS_TSK_PARA_1];
    taskCb->args[OS_TSK_PARA_2] = (uintptr_t)initParam->args[OS_TSK_PARA_2];
    taskCb->args[OS_TSK_PARA_3] = (uintptr_t)initParam->args[OS_TSK_PARA_3];
    taskCb->topOfStack = topStackAddr;
    taskCb->stackSize = curStackSize;
    taskCb->taskSem = NULL;
    taskCb->priority = initParam->taskPrio;
    taskCb->taskEntry = initParam->taskEntry;
#if defined(OS_OPTION_EVENT)
    taskCb->event = 0;
    taskCb->eventMask = 0;
#endif
    taskCb->lastErr = 0;

    INIT_LIST_OBJECT(&taskCb->semBList);
    INIT_LIST_OBJECT(&taskCb->pendList);
    INIT_LIST_OBJECT(&taskCb->timerList);

    return;
}

/*
* 描述：创建一个任务但不进行激活
*/
OS_SEC_L4_TEXT U32 OsTaskCreateOnly(TskHandle *taskPid, struct TskInitParam *initParam)
{
    U32 ret;
    U32 taskId;
    uintptr_t intSave;
    uintptr_t *topStack = NULL;
    void *stackPtr = NULL;
    struct TagTskCb *taskCb = NULL;
    uintptr_t curStackSize = 0;

    intSave = OsIntLock();
    // 获取一个空闲的任务控制块
    ret = OsTaskCreateChkAndGetTcb(&taskCb);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    taskId = taskCb->taskPid;
    // 分配堆栈空间资源
    ret = OsTaskCreateRsrcInit(taskId, initParam, taskCb, &topStack, &curStackSize);
    if (ret != OS_OK) {
        ListAdd(&taskCb->pendList, &g_tskCbFreeList);
        OsIntRestore(intSave);
        return ret;
    }
    // 栈初始化，就像刚发生过中断一样
    stackPtr = OsTskContextInit(taskId, curStackSize, topStack, (uintptr_t)OsTskEntry);
    // 任务控制块初始化
    OsTskCreateTcbInit((uintptr_t)stackPtr, initParam, (uintptr_t)topStack, curStackSize, taskCb);

    taskCb->taskStatus = OS_TSK_SUSPEND | OS_TSK_INUSE;
    // 出参ID传出
    *taskPid = taskId;
    OsIntRestore(intSave);
    return OS_OK;
}

/*
* 描述：创建一个任务但不进行激活
*/
OS_SEC_L4_TEXT U32 PRT_TaskCreate(TskHandle *taskPid, struct TskInitParam *initParam)
{
    return OsTaskCreateOnly(taskPid, initParam);
}


// src/core/kernel/task/prt_task_internal.h
OS_SEC_ALW_INLINE INLINE void OsMoveTaskToReady(struct TagTskCb *taskCb)
{
    if (TSK_STATUS_TST(taskCb, OS_TSK_DELAY_INTERRUPTIBLE)) {
        /* 可中断delay, 属于定时等待的任务时候，去掉其定时等待标志位*/
        if (TSK_STATUS_TST(taskCb, OS_TSK_TIMEOUT)) {
            OS_TSK_DELAY_LOCKED_DETACH(taskCb);
        }
        TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT | OS_TSK_DELAY_INTERRUPTIBLE);
    }

    /* If task is not blocked then move it to ready list */
    if ((taskCb->taskStatus & OS_TSK_BLOCK) == 0) {
        OsTskReadyAdd(taskCb);

        if ((OS_FLG_BGD_ACTIVE & UNI_FLAG) != 0) {
            OsTskSchedule();
            return;
        }
    }
}

/*
* 描述解挂任务
*/
OS_SEC_L2_TEXT U32 PRT_TaskResume(TskHandle taskPid)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    // 获取 taskPid 对应的任务控制块
    taskCb = GET_TCB_HANDLE(taskPid);

    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    if (((OS_TSK_RUNNING & taskCb->taskStatus) != 0) && (g_uniTaskLock != 0)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_ACTIVE_FAILED;
    }

    /* If task is not suspended and not in interruptible delay then return */
    if (((OS_TSK_SUSPEND | OS_TSK_DELAY_INTERRUPTIBLE) & taskCb->taskStatus) == 0) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_SUSPENDED;
    }

    TSK_STATUS_CLEAR(taskCb, OS_TSK_SUSPEND);

    /* If task is not blocked then move it to ready list */
    OsMoveTaskToReady(taskCb);
    OsIntRestore(intSave);

    return OS_OK;
}

/*
* 描述：AMP任务初始化
*/
extern U32 g_threadNum;
extern void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, U8 alignPow);
OS_SEC_L4_TEXT U32 OsTskAMPInit(void)
{
    uintptr_t size;
    U32 idx;

    // 简单处理，分配4096,存OS_MAX_TCB_NUM个任务。#define OS_MAX_TCB_NUM  (g_tskMaxNum + 1 + 1)  // 1个IDLE，1个无效任务
    g_tskCbArray = (struct TagTskCb *)OsMemAllocAlign((U32)OS_MID_TSK, 0,
                                                    4096, OS_TSK_STACK_SIZE_ALLOC_ALIGN);
    if (g_tskCbArray == NULL) {
        return OS_ERRNO_TSK_NO_MEMORY;
    }

    g_tskMaxNum = 4096 / sizeof(struct TagTskCb) - 2;


    // 1为Idle任务
    g_threadNum += (g_tskMaxNum + 1);

    // 初始化为全0
    for(int i = 0; i < OS_MAX_TCB_NUM - 1; i++){
        g_tskCbArray[i]=(struct TagTskCb){0};
    }
        

    g_tskBaseId = 0;

    // 将所有控制块加入g_tskCbFreeList链表，且设置控制块的初始状态和任务id
    INIT_LIST_OBJECT(&g_tskCbFreeList);
    for (idx = 0; idx < OS_MAX_TCB_NUM - 1; idx++) {
        g_tskCbArray[idx].taskStatus = OS_TSK_UNUSED;
        g_tskCbArray[idx].taskPid = (idx + g_tskBaseId);
        ListTailAdd(&g_tskCbArray[idx].pendList, &g_tskCbFreeList);
    }

    /* 在初始化时给RUNNING_TASK的PID赋一个合法的无效值，放置在Trace使用时出现异常 */
    RUNNING_TASK = OS_PST_ZOMBIE_TASK;

    /* 在初始化时给RUNNING_TASK的PID赋一个合法的无效值，放置在Trace使用时出现异常 */
    RUNNING_TASK->taskPid = idx + g_tskBaseId;

    INIT_LIST_OBJECT(&g_runQueue);

    /* 增加OS_TSK_INUSE状态，使得在Trace记录的第一条信息状态为OS_TSK_INUSE(创建状态) */
    RUNNING_TASK->taskStatus = (OS_TSK_INUSE | OS_TSK_RUNNING);
    RUNNING_TASK->priority = OS_TSK_PRIORITY_LOWEST + 1;

    return OS_OK;
}

/*
* 描述：任务初始化
*/
OS_SEC_L4_TEXT U32 OsTskInit(void)
{
    U32 ret;
    ret = OsTskAMPInit();
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

/*
* 描述：Idle背景任务
*/
OS_SEC_TEXT void OsTskIdleBgd(void)
{
    while (TRUE);
}

/*
* 描述：ilde任务创建.
*/
OS_SEC_L4_TEXT U32 OsIdleTskAMPCreate(void)
{
    U32 ret;
    TskHandle taskHdl;
    struct TskInitParam taskInitParam = {0};
    char tskName[OS_TSK_NAME_LEN] = "IdleTask";

    /* Create background task. */
    taskInitParam.taskEntry = (TskEntryFunc)OsTskIdleBgd;
    taskInitParam.stackSize = 4096;
    // taskInitParam.name = tskName;
    taskInitParam.taskPrio = OS_TSK_PRIORITY_LOWEST;
    taskInitParam.stackAddr = 0;

    /* 任务调度的必要条件就是有背景任务，此时背景任务还没有创建，因此不会发生任务切换 */
    ret = PRT_TaskCreate(&taskHdl, &taskInitParam);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_TaskResume(taskHdl);
    if (ret != OS_OK) {
        return ret;
    }
    IDLE_TASK_ID = taskHdl;

    return ret;
}

/*
* 描述：激活任务管理
*/
OS_SEC_L4_TEXT U32 OsActivate(void)
{
    U32 ret;
    ret = OsIdleTskAMPCreate();
    if (ret != OS_OK) {
        return ret;
    }

    OsTskHighestSet();

    /* Indicate that background task is running. */
    UNI_FLAG |= OS_FLG_BGD_ACTIVE | OS_FLG_TSK_REQ;

    /* Start Multitasking. */
    OsFirstTimeSwitch();
    // 正常情况不应执行到此
    return OS_ERRNO_TSK_ACTIVE_FAILED;
}