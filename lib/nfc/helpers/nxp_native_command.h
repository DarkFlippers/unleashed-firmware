#pragma once

#include "nxp_native_command_mode.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller.h>

// ISO 7816 command wrapping
#define NXP_NATIVE_COMMAND_ISO_CLA (0x90)
#define NXP_NATIVE_COMMAND_ISO_P1  (0x00)
#define NXP_NATIVE_COMMAND_ISO_P2  (0x00)
#define NXP_NATIVE_COMMAND_ISO_LE  (0x00)
// ISO 7816 status wrapping
#define NXP_NATIVE_COMMAND_ISO_SW1 (0x91)

// Successful operation
#define NXP_NATIVE_COMMAND_STATUS_OPERATION_OK                (0x00)
// No changes done to backup files, CommitTransaction / AbortTransaction not necessary
#define NXP_NATIVE_COMMAND_STATUS_NO_CHANGES                  (0x0C)
// Insufficient NV-Memory to complete command
#define NXP_NATIVE_COMMAND_STATUS_OUT_OF_EEPROM_ERROR         (0x0E)
// Command code not supported
#define NXP_NATIVE_COMMAND_STATUS_ILLEGAL_COMMAND_CODE        (0x1C)
// CRC or MAC does not match data Padding bytes not valid
#define NXP_NATIVE_COMMAND_STATUS_INTEGRITY_ERROR             (0x1E)
// Invalid key number specified
#define NXP_NATIVE_COMMAND_STATUS_NO_SUCH_KEY                 (0x40)
// Length of command string invalid
#define NXP_NATIVE_COMMAND_STATUS_LENGTH_ERROR                (0x7E)
// Current configuration / status does not allow the requested command
#define NXP_NATIVE_COMMAND_STATUS_PERMISSION_DENIED           (0x9D)
// Value of the parameter(s) invalid
#define NXP_NATIVE_COMMAND_STATUS_PARAMETER_ERROR             (0x9E)
// Requested AID not present on PICC
#define NXP_NATIVE_COMMAND_STATUS_APPLICATION_NOT_FOUND       (0xA0)
// Unrecoverable error within application, application will be disabled
#define NXP_NATIVE_COMMAND_STATUS_APPL_INTEGRITY_ERROR        (0xA1)
// Currently not allowed to authenticate. Keep trying until full delay is spent
#define NXP_NATIVE_COMMAND_STATUS_STATUS_AUTHENTICATION_DELAY (0xAD)
// Current authentication status does not allow the requested command
#define NXP_NATIVE_COMMAND_STATUS_AUTHENTICATION_ERROR        (0xAE)
// Additional data frame is expected to be sent
#define NXP_NATIVE_COMMAND_STATUS_ADDITIONAL_FRAME            (0xAF)
// Attempt to read/write data from/to beyond the file's/record's limits
// Attempt to exceed the limits of a value file.
#define NXP_NATIVE_COMMAND_STATUS_BOUNDARY_ERROR              (0xBE)
// Unrecoverable error within PICC, PICC will be disabled
#define NXP_NATIVE_COMMAND_STATUS_PICC_INTEGRITY_ERROR        (0xC1)
// Previous Command was not fully completed. Not all Frames were requested or provided by the PCD
#define NXP_NATIVE_COMMAND_STATUS_COMMAND_ABORTED             (0xCA)
// PICC was disabled by an unrecoverable error
#define NXP_NATIVE_COMMAND_STATUS_PICC_DISABLED_ERROR         (0xCD)
// Number of Applications limited to 28, no additional CreateApplication possible
#define NXP_NATIVE_COMMAND_STATUS_COUNT_ERROR                 (0xCE)
// Creation of file/application failed because file/application with same number already exists
#define NXP_NATIVE_COMMAND_STATUS_DUBLICATE_ERROR             (0xDE)
// Could not complete NV-write operation due to loss of power, internal backup/rollback mechanism activated
#define NXP_NATIVE_COMMAND_STATUS_EEPROM_ERROR                (0xEE)
// Specified file number does not exist
#define NXP_NATIVE_COMMAND_STATUS_FILE_NOT_FOUND              (0xF0)
// Unrecoverable error within file, file will be disabled
#define NXP_NATIVE_COMMAND_STATUS_FILE_INTEGRITY_ERROR        (0xF1)

typedef uint8_t NxpNativeCommandStatus;

/**
 * @brief Transmit and receive NXP Native Command chunks in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The result_buffer will be filled with any data received as a response to data
 * sent from input_buffer, with a timeout defined by the fwt parameter.
 *
 * The tx_buffer and rx_buffer are used as working areas to handle individual
 * command chunks and responses.
 *
 * @param[in, out] iso14443_4a_poller pointer to the instance to be used in the transaction.
 * @param[out] status_code pointer to a status variable to hold the result of the operation.
 * @param[in] input_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] result_buffer pointer to the buffer to be filled with received data.
 * @param[in] command_mode what command formatting mode to use for the transaction.
 * @param[in, out] tx_buffer pointer to the buffer for command construction.
 * @param[in, out] rx_buffer pointer to the buffer for response handling.
 * @return Iso14443_4aErrorNone and STATUS_OPERATION_OK on success, an error code on failure.
 */
Iso14443_4aError nxp_native_command_iso14443_4a_poller(
    Iso14443_4aPoller* iso14443_4a_poller,
    NxpNativeCommandStatus* status_code,
    const BitBuffer* input_buffer,
    BitBuffer* result_buffer,
    NxpNativeCommandMode command_mode,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer);
