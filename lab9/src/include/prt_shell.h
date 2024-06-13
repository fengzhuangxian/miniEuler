#ifndef _HWLITEOS_SHELL_H
#define _HWLITEOS_SHELL_H

#include "prt_typedef.h"

#define SHELL_SHOW_MAX_LEN    272
#define PATH_MAX        1024

typedef struct {
    U32   consoleID;
    U32   shellTaskHandle;
    U32   shellEntryHandle;
    void     *cmdKeyLink;
    void     *cmdHistoryKeyLink;
    void     *cmdMaskKeyLink;
    U32   shellBufOffset;
    U32   shellBufReadOffset;
    U32   shellKeyType;
    char     shellBuf[SHELL_SHOW_MAX_LEN];
    char     shellWorkingDirectory[PATH_MAX];
} ShellCB;

#endif /* _HWLITEOS_SHELL_H */