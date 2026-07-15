#include "list.h"

#include <stdlib.h>

typedef struct int_list_node {
    int value;
    struct int_list_node *previous;
    struct int_list_node *next;
} int_list_node;

struct int_list {
    int_list_node sentinel;
    int_list_node *cursor;
    size_t size;
};

static bool insert_between(int_list *list, int_list_node *previous,
                           int_list_node *next, int value) {
    int_list_node *node = malloc(sizeof(*node));
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

int_list *int_list_create(void) {
    int_list *list = malloc(sizeof(*list));
    if (list == NULL) {
        return NULL;
    }

    list->sentinel.previous = &list->sentinel;
    list->sentinel.next = &list->sentinel;
    list->cursor = &list->sentinel;
    list->size = 0;
    return list;
}

void int_list_destroy(int_list *list) {
    if (list == NULL) {
        return;
    }

    int_list_node *node = list->sentinel.next;
    while (node != &list->sentinel) {
        int_list_node *next = node->next;
        free(node);
        node = next;
    }
    free(list);
}

size_t int_list_size(const int_list *list) {
    return list == NULL ? 0 : list->size;
}

bool int_list_cursor_is_end(const int_list *list) {
    return list == NULL || list->cursor == &list->sentinel;
}

bool int_list_first(int_list *list) {
    if (list == NULL || list->size == 0) {
        return false;
    }
    list->cursor = list->sentinel.next;
    return true;
}

bool int_list_last(int_list *list) {
    if (list == NULL || list->size == 0) {
        return false;
    }
    list->cursor = list->sentinel.previous;
    return true;
}

bool int_list_next(int_list *list) {
    if (int_list_cursor_is_end(list)) {
        return false;
    }
    list->cursor = list->cursor->next;
    return !int_list_cursor_is_end(list);
}

bool int_list_previous(int_list *list) {
    if (int_list_cursor_is_end(list)) {
        return false;
    }
    list->cursor = list->cursor->previous;
    return !int_list_cursor_is_end(list);
}

bool int_list_get(const int_list *list, int *value) {
    if (int_list_cursor_is_end(list) || value == NULL) {
        return false;
    }
    *value = list->cursor->value;
    return true;
}

bool int_list_set(int_list *list, int value) {
    if (int_list_cursor_is_end(list)) {
        return false;
    }
    list->cursor->value = value;
    return true;
}

bool int_list_insert_before(int_list *list, int value) {
    if (list == NULL) {
        return false;
    }
    int_list_node *next = list->cursor;
    return insert_between(list, next->previous, next, value);
}

bool int_list_insert_after(int_list *list, int value) {
    if (list == NULL) {
        return false;
    }
    int_list_node *previous = list->cursor;
    return insert_between(list, previous, previous->next, value);
}

static bool erase_current(int_list *list, bool move_to_next) {
    if (int_list_cursor_is_end(list)) {
        return false;
    }

    int_list_node *node = list->cursor;
    list->cursor = move_to_next ? node->next : node->previous;
    node->previous->next = node->next;
    node->next->previous = node->previous;
    free(node);
    list->size--;
    return true;
}

bool int_list_erase_to_next(int_list *list) {
    return erase_current(list, true);
}

bool int_list_erase_to_previous(int_list *list) {
    return erase_current(list, false);
}
