#pragma once

#include "mf_ultralight.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MfUltralightPoller opaque type definition.
 */
typedef struct MfUltralightPoller MfUltralightPoller;

/**
 * @brief Enumeration of possible MfUltralight poller event types.
 */
typedef enum {
    MfUltralightPollerEventTypeRequestMode, /**< Poller requests for operating mode. */
    MfUltralightPollerEventTypeAuthRequest, /**< Poller requests to fill authentication context. */
    MfUltralightPollerEventTypeAuthSuccess, /**< Authentication succeeded. */
    MfUltralightPollerEventTypeAuthFailed, /**< Authentication failed. */
    MfUltralightPollerEventTypeReadSuccess, /**< Poller read card successfully. */
    MfUltralightPollerEventTypeReadFailed, /**< Poller failed to read card. */
    MfUltralightPollerEventTypeRequestWriteData, /**< Poller request card data for write operation. */
    MfUltralightPollerEventTypeCardMismatch, /**< Type of card for writing differs from presented one. */
    MfUltralightPollerEventTypeCardLocked, /**< Presented card is locked by password, AUTH0 or lock bytes. */
    MfUltralightPollerEventTypeWriteSuccess, /**< Poller wrote card successfully. */
    MfUltralightPollerEventTypeWriteFail, /**< Poller failed to write card. */
} MfUltralightPollerEventType;

/**
 * @brief Enumeration of possible MfUltralight poller operating modes.
 */
typedef enum {
    MfUltralightPollerModeRead, /**< Poller will only read card. It's a default mode. */
    MfUltralightPollerModeWrite, /**< Poller will write already saved card to another presented card. */
} MfUltralightPollerMode;

/**
 * @brief MfUltralight poller authentication context.
 */
typedef struct {
    MfUltralightAuthPassword password; /**< Password to be used for authentication. */
    MfUltralightC3DesAuthKey tdes_key;
    MfUltralightAuthPack pack; /**< Pack received on successfull authentication. */
    bool auth_success; /**< Set to true if authentication succeeded, false otherwise. */
    bool skip_auth; /**< Set to true if authentication should be skipped, false otherwise. */
} MfUltralightPollerAuthContext;

/**
 * @brief MfUltralight poller event data.
 */
typedef union {
    MfUltralightPollerAuthContext auth_context; /**< Authentication context. */
    MfUltralightError error; /**< Error code indicating reading fail reason. */
    const MfUltralightData* write_data;
    MfUltralightPollerMode poller_mode;
} MfUltralightPollerEventData;

/**
 * @brief MfUltralight poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    MfUltralightPollerEventType type; /**< Type of emmitted event. */
    MfUltralightPollerEventData* data; /**< Pointer to event specific data. */
} MfUltralightPollerEvent;

/**
 * @brief Perform authentication with password.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in, out] data pointer to the authentication context.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError mf_ultralight_poller_auth_pwd(
    MfUltralightPoller* instance,
    MfUltralightPollerAuthContext* data);

/**
 * @brief Start authentication procedure.
 *
 * Must ONLY be used inside the callback function.
 *
 * This function is used to start authentication process for Ultralight C cards.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] RndA Randomly generated block which is required for authentication process.
 * @param[out] output Authentication encryption result.
 * @return MfUltralightErrorNone if card supports authentication command, an error code on otherwise.
 */
MfUltralightError mf_ultralight_poller_authenticate_start(
    MfUltralightPoller* instance,
    const uint8_t* RndA,
    uint8_t* output);

/**
 * @brief End authentication procedure
 * 
 * This function is used to end authentication process for Ultralight C cards.
 * 
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] RndB Block received from the card (card generates it randomly) which is required for authentication process.
 * @param[in] request Contains data of RndA + RndB', where RndB' is decoded and shifted RndB received from the card on previous step.
 * @param[out] response Must return RndA' which an encrypted shifted RndA value received from the card and decrypted by this function.
*/
MfUltralightError mf_ultralight_poller_authenticate_end(
    MfUltralightPoller* instance,
    const uint8_t* RndB,
    const uint8_t* request,
    uint8_t* response);

/**
 * @brief Read page from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * Send read command and parse response. The response on this command is data of 4 pages starting
 * from the page specified in the command.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] start_page page number to be read.
 * @param[out] data pointer to the MfUltralightPageReadCommandData structure to be filled with page data.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError mf_ultralight_poller_read_page(
    MfUltralightPoller* instance,
    uint8_t start_page,
    MfUltralightPageReadCommandData* data);

/**
 * @brief Read page from sector.
 *
 * Must ONLY be used inside the callback function.
 *
 * This command should be used for NTAGI2C tags.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] sector sector number to be read.
 * @param[in] tag tag number to be read.
 * @param[out] data pointer to the MfUltralightPageReadCommandData structure to be filled with page data.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError mf_ultralight_poller_read_page_from_sector(
    MfUltralightPoller* instance,
    uint8_t sector,
    uint8_t tag,
    MfUltralightPageReadCommandData* data);

/**
 * @brief Write page to card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] page page number to be written.
 * @param[in] data pointer to the MfUltralightPage structure to be written.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError mf_ultralight_poller_write_page(
    MfUltralightPoller* instance,
    uint8_t page,
    const MfUltralightPage* data);

/**
 * @brief Read version from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfUltralightVersion structure to be filled.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError
    mf_ultralight_poller_read_version(MfUltralightPoller* instance, MfUltralightVersion* data);

/**
 * @brief Read signature from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfUltralightSignature structure to be filled.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError
    mf_ultralight_poller_read_signature(MfUltralightPoller* instance, MfUltralightSignature* data);

/**
 * @brief Read counter from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] counter_num counter number to be read.
 * @param[out] data pointer to the MfUltralightCounter structure to be filled.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError mf_ultralight_poller_read_counter(
    MfUltralightPoller* instance,
    uint8_t counter_num,
    MfUltralightCounter* data);

/**
 * @brief Read tearing flag from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tearing_falg_num tearing flag number to be read.
 * @param[out] data pointer to the MfUltralightTearingFlag structure to be filled.
 * @return MfUltralightErrorNone on success, an error code on failure.
 */
MfUltralightError mf_ultralight_poller_read_tearing_flag(
    MfUltralightPoller* instance,
    uint8_t tearing_falg_num,
    MfUltralightTearingFlag* data);

#ifdef __cplusplus
}
#endif
