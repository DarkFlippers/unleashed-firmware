#pragma once

#include "type_4_tag.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type4TagPoller opaque type definition.
 */
typedef struct Type4TagPoller Type4TagPoller;

/**
 * @brief Enumeration of possible Type4Tag poller event types.
 */
typedef enum {
    Type4TagPollerEventTypeRequestMode, /**< Poller requests for operating mode. */
    Type4TagPollerEventTypeReadSuccess, /**< Card was read successfully. */
    Type4TagPollerEventTypeReadFailed, /**< Poller failed to read card. */
    Type4TagPollerEventTypeWriteSuccess, /**< Poller wrote card successfully. */
    Type4TagPollerEventTypeWriteFail, /**< Poller failed to write card. */
} Type4TagPollerEventType;

/**
 * @brief Enumeration of possible Type4Tag poller operating modes.
 */
typedef enum {
    Type4TagPollerModeRead, /**< Poller will only read card. It's a default mode. */
    Type4TagPollerModeWrite, /**< Poller will write already saved card to another presented card. */
} Type4TagPollerMode;

/**
 * @brief Type4Tag poller request mode event data.
 *
 * This instance of this structure must be filled on Type4TagPollerEventTypeRequestMode event.
 */
typedef struct {
    Type4TagPollerMode mode; /**< Mode to be used by poller. */
    const Type4TagData* data; /**< Data to be used by poller. */
} Type4TagPollerEventDataRequestMode;

/**
 * @brief Type4Tag poller event data.
 */
typedef union {
    Type4TagError error; /**< Error code indicating card reading fail reason. */
    Type4TagPollerEventDataRequestMode poller_mode; /**< Poller mode context. */
} Type4TagPollerEventData;

/**
 * @brief Type4Tag poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Type4TagPollerEventType type; /**< Type of emmitted event. */
    Type4TagPollerEventData* data; /**< Pointer to event specific data. */
} Type4TagPollerEvent;

#ifdef __cplusplus
}
#endif
