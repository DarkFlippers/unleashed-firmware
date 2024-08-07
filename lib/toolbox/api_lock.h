#pragma once
#include <furi/furi.h>

/*
Testing 10000 api calls

No lock
    Time diff: 445269.218750 us
    Time per call: 44.526920 us

furi_thread_flags
    Time diff: 430279.875000 us // lol wtf? smaller than no lock?
    Time per call: 43.027988 us // I tested it many times, it's always smaller

FuriEventFlag
    Time diff: 831523.625000 us
    Time per call: 83.152359 us

FuriSemaphore
    Time diff: 999807.125000 us
    Time per call: 99.980713 us

FuriMutex
    Time diff: 1071417.500000 us
    Time per call: 107.141747 us
*/

typedef FuriEventFlag* FuriApiLock;

#define API_LOCK_EVENT (1U << 0)

#define api_lock_alloc_locked() furi_event_flag_alloc()

#define api_lock_wait_unlock(_lock) \
    furi_event_flag_wait(_lock, API_LOCK_EVENT, FuriFlagWaitAny, FuriWaitForever)

#define api_lock_free(_lock) furi_event_flag_free(_lock)

#define api_lock_unlock(_lock) furi_event_flag_set(_lock, API_LOCK_EVENT)

#define api_lock_wait_unlock_and_free(_lock) \
    api_lock_wait_unlock(_lock);             \
    api_lock_free(_lock);

#define api_lock_is_locked(_lock) (!(furi_event_flag_get(_lock) & API_LOCK_EVENT))

#define api_lock_relock(_lock) furi_event_flag_clear(_lock, API_LOCK_EVENT)
