#include "prt_typedef.h"
#include "prt_shell.h"
#include "os_attr_armv8_external.h"
#include "prt_task.h"
#include "prt_sem.h"

extern SemHandle sem_uart_rx;
extern U32 PRT_Printf(const char *format, ...);
extern void OsDisplayTasksInfo(void);
extern void OsDisplayCurTick(void);


OS_SEC_TEXT void ShellTask(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 ret;
    char ch;
    char cmd[SHELL_SHOW_MAX_LEN];
    U32 idx;
    ShellCB *shellCB = (ShellCB *)param1;

    while (1) {
        PRT_Printf("\nminiEuler # ");
        idx = 0;
        for(int i = 0; i < SHELL_SHOW_MAX_LEN; i++)
        {
            cmd[i] = 0;
        }

        while (1){
            PRT_SemPend(sem_uart_rx, OS_WAIT_FOREVER);

            // 读取shellCB缓冲区的字符
            ch = shellCB->shellBuf[shellCB->shellBufReadOffset];
            cmd[idx] = ch;
            idx++;
            shellCB->shellBufReadOffset++;
            if(shellCB->shellBufReadOffset == SHELL_SHOW_MAX_LEN)
                shellCB->shellBufReadOffset = 0;

            PRT_Printf("%c", ch); //回显
            if (ch == '\r'){
                // PRT_Printf("\n");
                if(cmd[0]=='t' && cmd[1]=='o' && cmd[2]=='p'){
                    OsDisplayTasksInfo();
                } else if(cmd[0]=='t' && cmd[1]=='i' && cmd[2]=='c' && cmd[3]=='k'){
                    OsDisplayCurTick();
                }
                break;
            }

        }
    }
}

OS_SEC_TEXT U32 ShellTaskInit(ShellCB *shellCB)
{
    U32 ret = 0;
    struct TskInitParam param = {0};

    // task 1
    // param.stackAddr = 0;
    param.taskEntry = (TskEntryFunc)ShellTask;
    param.taskPrio = 9;
    // param.name = "Test1Task";
    param.stackSize = 0x1000; //固定4096，参见prt_task_init.c的OsMemAllocAlign
    param.args[0] = (uintptr_t)shellCB;

    TskHandle tskHandle1;
    ret = PRT_TaskCreate(&tskHandle1, &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(tskHandle1);
    if (ret) {
        return ret;
    }
}