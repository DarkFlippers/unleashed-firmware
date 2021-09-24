#pragma once

typedef osEventFlagsId_t FuriApiLock;

#define API_LOCK_EVENT (1U << 0)

#define API_LOCK_INIT_LOCKED() osEventFlagsNew(NULL);

#define API_LOCK_WAIT_UNTIL_UNLOCK(_lock) \
    osEventFlagsWait(_lock, API_LOCK_EVENT, osFlagsWaitAny, osWaitForever);

#define API_LOCK_FREE(_lock) osEventFlagsDelete(_lock);

#define API_LOCK_UNLOCK(_lock) osEventFlagsSet(_lock, API_LOCK_EVENT);

#define API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(_lock) \
    API_LOCK_WAIT_UNTIL_UNLOCK(_lock);             \
    API_LOCK_FREE(_lock);
