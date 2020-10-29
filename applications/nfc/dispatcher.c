#include "dispatcher.h"

#include <flipper.h>
#include <flipper_v2.h>

struct Dispatcher {
    void* message;
    size_t message_size;
    osMessageQueueId_t mqueue;
    osMutexId_t lock_mutex;
};

Dispatcher* dispatcher_alloc(size_t queue_size, size_t message_size) {
    Dispatcher* dispatcher = furi_alloc(sizeof(Dispatcher));

    dispatcher->message = furi_alloc(message_size);
    dispatcher->message_size = message_size;

    dispatcher->mqueue = osMessageQueueNew(queue_size, message_size, NULL);
    furi_check(dispatcher->mqueue);

    dispatcher->lock_mutex = osMutexNew(NULL);
    furi_check(dispatcher->lock_mutex);
    dispatcher_lock(dispatcher);

    return dispatcher;
}

void dispatcher_free(Dispatcher* dispatcher) {
    furi_assert(dispatcher);
    free(dispatcher);
}

void dispatcher_send(Dispatcher* dispatcher, Message* message) {
    furi_assert(dispatcher);
    furi_assert(message);
    furi_check(osMessageQueuePut(dispatcher->mqueue, message, 0, osWaitForever) == osOK);
}

// TODO: bad side-effect
void dispatcher_recieve(Dispatcher* dispatcher, Message* message) {
    furi_assert(dispatcher);
    furi_assert(message);
    dispatcher_unlock(dispatcher);
    furi_check(osMessageQueueGet(dispatcher->mqueue, message, NULL, osWaitForever) == osOK);
    dispatcher_lock(dispatcher);
}

void dispatcher_lock(Dispatcher* dispatcher) {
    furi_assert(dispatcher);
    furi_check(osMutexAcquire(dispatcher->lock_mutex, osWaitForever) == osOK);
}

void dispatcher_unlock(Dispatcher* dispatcher) {
    furi_assert(dispatcher);
    furi_check(osMutexRelease(dispatcher->lock_mutex) == osOK);
}
