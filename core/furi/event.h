#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <cmsis_os2.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    osSemaphoreId_t semaphore_id;
} Event;

/*
Creates Event.
*/
bool init_event(Event* event);

/*
Free resources allocated by `init_event`.
This function doesn't free the memory occupied by `Event` itself.
*/
bool delete_event(Event* event);

/*
Signals the event.
If the event is already in "signalled" state, nothing happens.
*/
void signal_event(Event* event);

/*
Waits until the event is signalled.
*/
void wait_event(Event* event);

/*
Waits with a timeout until the event is signalled.
*/
bool wait_event_with_timeout(Event* event, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
