#pragma once

#include "flipper.h"
#include "valuemutex.h"
#include "pubsub.h"
#include "event.h"
#include "m-list.h"

/*
== Value composer ==
*/

typedef struct ValueComposer ValueComposer;

typedef void (*ValueComposerCallback)(void* ctx, void* state);

typedef enum { UiLayerBelowNotify, UiLayerNotify, UiLayerAboveNotify } UiLayer;

typedef struct {
    ValueComposerCallback cb;
    void* ctx;
    UiLayer layer;
    ValueComposer* composer;
} ValueComposerHandle;

LIST_DEF(list_composer_cb, ValueComposerHandle, M_POD_OPLIST);

struct ValueComposer {
    ValueMutex value;
    list_composer_cb_t layers[3];
    osMutexId_t mutex;
    Event request;
};

void COPY_COMPOSE(void* ctx, void* state);

bool init_composer(ValueComposer* composer, void* value);

/*
Free resources allocated by `init_composer`.
This function doesn't free the memory occupied by `ValueComposer` itself.
*/
bool delete_composer(ValueComposer* composer);

ValueComposerHandle*
add_compose_layer(ValueComposer* composer, ValueComposerCallback cb, void* ctx, UiLayer layer);

bool remove_compose_layer(ValueComposerHandle* handle);

void request_compose(ValueComposerHandle* handle);

/*
Perform composition if requested.

`start_cb` and `end_cb` will be called before and after all layer callbacks, respectively.
Both `start_cb` and `end_cb` can be NULL. They can be used to set initial state (e.g. clear screen)
and commit the final state.
*/
void perform_compose(
    ValueComposer* composer,
    ValueComposerCallback start_cb,
    ValueComposerCallback end_cb,
    void* ctx);

/*
Perform composition.

This function should be called with value mutex acquired.
This function is here for convenience, so that developers can write their own compose loops.
See `perform_compose` function body for an example.
*/
void perform_compose_internal(ValueComposer* composer, void* state);

// See [LED](LED-API) or [Display](Display-API) API for examples.

/*
== ValueManager ==

More complicated concept is ValueManager.
It is like ValueMutex, but user can subscribe to value updates.

First of all you can use value and pubsub part as showing above:
aquire/release mutex, read value, subscribe/unsubscribe pubsub.
There are two specific methods for ValueManager: write_managed, commit_managed
*/

typedef struct {
    ValueMutex value;
    PubSub pubsub;
} ValueManager;

bool init_managed(ValueManager* managed, void* value, size_t size);

/*
Free resources allocated by `init_managed`.
This function doesn't free the memory occupied by `ValueManager` itself.
*/
bool delete_managed(ValueManager* managed);

/*
acquire value, changes it and send notify with current value.
*/
bool write_managed(ValueManager* managed, void* data, size_t len, uint32_t timeout);

/*
commit_managed works as `release_mutex` but send notify with current value.
*/
bool commit_managed(ValueManager* managed, void* value);
