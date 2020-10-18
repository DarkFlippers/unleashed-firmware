#pragma once

#include <stddef.h>
#include <stdint.h>

enum MessageTypeBase {
    MessageTypeExit = 0x00,
    MessageTypeMemoryLow = 0x01,
    MessageTypeBatteryLow = 0x02,
};

typedef struct {
    enum MessageTypeBase type;
} Message;

typedef struct Dispatcher Dispatcher;

Dispatcher* dispatcher_alloc(size_t queue_size, size_t message_size);

void dispatcher_free(Dispatcher* dispatcher);

void dispatcher_send(Dispatcher* dispatcher, Message* message);

void dispatcher_recieve(Dispatcher* dispatcher, Message* message);

void dispatcher_lock(Dispatcher* dispatcher);

void dispatcher_unlock(Dispatcher* dispatcher);
