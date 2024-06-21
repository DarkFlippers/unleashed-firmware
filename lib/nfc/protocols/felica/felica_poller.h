#pragma once

#include "felica.h"
#include <lib/nfc/nfc.h>

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FelicaPoller opaque type definition.
 */
typedef struct FelicaPoller FelicaPoller;

/**
 * @brief Enumeration of possible Felica poller event types.
 */
typedef enum {
    FelicaPollerEventTypeError, /**< An error occured during activation procedure. */
    FelicaPollerEventTypeReady, /**< The card was activated and fully read by the poller. */
    FelicaPollerEventTypeIncomplete, /**< The card was activated and partly read by the poller. */
    FelicaPollerEventTypeRequestAuthContext, /**< Authentication context was requested by poller. */
} FelicaPollerEventType;

/**
 * @brief Felica poller event data.
 */
typedef union {
    FelicaError error; /**< Error code indicating card activation fail reason. */
    FelicaAuthenticationContext* auth_context; /**< Authentication context to be filled by user. */
} FelicaPollerEventData;

/**
 * @brief FelicaPoller poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    FelicaPollerEventType type; /**< Type of emmitted event. */
    FelicaPollerEventData* data; /**< Pointer to event specific data. */
} FelicaPollerEvent;

/**
 * @brief Perform collision resolution procedure.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perfoms the collision resolution procedure as defined in FeliCa standars. The data
 * field will be filled with Felica data on success.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the Felica data structure to be filled.
 * @return FelicaErrorNone on success, an error code on failure.
 */
FelicaError felica_poller_activate(FelicaPoller* instance, FelicaData* data);

#ifdef __cplusplus
}
#endif
