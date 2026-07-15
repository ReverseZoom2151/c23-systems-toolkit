#ifndef IMPERATIVE_TOOLKIT_GENERIC_LIST_H
#define IMPERATIVE_TOOLKIT_GENERIC_LIST_H

#include <stdbool.h>
#include <stddef.h>

/*
 * An owning, circular doubly linked list of pointers with a movable cursor.
 *
 * Values passed to an insert or set function become owned by the list.  The
 * destructor supplied at creation is called once for every still-owned value
 * when it is replaced, erased, or when the list is destroyed.  Passing NULL
 * as the destructor makes the list non-owning.  generic_list_release_current
 * transfers ownership back to the caller instead of calling the destructor.
 * The caller must not insert the same owned pointer more than once.
 */
typedef struct generic_list generic_list;
typedef void (*generic_list_destructor)(void *value);

generic_list *generic_list_create(generic_list_destructor destructor);
void generic_list_destroy(generic_list *list);

size_t generic_list_size(const generic_list *list);
bool generic_list_cursor_is_end(const generic_list *list);
bool generic_list_first(generic_list *list);
bool generic_list_last(generic_list *list);
bool generic_list_next(generic_list *list);
bool generic_list_previous(generic_list *list);

/* Returns false at end or when value is NULL; stored NULL values are valid. */
bool generic_list_get(const generic_list *list, void **value);

/* Replaces the current value, destroying the former value if it differs. */
bool generic_list_set(generic_list *list, void *value);

/* Insert beside the cursor. At end, both functions append to the list. */
bool generic_list_insert_before(generic_list *list, void *value);
bool generic_list_insert_after(generic_list *list, void *value);

/* Erase destroys the current value and moves to its successor or predecessor. */
bool generic_list_erase_to_next(generic_list *list);
bool generic_list_erase_to_previous(generic_list *list);

/* Remove the current item without destroying it and return its value. */
bool generic_list_release_current(generic_list *list, void **value);

#endif
