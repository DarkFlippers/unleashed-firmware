#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INFRARED_COMMON_CARRIER_FREQUENCY ((uint32_t)38000)
#define INFRARED_COMMON_DUTY_CYCLE        ((float)0.33)

/* if we want to see split raw signals during bruteforce,
 * we have to have RX raw timing delay less than TX */
#define INFRARED_RAW_RX_TIMING_DELAY_US 150000
#define INFRARED_RAW_TX_TIMING_DELAY_US 180000

typedef struct InfraredDecoderHandler InfraredDecoderHandler;
typedef struct InfraredEncoderHandler InfraredEncoderHandler;

typedef enum {
    InfraredProtocolUnknown = -1,
    InfraredProtocolNEC = 0,
    InfraredProtocolNECext,
    InfraredProtocolNEC42,
    InfraredProtocolNEC42ext,
    InfraredProtocolSamsung32,
    InfraredProtocolRC6,
    InfraredProtocolRC5,
    InfraredProtocolRC5X,
    InfraredProtocolSIRC,
    InfraredProtocolSIRC15,
    InfraredProtocolSIRC20,
    InfraredProtocolKaseikyo,
    InfraredProtocolRCA,
    InfraredProtocolPioneer,
    /* Add new protocols here */
    InfraredProtocolMAX,
} InfraredProtocol;

typedef struct {
    InfraredProtocol protocol;
    uint32_t address;
    uint32_t command;
    bool repeat;
} InfraredMessage;

typedef enum {
    InfraredStatusError,
    InfraredStatusOk,
    InfraredStatusDone,
    InfraredStatusReady,
} InfraredStatus;

/**
 * Initialize decoder.
 *
 * \return      returns pointer to INFRARED decoder handler if success, otherwise - error.
 */
InfraredDecoderHandler* infrared_alloc_decoder(void);

/**
 * Provide to decoder next timing.
 *
 * \param[in]   handler     - handler to INFRARED decoders. Should be acquired with \c infrared_alloc_decoder().
 * \param[in]   level       - high(true) or low(false) level of input signal to analyze.
 *                          it should alternate every call, otherwise it is an error case,
 *                          and decoder resets its state and start decoding from the start.
 * \param[in]   duration    - duration of steady high/low input signal.
 * \return      if message is ready, returns pointer to decoded message, returns NULL.
 *              Note: ownership of returned ptr belongs to handler. So pointer is valid
 *              up to next infrared_free_decoder(), infrared_reset_decoder(),
 *              infrared_decode(), infrared_check_decoder_ready() calls.
 */
const InfraredMessage*
    infrared_decode(InfraredDecoderHandler* handler, bool level, uint32_t duration);

/**
 * Check whether decoder is ready.
 * Functionality is quite similar to infrared_decode(), but with no timing providing.
 * Some protocols (e.g. Sony SIRC) has variable payload length, which means we
 * can't recognize end of message right after receiving last bit. That's why
 * application should call to infrared_check_decoder_ready() after some timeout to
 * retrieve decoded message, if so.
 *
 * \param[in]   handler     - handler to INFRARED decoders. Should be acquired with \c infrared_alloc_decoder().
 * \return      if message is ready, returns pointer to decoded message, returns NULL.
 *              Note: ownership of returned ptr belongs to handler. So pointer is valid
 *              up to next infrared_free_decoder(), infrared_reset_decoder(),
 *              infrared_decode(), infrared_check_decoder_ready() calls.
 */
const InfraredMessage* infrared_check_decoder_ready(InfraredDecoderHandler* handler);

/**
 * Deinitialize decoder and free allocated memory.
 *
 * \param[in]   handler     - handler to INFRARED decoders. Should be acquired with \c infrared_alloc_decoder().
 */
void infrared_free_decoder(InfraredDecoderHandler* handler);

/**
 * Reset INFRARED decoder.
 *
 * \param[in]   handler     - handler to INFRARED decoders. Should be acquired with \c infrared_alloc_decoder().
 */
void infrared_reset_decoder(InfraredDecoderHandler* handler);

/**
 * Get protocol name by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      string to protocol name.
 */
const char* infrared_get_protocol_name(InfraredProtocol protocol);

/**
 * Get protocol enum by protocol name.
 *
 * \param[in]   protocol_name   - string to protocol name.
 * \return      protocol identifier.
 */
InfraredProtocol infrared_get_protocol_by_name(const char* protocol_name);

/**
 * Get address length by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      length of address in bits.
 */
uint8_t infrared_get_protocol_address_length(InfraredProtocol protocol);

/**
 * Get command length by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      length of command in bits.
 */
uint8_t infrared_get_protocol_command_length(InfraredProtocol protocol);

/**
 * Checks whether protocol valid.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      true if protocol is valid, false otherwise.
 */
bool infrared_is_protocol_valid(InfraredProtocol protocol);

/**
 * Allocate INFRARED encoder.
 *
 * \return      encoder handler.
 */
InfraredEncoderHandler* infrared_alloc_encoder(void);

/**
 * Free encoder handler previously allocated with \c infrared_alloc_encoder().
 *
 * \param[in]   handler     - handler to INFRARED encoder. Should be acquired with \c infrared_alloc_encoder().
 */
void infrared_free_encoder(InfraredEncoderHandler* handler);

/**
 * Encode previously set INFRARED message.
 * Usage:
 *  1) alloc with \c infrared_alloc_encoder()
 *  2) set message to encode with \c infrared_reset_encoder()
 *  3) call for \c infrared_encode() to continuously get one at a time timings.
 *  4) when \c infrared_encode() returns InfraredStatusDone, it means new message is fully encoded.
 *  5) to encode additional timings, just continue calling \c infrared_encode().
 *
 * \param[in]   handler     - handler to INFRARED encoder. Should be acquired with \c infrared_alloc_encoder().
 * \param[out]  duration    - encoded timing.
 * \param[out]  level       - encoded level.
 *
 * \return      status of encode operation.
 */
InfraredStatus infrared_encode(InfraredEncoderHandler* handler, uint32_t* duration, bool* level);

/**
 * Reset INFRARED encoder and set new message to encode. If it's not called after receiveing
 * InfraredStatusDone in \c infrared_encode(), encoder will encode repeat messages
 * till the end of time.
 *
 * \param[in]   handler     - handler to INFRARED encoder. Should be acquired with \c infrared_alloc_encoder().
 * \param[in]   message     - message to encode.
 */
void infrared_reset_encoder(InfraredEncoderHandler* handler, const InfraredMessage* message);

/**
 * Get PWM frequency value for selected protocol
 *
 * \param[in]   protocol    - protocol to get from PWM frequency
 *
 * \return      frequency
 */
uint32_t infrared_get_protocol_frequency(InfraredProtocol protocol);

/**
 * Get PWM duty cycle value for selected protocol
 *
 * \param[in]   protocol    - protocol to get from PWM duty cycle
 *
 * \return      duty cycle
 */
float infrared_get_protocol_duty_cycle(InfraredProtocol protocol);

/**
 * Get the minimum count of signal repeats for the selected protocol
 *
 * \param[in]   protocol    - protocol to get the repeat count from
 *
 * \return      repeat count
 */
size_t infrared_get_protocol_min_repeat_count(InfraredProtocol protocol);

#ifdef __cplusplus
}
#endif
