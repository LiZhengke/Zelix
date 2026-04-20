#pragma once

#include "interrupt.h"

struct task;

void arch_task_setup_frame_context(struct task *t);
void save_user_ctx(struct task *t, struct trap_frame *tf);
void load_user_ctx(struct task *t, struct trap_frame *tf);
void return_to_user(struct trap_frame *tf);
