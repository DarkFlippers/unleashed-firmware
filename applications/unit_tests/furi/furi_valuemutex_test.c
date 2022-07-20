#include <stdio.h>
#include <string.h>
#include <furi.h>

#include "../minunit.h"

void test_furi_valuemutex() {
    const int init_value = 0xdeadbeef;
    const int changed_value = 0x12345678;

    int value = init_value;
    bool result;
    ValueMutex valuemutex;

    // init mutex case
    result = init_mutex(&valuemutex, &value, sizeof(value));
    mu_assert(result, "init mutex failed");

    // acquire mutex case
    int* value_pointer = acquire_mutex(&valuemutex, 100);
    mu_assert_pointers_eq(value_pointer, &value);

    // second acquire mutex case
    int* value_pointer_second = acquire_mutex(&valuemutex, 100);
    mu_assert_pointers_eq(value_pointer_second, NULL);

    // change value case
    *value_pointer = changed_value;
    mu_assert_int_eq(value, changed_value);

    // release mutex case
    result = release_mutex(&valuemutex, &value);
    mu_assert(result, "release mutex failed");

    // TODO
    //acquire mutex blocking case
    //write mutex blocking case
    //read mutex blocking case

    mu_check(delete_mutex(&valuemutex));
}
