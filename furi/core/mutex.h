/**
 * @file mutex.h
 * FuriMutex
 */
#pragma once

#include "base.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriMutexTypeNormal,
    FuriMutexTypeRecursive,
} FuriMutexType;

typedef struct FuriMutex FuriMutex;

/** Allocate FuriMutex
 *
 * @param[in]  type  The mutex type
 *
 * @return     pointer to FuriMutex instance
 */
FuriMutex* furi_mutex_alloc(FuriMutexType type);

/** Free FuriMutex
 *
 * @param      instance  The pointer to FuriMutex instance
 */
void furi_mutex_free(FuriMutex* instance);

/** Acquire mutex
 *
 * @param      instance  The pointer to FuriMutex instance
 * @param[in]  timeout   The timeout
 *
 * @return     The furi status.
 */
FuriStatus furi_mutex_acquire(FuriMutex* instance, uint32_t timeout);

/** Release mutex
 *
 * @param      instance  The pointer to FuriMutex instance
 *
 * @return     The furi status.
 */
FuriStatus furi_mutex_release(FuriMutex* instance);

/** Get mutex owner thread id
 *
 * @param      instance  The pointer to FuriMutex instance
 *
 * @return     The furi thread identifier.
 */
FuriThreadId furi_mutex_get_owner(FuriMutex* instance);

#ifdef __cplusplus
}
#endif
