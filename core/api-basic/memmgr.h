#pragma once
#include <stddef.h>

// define for test case "link against furi memmgr"
#define FURI_MEMMGR_GUARD 1

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t count, size_t size);

size_t memmgr_get_free_heap(void);
size_t memmgr_get_minimum_free_heap(void);
