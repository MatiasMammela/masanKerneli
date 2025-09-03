#pragma once
#include "lib.h"
#include "logger.h"
#define LIST_INIT (linked_list_t){.head = NULL, .tail = NULL, .count = 0}
#define ELEM_INIT \
    (list_elem_t) { .next = NULL, .prev = NULL }
#define LIST_LOOP(LIST, NODE) for (list_elem_t * (NODE) = (LIST)->head; (NODE) != NULL; (NODE) = (NODE)->next)
#define LIST_ENTRY(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

typedef struct list_elem_t
{
    struct list_elem_t *next;
    struct list_elem_t *prev;
} list_elem_t;

typedef struct linked_list
{
    struct list_elem_t *head;
    struct list_elem_t *tail;
    size_t count;
} linked_list_t;
void list_insert_after(linked_list_t *list, list_elem_t *node, list_elem_t *new_elem);
void list_insert_before(linked_list_t *list, list_elem_t *node, list_elem_t *new_elem);
void list_print(linked_list_t *list);
list_elem_t *list_append(linked_list_t *list, list_elem_t *element);
void list_delete(linked_list_t *list, list_elem_t *element);
void list_move_elem_back(linked_list_t *list, list_elem_t *node);
list_elem_t *list_prepend(linked_list_t *list, list_elem_t *element);