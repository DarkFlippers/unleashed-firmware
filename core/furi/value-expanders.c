#include "value-expanders.h"

bool init_composer(ValueComposer* composer, void* value) {
    if(!init_mutex(&composer->value, value, 0)) return false;

    for(size_t i = 0; i < sizeof(composer->layers) / sizeof(composer->layers[0]); i++) {
        list_composer_cb_init(composer->layers[i]);
    }

    // mutex without name,
    // no attributes (unfortunatly robust mutex is not supported by FreeRTOS),
    // with dynamic memory allocation
    composer->mutex = osMutexNew(NULL);
    if(composer->mutex == NULL) return false;

    if(!init_event(&composer->request)) return false;

    return true;
}

bool delete_composer(ValueComposer* composer) {
    if(osMutexAcquire(composer->mutex, osWaitForever) == osOK) {
        bool result = true;
        result &= delete_mutex(&composer->value);

        for(size_t i = 0; i < sizeof(composer->layers) / sizeof(composer->layers[0]); i++) {
            list_composer_cb_clear(composer->layers[i]);
        }

        result &= osMutexDelete(composer->mutex) == osOK;

        return result;
    } else {
        return false;
    }
}

ValueComposerHandle*
add_compose_layer(ValueComposer* composer, ValueComposerCallback cb, void* ctx, UiLayer layer) {
    if(osMutexAcquire(composer->mutex, osWaitForever) == osOK) {
        // put uninitialized item to the list
        ValueComposerHandle* handle = list_composer_cb_push_raw(composer->layers[layer]);

        handle->cb = cb;
        handle->ctx = ctx;
        handle->layer = layer;
        handle->composer = composer;

        // TODO unregister handle on app exit
        //flapp_on_exit(remove_compose_layer, handle);

        osMutexRelease(composer->mutex);

        // Layers changed, request composition
        signal_event(&composer->request);

        return handle;
    } else {
        return NULL;
    }
}

bool remove_compose_layer(ValueComposerHandle* handle) {
    ValueComposer* composer = handle->composer;

    if(osMutexAcquire(composer->mutex, osWaitForever) == osOK) {
        bool result = false;

        // iterate over items
        list_composer_cb_it_t it;
        for(list_composer_cb_it(it, composer->layers[handle->layer]); !list_composer_cb_end_p(it);
            list_composer_cb_next(it)) {
            const ValueComposerHandle* item = list_composer_cb_cref(it);

            // if the iterator is equal to our element
            if(item == handle) {
                list_composer_cb_remove(composer->layers[handle->layer], it);
                result = true;
                break;
            }
        }

        osMutexRelease(composer->mutex);

        // Layers changed, request composition
        signal_event(&composer->request);

        return result;
    } else {
        return false;
    }
}

void request_compose(ValueComposerHandle* handle) {
    ValueComposer* composer = handle->composer;
    signal_event(&composer->request);
}

void perform_compose(
    ValueComposer* composer,
    ValueComposerCallback start_cb,
    ValueComposerCallback end_cb,
    void* ctx) {
    if(!wait_event_with_timeout(&composer->request, 0)) return;

    void* state = acquire_mutex(&composer->value, 0);
    if(state == NULL) return;

    if(start_cb != NULL) start_cb(ctx, state);
    perform_compose_internal(composer, state);
    if(end_cb != NULL) end_cb(ctx, state);

    release_mutex(&composer->value, state);
}

void perform_compose_internal(ValueComposer* composer, void* state) {
    if(osMutexAcquire(composer->mutex, osWaitForever) == osOK) {
        // Compose all levels for now
        for(size_t i = 0; i < sizeof(composer->layers) / sizeof(composer->layers[0]); i++) {
            // iterate over items
            list_composer_cb_it_t it;
            for(list_composer_cb_it(it, composer->layers[i]); !list_composer_cb_end_p(it);
                list_composer_cb_next(it)) {
                const ValueComposerHandle* h = list_composer_cb_cref(it);
                h->cb(h->ctx, state);
            }
        }

        osMutexRelease(composer->mutex);
    }
}

void COPY_COMPOSE(void* ctx, void* state) {
    read_mutex((ValueMutex*)ctx, state, 0, osWaitForever);
}

bool init_managed(ValueManager* managed, void* value, size_t size) {
    if(!init_pubsub(&managed->pubsub)) return false;
    if(!init_mutex(&managed->value, value, size)) {
        delete_pubsub(&managed->pubsub);
        return false;
    }
    return true;
}

bool delete_managed(ValueManager* managed) {
    bool result = true;
    result &= delete_mutex(&managed->value);
    result &= delete_pubsub(&managed->pubsub);
    return result;
}

bool write_managed(ValueManager* managed, void* data, size_t len, uint32_t timeout) {
    void* value = acquire_mutex(&managed->value, timeout);
    if(value == NULL) return false;

    memcpy(value, data, len);

    notify_pubsub(&managed->pubsub, value);

    if(!release_mutex(&managed->value, value)) return false;

    return true;
}

bool commit_managed(ValueManager* managed, void* value) {
    if(value != managed->value.value) return false;

    notify_pubsub(&managed->pubsub, value);

    if(!release_mutex(&managed->value, value)) return false;

    return true;
}
