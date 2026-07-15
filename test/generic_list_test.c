#include "generic_list.h"

#include <assert.h>
#include <stdlib.h>

static size_t destroyed_count;

static int *number(int value) {
    int *result = malloc(sizeof(*result));
    assert(result != NULL);
    *result = value;
    return result;
}

static void count_and_free(void *value) {
    destroyed_count++;
    free(value);
}

static void expect_current(const generic_list *list, int expected) {
    void *value = NULL;
    assert(generic_list_get(list, &value));
    assert(value != NULL);
    assert(*(int *)value == expected);
}

int main(void) {
    generic_list *list = generic_list_create(count_and_free);
    assert(list != NULL);
    assert(generic_list_size(list) == 0);
    assert(generic_list_cursor_is_end(list));
    assert(!generic_list_first(list));
    assert(!generic_list_get(list, NULL));

    assert(generic_list_insert_after(list, number(20)));
    expect_current(list, 20);
    assert(generic_list_insert_before(list, number(10)));
    expect_current(list, 10);
    assert(generic_list_insert_after(list, number(15)));
    expect_current(list, 15);
    assert(generic_list_size(list) == 3);

    assert(generic_list_first(list));
    expect_current(list, 10);
    assert(generic_list_next(list));
    expect_current(list, 15);
    assert(generic_list_set(list, number(16)));
    assert(destroyed_count == 1);
    expect_current(list, 16);
    assert(generic_list_next(list));
    expect_current(list, 20);
    assert(!generic_list_next(list));
    assert(generic_list_cursor_is_end(list));

    assert(generic_list_last(list));
    assert(generic_list_erase_to_previous(list));
    assert(destroyed_count == 2);
    expect_current(list, 16);
    assert(generic_list_release_current(list, NULL) == false);
    void *released = NULL;
    assert(generic_list_release_current(list, &released));
    assert(*(int *)released == 16);
    assert(destroyed_count == 2);
    free(released);
    assert(generic_list_size(list) == 1);
    assert(generic_list_first(list));
    expect_current(list, 10);
    assert(generic_list_erase_to_next(list));
    assert(destroyed_count == 3);
    assert(generic_list_size(list) == 0);
    assert(generic_list_cursor_is_end(list));
    assert(!generic_list_erase_to_next(list));
    generic_list_destroy(list);
    assert(destroyed_count == 3);

    generic_list *null_values = generic_list_create(NULL);
    assert(null_values != NULL);
    assert(generic_list_insert_before(null_values, NULL));
    void *null_value = (void *)1;
    assert(generic_list_get(null_values, &null_value));
    assert(null_value == NULL);
    generic_list_destroy(null_values);
    generic_list_destroy(NULL);
    return 0;
}
