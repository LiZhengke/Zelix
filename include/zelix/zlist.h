#pragma once

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void list_add(
    struct list_head *new,
    struct list_head *head)
{
    new->next = head->next;
    new->prev = head;

    head->next->prev = new;
    head->next = new;
}

static inline void list_add_tail(
    struct list_head *new,
    struct list_head *head)
{
    new->prev = head->prev;
    new->next = head;

    head->prev->next = new;
    head->prev = new;
}

static inline void list_del(struct list_head *entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;

    entry->next = NULL;
    entry->prev = NULL;
}

#ifndef offsetof
#define offsetof(TYPE, MEMBER) \
    ((size_t)&((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)
