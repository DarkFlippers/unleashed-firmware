/**
 * @file nfc_scanner.h
 * @brief NFC card detection library.
 *
 * Once started, a NfcScanner instance will iterate over all available protocols
 * and return a list of one or more detected protocol identifiers via a user-provided callback.
 *
 * The NfcScanner behaviour is greedy, i.e. it will not stop scanning upon detection of
 * a just one protocol and will try others as well until all possibilities are exhausted.
 * This is to allow for multi-protocol card support.
 *
 * If no supported cards are in the vicinity, the scanning process will continue
 * until stopped explicitly.
 */
#pragma once

#include <nfc/nfc.h>
#include <nfc/protocols/nfc_protocol.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NfcScanner opaque type definition.
 */
typedef struct NfcScanner NfcScanner;

/**
 * @brief Event type passed to the user callback.
 */
typedef enum {
    NfcScannerEventTypeDetected, /**< One or more protocols have been detected. */
} NfcScannerEventType;

/**
 * @brief Event data passed to the user callback.
 */
typedef struct {
    size_t protocol_num; /**< Number of detected protocols (one or more). */
    NfcProtocol* protocols; /**< Pointer to the array of detected protocol identifiers. */
} NfcScannerEventData;

/**
 * @brief Event passed to the user callback.
 */
typedef struct {
    NfcScannerEventType type; /**< Type of event. Determines how the data must be handled. */
    NfcScannerEventData data; /**< Event-specific data. Handled accordingly to the even type. */
} NfcScannerEvent;

/**
 * @brief User callback function signature.
 *
 * A function with such signature must be provided by the user upon calling nfc_scanner_start().
 *
 * @param[in] event occurred event, complete with type and data.
 * @param[in] context pointer to the context data provided in nfc_scanner_start() call.
 */
typedef void (*NfcScannerCallback)(NfcScannerEvent event, void* context);

/**
 * @brief Allocate an NfcScanner instance.
 *
 * @param[in] nfc pointer to an Nfc instance.
 * @returns pointer to the allocated NfcScanner instance.
 *
 * @see nfc.h
 */
NfcScanner* nfc_scanner_alloc(Nfc* nfc);

/**
 * @brief Delete an NfcScanner instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void nfc_scanner_free(NfcScanner* instance);

/**
 * @brief Start an NfcScanner.
 *
 * @param[in,out] instance pointer to the instance to be started.
 * @param[in] callback pointer to the callback function (will be called upon a detection event).
 * @param[in] context pointer to the caller-specific context (will be passed to the callback).
 */
void nfc_scanner_start(NfcScanner* instance, NfcScannerCallback callback, void* context);

/**
 * @brief Stop an NfcScanner.
 *
 * @param[in,out] instance pointer to the instance to be stopped.
 */
void nfc_scanner_stop(NfcScanner* instance);

#ifdef __cplusplus
}
#endif
