/**
 * @file nfc_poller_base.h
 * @brief Abstract interface definitions for the NFC poller system.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 *
 * @see nfc_poller.h
 *
 */
#pragma once

#include "nfc_generic_event.h"
#include "nfc_device_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate a protocol-specific poller instance.
 *
 * For base pollers pass a pointer to an instance of type Nfc
 * as the base_poller parameter, otherwise it must be a pointer to another poller instance
 * (compare iso14443_3a/iso14443_3a_poller.c and iso14443_4a/iso14443_4a_poller.c).
 *
 * @see nfc_protocol.c
 *
 * @param[in] base_poller pointer to the parent poller instance.
 * @returns pointer to the allocated poller instance.
 */
typedef NfcGenericInstance* (*NfcPollerAlloc)(NfcGenericInstance* base_poller);

/**
 * @brief Delete a protocol-specific poller instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
typedef void (*NfcPollerFree)(NfcGenericInstance* instance);

/**
 * @brief Set the callback function to handle events emitted by the poller instance.
 *
 * @see nfc_generic_event.h
 *
 * @param[in,out] poller pointer to the protocol-specific poller instance.
 * @param[in] callback pointer to the user-defined callback function which will receive events.
 * @param[in] context pointer to the user-specific context (will be passed to the callback).
 */
typedef void (
    *NfcPollerSetCallback)(NfcGenericInstance* poller, NfcGenericCallback callback, void* context);

/**
 * @brief Activate and read a supported NFC card.
 *
 * Ths function is passed to the parent poller's ${POLLER_NAME}_set_callback function as
 * the callback parameter. This is done automatically by the NfcPoller implementation based
 * on the protocol hierarchy defined in nfc_protocol.c, so there is no need to call it explicitly.
 *
 * Thus, it will be called each time the parent poller emits an event. Usually it happens
 * only after the parent poller has successfully completed its job.
 *
 * Example for an application reading a card with a compound (non-base) protocol (simplified):
 *
 * ```
 * start()   <--     set_callback()    <--    set_callback()     <--  nfc_poller_start()
 *            |                         |                         |
 *   Nfc      |        Base Poller      |      Child Poller       |      Application
 *            |                         |                         |
 * worker()  -->         run()         -->         run()         --->    handle_event()
 * ```
 *
 * The base poller receives events directly from an Nfc instance, from which they are
 * propagated as needed to however many other pollers there are in the current hierarchy.
 *
 * This function can be thought of as the poller's "main loop" function. Depending
 * on the particular poller implementation, it may perform actions such as reading
 * and writing to an NFC card, state changes and control of the parent poller.
 *
 * @see nfc_generic_event.h
 *
 * @param[in] event protocol-specific event passed by the parent poller instance.
 * @param[in,out] context pointer to the protocol-specific poller instance.
 * @returns command to be executed by the parent poller instance.
 */
typedef NfcCommand (*NfcPollerRun)(NfcGenericEvent event, void* context);

/**
 * @brief Determine whether there is a supported card in the vicinity.
 *
 * The behaviour is mostly the same as of NfcPollerRun, with the difference in the
 * procedure and return value.
 * The procedure implemented in this function must do whatever it needs to unambigiously
 * determine whether a supported and valid NFC card is in the vicinity.
 *
 * Like the previously described NfcPollerRun, it is called automatically by the NfcPoller
 * implementation, so there is no need to call it explicitly.
 *
 * @param[in] event protocol-specific event passed by the parent poller instance.
 * @param[in,out] context pointer to the protocol-specific poller instance.
 * @returns true if a supported card was detected, false otherwise.
 */
typedef bool (*NfcPollerDetect)(NfcGenericEvent event, void* context);

/**
 * @brief Get the data that was that was gathered during the reading process.
 *
 * @param[in] instance pointer to the protocol-specific poller instance.
 * @returns pointer to the NFC device data.
 */
typedef const NfcDeviceData* (*NfcPollerGetData)(const NfcGenericInstance* instance);

/**
 * @brief Generic NFC poller interface.
 *
 * Each protocol must fill this structure with its own function implementations.
 * See above for the function signatures and descriptions.
 *
 * Additionally, see ${PROTOCOL_NAME}/${PROTOCOL_NAME}_poller.c for usage examples.
 */
typedef struct {
    NfcPollerAlloc alloc; /**< Pointer to the alloc() function. */
    NfcPollerFree free; /**< Pointer to the free() function. */
    NfcPollerSetCallback set_callback; /**< Pointer to the set_callback() function. */
    NfcPollerRun run; /**< Pointer to the run() function. */
    NfcPollerDetect detect; /**< Pointer to the detect() function. */
    NfcPollerGetData get_data; /**< Pointer to the get_data() function. */
} NfcPollerBase;

#ifdef __cplusplus
}
#endif
