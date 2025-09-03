#include "list.h"

list_elem_t *list_append(linked_list_t *list, list_elem_t *element)
{
    list->count++;

    element->next = NULL;
    element->prev = NULL;

    if (!list->head)
    {
        list->head = list->tail = element;
        return element;
    }

    element->prev = list->tail;
    list->tail->next = element;
    list->tail = element;

    return element;
}
list_elem_t *list_prepend(linked_list_t *list, list_elem_t *element)
{
    list->count++;

    element->prev = NULL;
    element->next = NULL;

    if (!list->head)
    {
        // List empty, new element is both head and tail
        list->head = list->tail = element;
        return element;
    }

    // Insert before current head
    element->next = list->head;
    list->head->prev = element;
    list->head = element;

    return element;
}

void list_print(linked_list_t *list)
{

    printf("\nHEAD: %lx\n", (void *)list->head);
    list_elem_t *tmp = list->head->next;
    while ((void *)tmp != (void *)list->tail)
    {
        printf("%lx\n", (void *)tmp);
        tmp = tmp->next;
    }
    printf("TAIL: %lx\n", (void *)list->tail);
}

void list_delete(linked_list_t *list, list_elem_t *elem)
{
    if (elem->prev)
    {
        elem->prev->next = elem->next;
    }
    else
    {
        list->head = elem->next; // update head if deleting first element
    }

    if (elem->next)
    {
        elem->next->prev = elem->prev;
    }

    // Clear the removed elements pointers to mark it as unlinked
    elem->next = NULL;
    elem->prev = NULL;
}

void list_insert_before(linked_list_t *list, list_elem_t *node, list_elem_t *new_elem)
{
    if (!list || !node || !new_elem)
        return;

    new_elem->next = node;
    new_elem->prev = node->prev;

    if (node->prev)
    {
        node->prev->next = new_elem;
    }
    else
    { // node was head
        list->head = new_elem;
    }

    node->prev = new_elem;
    list->count++;
}

void list_insert_after(linked_list_t *list, list_elem_t *node, list_elem_t *new_elem)
{
    if (!list || !node || !new_elem)
    {
        printf("Param was null on list_insert_after\n");
        return;
    }

    new_elem->prev = node;
    new_elem->next = node->next;

    if (node->next)
    {
        node->next->prev = new_elem;
    }
    else
    { // If was tail
        list->tail = new_elem;
    }

    node->next = new_elem;
    list->count++;
}

void list_move_elem_back(linked_list_t *list, list_elem_t *elem)
{
    // If empty list or already tail, nothing to do
    if (!list->head || elem == list->tail)
    {
        return;
    }

    // Remove the element from its current position
    if (elem->prev)
    {
        elem->prev->next = elem->next;
    }
    else
    {
        // It was the head, update head
        list->head = elem->next;
    }

    if (elem->next)
    {
        elem->next->prev = elem->prev;
    }

    // Make it the new tail
    elem->prev = list->tail;
    elem->next = NULL;

    if (list->tail)
    {
        list->tail->next = elem;
    }
    list->tail = elem;

    // If list was empty except for this element
    if (!list->head)
    {
        list->head = elem;
    }
}