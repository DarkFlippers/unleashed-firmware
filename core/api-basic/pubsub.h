#pragma once

#include "flipper.h"

/*
== PubSub ==

PubSub allows users to subscribe on notifies and notify subscribers.
Notifier side can pass `void*` arg to subscriber callback,
and also subscriber can set `void*` context pointer that pass into
callback (you can see callback signature below).
*/

typedef void(PubSubCallback*)(void*, void*);

typedef struct {
    PubSubCallback cb;
    void* ctx;
} PubSubItem;

typedef struct {
    PubSub* self;
    PubSubItem* item;
} PubSubId;

typedef struct {
    PubSubItem items[NUM_OF_CALLBACKS];
    PubSubId ids[NUM_OF_CALLBACKS]; ///< permanent links to item
    size_t count; ///< count of callbacks
} PubSub;

/*
To create PubSub you should create PubSub instance and call `init_pubsub`.
*/
void init_pubsub(PubSub* pubsub);

/*
Use `subscribe_pubsub` to register your callback.
*/
PubSubId* subscribe_pubsub(PubSub* pubsub, PubSubCallback cb, void* ctx);

/*
Use `unsubscribe_pubsub` to unregister callback.
*/
void unsubscribe_pubsub(PubSubId* pubsub_id);

/*
Use `notify_pubsub` to notify subscribers.
*/
void notify_pubsub(PubSub* pubsub, void* arg);

/*

```C
// MANIFEST
// name="test"
// stack=128

void example_pubsub_handler(void* arg, void* ctx) {
    printf("get %d from %s\n", *(uint32_t*)arg, (const char*)ctx);
}

void pubsub_test() {
    const char* app_name = "test app";

    PubSub example_pubsub;
    init_pubsub(&example_pubsub);

    if(!subscribe_pubsub(&example_pubsub, example_pubsub_handler, (void*)app_name)) {
        printf("critical error\n");
        flapp_exit(NULL);
    }

    uint32_t counter = 0;
    while(1) {
        notify_pubsub(&example_pubsub, (void*)&counter);
        counter++;

        osDelay(100);
    }
}
```
*/