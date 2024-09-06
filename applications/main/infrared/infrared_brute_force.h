/**
 * @file infrared_brute_force.h
 * @brief Infrared signal brute-forcing library.
 *
 * The BruteForce library is used to send large quantities of signals,
 * sorted by a category. It is used to implement the Universal Remote
 * feature.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "infrared_error_code.h"

/**
 * @brief InfraredBruteForce opaque type declaration.
 */
typedef struct InfraredBruteForce InfraredBruteForce;

/**
 * @brief Create a new InfraredBruteForce instance.
 *
 * @returns pointer to the created instance.
 */
InfraredBruteForce* infrared_brute_force_alloc(void);

/**
 * @brief Delete an InfraredBruteForce instance.
 *
 * @param[in,out] brute_force pointer to the instance to be deleted.
 */
void infrared_brute_force_free(InfraredBruteForce* brute_force);

/**
 * @brief Set an InfraredBruteForce instance to use a signal database contained in a file.
 *
 * @param[in,out] brute_force pointer to the instance to be configured.
 * @param[in] db_filename pointer to a zero-terminated string containing a full path to the database file.
 */
void infrared_brute_force_set_db_filename(InfraredBruteForce* brute_force, const char* db_filename);

/**
 * @brief Build a signal dictionary from a previously set database file.
 *
 * This function must be called each time after setting the database via
 * a infrared_brute_force_set_db_filename() call.
 *
 * @param[in,out] brute_force pointer to the instance to be updated.
 * @returns InfraredErrorCodeNone on success, otherwise error code.
 */
InfraredErrorCode infrared_brute_force_calculate_messages(InfraredBruteForce* brute_force);

/**
 * @brief Start transmitting signals from a category stored in an InfraredBruteForce's instance dictionary.
 *
 * @param[in,out] brute_force pointer to the instance to be started.
 * @param[in] index index of the signal category in the dictionary.
 * @returns true on success, false otherwise.
 */
bool infrared_brute_force_start(
    InfraredBruteForce* brute_force,
    uint32_t index,
    uint32_t* record_count);

/**
 * @brief Determine whether the transmission was started.
 *
 * @param[in] brute_force pointer to the instance to be tested.
 * @returns true if transmission was started, false otherwise.
 */
bool infrared_brute_force_is_started(const InfraredBruteForce* brute_force);

/**
 * @brief Stop transmitting the signals.
 *
 * @param[in] brute_force pointer to the instance to be stopped.
 */
void infrared_brute_force_stop(InfraredBruteForce* brute_force);

/**
 * @brief Send the next signal from the chosen category.
 *
 * This function is called repeatedly until no more signals are left
 * in the chosen signal category.
 *
 * @warning Transmission must be started first by calling infrared_brute_force_start()
 * before calling this function.
 *
 * @param[in,out] brute_force pointer to the instance to be used.
 * @returns true if the next signal existed and could be transmitted, false otherwise.
 */
bool infrared_brute_force_send_next(InfraredBruteForce* brute_force);

/**
 * @brief Add a signal category to an InfraredBruteForce instance's dictionary.
 *
 * @param[in,out] brute_force pointer to the instance to be updated.
 * @param[in] index index of the category to be added.
 * @param[in] name name of the category to be added.
 */
void infrared_brute_force_add_record(
    InfraredBruteForce* brute_force,
    uint32_t index,
    const char* name);

/**
 * @brief Reset an InfraredBruteForce instance.
 *
 * @param[in,out] brute_force pointer to the instance to be reset.
 */
void infrared_brute_force_reset(InfraredBruteForce* brute_force);
