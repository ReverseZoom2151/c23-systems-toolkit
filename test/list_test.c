#include "list.h"

#include <assert.h>
#include <stddef.h>

static void expect_current(const int_list *list, int expected) {
    int value = 0;
    assert(int_list_get(list, &value));
    assert(value == expected);
}

int main(void) {
    int_list *list = int_list_create();
    assert(list != NULL);
    assert(int_list_size(list) == 0);
    assert(int_list_cursor_is_end(list));
    assert(!int_list_first(list));
    assert(!int_list_get(list, NULL));

    assert(int_list_insert_after(list, 20));
    expect_current(list, 20);
    assert(int_list_insert_before(list, 10));
    expect_current(list, 10);
    assert(int_list_insert_after(list, 15));
    expect_current(list, 15);
    assert(int_list_size(list) == 3);

    assert(int_list_first(list));
    expect_current(list, 10);
    assert(int_list_next(list));
    expect_current(list, 15);
    assert(int_list_set(list, 16));
    expect_current(list, 16);
    assert(int_list_next(list));
    expect_current(list, 20);
    assert(!int_list_next(list));
    assert(int_list_cursor_is_end(list));
    assert(!int_list_next(list));

    assert(int_list_last(list));
    assert(int_list_erase_to_previous(list));
    expect_current(list, 16);
    assert(int_list_erase_to_next(list));
    assert(int_list_cursor_is_end(list));
    assert(int_list_size(list) == 1);
    assert(int_list_first(list));
    expect_current(list, 10);
    assert(int_list_erase_to_next(list));
    assert(int_list_size(list) == 0);
    assert(int_list_cursor_is_end(list));
    assert(!int_list_erase_to_next(list));

    int_list_destroy(list);
    int_list_destroy(NULL);
    return 0;
}
