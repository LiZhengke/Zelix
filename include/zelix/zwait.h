#pragma once

#include "zelix/zlist.h"

struct task;

typedef struct wait_node {
    struct task *task;
    struct list_head link;
} wait_node_t;

typedef struct wait_queue {
    struct list_head tasks;
} wait_queue_t;
