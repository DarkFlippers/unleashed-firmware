#include "pubsub.h"
#include "memmgr.h"
#include "check.h"
#include "mutex.h"

#include <m-list.h>

struct FuriPubSubSubscription {
    FuriPubSubCallback callback;
    void* callback_context;
};

LIST_DEF(FuriPubSubSubscriptionList, FuriPubSubSubscription, M_POD_OPLIST);

struct FuriPubSub {
    FuriPubSubSubscriptionList_t items;
    osMutexId_t mutex;
};

FuriPubSub* furi_pubsub_alloc() {
    FuriPubSub* pubsub = malloc(sizeof(FuriPubSub));

    pubsub->mutex = osMutexNew(NULL);
    furi_assert(pubsub->mutex);

    FuriPubSubSubscriptionList_init(pubsub->items);

    return pubsub;
}

void furi_pubsub_free(FuriPubSub* pubsub) {
    furi_assert(pubsub);

    furi_check(FuriPubSubSubscriptionList_size(pubsub->items) == 0);

    FuriPubSubSubscriptionList_clear(pubsub->items);

    furi_check(osMutexDelete(pubsub->mutex) == osOK);

    free(pubsub);
}

FuriPubSubSubscription*
    furi_pubsub_subscribe(FuriPubSub* pubsub, FuriPubSubCallback callback, void* callback_context) {
    furi_check(osMutexAcquire(pubsub->mutex, osWaitForever) == osOK);
    // put uninitialized item to the list
    FuriPubSubSubscription* item = FuriPubSubSubscriptionList_push_raw(pubsub->items);

    // initialize item
    item->callback = callback;
    item->callback_context = callback_context;

    furi_check(osMutexRelease(pubsub->mutex) == osOK);

    return item;
}

void furi_pubsub_unsubscribe(FuriPubSub* pubsub, FuriPubSubSubscription* pubsub_subscription) {
    furi_assert(pubsub);
    furi_assert(pubsub_subscription);

    furi_check(osMutexAcquire(pubsub->mutex, osWaitForever) == osOK);
    bool result = false;

    // iterate over items
    FuriPubSubSubscriptionList_it_t it;
    for(FuriPubSubSubscriptionList_it(it, pubsub->items); !FuriPubSubSubscriptionList_end_p(it);
        FuriPubSubSubscriptionList_next(it)) {
        const FuriPubSubSubscription* item = FuriPubSubSubscriptionList_cref(it);

        // if the iterator is equal to our element
        if(item == pubsub_subscription) {
            FuriPubSubSubscriptionList_remove(pubsub->items, it);
            result = true;
            break;
        }
    }

    furi_check(osMutexRelease(pubsub->mutex) == osOK);
    furi_check(result);
}

void furi_pubsub_publish(FuriPubSub* pubsub, void* message) {
    furi_check(osMutexAcquire(pubsub->mutex, osWaitForever) == osOK);

    // iterate over subscribers
    FuriPubSubSubscriptionList_it_t it;
    for(FuriPubSubSubscriptionList_it(it, pubsub->items); !FuriPubSubSubscriptionList_end_p(it);
        FuriPubSubSubscriptionList_next(it)) {
        const FuriPubSubSubscription* item = FuriPubSubSubscriptionList_cref(it);
        item->callback(message, item->callback_context);
    }

    furi_check(osMutexRelease(pubsub->mutex) == osOK);
}
