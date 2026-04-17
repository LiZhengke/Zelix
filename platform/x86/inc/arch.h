#pragma once

#include <stdint.h>

#define ENOSYS   38   // syscall not implemented
#define EINVAL   22   // invalid argument
#define EPERM     1   // operation not permitted

typedef enum {
    OS_ERR_NONE = 0,
    OS_ERR_INVALID,
    OS_ERR_TIMEOUT,
    OS_ERR_RESOURCE,
    OS_ERR_PERM,
} OS_ERR;


static inline uint32_t get_cpl( void )
{
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    return cs & 0x3;
}
