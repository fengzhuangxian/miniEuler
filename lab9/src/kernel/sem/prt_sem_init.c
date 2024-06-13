#include "prt_sem_external.h"
#include "os_attr_armv8_external.h"
#include "os_cpu_armv8_external.h"

OS_SEC_BSS struct TagListObject g_unusedSemList;
OS_SEC_BSS struct TagSemCb *g_allSem;

extern void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, U8 alignPow);
/*
* 描述：信号量初始化
*/
OS_SEC_L4_TEXT U32 OsSemInit(void)
{
    struct TagSemCb *semNode = NULL;
    U32 idx;
    U32 ret = OS_OK;

    g_allSem = (struct TagSemCb *)OsMemAllocAlign((U32)OS_MID_SEM,
                                                0,
                                                4096,
                                                OS_SEM_ADDR_ALLOC_ALIGN);

    if (g_allSem == NULL) {
        return OS_ERRNO_SEM_NO_MEMORY;
    }

    g_maxSem = 4096 / sizeof(struct TagSemCb);

    char *cg_allSem = (char *)g_allSem;
    for(int i = 0; i < 4096; i++)
        cg_allSem[i] = 0;

    INIT_LIST_OBJECT(&g_unusedSemList);
    for (idx = 0; idx < g_maxSem; idx++) {
        semNode = ((struct TagSemCb *)g_allSem) + idx; //指针操作
        semNode->semId = (U16)idx;
        ListTailAdd(&semNode->semList, &g_unusedSemList);
    }

    return ret;
}

/*
* 描述：创建一个信号量
*/
OS_SEC_L4_TEXT U32 OsSemCreate(U32 count, U32 semType, enum SemMode semMode,
                            SemHandle *semHandle, U32 cookie)
{
    uintptr_t intSave;
    struct TagSemCb *semCreated = NULL;
    struct TagListObject *unusedSem = NULL;
    (void)cookie;

    if (semHandle == NULL) {
        return OS_ERRNO_SEM_PTR_NULL;
    }

    intSave = OsIntLock();

    if (ListEmpty(&g_unusedSemList)) {
        OsIntRestore(intSave);
        return OS_ERRNO_SEM_ALL_BUSY;
    }

    /* 在空闲链表中取走一个控制节点 */
    unusedSem = OS_LIST_FIRST(&(g_unusedSemList));
    ListDelete(unusedSem);

    /* 获取到空闲节点对应的信号量控制块，并开始填充控制块 */
    semCreated = (GET_SEM_LIST(unusedSem));
    semCreated->semCount = count;
    semCreated->semStat = OS_SEM_USED;
    semCreated->semMode = semMode;
    semCreated->semType = semType;
    semCreated->semOwner = OS_INVALID_OWNER_ID;
    if (GET_SEM_TYPE(semType) == SEM_TYPE_BIN) {
        INIT_LIST_OBJECT(&semCreated->semBList);
#if defined(OS_OPTION_SEM_RECUR_PV)
        if (GET_MUTEX_TYPE(semType) == PTHREAD_MUTEX_RECURSIVE) {
            semCreated->recurCount = 0;
        }
#endif
    }

    INIT_LIST_OBJECT(&semCreated->semList);
    *semHandle = (SemHandle)semCreated->semId;

    OsIntRestore(intSave);
    return OS_OK;
}

/*
* 描述：创建一个信号量
*/
OS_SEC_L4_TEXT U32 PRT_SemCreate(U32 count, SemHandle *semHandle)
{
    U32 ret;

    if (count > OS_SEM_COUNT_MAX) {
        return OS_ERRNO_SEM_OVERFLOW;
    }

    ret = OsSemCreate(count, SEM_TYPE_COUNT, SEM_MODE_FIFO, semHandle, (U32)(uintptr_t)semHandle);
    return ret;
}