#include "generic_list.h"

#include <stdlib.h>

typedef struct generic_list_node {
    void *value;
    struct generic_list_node *previous;
    struct generic_list_node *next;
} generic_list_node;

struct generic_list {
    generic_list_node sentinel;
    generic_list_node *cursor;
    generic_list_destructor destructor;
    size_t size;
};

static bool generic_list_insert_between(generic_list *list,
                                        generic_list_node *previous,
                                        generic_list_node *next, void *value) {
    generic_list_node *node = malloc(sizeof(*node));
    if (node == NULL) {
        return false;
    }

    node->value = value;
    node->previous = previous;
    node->next = next;
    previous->next = node;
    next->previous = node;
    list->cursor = node;
    list->size++;
    return true;
}

static void generic_list_destroy_value(const generic_list *list, void *value) {
    if (list->destructor != NULL) {
        list->destructor(value);
    }
}

generic_list *generic_list_create(generic_list_destructor destructor) {
    generic_list *list = malloc(sizeof(*list));
    if (list == NULL) {
        return NULL;
    }

    list->sentinel.previous = &list->sentinel;
    list->sentinel.next = &list->sentinel;
    list->cursor = &list->sentinel;
    list->destructor = destructor;
    list->size = 0;
    return list;
}

void generic_list_destroy(generic_list *list) {
    if (list == NULL) {
        return;
    }

    generic_list_node *node = list->sentinel.next;
    while (node != &list->sentinel) {
        generic_list_node *next = node->next;
        generic_list_destroy_value(list, node->value);
        free(node);
        node = next;
    }
    free(list);
}

size_t generic_list_size(const generic_list *list) {
    return list == NULL ? 0 : list->size;
}

bool generic_list_cursor_is_end(const generic_list *list) {
    return list == NULL || list->cursor == &list->sentinel;
}

bool generic_list_first(generic_list *list) {
    if (list == NULL || list->size == 0) {
        return false;
    }
    list->cursor = list->sentinel.next;
    return true;
}

bool generic_list_last(generic_list *list) {
    if (list == NULL || list->size == 0) {
        return false;
    }
    list->cursor = list->sentinel.previous;
    return true;
}

bool generic_list_next(generic_list *list) {
    if (generic_list_cursor_is_end(list)) {
        return false;
    }
    list->cursor = list->cursor->next;
    return !generic_list_cursor_is_end(list);
}

bool generic_list_previous(generic_list *list) {
    if (generic_list_cursor_is_end(list)) {
        return false;
    }
    list->cursor = list->cursor->previous;
    return !generic_list_cursor_is_end(list);
}

bool generic_list_get(const generic_list *list, void **value) {
    if (generic_list_cursor_is_end(list) || value == NULL) {
        return false;
    }
    *value = list->cursor->value;
    return true;
}

bool generic_list_set(generic_list *list, void *value) {
    if (generic_list_cursor_is_end(list)) {
        return false;
    }
    if (list->cursor->value != value) {
        generic_list_destroy_value(list, list->cursor->value);
        list->cursor->value = value;
    }
    return true;
}

bool generic_list_insert_before(generic_list *list, void *value) {
    if (list == NULL) {
        return false;
    }
    return generic_list_insert_between(list, list->cursor->previous,
                                       list->cursor, value);
}

bool generic_list_insert_after(generic_list *list, void *value) {
    if (list == NULL) {
        return false;
    }
    return generic_list_insert_between(list, list->cursor,
                                       list->cursor->next, value);
}

static bool generic_list_remove_current(generic_list *list, bool move_to_next,
                                        void **value) {
    if (generic_list_cursor_is_end(list)) {
        return false;
    }

    generic_list_node *node = list->cursor;
    list->cursor = move_to_next ? node->next : node->previous;
    node->previous->next = node->next;
    node->next->previous = node->previous;
    list->size--;

    if (value != NULL) {
        *value = node->value;
    } else {
        generic_list_destroy_value(list, node->value);
    }
    free(node);
    return true;
}

bool generic_list_erase_to_next(generic_list *list) {
    return generic_list_remove_current(list, true, NULL);
}

bool generic_list_erase_to_previous(generic_list *list) {
    return generic_list_remove_current(list, false, NULL);
}

bool generic_list_release_current(generic_list *list, void **value) {
    if (value == NULL) {
        return false;
    }
    return generic_list_remove_current(list, true, value);
}
