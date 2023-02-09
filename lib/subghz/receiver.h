#pragma once

#include "types.h"
#include "protocols/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzReceiver SubGhzReceiver;

typedef void (*SubGhzReceiverCallback)(
    SubGhzReceiver* decoder,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context);

/**
 * Allocate and init SubGhzReceiver.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzReceiver* pointer to a SubGhzReceiver instance
 */
SubGhzReceiver* subghz_receiver_alloc_init(SubGhzEnvironment* environment);

/**
 * Free SubGhzReceiver.
 * @param instance Pointer to a SubGhzReceiver instance
 */
void subghz_receiver_free(SubGhzReceiver* instance);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param instance Pointer to a SubGhzReceiver instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_receiver_decode(SubGhzReceiver* instance, bool level, uint32_t duration);

/**
 * Reset decoder SubGhzReceiver.
 * @param instance Pointer to a SubGhzReceiver instance
 */
void subghz_receiver_reset(SubGhzReceiver* instance);

/**
 * Set a callback upon completion of successful decoding of one of the protocols.
 * @param instance Pointer to a SubGhzReceiver instance
 * @param callback Callback, SubGhzReceiverCallback
 * @param context Context
 */
void subghz_receiver_set_rx_callback(
    SubGhzReceiver* instance,
    SubGhzReceiverCallback callback,
    void* context);

/**
 * Set the filter of receivers that will work at the moment.
 * @param instance Pointer to a SubGhzReceiver instance
 * @param filter Filter, SubGhzProtocolFlag
 */
void subghz_receiver_set_filter(SubGhzReceiver* instance, SubGhzProtocolFlag filter);

/**
 * Search for a cattery by his name.
 * @param instance Pointer to a SubGhzReceiver instance
 * @param decoder_name Receiver name
 * @return SubGhzProtocolDecoderBase* pointer to a SubGhzProtocolDecoderBase instance
 */
SubGhzProtocolDecoderBase*
    subghz_receiver_search_decoder_base_by_name(SubGhzReceiver* instance, const char* decoder_name);

#ifdef __cplusplus
}
#endif
