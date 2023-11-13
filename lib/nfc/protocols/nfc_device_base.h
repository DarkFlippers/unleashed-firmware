/**
 * @file nfc_device_base.h
 * @brief Common top-level types for the NFC protocol stack.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verbosity level of the displayed NFC device name.
 */
typedef enum {
    NfcDeviceNameTypeFull, /**< Display full(verbose) name. */
    NfcDeviceNameTypeShort, /**< Display shortened name. */
} NfcDeviceNameType;

/**
 * @brief Generic opaque type for protocol-specific NFC device data.
 */
typedef void NfcDeviceData;

#ifdef __cplusplus
}
#endif
