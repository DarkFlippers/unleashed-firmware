#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IRDA_COMMON_CARRIER_FREQUENCY      38000
#define IRDA_COMMON_DUTY_CYCLE             0.33

typedef struct IrdaDecoderHandler IrdaDecoderHandler;
typedef struct IrdaEncoderHandler IrdaEncoderHandler;

// Do not change protocol order, as it can be saved into memory and fw update can be performed!
typedef enum {
    IrdaProtocolUnknown = -1,
    IrdaProtocolNEC = 0,
    IrdaProtocolNECext = 1,
    IrdaProtocolSamsung32 = 2,
    IrdaProtocolRC6 = 3,
} IrdaProtocol;

typedef struct {
    IrdaProtocol protocol;
    uint32_t address;
    uint32_t command;
    bool repeat;
} IrdaMessage;

typedef enum {
    IrdaStatusError,
    IrdaStatusOk,
    IrdaStatusDone,
    IrdaStatusReady,
} IrdaStatus;

/**
 * Initialize decoder.
 *
 * \return      returns pointer to IRDA decoder handler if success, otherwise - error.
 */
IrdaDecoderHandler* irda_alloc_decoder(void);

/**
 * Provide to decoder next timing.
 *
 * \param[in]   handler     - handler to IRDA decoders. Should be aquired with \c irda_alloc_decoder().
 * \param[in]   level       - high(true) or low(false) level of input signal to analyze.
 *                          it should alternate every call, otherwise it is an error case,
 *                          and decoder resets its state and start decoding from the start.
 * \param[in]   duration    - duration of steady high/low input signal.
 * \return      if message is ready, returns pointer to decoded message, returns NULL.
 */
const IrdaMessage* irda_decode(IrdaDecoderHandler* handler, bool level, uint32_t duration);

/**
 * Deinitialize decoder and free allocated memory.
 *
 * \param[in]   handler     - handler to IRDA decoders. Should be aquired with \c irda_alloc_decoder().
 */
void irda_free_decoder(IrdaDecoderHandler* handler);

/**
 * Reset IRDA decoder.
 *
 * \param[in]   handler     - handler to IRDA decoders. Should be aquired with \c irda_alloc_decoder().
 */
void irda_reset_decoder(IrdaDecoderHandler* handler);

/**
 * Get protocol name by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      string to protocol name.
 */
const char* irda_get_protocol_name(IrdaProtocol protocol);

/**
 * Get protocol enum by protocol name.
 *
 * \param[in]   protocol_name   - string to protocol name.
 * \return      protocol identifier.
 */
IrdaProtocol irda_get_protocol_by_name(const char* protocol_name);

/**
 * Get address length by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      length of address in nibbles.
 */
uint8_t irda_get_protocol_address_length(IrdaProtocol protocol);

/**
 * Get command length by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      length of command in nibbles.
 */
uint8_t irda_get_protocol_command_length(IrdaProtocol protocol);

/**
 * Checks whether protocol valid.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      true if protocol is valid, false otherwise.
 */
bool irda_is_protocol_valid(IrdaProtocol protocol);

/**
 * Allocate IRDA encoder.
 *
 * \return      encoder handler.
 */
IrdaEncoderHandler* irda_alloc_encoder(void);

/**
 * Free encoder handler previously allocated with \c irda_alloc_encoder().
 *
 * \param[in]   handler     - handler to IRDA encoder. Should be aquired with \c irda_alloc_encoder().
 */
void irda_free_encoder(IrdaEncoderHandler* handler);

/**
 * Encode previously set IRDA message.
 * Usage:
 *  1) alloc with \c irda_alloc_encoder()
 *  2) set message to encode with \c irda_reset_encoder()
 *  3) call for \c irda_encode() to continuously get one at a time timings.
 *  4) when \c irda_encode() returns IrdaStatusDone, it means new message is fully encoded.
 *  5) to encode additional timings, just continue calling \c irda_encode().
 *
 * \param[in]   handler     - handler to IRDA encoder. Should be aquired with \c irda_alloc_encoder().
 * \param[out]  duration    - encoded timing.
 * \param[out]  level       - encoded level.
 *
 * \return      status of encode operation.
 */
IrdaStatus irda_encode(IrdaEncoderHandler* handler, uint32_t* duration, bool* level);

/**
 * Reset IRDA encoder and set new message to encode. If it's not called after receiveing
 * IrdaStatusDone in \c irda_encode(), encoder will encode repeat messages
 * till the end of time.
 *
 * \param[in]   handler     - handler to IRDA encoder. Should be aquired with \c irda_alloc_encoder().
 * \param[in]   message     - message to encode.
 */
void irda_reset_encoder(IrdaEncoderHandler* handler, const IrdaMessage* message);

#ifdef __cplusplus
}
#endif

