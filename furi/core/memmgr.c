#include "memmgr.h"
#include "common_defines.h"
#include <string.h>

extern void* pvPortMalloc(size_t xSize);
extern void vPortFree(void* pv);
extern size_t xPortGetFreeHeapSize(void);
extern size_t xPortGetTotalHeapSize(void);
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

    void* p = pvPortMalloc(size);
    if(ptr != NULL) {
        memcpy(p, ptr, size);
        vPortFree(ptr);
    }

    return p;
}

void* calloc(size_t count, size_t size) {
    return pvPortMalloc(count * size);
}

char* strdup(const char* s) {
    // arg s marked as non-null, so we need hack to check for NULL
    furi_check(((uint32_t)s << 2) != 0);

    size_t siz = strlen(s) + 1;
    char* y = pvPortMalloc(siz);
    memcpy(y, s, siz);

    return y;
}

size_t memmgr_get_free_heap(void) {
    return xPortGetFreeHeapSize();
}

size_t memmgr_get_total_heap(void) {
    return xPortGetTotalHeapSize();
}

size_t memmgr_get_minimum_free_heap(void) {
    return xPortGetMinimumEverFreeHeapSize();
}

void* __wrap__malloc_r(struct _reent* r, size_t size) {
    UNUSED(r);
    return pvPortMalloc(size);
}

void __wrap__free_r(struct _reent* r, void* ptr) {
    UNUSED(r);
    vPortFree(ptr);
}

void* __wrap__calloc_r(struct _reent* r, size_t count, size_t size) {
    UNUSED(r);
    return calloc(count, size);
}

void* __wrap__realloc_r(struct _reent* r, void* ptr, size_t size) {
    UNUSED(r);
    return realloc(ptr, size);
}
