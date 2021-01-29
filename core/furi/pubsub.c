#include "pubsub.h"
#include <furi.h>

bool init_pubsub(PubSub* pubsub) {
    // mutex without name,
    // no attributes (unfortunatly robust mutex is not supported by FreeRTOS),
    // with dynamic memory allocation
    pubsub->mutex = osMutexNew(NULL);
    if(pubsub->mutex == NULL) return false;

    // construct list
    list_pubsub_cb_init(pubsub->items);

    return true;
}

bool delete_pubsub(PubSub* pubsub) {
    if(osMutexAcquire(pubsub->mutex, osWaitForever) == osOK) {
        bool result = osMutexDelete(pubsub->mutex) == osOK;
        list_pubsub_cb_clear(pubsub->items);
        return result;
    } else {
        return false;
    }
}

PubSubItem* subscribe_pubsub(PubSub* pubsub, PubSubCallback cb, void* ctx) {
    if(osMutexAcquire(pubsub->mutex, osWaitForever) == osOK) {
        // put uninitialized item to the list
        PubSubItem* item = list_pubsub_cb_push_raw(pubsub->items);

        // initialize item
        item->cb = cb;
        item->ctx = ctx;
        item->self = pubsub;

        // TODO unsubscribe pubsub on app exit
        //flapp_on_exit(unsubscribe_pubsub, item);

        osMutexRelease(pubsub->mutex);

        return item;
    } else {
        return NULL;
    }
}

bool unsubscribe_pubsub(PubSubItem* pubsub_id) {
    if(osMutexAcquire(pubsub_id->self->mutex, osWaitForever) == osOK) {
        bool result = false;

        // iterate over items
        list_pubsub_cb_it_t it;
        for(list_pubsub_cb_it(it, pubsub_id->self->items); !list_pubsub_cb_end_p(it);
            list_pubsub_cb_next(it)) {
            const PubSubItem* item = list_pubsub_cb_cref(it);

            // if the iterator is equal to our element
            if(item == pubsub_id) {
                list_pubsub_cb_remove(pubsub_id->self->items, it);
                result = true;
                break;
            }
        }

        osMutexRelease(pubsub_id->self->mutex);
        return result;
    } else {
        return false;
    }
}

bool notify_pubsub(PubSub* pubsub, void* arg) {
    if(osMutexAcquire(pubsub->mutex, osWaitForever) == osOK) {
        // iterate over subscribers
        list_pubsub_cb_it_t it;
        for(list_pubsub_cb_it(it, pubsub->items); !list_pubsub_cb_end_p(it);
            list_pubsub_cb_next(it)) {
            const PubSubItem* item = list_pubsub_cb_cref(it);
            item->cb(arg, item->ctx);
        }

        osMutexRelease(pubsub->mutex);
        return true;
    } else {
        return false;
    }
}
