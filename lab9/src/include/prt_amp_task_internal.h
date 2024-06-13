#ifndef PRT_AMP_TASK_INTERNAL_H
#define PRT_AMP_TASK_INTERNAL_H

#include "prt_task_external.h"
#include "prt_list_external.h"

#define OS_TSK_EN_QUE(runQue, tsk, flags) OsEnqueueTaskAmp((runQue), (tsk))
#define OS_TSK_EN_QUE_HEAD(runQue, tsk, flags) OsEnqueueTaskHeadAmp((runQue), (tsk))
#define OS_TSK_DE_QUE(runQue, tsk, flags) OsDequeueTaskAmp((runQue), (tsk))

extern U32 OsTskAMPInit(void);
extern U32 OsIdleTskAMPCreate(void);

OS_SEC_ALW_INLINE INLINE void OsEnqueueTaskAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    ListTailAdd(&tsk->pendList, runQue);
    return;
}

OS_SEC_ALW_INLINE INLINE void OsEnqueueTaskHeadAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    ListAdd(&tsk->pendList, runQue);
    return;
}

OS_SEC_ALW_INLINE INLINE void OsDequeueTaskAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    ListDelete(&tsk->pendList);
    return;
}

#endif /* PRT_AMP_TASK_INTERNAL_H */