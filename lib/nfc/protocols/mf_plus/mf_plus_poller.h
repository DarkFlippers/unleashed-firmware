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
    MfPlusPollerEventTypeRequestMode, /**< Poller requests the app to choose a mode. */
    MfPlusPollerEventTypeRequestKey, /**< Poller requests a candidate key for a sector. */
    MfPlusPollerEventTypeDataUpdate, /**< Poller reports read progress. */
    MfPlusPollerEventTypeReadSuccess, /**< Card was read successfully. */
    MfPlusPollerEventTypeReadFailed, /**< Poller failed to read the card. */
} MfPlusPollerEventType;

/**
 * @brief MIFARE Plus poller mode.
 */
typedef enum {
    MfPlusPollerModeInfo, /**< Identify only (default): version/type/SL/size, no authentication. */
    MfPlusPollerModeRead, /**< SL3 read: signature + auth + read all sectors with supplied keys. */
} MfPlusPollerMode;

/** Filled by the app on MfPlusPollerEventTypeRequestMode. */
typedef struct {
    MfPlusPollerMode mode;
} MfPlusPollerEventDataRequestMode;

/** The poller sets the request target; the app fills key + key_provided on RequestKey. The app
 *  MUST eventually answer key_provided = false (dictionary exhausted) so the scan can advance.
 *  When is_admin is false the target is (sector, key_type); when true it is admin_type (a 0x90xx
 *  key), and sector/key_type are unused. */
typedef struct {
    bool is_admin;
    uint8_t sector; // valid when !is_admin
    MfPlusKeyType key_type; // valid when !is_admin
    MfPlusAdminKeyType admin_type; // valid when is_admin
    MfPlusKey key;
    bool key_provided;
} MfPlusPollerEventDataKeyRequest;

/** Filled by the poller on MfPlusPollerEventTypeDataUpdate. */
typedef struct {
    uint8_t current_sector;
    uint8_t sectors_read;
    uint8_t keys_found;
} MfPlusPollerEventDataUpdate;

/**
 * @brief MIFARE Plus poller event data.
 */
typedef union {
    MfPlusError error; /**< Error code indicating card reading fail reason. */
    MfPlusPollerEventDataRequestMode mode_request; /**< Mode request context. */
    MfPlusPollerEventDataKeyRequest key_request; /**< Key request context. */
    MfPlusPollerEventDataUpdate data_update; /**< Read progress context. */
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
