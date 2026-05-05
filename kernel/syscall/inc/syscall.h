#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "arch.h"

#define SYSINT portSYSCALL_INT_VECTOR
#define STR2(x) #x
#define STR(x) STR2(x)

#define ENOSYS   38   // Syscall does not exist
#define EINVAL   22   // Invalid argument
#define EPERM     1   // Permission denied

enum {
    SYS_YIELD = 0,
    SYS_WRITE,
    SYS_DELAY,
    SYS_EXIT,
    SYS_TIME_GET,
    SYS_SEM_PEND,
    SYS_SEM_POST,
    SYS_PUTC,
    SYS_PRINTF,
    SYS_PANIC,
    SYS_TASK_CREATE,
    SYS_TICK_COUNT,
    SYS_GET_TASK_NAME,
    SYS_EXEC,
    SYS_READ,
    SYS_MAX
};

int syscall_dispatch(void);
int os_err_to_errno(OS_ERR err);

int32_t uSysPutChar(char c);
int32_t uSysDelay(uint16_t ticks);
int32_t uSysTaskCreate(void (*taskFunction)(void *), const char *taskName,
                       uint16_t stackDepth, uint32_t priority,
                       void *pvParameters);
int32_t uSysGetTickCount(void);
int32_t uSysPrintf(const char *fmt, ...);
int32_t uSysGetTaskName(char *buf, uint32_t len);
#endif /* SYSCALL_H */
