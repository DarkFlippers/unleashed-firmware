#pragma once

#include "ntag4xx.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ntag4xxPoller opaque type definition.
 */
typedef struct Ntag4xxPoller Ntag4xxPoller;

/**
 * @brief Enumeration of possible Ntag4xx poller event types.
 */
typedef enum {
    Ntag4xxPollerEventTypeReadSuccess, /**< Card was read successfully. */
    Ntag4xxPollerEventTypeReadFailed, /**< Poller failed to read card. */
} Ntag4xxPollerEventType;

/**
 * @brief Ntag4xx poller event data.
 */
typedef union {
    Ntag4xxError error; /**< Error code indicating card reading fail reason. */
} Ntag4xxPollerEventData;

/**
 * @brief Ntag4xx poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Ntag4xxPollerEventType type; /**< Type of emmitted event. */
    Ntag4xxPollerEventData* data; /**< Pointer to event specific data. */
} Ntag4xxPollerEvent;

#ifdef __cplusplus
}
#endif
