#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IrdaHandler IrdaHandler;

// Do not change protocol order, as it can be saved into memory and fw update can be performed!
typedef enum {
    IrdaProtocolSamsung32 = 0,
    IrdaProtocolNEC = 1,
    IrdaProtocolNECext = 2,
} IrdaProtocol;

typedef struct {
    IrdaProtocol protocol;
    uint32_t address;
    uint32_t command;
    bool repeat;
} IrdaMessage;


/**
 * Initialize decoder.
 *
 * \return      returns pointer to IRDA decoder handler if success, otherwise - error.
 */
IrdaHandler* irda_alloc_decoder(void);

/**
 * Provide to decoder next timing. If message is ready, it returns decoded message,
 * otherwise NULL.
 *
 * \param[in]   handler     - handler to irda decoders. Should be aquired with \c irda_alloc_decoder().
 * \param[in]   level       - high(true) or low(false) level of input signal to analyze.
 *                          it should alternate every call, otherwise it is an error case,
 *                          and decoder resets its state and start decoding from the start.
 * \param[in]   duration    - duration of steady high/low input signal.
 * \return      if message is ready, returns pointer to decoded message, returns NULL.
 */
const IrdaMessage* irda_decode(IrdaHandler* handler, bool level, uint32_t duration);

/**
 * Deinitialize decoder and free allocated memory.
 *
 * \param[in]   handler     - handler to irda decoders. Should be aquired with \c irda_alloc_decoder().
 */
void irda_free_decoder(IrdaHandler* handler);

/**
 * Reset IRDA decoder.
 *
 * \param[in]   handler     - handler to irda decoders. Should be aquired with \c irda_alloc_decoder().
 */
void irda_reset_decoder(IrdaHandler* handler);

/**
 * Send message over IRDA.
 *
 * \param[in]   message     - message to send.
 * \param[in]   times       - number of times message should be sent.
 */
void irda_send(const IrdaMessage* message, int times);

/**
 * Get protocol name by protocol enum.
 *
 * \param[in]   protocol    - protocol identifier.
 * \return      string to protocol name.
 */
const char* irda_get_protocol_name(IrdaProtocol protocol);

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

#ifdef __cplusplus
}
#endif

