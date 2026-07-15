#include "list.h"

#include <stdio.h>
#include <stdlib.h>

static void print_state(int_list *list, const char *label) {
    int cursor = 0;
    int has_cursor = int_list_get(list, &cursor);
    printf("%-24s ", label);
    if (!int_list_first(list)) {
        puts("[end]");
        return;
    }
    do {
        int value = 0;
        (void)int_list_get(list, &value);
        printf("[%d]", value);
        if (int_list_next(list)) {
            fputs(" <-> ", stdout);
        }
    } while (!int_list_cursor_is_end(list));
    if (has_cursor) {
        printf("    cursor: %d\n", cursor);
    } else {
        puts("    cursor: end");
    }
}

int main(void) {
    int_list *list = int_list_create();
    if (list == NULL) {
        fputs("list-demo: allocation failure\n", stderr);
        return EXIT_FAILURE;
    }

    if (!int_list_insert_after(list, 20)) {
        int_list_destroy(list);
        return EXIT_FAILURE;
    }
    print_state(list, "insert after end (20):");

    (void)int_list_first(list);
    if (!int_list_insert_before(list, 10)) {
        int_list_destroy(list);
        return EXIT_FAILURE;
    }
    print_state(list, "insert before (10):");

    (void)int_list_last(list);
    if (!int_list_insert_before(list, 15)) {
        int_list_destroy(list);
        return EXIT_FAILURE;
    }
    print_state(list, "insert before (15):");

    (void)int_list_last(list);
    if (!int_list_erase_to_previous(list)) {
        int_list_destroy(list);
        return EXIT_FAILURE;
    }
    print_state(list, "erase 20, move back:");

    int_list_destroy(list);
    return EXIT_SUCCESS;
}
