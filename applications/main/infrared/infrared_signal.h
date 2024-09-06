/**
 * @file infrared_signal.h
 * @brief Infrared signal library.
 *
 * Infrared signals may be of two types:
 * - known to the infrared signal decoder, or *parsed* signals
 * - the rest, or *raw* signals, which are treated merely as a set of timings.
 */
#pragma once

#include "infrared_error_code.h"
#include <flipper_format/flipper_format.h>
#include <infrared/encoder_decoder/infrared.h>

/**
 * @brief InfraredSignal opaque type declaration.
 */
typedef struct InfraredSignal InfraredSignal;

/**
 * @brief Raw signal type definition.
 *
 * Measurement units used:
 * - time: microseconds (uS)
 * - frequency: Hertz (Hz)
 * - duty_cycle: no units, fraction between 0 and 1.
 */
typedef struct {
    size_t timings_size; /**< Number of elements in the timings array. */
    uint32_t* timings; /**< Pointer to an array of timings describing the signal. */
    uint32_t frequency; /**< Carrier frequency of the signal. */
    float duty_cycle; /**< Duty cycle of the signal. */
} InfraredRawSignal;

/**
 * @brief Create a new InfraredSignal instance.
 *
 * @returns pointer to the instance created.
 */
InfraredSignal* infrared_signal_alloc(void);

/**
 * @brief Delete an InfraredSignal instance.
 *
 * @param[in,out] signal pointer to the instance to be deleted.
 */
void infrared_signal_free(InfraredSignal* signal);

/**
 * @brief Test whether an InfraredSignal instance holds a raw signal.
 *
 * @param[in] signal pointer to the instance to be tested.
 * @returns true if the instance holds a raw signal, false otherwise.
 */
bool infrared_signal_is_raw(const InfraredSignal* signal);

/**
 * @brief Test whether an InfraredSignal instance holds any signal.
 *
 * @param[in] signal pointer to the instance to be tested.
 * @returns true if the instance holds raw signal, false otherwise.
 */
bool infrared_signal_is_valid(const InfraredSignal* signal);

/**
 * @brief Set an InfraredInstance to hold the signal from another one.
 *
 * Any instance's previous contents will be automatically deleted before
 * copying the source instance's contents.
 *
 * @param[in,out] signal pointer to the destination instance.
 * @param[in] other pointer to the source instance.
 */
void infrared_signal_set_signal(InfraredSignal* signal, const InfraredSignal* other);

/**
 * @brief Set an InfraredInstance to hold a raw signal.
 *
 * Any instance's previous contents will be automatically deleted before
 * copying the raw signal.
 *
 * After this call, infrared_signal_is_raw() will return true.
 *
 * @param[in,out] signal pointer to the destination instance.
 * @param[in] timings pointer to an array containing the raw signal timings.
 * @param[in] timings_size number of elements in the timings array.
 * @param[in] frequency signal carrier frequency, in Hertz.
 * @param[in] duty_cycle signal duty cycle, fraction between 0 and 1.
 */
void infrared_signal_set_raw_signal(
    InfraredSignal* signal,
    const uint32_t* timings,
    size_t timings_size,
    uint32_t frequency,
    float duty_cycle);

/**
 * @brief Get the raw signal held by an InfraredSignal instance.
 *
 * @warning the instance MUST hold a *raw* signal, otherwise undefined behaviour will occur.
 *
 * @param[in] signal pointer to the instance to be queried.
 * @returns pointer to the raw signal structure held by the instance.
 */
const InfraredRawSignal* infrared_signal_get_raw_signal(const InfraredSignal* signal);

/**
 * @brief Set an InfraredInstance to hold a parsed signal.
 *
 * Any instance's previous contents will be automatically deleted before
 * copying the raw signal.
 *
 * After this call, infrared_signal_is_raw() will return false.
 *
 * @param[in,out] signal pointer to the destination instance.
 * @param[in] message pointer to the message containing the parsed signal.
 */
void infrared_signal_set_message(InfraredSignal* signal, const InfraredMessage* message);

/**
 * @brief Get the parsed signal held by an InfraredSignal instance.
 *
 * @warning the instance MUST hold a *parsed* signal, otherwise undefined behaviour will occur.
 *
 * @param[in] signal pointer to the instance to be queried.
 * @returns pointer to the parsed signal structure held by the instance.
 */
