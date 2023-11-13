/**
 * @file iso14443_3a_signal.h
 * @brief DigitalSequence preset for generating ISO14443-3A compliant signals.
 */
#pragma once

#include <furi_hal_resources.h>

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_3aSignal Iso14443_3aSignal;

/**
 * @brief Allocate an Iso14443_3aSignal instance with a set GPIO pin.
 *
 * @param[in] pin GPIO pin to use during transmission.
 * @returns pointer to the allocated instance.
 */
Iso14443_3aSignal* iso14443_3a_signal_alloc(const GpioPin* pin);

/**
 * @brief Delete an Iso14443_3aSignal instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void iso14443_3a_signal_free(Iso14443_3aSignal* instance);

/**
 * @brief Transmit arbitrary bytes using an Iso14443_3aSignal instance.
 *
 * This function will block until the transmisson has been completed.
 *
 * @param[in] instance pointer to the instance used in transmission.
 * @param[in] tx_data pointer to the data to be transmitted.
 * @param[in] tx_parity pointer to the bit-packed parity array.
 * @param[in] tx_bits size of the data to be transmitted in bits.
 */
void iso14443_3a_signal_tx(
    Iso14443_3aSignal* instance,
    const uint8_t* tx_data,
    const uint8_t* tx_parity,
    size_t tx_bits);

#ifdef __cplusplus
}
#endif
