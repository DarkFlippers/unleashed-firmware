/**
 * @file message_queue.h
 * FuriMessageQueue
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriMessageQueue FuriMessageQueue;

/** Allocate furi message queue
 *
 * @param[in]  msg_count  The message count
 * @param[in]  msg_size   The message size
 *
 * @return     pointer to FuriMessageQueue instance
 */
FuriMessageQueue* furi_message_queue_alloc(uint32_t msg_count, uint32_t msg_size);

/** Free queue
 *
 * @param      instance  pointer to FuriMessageQueue instance
 */
void furi_message_queue_free(FuriMessageQueue* instance);

/** Put message into queue
 *
 * @param      instance  pointer to FuriMessageQueue instance
 * @param[in]  msg_ptr   The message pointer
 * @param[in]  timeout   The timeout
 *
 * @return     The furi status.
 */
FuriStatus
    furi_message_queue_put(FuriMessageQueue* instance, const void* msg_ptr, uint32_t timeout);

/** Get message from queue
 *
 * @param      instance  pointer to FuriMessageQueue instance
 * @param      msg_ptr   The message pointer
 * @param[in]  timeout   The timeout
 *
 * @return     The furi status.
 */
FuriStatus furi_message_queue_get(FuriMessageQueue* instance, void* msg_ptr, uint32_t timeout);

/** Get queue capacity
 *
 * @param      instance  pointer to FuriMessageQueue instance
 *
 * @return     capacity in object count
 */
uint32_t furi_message_queue_get_capacity(FuriMessageQueue* instance);

/** Get message size
 *
 * @param      instance  pointer to FuriMessageQueue instance
 *
 * @return     Message size in bytes
 */
uint32_t furi_message_queue_get_message_size(FuriMessageQueue* instance);

/** Get message count in queue
 *
 * @param      instance  pointer to FuriMessageQueue instance
 *
 * @return     Message count
 */
uint32_t furi_message_queue_get_count(FuriMessageQueue* instance);

/** Get queue available space
 *
 * @param      instance  pointer to FuriMessageQueue instance
 *
 * @return     Message count
 */
uint32_t furi_message_queue_get_space(FuriMessageQueue* instance);

/** Reset queue
 *
 * @param      instance  pointer to FuriMessageQueue instance
 *
 * @return     The furi status.
 */
FuriStatus furi_message_queue_reset(FuriMessageQueue* instance);

#ifdef __cplusplus
}
#endif