const InfraredMessage* infrared_signal_get_message(const InfraredSignal* signal);

/**
 * @brief Read a signal and its name from a FlipperFormat file into an InfraredSignal instance.
 *
 * The file must be allocated and open prior to this call. The seek position determines
 * which signal will be read (if there is more than one in the file). Calling this function
 * repeatedly will result in all signals in the file to be read until no more are left.
 *
 * @param[in,out] signal pointer to the instance to be read into.
 * @param[in,out] ff pointer to the FlipperFormat file instance to read from.
 * @param[out] name pointer to the string to hold the signal name. Must be properly allocated.
 * @returns InfraredErrorCodeNone if a signal was successfully read, otherwise error code
 */
InfraredErrorCode
    infrared_signal_read(InfraredSignal* signal, FlipperFormat* ff, FuriString* name);

/**
 * @brief Read a signal name from a FlipperFormat file.
 *
 * Same behaviour as infrared_signal_read(), but only the name is read.
 *
 * @param[in,out] ff pointer to the FlipperFormat file instance to read from.
 * @param[out] name pointer to the string to hold the signal name. Must be properly allocated.
 * @returns InfraredErrorCodeNone if a signal name was successfully read, otherwise error code
 */
InfraredErrorCode infrared_signal_read_name(FlipperFormat* ff, FuriString* name);

/**
 * @brief Read a signal from a FlipperFormat file.
 *
 * Same behaviour as infrared_signal_read(), but only the body is read.
 *
 * @param[in,out] ff pointer to the FlipperFormat file instance to read from.
 * @param[out] body pointer to the InfraredSignal instance to hold the signal body. Must be properly allocated.
 * @returns InfraredErrorCodeNone if a signal body was successfully read, otherwise error code.
 */
InfraredErrorCode infrared_signal_read_body(InfraredSignal* signal, FlipperFormat* ff);

/**
 * @brief Read a signal with a particular name from a FlipperFormat file into an InfraredSignal instance.
 *
 * This function will look for a signal with the given name and if found, attempt to read it.
 * Same considerations apply as to infrared_signal_read().
 *
 * @param[in,out] signal pointer to the instance to be read into.
 * @param[in,out] ff pointer to the FlipperFormat file instance to read from.
 * @param[in] name pointer to a zero-terminated string containing the requested signal name.
 * @returns InfraredErrorCodeNone if a signal was found and successfully read, otherwise error code.
 */
InfraredErrorCode infrared_signal_search_by_name_and_read(
    InfraredSignal* signal,
    FlipperFormat* ff,
    const char* name);

/**
 * @brief Read a signal with a particular index from a FlipperFormat file into an InfraredSignal instance.
 *
 * This function will look for a signal with the given index and if found, attempt to read it.
 * Same considerations apply as to infrared_signal_read().
 *
 * @param[in,out] signal pointer to the instance to be read into.
 * @param[in,out] ff pointer to the FlipperFormat file instance to read from.
 * @param[in] index the requested signal index.
 * @returns InfraredErrorCodeNone if a signal was found and successfully read, otherwise error code.
 */
InfraredErrorCode infrared_signal_search_by_index_and_read(
    InfraredSignal* signal,
    FlipperFormat* ff,
    size_t index);

/**
 * @brief Save a signal contained in an InfraredSignal instance to a FlipperFormat file.
 *
 * The file must be allocated and open prior to this call. Additionally, an appropriate header
 * must be already written into the file.
 *
 * @param[in] signal pointer to the instance holding the signal to be saved.
 * @param[in,out] ff pointer to the FlipperFormat file instance to write to.
 * @param[in] name pointer to a zero-terminated string contating the name of the signal.
 * @returns InfraredErrorCodeNone if a signal was successfully saved, otherwise error code
 */
InfraredErrorCode
    infrared_signal_save(const InfraredSignal* signal, FlipperFormat* ff, const char* name);

/**
 * @brief Transmit a signal contained in an InfraredSignal instance.
 *
 * The transmission happens once per call using the built-in hardware (via HAL calls).
 *
 * @param[in] signal pointer to the instance holding the signal to be transmitted.
 */
void infrared_signal_transmit(const InfraredSignal* signal);
