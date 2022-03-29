/**
 * @file pulse_protocol.h
 * 
 * Generic pulse protocol decoder library, protocol interface
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Anonymous PulseProtocol struct
 */
typedef struct PulseProtocol PulseProtocol;

/**
 * Process pulse callback
 */
typedef void (*PulseProtocolPulseCallback)(void* context, bool polarity, uint32_t length);

/**
 * Reset protocol callback
 */
typedef void (*PulseProtocolResetCallback)(void* context);

/**
 * Get decoded data callback
 */
typedef void (*PulseProtocolGetDataCallback)(void* context, uint8_t* data, size_t length);

/**
 * Is protocol decoded callback
 */
typedef bool (*PulseProtocolDecodedCallback)(void* context);

/**
 * Allocate protocol
 * @return PulseProtocol* 
 */
PulseProtocol* pulse_protocol_alloc();

/**
 * Deallocate protocol
 * @param protocol 
 */
void pulse_protocol_free(PulseProtocol* protocol);

/**
 * Set context for callbacks
 * @param protocol 
 * @param context 
 */
void pulse_protocol_set_context(PulseProtocol* protocol, void* context);

/**
 * Set "Process pulse" callback. Called from the decoder when a new pulse is received.
 * @param protocol 
 * @param callback 
 */
void pulse_protocol_set_pulse_cb(PulseProtocol* protocol, PulseProtocolPulseCallback callback);

/**
 * Set "Reset protocol" callback. Called from the decoder when the decoder is reset.
 * @param protocol 
 * @param callback 
 */
void pulse_protocol_set_reset_cb(PulseProtocol* protocol, PulseProtocolResetCallback callback);

/**
 * Set "Get decoded data" callback. Called from the decoder when the decoder wants to get decoded data.
 * @param protocol 
 * @param callback 
 */
void pulse_protocol_set_get_data_cb(PulseProtocol* protocol, PulseProtocolGetDataCallback callback);

/**
 * Set "Is protocol decoded" callback. Called from the decoder when the decoder wants to know if a protocol has been decoded.
 * @param protocol 
 * @param callback 
 */
void pulse_protocol_set_decoded_cb(PulseProtocol* protocol, PulseProtocolDecodedCallback callback);

/**
 * Part of decoder interface.
 * @param protocol 
 * @param polarity 
 * @param length 
 */
void pulse_protocol_process_pulse(PulseProtocol* protocol, bool polarity, uint32_t length);

/**
 * Part of decoder interface.
 * @param protocol 
 * @return true 
 * @return false 
 */
bool pulse_protocol_decoded(PulseProtocol* protocol);

/**
 * Part of decoder interface.
 * @param protocol 
 * @return true 
 * @return false 
 */
void pulse_protocol_get_data(PulseProtocol* protocol, uint8_t* data, size_t length);

/**
 * Part of decoder interface.
 * @param protocol 
 * @return true 
 * @return false 
 */
void pulse_protocol_reset(PulseProtocol* protocol);

#ifdef __cplusplus
}
#endif