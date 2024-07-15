#pragma once

#include "mf_plus.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MIFARE Plus poller opaque type definition.
 */
typedef struct MfPlusPoller MfPlusPoller;

/**
 * @brief Enumeration of possible MfPlus poller event types.
 */

typedef enum {
    MfPlusPollerEventTypeReadSuccess, /**< Card was read successfully. */
    MfPlusPollerEventTypeReadFailed, /**< Poller failed to read the card. */
} MfPlusPollerEventType;

/**
 * @brief MIFARE Plus poller event data.
 */
typedef union {
    MfPlusError error; /**< Error code indicating card reading fail reason. */
} MfPlusPollerEventData;

/**
 * @brief MIFARE Plus poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    MfPlusPollerEventType type; /**< Type of emitted event. */
    MfPlusPollerEventData* data; /**< Pointer to event specific data. */
} MfPlusPollerEvent;

/**
 * @brief Read MfPlus card version.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfPlusVersion structure to be filled with version data.
 * @return MfPlusErrorNone on success, an error code on failure.
 */
MfPlusError mf_plus_poller_read_version(MfPlusPoller* instance, MfPlusVersion* data);

#ifdef __cplusplus
}
#endif
