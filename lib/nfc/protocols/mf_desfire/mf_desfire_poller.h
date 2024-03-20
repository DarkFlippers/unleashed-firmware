#pragma once

#include "mf_desfire.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MfDesfirePoller opaque type definition.
 */
typedef struct MfDesfirePoller MfDesfirePoller;

/**
 * @brief Enumeration of possible MfDesfire poller event types.
 */
typedef enum {
    MfDesfirePollerEventTypeReadSuccess, /**< Card was read successfully. */
    MfDesfirePollerEventTypeReadFailed, /**< Poller failed to read card. */
} MfDesfirePollerEventType;

/**
 * @brief MfDesfire poller event data.
 */
typedef union {
    MfDesfireError error; /**< Error code indicating card reading fail reason. */
} MfDesfirePollerEventData;

/**
 * @brief MfDesfire poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    MfDesfirePollerEventType type; /**< Type of emmitted event. */
    MfDesfirePollerEventData* data; /**< Pointer to event specific data. */
} MfDesfirePollerEvent;

/**
 * @brief Transmit and receive MfDesfire chunks in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_send_chunks(
    MfDesfirePoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer);

/**
 * @brief Read MfDesfire card version.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfDesfireVersion structure to be filled with version data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_version(MfDesfirePoller* instance, MfDesfireVersion* data);

/**
 * @brief Read free memory available on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfDesfireFreeMemory structure to be filled with free memory data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError
    mf_desfire_poller_read_free_memory(MfDesfirePoller* instance, MfDesfireFreeMemory* data);

/**
 * @brief Read key settings on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfDesfireKeySettings structure to be filled with key settings data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError
    mf_desfire_poller_read_key_settings(MfDesfirePoller* instance, MfDesfireKeySettings* data);

/**
 * @brief Read key version on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] key_num key number.
 * @param[in] data pointer to the MfDesfireKeyVersion structure to be filled with key version data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_key_version(
    MfDesfirePoller* instance,
    uint8_t key_num,
    MfDesfireKeyVersion* data);

/**
 * @brief Read key versions on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the SimpleArray structure to be filled with key versions data.
 * @param[in] count number of key versions to read.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_key_versions(
    MfDesfirePoller* instance,
    SimpleArray* data,
    uint32_t count);

/**
 * @brief Read applications IDs on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the SimpleArray structure to be filled with application ids data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError
    mf_desfire_poller_read_application_ids(MfDesfirePoller* instance, SimpleArray* data);

/**
 * @brief Select application on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] id pointer to the MfDesfireApplicationId structure with application id to select.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_select_application(
    MfDesfirePoller* instance,
    const MfDesfireApplicationId* id);

/**
 * @brief Read file IDs for selected application on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the SimpleArray structure to be filled with file ids data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_ids(MfDesfirePoller* instance, SimpleArray* data);

/**
 * @brief Read file settings on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] id file id to read settings for.
 * @param[out] data pointer to the MfDesfireFileSettings structure to be filled with file settings data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_settings(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileSettings* data);

/**
 * @brief Read multiple file settings on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] file_ids pointer to the SimpleArray structure array with file ids to read settings for.
 * @param[out] data pointer to the SimpleArray structure array to be filled with file settings data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_settings_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    SimpleArray* data);

/**
 * @brief Read file data on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] id file id to read data from.
 * @param[in] offset offset in bytes to start reading from.
 * @param[in] size number of bytes to read.
 * @param[out] data pointer to the MfDesfireFileData structure to be filled with file data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_data(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data);

/**
 * @brief Read file value on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] id file id to read value from.
 * @param[out] data pointer to the MfDesfireFileData structure to be filled with file value.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_value(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileData* data);

/**
 * @brief Read file records on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] id file id to read data from.
 * @param[in] offset offset in bytes to start reading from.
 * @param[in] size number of bytes to read.
 * @param[out] data pointer to the MfDesfireFileData structure to be filled with file records data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_records(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data);

/**
 * @brief Read data from multiple files on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] file_ids pointer to the SimpleArray structure array with files ids to read data from.
 * @param[in] file_settings pointer to the SimpleArray structure array with files settings to read data from.
 * @param[out] data pointer to the SimpleArray structure array to be filled with files data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_file_data_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    const SimpleArray* file_settings,
    SimpleArray* data);

/**
 * @brief Read application data for selected application on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the MfDesfireApplication structure to be filled with application data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError
    mf_desfire_poller_read_application(MfDesfirePoller* instance, MfDesfireApplication* data);

/**
 * @brief Read multiple applications data on MfDesfire card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] app_ids pointer to the SimpleArray structure array with application ids to read data from.
 * @param[out] data pointer to the SimpleArray structure array to be filled with applications data.
 * @return MfDesfireErrorNone on success, an error code on failure.
 */
MfDesfireError mf_desfire_poller_read_applications(
    MfDesfirePoller* instance,
    const SimpleArray* app_ids,
    SimpleArray* data);

#ifdef __cplusplus
}
#endif
