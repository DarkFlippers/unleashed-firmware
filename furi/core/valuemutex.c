#include "valuemutex.h"

#include <string.h>

bool init_mutex(ValueMutex* valuemutex, void* value, size_t size) {
    // mutex without name,
    // no attributes (unfortunately robust mutex is not supported by FreeRTOS),
    // with dynamic memory allocation
    valuemutex->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(valuemutex->mutex == NULL) return false;

    valuemutex->value = value;
    valuemutex->size = size;

    return true;
}

bool delete_mutex(ValueMutex* valuemutex) {
    if(furi_mutex_acquire(valuemutex->mutex, FuriWaitForever) == FuriStatusOk) {
        furi_mutex_free(valuemutex->mutex);
        return true;
    } else {
        return false;
    }
}

void* acquire_mutex(ValueMutex* valuemutex, uint32_t timeout) {
    if(furi_mutex_acquire(valuemutex->mutex, timeout) == FuriStatusOk) {
        return valuemutex->value;
    } else {
        return NULL;
    }
}

bool release_mutex(ValueMutex* valuemutex, const void* value) {
    if(value != valuemutex->value) return false;

    if(furi_mutex_release(valuemutex->mutex) != FuriStatusOk) return false;

    return true;
}

bool read_mutex(ValueMutex* valuemutex, void* data, size_t len, uint32_t timeout) {
    void* value = acquire_mutex(valuemutex, timeout);
    if(value == NULL || len > valuemutex->size) return false;
    memcpy(data, value, len > 0 ? len : valuemutex->size);
    if(!release_mutex(valuemutex, value)) return false;

    return true;
}

bool write_mutex(ValueMutex* valuemutex, void* data, size_t len, uint32_t timeout) {
    void* value = acquire_mutex(valuemutex, timeout);
    if(value == NULL || len > valuemutex->size) return false;
    memcpy(value, data, len > 0 ? len : valuemutex->size);
    if(!release_mutex(valuemutex, value)) return false;

    return true;
}
