#pragma once

#include "mf_plus.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MIFARE Plus listener opaque type definition.
 */
typedef struct MfPlusListener MfPlusListener;

/**
 * @brief Enumeration of possible MfPlus listener event types.
 *
 * The SL3 listener currently emulates without emitting app-level events (started with a NULL
 * callback), so this exists only to complete the generic listener event scaffolding.
 */
typedef enum {
    MfPlusListenerEventTypeUpdate, /**< Reserved: listener state changed. */
} MfPlusListenerEventType;

/**
 * @brief MIFARE Plus listener event data.
 */
typedef union {
    void* reserved;
} MfPlusListenerEventData;

/**
 * @brief MIFARE Plus listener event structure.
 */
typedef struct {
    MfPlusListenerEventType type; /**< Type of emitted event. */
    MfPlusListenerEventData* data; /**< Pointer to event specific data. */
} MfPlusListenerEvent;

#ifdef __cplusplus
}
#endif
