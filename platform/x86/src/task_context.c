#include <string.h>

#include "ktask.h"
#include "task_context.h"

#define USER_CS         0x1B
#define USER_DS         0x23
#define USER_EFLAGS     0x202


void arch_task_setup_frame_context(struct task *t)
{
    struct trap_frame *tf;
    uint32_t* sp = (uint32_t*)t->tcb;

    sp -= sizeof(struct trap_frame) / sizeof(uint32_t); /* Reserve space for trap frame */
    tf = (struct trap_frame *)sp;

    t->tcb = (tcb_t)sp;  /* Set the task's top of stack to the new value after reserving space for the trap frame. */

    memset(tf, 0, sizeof(*tf));
    tf->eip = (uint32_t)t->entry_virt; /* User entry point virtual address. */

    tf->ds = tf->es = tf->fs = tf->gs = tf->ss = USER_DS;

    tf->cs = USER_CS;
    tf->eflags = USER_EFLAGS;
    tf->esp = (uint32_t)t->user_stack_top;

    tf->eax = 0;

    t->frame_ctx = tf;
}

void save_user_ctx(struct task *t, struct trap_frame *tf)
{
    /*t->frame_ctx->eip = tf->eip;
    t->frame_ctx->esp = (tf->cs & 0x3) ? tf->esp : t->frame_ctx->esp;
    t->frame_ctx->eflags = tf->eflags;

    t->frame_ctx->eax = tf->eax;
    t->frame_ctx->ebx = tf->ebx;
    t->frame_ctx->ecx = tf->ecx;
    t->frame_ctx->edx = tf->edx;
    t->frame_ctx->esi = tf->esi;
    t->frame_ctx->edi = tf->edi;
    t->frame_ctx->ebp = tf->ebp;*/
}

void load_user_ctx(struct task *t, struct trap_frame *tf)
{
    /*tf->eip = t->frame_ctx->eip;
    tf->esp = t->frame_ctx->esp;
    tf->eflags = t->frame_ctx->eflags;

    tf->eax = t->frame_ctx->eax;
    tf->ebx = t->frame_ctx->ebx;
    tf->ecx = t->frame_ctx->ecx;
    tf->edx = t->frame_ctx->edx;
    tf->esi = t->frame_ctx->esi;
    tf->edi = t->frame_ctx->edi;
    tf->ebp = t->frame_ctx->ebp;*/
}
