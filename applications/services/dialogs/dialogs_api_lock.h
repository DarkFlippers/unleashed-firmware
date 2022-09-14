#pragma once

typedef FuriEventFlag* FuriApiLock;

#define API_LOCK_EVENT (1U << 0)

#define API_LOCK_INIT_LOCKED() furi_event_flag_alloc();

#define API_LOCK_WAIT_UNTIL_UNLOCK(_lock) \
    furi_event_flag_wait(_lock, API_LOCK_EVENT, FuriFlagWaitAny, FuriWaitForever);

#define API_LOCK_FREE(_lock) furi_event_flag_free(_lock);

#define API_LOCK_UNLOCK(_lock) furi_event_flag_set(_lock, API_LOCK_EVENT);

#define API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(_lock) \
    API_LOCK_WAIT_UNTIL_UNLOCK(_lock);             \
    API_LOCK_FREE(_lock);
