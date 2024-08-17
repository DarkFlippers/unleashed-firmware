/**
 * @file semaphore.h
 * FuriSemaphore
 */
#pragma once

#include "base.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriSemaphore FuriSemaphore;

/** Allocate semaphore
 *
 * @param[in]  max_count      The maximum count
 * @param[in]  initial_count  The initial count
 *
 * @return     pointer to FuriSemaphore instance
 */
FuriSemaphore* furi_semaphore_alloc(uint32_t max_count, uint32_t initial_count);

/** Free semaphore
 *
 * @param      instance  The pointer to FuriSemaphore instance
 */
void furi_semaphore_free(FuriSemaphore* instance);

/** Acquire semaphore
 *
 * @param      instance  The pointer to FuriSemaphore instance
 * @param[in]  timeout   The timeout
 *
 * @return     The furi status.
 */
FuriStatus furi_semaphore_acquire(FuriSemaphore* instance, uint32_t timeout);

/** Release semaphore
 *
 * @param      instance  The pointer to FuriSemaphore instance
 *
 * @return     The furi status.
 */
FuriStatus furi_semaphore_release(FuriSemaphore* instance);

/** Get semaphore count
 *
 * @param      instance  The pointer to FuriSemaphore instance
 *
 * @return     Semaphore count
 */
uint32_t furi_semaphore_get_count(FuriSemaphore* instance);

/** Get available space
 *
 * @param      instance  The pointer to FuriSemaphore instance
 *
 * @return     Semaphore available space
 */
uint32_t furi_semaphore_get_space(FuriSemaphore* instance);

#ifdef __cplusplus
}
#endif
