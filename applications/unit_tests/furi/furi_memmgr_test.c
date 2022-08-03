#include "../minunit.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// this test is not accurate, but gives a basic understanding
// that memory management is working fine

// do not include memmgr.h here
// we also test that we are linking against stdlib
extern size_t memmgr_get_free_heap(void);
extern size_t memmgr_get_minimum_free_heap(void);

// current heap managment realization consume:
// X bytes after allocate and 0 bytes after allocate and free,
// where X = sizeof(void*) + sizeof(size_t), look to BlockLink_t
const size_t heap_overhead_max_size = sizeof(void*) + sizeof(size_t);

bool heap_equal(size_t heap_size, size_t heap_size_old) {
    // heap borders with overhead
    const size_t heap_low = heap_size_old - heap_overhead_max_size;
    const size_t heap_high = heap_size_old + heap_overhead_max_size;

    // not extact, so we must test it against bigger numbers than "overhead size"
    const bool result = ((heap_size >= heap_low) && (heap_size <= heap_high));

    // debug allocation info
    if(!result) {
        printf("\n(hl: %zu) <= (p: %zu) <= (hh: %zu)\n", heap_low, heap_size, heap_high);
    }

    return result;
}

void test_furi_memmgr() {
    size_t heap_size = 0;
    size_t heap_size_old = 0;
    const int alloc_size = 128;

    void* ptr = NULL;
    void* original_ptr = NULL;

    // do not include furi memmgr.h case
#ifdef FURI_MEMMGR_GUARD
    mu_fail("do not link against furi memmgr.h");
#endif

    // allocate memory case
    heap_size_old = memmgr_get_free_heap();
    ptr = malloc(alloc_size);
    heap_size = memmgr_get_free_heap();
    mu_assert_pointers_not_eq(ptr, NULL);
    mu_assert(heap_equal(heap_size, heap_size_old - alloc_size), "allocate failed");

    // free memory case
    heap_size_old = memmgr_get_free_heap();
    free(ptr);
    ptr = NULL;
    heap_size = memmgr_get_free_heap();
    mu_assert(heap_equal(heap_size, heap_size_old + alloc_size), "free failed");

    // reallocate memory case

    // get filled array with some data
    original_ptr = malloc(alloc_size);
    mu_assert_pointers_not_eq(original_ptr, NULL);
    for(int i = 0; i < alloc_size; i++) {
        *(unsigned char*)(original_ptr + i) = i;
    }

    // malloc array and copy data
    ptr = malloc(alloc_size);
    mu_assert_pointers_not_eq(ptr, NULL);
    memcpy(ptr, original_ptr, alloc_size);

    // reallocate array
    heap_size_old = memmgr_get_free_heap();
    ptr = realloc(ptr, alloc_size * 2);
    heap_size = memmgr_get_free_heap();
    mu_assert(heap_equal(heap_size, heap_size_old - alloc_size), "reallocate failed");
    mu_assert_int_eq(memcmp(original_ptr, ptr, alloc_size), 0);
    free(original_ptr);
    free(ptr);

    // allocate and zero-initialize array (calloc)
    original_ptr = malloc(alloc_size);
    mu_assert_pointers_not_eq(original_ptr, NULL);

    for(int i = 0; i < alloc_size; i++) {
        *(unsigned char*)(original_ptr + i) = 0;
    }
    heap_size_old = memmgr_get_free_heap();
    ptr = calloc(1, alloc_size);
    heap_size = memmgr_get_free_heap();
    mu_assert(heap_equal(heap_size, heap_size_old - alloc_size), "callocate failed");
    mu_assert_int_eq(memcmp(original_ptr, ptr, alloc_size), 0);

    free(original_ptr);
    free(ptr);
}
