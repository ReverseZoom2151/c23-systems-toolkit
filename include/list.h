#ifndef IMPERATIVE_TOOLKIT_LIST_H
#define IMPERATIVE_TOOLKIT_LIST_H

#include <stdbool.h>
#include <stddef.h>

/* A circular, doubly linked list of integers with a movable cursor. */
typedef struct int_list int_list;

int_list *int_list_create(void);
void int_list_destroy(int_list *list);

size_t int_list_size(const int_list *list);
bool int_list_cursor_is_end(const int_list *list);
bool int_list_first(int_list *list);
bool int_list_last(int_list *list);
bool int_list_next(int_list *list);
bool int_list_previous(int_list *list);

bool int_list_get(const int_list *list, int *value);
bool int_list_set(int_list *list, int value);

/* Insert beside the cursor. At end, both functions append to the list. */
bool int_list_insert_before(int_list *list, int value);
bool int_list_insert_after(int_list *list, int value);

/* Remove the cursor item and leave the cursor at its successor/predecessor. */
bool int_list_erase_to_next(int_list *list);
bool int_list_erase_to_previous(int_list *list);

#endif
