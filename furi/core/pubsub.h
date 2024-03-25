/**
 * @file pubsub.h
 * FuriPubSub
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** FuriPubSub Callback type */
typedef void (*FuriPubSubCallback)(const void* message, void* context);

/** FuriPubSub type */
typedef struct FuriPubSub FuriPubSub;

/** FuriPubSubSubscription type */
typedef struct FuriPubSubSubscription FuriPubSubSubscription;

/** Allocate FuriPubSub
 *
 * Reentrable, Not threadsafe, one owner
 *
 * @return     pointer to FuriPubSub instance
 */
FuriPubSub* furi_pubsub_alloc(void);

/** Free FuriPubSub
 * 
 * @param      pubsub  FuriPubSub instance
 */
void furi_pubsub_free(FuriPubSub* pubsub);

/** Subscribe to FuriPubSub
 * 
 * Threadsafe, Reentrable
 * 
 * @param      pubsub            pointer to FuriPubSub instance
 * @param[in]  callback          The callback
 * @param      callback_context  The callback context
 *
 * @return     pointer to FuriPubSubSubscription instance
 */
FuriPubSubSubscription*
    furi_pubsub_subscribe(FuriPubSub* pubsub, FuriPubSubCallback callback, void* callback_context);

/** Unsubscribe from FuriPubSub
 * 
 * No use of `pubsub_subscription` allowed after call of this method
 * Threadsafe, Reentrable.
 *
 * @param      pubsub               pointer to FuriPubSub instance
 * @param      pubsub_subscription  pointer to FuriPubSubSubscription instance
 */
void furi_pubsub_unsubscribe(FuriPubSub* pubsub, FuriPubSubSubscription* pubsub_subscription);

/** Publish message to FuriPubSub
 *
 * Threadsafe, Reentrable.
 * 
 * @param      pubsub   pointer to FuriPubSub instance
 * @param      message  message pointer to publish
 */
void furi_pubsub_publish(FuriPubSub* pubsub, void* message);

#ifdef __cplusplus
}
#endif
