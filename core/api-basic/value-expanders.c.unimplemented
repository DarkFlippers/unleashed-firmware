#include "value-expanders.h"

bool commit_managed(ValueManager* managed, void* value) {
    if(value != managed->mutex->value) return false;

    notify_pubsub(&managed->pubsub, value);
    
    if(!osMutexGive(managed->mutex)) return false;
    
    return true;
}

bool write_managed(ValueManager* managed, void* data, size_t len, uint32_t timeout) {
    void* value = acquire_mutex(managed->mutex, timeout);
    if(value == NULL) return false;

    memcpy(value, data, len):

    notify_pubsub(&managed->pubsub, value);

    if(!release_mutex(managed->mutex, value)) return false;
    
    return true;
}
