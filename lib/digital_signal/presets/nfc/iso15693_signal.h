/**
 * @file iso15693_signal.h
 * @brief DigitalSequence preset for generating ISO15693-compliant signals.
 *
 */
#pragma once

#include <furi_hal_resources.h>

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso15693Signal Iso15693Signal;

/**
 * @brief Supported data rates.
 */
typedef enum {
    Iso15693SignalDataRateHi, /**< High data rate. */
    Iso15693SignalDataRateLo, /**< Low data rate. */
    Iso15693SignalDataRateNum, /**< Data rate mode count. Internal use. */
} Iso15693SignalDataRate;

/**
 * @brief Allocate an Iso15693Signal instance with a set GPIO pin.
 *
 * @param[in] pin GPIO pin to use during transmission.
 * @returns pointer to the allocated instance.
 */
Iso15693Signal* iso15693_signal_alloc(const GpioPin* pin);

/**
 * @brief Delete an Iso15693Signal instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void iso15693_signal_free(Iso15693Signal* instance);

/**
 * @brief Transmit arbitrary bytes using an Iso15693Signal instance.
 * @see Iso15693SignalDataRate
 *
 * This function will block until the transmisson has been completed.
 *
 * @param[in] instance pointer to the instance used in transmission.
 * @param[in] data_rate data rate to transmit at.
 * @param[in] tx_data pointer to the data to be transmitted.
 * @param[in] tx_data_size size of the data to be transmitted in bytes.
 */
void iso15693_signal_tx(
    Iso15693Signal* instance,
    Iso15693SignalDataRate data_rate,
    const uint8_t* tx_data,
    size_t tx_data_size);

/**
 * @brief Transmit Start of Frame using an Iso15693Signal instance.
 * @see Iso15693SignalDataRate
 *
 * This function will block until the transmisson has been completed.
 *
 * @param[in] instance pointer to the instance used in transmission.
 * @param[in] data_rate data rate to transmit at.
 */
void iso15693_signal_tx_sof(Iso15693Signal* instance, Iso15693SignalDataRate data_rate);

#ifdef __cplusplus
}
#endif
