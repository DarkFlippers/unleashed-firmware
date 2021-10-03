#include "memmgr.h"
#include <string.h>

extern void* pvPortMalloc(size_t xSize);
extern void vPortFree(void* pv);
extern size_t xPortGetFreeHeapSize(void);
extern size_t xPortGetMinimumEverFreeHeapSize(void);

void* malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void* ptr) {
    vPortFree(ptr);
}

void* realloc(void* ptr, size_t size) {
    if(size == 0) {
        vPortFree(ptr);
        return NULL;
    }

    void* p;
    p = pvPortMalloc(size);
    if(p) {
        // TODO implement secure realloc
        // insecure, but will do job in our case
        if(ptr != NULL) {
            memcpy(p, ptr, size);
            vPortFree(ptr);
        }
    }
    return p;
}

void* calloc(size_t count, size_t size) {
    void* ptr = pvPortMalloc(count * size);
    if(ptr) {
        // zero the memory
        memset(ptr, 0, count * size);
    }
    return ptr;
}

char* strdup(const char* s) {
    const char* s_null = s;
    if(s_null == NULL) {
        return NULL;
    }

    size_t siz = strlen(s) + 1;
    char* y = malloc(siz);

    if(y != NULL) {
        memcpy(y, s, siz);
    } else {
        return NULL;
    }

    return y;
}

size_t memmgr_get_free_heap(void) {
    return xPortGetFreeHeapSize();
}

size_t memmgr_get_minimum_free_heap(void) {
    return xPortGetMinimumEverFreeHeapSize();
}

void* furi_alloc(size_t size) {
    void* p = malloc(size);
    furi_check(p);
    return memset(p, 0, size);
}

void* __wrap__malloc_r(struct _reent* r, size_t size) {
    void* pointer = malloc(size);
    return pointer;
}

void __wrap__free_r(struct _reent* r, void* ptr) {
    free(ptr);
}

void* __wrap__calloc_r(struct _reent* r, size_t count, size_t size) {
    void* pointer = calloc(count, size);
    return pointer;
}

void* __wrap__realloc_r(struct _reent* r, void* ptr, size_t size) {
    void* pointer = realloc(ptr, size);
    return pointer;
}