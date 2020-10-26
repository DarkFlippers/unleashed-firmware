#include "event.h"
#include <string.h>

bool init_event(Event* event) {
    event->semaphore_id = osSemaphoreNew(1, 0, NULL);
    return event->semaphore_id != NULL;
}

bool delete_event(Event* event) {
    return osSemaphoreDelete(event->semaphore_id) == osOK;
}

void signal_event(Event* event) {
    // Ignore the result, as we do not care about repeated event signalling.
    osSemaphoreRelease(event->semaphore_id);
}

void wait_event(Event* event) {
    wait_event_with_timeout(event, osWaitForever);
}

bool wait_event_with_timeout(Event* event, uint32_t timeout_ms) {
    return osSemaphoreAcquire(event->semaphore_id, timeout_ms) == osOK;
}
