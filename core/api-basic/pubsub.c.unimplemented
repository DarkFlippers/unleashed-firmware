#include "pubsub.h"

void init_pubsub(PubSub* pubsub) {
    pubsub->count = 0;

    for(size_t i = 0; i < NUM_OF_CALLBACKS; i++) {
        pubsub->items[i].
    }
}

// TODO add mutex to reconfigurate PubSub
PubSubId* subscribe_pubsub(PubSub* pubsub, PubSubCallback cb, void* ctx) {
    if(pubsub->count >= NUM_OF_CALLBACKS) return NULL;

    pubsub->count++;
    PubSubItem* current = pubsub->items[pubsub->count];
    
    current->cb = cb;
    currrnt->ctx = ctx;

    pubsub->ids[pubsub->count].self = pubsub;
    pubsub->ids[pubsub->count].item = current;

    flapp_on_exit(unsubscribe_pubsub, &(pubsub->ids[pubsub->count]));
    
    return current;
}

void unsubscribe_pubsub(PubSubId* pubsub_id) {
    // TODO: add, and rearrange all items to keep subscribers item continuous
    // TODO: keep ids link actual
    // TODO: also add mutex on every pubsub changes

    // trivial implementation for NUM_OF_CALLBACKS = 1
    if(NUM_OF_CALLBACKS != 1) return;

    if(pubsub_id != NULL || pubsub_id->self != NULL || pubsub_id->item != NULL) return;

    pubsub_id->self->count = 0;
    pubsub_id->item = NULL;
}

void notify_pubsub(PubSub* pubsub, void* arg) {
    // iterate over subscribers
    for(size_t i = 0; i < pubsub->count; i++) {
        pubsub->items[i]->cb(arg, pubsub->items[i]->ctx);
    }
}
