/**
 * @file texkom_signal.h
 * @brief DigitalSequence preset for generating Texkom tag signals.
 *
 */
#pragma once

#include <furi_hal_resources.h>

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bytes in one Texkom frame. Pinned to TEXKOM_FRAME_SIZE in texkom_signal.c. */
#define TEXKOM_SIGNAL_FRAME_SIZE (8U)

typedef struct TexkomSignal TexkomSignal;

/**
 * @brief Supported line codings.
 *
 * A Texkom tag spaces its load-modulation impulses apart to carry data; these are the two ways it
 * does so. TK13, TK15 and MMBIT all use the first one. Mirrors TexkomCoding, so that this header
 * stays free of an nfc dependency; the two are pinned together in texkom_signal.c.
 */
typedef enum {
    TexkomSignalCodingTk13, /**< Two gaps per bit, long gap first for a one. */
    TexkomSignalCodingTk17, /**< A pair of gaps splits a fixed total to carry two bits. */
    TexkomSignalCodingNum, /**< Coding count. Internal use. */
} TexkomSignalCoding;

/**
 * @brief Allocate a TexkomSignal instance with a set GPIO pin.
 *
 * @param[in] pin GPIO pin to use during transmission.
 * @returns pointer to the allocated instance.
 */
TexkomSignal* texkom_signal_alloc(const GpioPin* pin);

/**
 * @brief Delete a TexkomSignal instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void texkom_signal_free(TexkomSignal* instance);

/**
 * @brief Transmit one Texkom frame.
 *
 * This function will block until the transmission has been completed, which takes about 7 ms for
 * TK13 and 5 ms for TK17. A tag repeats its frame for as long as it is in a field, so the caller
 * is expected to call this over and over.
 *
 * @param[in] instance pointer to the instance used in transmission.
 * @param[in] coding line coding to transmit in.
 * @param[in] frame pointer to the 8 frame bytes to be transmitted.
 */
void texkom_signal_tx(TexkomSignal* instance, TexkomSignalCoding coding, const uint8_t* frame);

#ifdef __cplusplus
}
#endif
