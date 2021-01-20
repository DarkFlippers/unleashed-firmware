#pragma once

#include "cmsis_os.h"
#include "m-list.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
== PubSub ==

PubSub allows users to subscribe on notifies and notify subscribers.
Notifier side can pass `void*` arg to subscriber callback,
and also subscriber can set `void*` context pointer that pass into
callback (you can see callback signature below).
*/

typedef void (*PubSubCallback)(const void*, void*);
typedef struct PubSubType PubSub;

typedef struct {
    PubSubCallback cb;
    void* ctx;
    PubSub* self;
} PubSubItem;

LIST_DEF(list_pubsub_cb, PubSubItem, M_POD_OPLIST);

struct PubSubType {
    list_pubsub_cb_t items;
    osMutexId_t mutex;
};

/*
To create PubSub you should create PubSub instance and call `init_pubsub`.
*/
bool init_pubsub(PubSub* pubsub);

/*
Since we use dynamic memory - we must explicity delete pubsub
*/
bool delete_pubsub(PubSub* pubsub);

/*
Use `subscribe_pubsub` to register your callback.
*/
PubSubItem* subscribe_pubsub(PubSub* pubsub, PubSubCallback cb, void* ctx);

/*
Use `unsubscribe_pubsub` to unregister callback.
*/
bool unsubscribe_pubsub(PubSubItem* pubsub_id);

/*
Use `notify_pubsub` to notify subscribers.
*/
bool notify_pubsub(PubSub* pubsub, void* arg);

#ifdef __cplusplus
}
#endif

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
