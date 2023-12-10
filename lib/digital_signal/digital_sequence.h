/**
 * @file digital_sequence.h
 * @brief Fast and precise digital signal generation library.
 *
 * Each sequence is represented by one or more (up to 32) registered signals, which are addressed
 * by their indices, and a list of signal indices to be transmitted.
 *
 * The registered signals must be set up prior to actually defining the sequence.
 *
 * Example: A sequence containing 4 registered signals and n indices to transmit.
 *
 * |Signal | Index |
 * |:-----:|:-----:|
 * |  SOF  |   0   |
 * |  EOF  |   1   |
 * | Zero  |   2   |
 * |  One  |   3   |
 *
 * ```
 * Signal index | 0 | 3 | 2 | 2 | ... |   3   |   1   |
 *                0   1   2   3   ...   n - 2   n - 1
 * ```
 *
 * The above sequence starts by transmitting the signal with index 0, which is SOF in this case,
 * then it proceeds with indices 3, 2, 2, which are One, Zero, Zero and after n - 2 signals,
 * it will conclude with indices 3 and 1 which are One and EOF respectively.
 *
 * This way, only the order in which the signals are sent is stored, while the signals themselves
 * are not duplicated.
 */
#pragma once

#include <furi_hal_gpio.h>

#include <digital_signal/digital_signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DigitalSequence DigitalSequence;

/**
 * @brief Allocate a DigitalSequence instance of a given size which will operate on a set GPIO pin.
 *
 * @param[in] size maximum number of signal indices contained in the instance.
 * @param[in] gpio the GPIO pin used to generate the signal.
 * @returns pointer to the allocated DigitalSequence instance.
 */
DigitalSequence* digital_sequence_alloc(uint32_t size, const GpioPin* gpio);

/**
 * @brief Delete a previously allocated DigitalSequence instance.
 *
 * @param[in,out] sequence pointer to the instance to be deleted.
 */
void digital_sequence_free(DigitalSequence* sequence);

/**
 * @brief Register a signal within a DigitalSequence instance by its index.
 *
 * This function must be called for each signal to be used in the sequence. The DigitalSequence
 * instance does not own the signals, therefore, their lifetime must be no less than the instance's.
 *
 * The user is responsible for creation and deletion of DigitalSignal instances and
 * also for keeping track of their respective indices.
 *
 * @param[in,out] sequence pointer to the instance to be modified.
 * @param[in] signal_index index to register the signal under (must be less than 32).
 * @param[in] signal pointer to the DigitalSignal instance to be registered.
 */
void digital_sequence_register_signal(
    DigitalSequence* sequence,
    uint8_t signal_index,
    const DigitalSignal* signal);

/**
 * @brief Append a signal index to a DigitalSequence instance.
 *
 * The signal under the index must be registered beforehand by calling digital_sequence_set_signal().
 *
 * @param[in,out] sequence pointer to the instance to be modified.
 * @param[in] signal_index signal index to be appended to the sequence (must be less than 32).
 */
void digital_sequence_add_signal(DigitalSequence* sequence, uint8_t signal_index);

/**
 * @brief Transmit the sequence contained in the DigitalSequence instance.
 *
 * Must contain at least one registered signal and one signal index.
 *
 * NOTE: The current implementation will properly initialise the GPIO provided during construction,
 * but it is the caller's responsibility to reconfigure it back before reusing for other purposes.
 * This is due to performance reasons.
 *
 * @param[in] sequence pointer to the sequence to be transmitted.
 */
void digital_sequence_transmit(DigitalSequence* sequence);

/**
 * @brief Clear the signal sequence in a DigitalSequence instance.
 *
 * Calling this function does not un-register the registered signals, so it is
 * safe to call digital_sequence_add_signal() right afterwards.
 *
 * @param[in,out] sequence pointer to the instance to be cleared.
 */
void digital_sequence_clear(DigitalSequence* sequence);

#ifdef __cplusplus
}
#endif
