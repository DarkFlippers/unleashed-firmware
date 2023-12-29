#pragma once

#include "iso15693_3.h"

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Iso15693_3Poller opaque type definition.
 */
typedef struct Iso15693_3Poller Iso15693_3Poller;

/**
 * @brief Enumeration of possible Iso15693_3 poller event types.
 */
typedef enum {
    Iso15693_3PollerEventTypeError, /**< An error occured during activation procedure. */
    Iso15693_3PollerEventTypeReady, /**< The card was activated by the poller. */
} Iso15693_3PollerEventType;

/**
 * @brief Iso15693_3 poller event data.
 */
typedef union {
    Iso15693_3Error error; /**< Error code indicating card activation fail reason. */
} Iso15693_3PollerEventData;

/**
 * @brief Iso15693_3 poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Iso15693_3PollerEventType type; /**< Type of emmitted event. */
    Iso15693_3PollerEventData* data; /**< Pointer to event specific data. */
} Iso15693_3PollerEvent;

/**
 * @brief Transmit and receive Iso15693_3 frames in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error iso15693_3_poller_send_frame(
    Iso15693_3Poller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Perform activation procedure.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perfoms the activation procedure as defined in Iso15693_3. The data
 * field will be filled with Iso15693_3Data data on success.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the Iso15693_3 data structure to be filled.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error iso15693_3_poller_activate(Iso15693_3Poller* instance, Iso15693_3Data* data);

/**
 * @brief Send invertory command and parse response.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] uid pointer to the buffer to be filled with the UID.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error iso15693_3_poller_inventory(Iso15693_3Poller* instance, uint8_t* uid);

/**
 * @brief Send get system info command and parse response.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the Iso15693_3SystemInfo structure to be filled.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error
    iso15693_3_poller_get_system_info(Iso15693_3Poller* instance, Iso15693_3SystemInfo* data);

/**
 * @brief Read Iso15693_3 block.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the buffer to be filled with the block data.
 * @param[in] block_number block number to be read.
 * @param[in] block_size size of the block to be read.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error iso15693_3_poller_read_block(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint8_t block_number,
    uint8_t block_size);

/**
 * @brief Read multiple Iso15693_3 blocks.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the buffer to be filled with the block data.
 * @param[in] block_count number of blocks to be read.
 * @param[in] block_size size of the blocks to be read.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error iso15693_3_poller_read_blocks(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint16_t block_count,
    uint8_t block_size);

/**
 * @brief Get Iso15693_3 block security status.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the buffer to be filled with the block security status.
 * @param[in] block_count block security number to be read.
 * @return Iso15693_3ErrorNone on success, an error code on failure.
 */
Iso15693_3Error iso15693_3_poller_get_blocks_security(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint16_t block_count);

#ifdef __cplusplus
}
#endif
