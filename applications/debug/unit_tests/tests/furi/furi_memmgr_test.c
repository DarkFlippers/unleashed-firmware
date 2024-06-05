#include "../test.h" // IWYU pragma: keep
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

void test_furi_memmgr(void) {
    void* ptr;

    // allocate memory case
    ptr = malloc(100);
    mu_check(ptr != NULL);
    // test that memory is zero-initialized after allocation
    for(int i = 0; i < 100; i++) {
        mu_assert_int_eq(0, ((uint8_t*)ptr)[i]);
    }
    free(ptr);

    // reallocate memory case
    ptr = malloc(100);
    memset(ptr, 66, 100);
    ptr = realloc(ptr, 200);
    mu_check(ptr != NULL);

    // test that memory is really reallocated
    for(int i = 0; i < 100; i++) {
        mu_assert_int_eq(66, ((uint8_t*)ptr)[i]);
    }

    free(ptr);

    // allocate and zero-initialize array (calloc)
    ptr = calloc(100, 2);
    mu_check(ptr != NULL);
    for(int i = 0; i < 100 * 2; i++) {
        mu_assert_int_eq(0, ((uint8_t*)ptr)[i]);
    }
    free(ptr);
}
