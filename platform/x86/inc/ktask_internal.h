#pragma once
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"

typedef TaskHandle_t tcb_t;
/* =========================
   用户态上下文
   ========================= */
struct user_context {
    uint32_t eip;
    uint32_t esp;
    uint32_t eflags;

    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp;
};
