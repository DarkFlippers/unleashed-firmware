/**
 * @file nfc.h
 * @brief Transport layer Nfc library.
 *
 * The Nfc layer is responsible for setting the operating mode (poller or listener)
 * and technology (ISO14443-3A/B, ISO15693, ...), data exchange between higher
 * protocol-specific levels and underlying NFC hardware, as well as timings handling.
 *
 * In applications using the NFC protocol system there is no need to neiter explicitly
 * create an Nfc instance nor call any of its functions, as it is all handled
 * automatically under the hood.
 *
 * If the NFC protocol system is not suitable for the application's intended purpose
 * or there is need of having direct access to the NFC transport layer, then an Nfc
 * instance must be allocated and the below functions shall be used to implement
 * the required logic.
 */
#pragma once

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Nfc opaque type definition.
 */
typedef struct Nfc Nfc;

/**
 * @brief Enumeration of possible Nfc event types.
 *
 * Not all technologies implement all events (this is due to hardware limitations).
 */
typedef enum {
    NfcEventTypeUserAbort, /**< User code explicitly aborted the current operation. */
    NfcEventTypeFieldOn, /**< Reader's field was detected by the NFC hardware. */
    NfcEventTypeFieldOff, /**< Reader's field was lost. */
    NfcEventTypeTxStart, /**< Data transmission has started. */
    NfcEventTypeTxEnd, /**< Data transmission has ended. */
    NfcEventTypeRxStart, /**< Data reception has started. */
    NfcEventTypeRxEnd, /**< Data reception has ended. */

    NfcEventTypeListenerActivated, /**< The listener has been activated by the reader. */
    NfcEventTypePollerReady, /**< The card has been activated by the poller. */
} NfcEventType;

/**
 * @brief Nfc event data structure.
 */
typedef struct {
    BitBuffer* buffer; /**< Pointer to the received data buffer. */
} NfcEventData;

/**
 * @brief Nfc event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    NfcEventType type; /**< Type of the emitted event. */
    NfcEventData data; /**< Event-specific data. */
} NfcEvent;

/**
 * @brief Enumeration of possible Nfc commands.
 *
 * The event callback must return one of these to determine the next action.
 */
typedef enum {
    NfcCommandContinue, /**< Continue operation normally. */
    NfcCommandReset, /**< Reset the current state. */
    NfcCommandStop, /**< Stop the current operation. */
    NfcCommandSleep, /**< Switch Nfc hardware to low-power mode. */
} NfcCommand;

/**
 * @brief Nfc event callback type.
 *
 * A function of this type must be passed as the callback parameter upon start of a an Nfc instance.
 *
 * @param [in] event Nfc event, passed by value, complete with protocol type and data.
 * @param [in,out] context pointer to the user-specific context (set when starting an Nfc instance).
 * @returns command which the event producer must execute.
 */
typedef NfcCommand (*NfcEventCallback)(NfcEvent event, void* context);

/**
 * @brief Enumeration of possible operating modes.
 *
 * Not all technologies implement the listener operating mode.
 */
typedef enum {
    NfcModePoller, /**< Configure the Nfc instance as a poller. */
    NfcModeListener, /**< Configure the Nfc instance as a listener. */

    NfcModeNum, /**< Operating mode count. Internal use. */
} NfcMode;

/**
 * @brief Enumeration of available technologies.
 */
typedef enum {
    NfcTechIso14443a, /**< Configure the Nfc instance to use the ISO14443-3A technology. */
    NfcTechIso14443b, /**< Configure the Nfc instance to use the ISO14443-3B technology. */
    NfcTechIso15693, /**< Configure the Nfc instance to use the ISO15693 technology. */
    NfcTechFelica, /**< Configure the Nfc instance to use the FeliCa technology. */

    NfcTechNum, /**< Technologies count. Internal use. */
} NfcTech;

/**
 * @brief Enumeration of possible Nfc error codes.
 */
typedef enum {
    NfcErrorNone, /**< No error has occurred. */
    NfcErrorInternal, /**< An unknown error has occured on the lower level. */
    NfcErrorTimeout, /**< Operation is taking too long (e.g. card does not respond). */
    NfcErrorIncompleteFrame, /**< An incomplete data frame has been received. */
    NfcErrorDataFormat, /**< Data has not been parsed due to wrong/unknown format. */
} NfcError;

/**
 * @brief Allocate an Nfc instance.
 *
 * Will exclusively take over the NFC HAL until deleted.
 *
 * @returns pointer to the allocated Nfc instance.
 */
Nfc* nfc_alloc(void);

/**
 * @brief Delete an Nfc instance.
 *
 * Will release the NFC HAL lock, making it available for use by others.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void nfc_free(Nfc* instance);

/**
 * @brief Configure the Nfc instance to work in a particular mode.
 *
 * Not all technologies implement the listener operating mode.
 *
 * @param[in,out] instance pointer to the instance to be configured.
 * @param[in] mode required operating mode.
 * @param[in] tech required technology configuration.
 */
void nfc_config(Nfc* instance, NfcMode mode, NfcTech tech);

/**
 * @brief Set poller frame delay time.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] fdt_poll_fc frame delay time, in carrier cycles.
*/
void nfc_set_fdt_poll_fc(Nfc* instance, uint32_t fdt_poll_fc);

/**
 * @brief Set listener frame delay time.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] fdt_listen_fc frame delay time, in carrier cycles.
 */
void nfc_set_fdt_listen_fc(Nfc* instance, uint32_t fdt_listen_fc);

/**
 * @brief Set mask receive time.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] mask_rx_time_fc mask receive time, in carrier cycles.
 */
void nfc_set_mask_receive_time_fc(Nfc* instance, uint32_t mask_rx_time_fc);

/**
 * @brief Set frame delay time.
 *
 * Frame delay time is the minimum time between two consecutive poll frames.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] fdt_poll_poll_us frame delay time, in microseconds.
 */
void nfc_set_fdt_poll_poll_us(Nfc* instance, uint32_t fdt_poll_poll_us);

/**
 * @brief Set guard time.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] guard_time_us guard time, in microseconds.
 */
void nfc_set_guard_time_us(Nfc* instance, uint32_t guard_time_us);

/**
 * @brief Start the Nfc instance.
 *
 * The instance must be configured to work with a specific technology
 * in a specific operating mode with a nfc_config() call before starting.
 *
 * Once started, the user code will be receiving events through the provided
 * callback which must handle them according to the logic required.
 *
 * @param[in,out] instance pointer to the instance to be started.
 * @param[in] callback pointer to a user-defined callback function which will receive events.
 * @param[in] context pointer to a user-specific context (will be passed to the callback).
 */
void nfc_start(Nfc* instance, NfcEventCallback callback, void* context);

/**
 * @brief Stop Nfc instance.
 *
 * The instance can only be stopped if it is running.
 *
 * @param[in,out] instance pointer to the instance to be stopped.
 */
void nfc_stop(Nfc* instance);

/**
 * @brief Transmit and receive a data frame in poller mode.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * The data being transmitted and received may be either bit- or byte-oriented.
 * It shall not contain any technology-specific sequences as start or stop bits
 * and/or other special symbols, as this is handled on the underlying HAL level.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in,out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError
    nfc_poller_trx(Nfc* instance, const BitBuffer* tx_buffer, BitBuffer* rx_buffer, uint32_t fwt);

/**
 * @brief Transmit a data frame in listener mode.
 *
 * Used to transmit a response to the reader request in listener mode.
 *
 * The data being transmitted may be either bit- or byte-oriented.
 * It shall not contain any technology-specific sequences as start or stop bits
 * and/or other special symbols, as this is handled on the underlying HAL level.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in,out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_listener_tx(Nfc* instance, const BitBuffer* tx_buffer);

/*
 * Technology-specific functions.
 *
 * In a perfect world, this would not be necessary.
 * However, the current implementation employs NFC hardware that partially implements
 * certain protocols (e.g. ISO14443-3A), thus requiring methods to access such features.
 */

/******************* ISO14443-3A specific API *******************/

/**
 * @brief Enumeration of possible ISO14443-3A short frame types.
 */
typedef enum {
    NfcIso14443aShortFrameSensReq,
    NfcIso14443aShortFrameAllReqa,
} NfcIso14443aShortFrame;

/**
 * @brief Transmit an ISO14443-3A short frame and receive the response in poller mode.
 *
 * @param[in,out] instance pointer to the instance to be used in the transaction.
 * @param[in] frame type of short frame to be sent.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_iso14443a_poller_trx_short_frame(
    Nfc* instance,
    NfcIso14443aShortFrame frame,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Transmit an ISO14443-3A SDD frame and receive the response in poller mode.
 *
 * @param[in,out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_iso14443a_poller_trx_sdd_frame(
    Nfc* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Transmit an ISO14443-3A data frame with custom parity bits and receive the response in poller mode.
 *
 * Same as nfc_poller_trx(), but uses the parity bits provided by the user code
 * instead of calculating them automatically.
 *
 * @param[in,out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_iso14443a_poller_trx_custom_parity(
    Nfc* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Transmit an ISO14443-3A frame with custom parity bits in listener mode.
 *
 * Same as nfc_listener_tx(), but uses the parity bits provided by the user code
 * instead of calculating them automatically.
 *
 * @param[in,out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_iso14443a_listener_tx_custom_parity(Nfc* instance, const BitBuffer* tx_buffer);

/**
 * @brief Set ISO14443-3A collision resolution parameters in listener mode.
 *
 * Configures the NFC hardware for automatic collision resolution.
 *
 * @param[in,out] instance pointer to the instance to be configured.
 * @param[in] uid pointer to a byte array containing the UID.
 * @param[in] uid_len UID length in bytes (must be supported by the protocol).
 * @param[in] atqa ATQA byte value.
 * @param[in] sak SAK byte value.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_iso14443a_listener_set_col_res_data(
    Nfc* instance,
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t* atqa,
    uint8_t sak);

/**
 * @brief Set FeliCa collision resolution parameters in listener mode.
 * 
 * Configures the NFC hardware for automatic collision resolution.
 *
 * @param[in,out] instance pointer to the instance to be configured.
 * @param[in] idm pointer to a byte array containing the IDm.
 * @param[in] idm_len IDm length in bytes.
 * @param[in] pmm pointer to a byte array containing the PMm.
 * @param[in] pmm_len PMm length in bytes.
 * @param[in] sys_code System code from SYS_C block
 * @returns NfcErrorNone on success, any other error code on failure.
*/
NfcError nfc_felica_listener_set_sensf_res_data(
    Nfc* instance,
    const uint8_t* idm,
    const uint8_t idm_len,
    const uint8_t* pmm,
    const uint8_t pmm_len,
    const uint16_t sys_code);

/**
 * @brief Send ISO15693 Start of Frame pattern in listener mode
 *
 * @param[in,out] instance pointer to the instance to be configured.
 * @returns NfcErrorNone on success, any other error code on failure.
 */
NfcError nfc_iso15693_listener_tx_sof(Nfc* instance);

#ifdef __cplusplus
}
#endif
