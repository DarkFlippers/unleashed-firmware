/**
 * @file nfc_protocol_support_render_common.h
 * @brief Common formatting-related defines.
 */
#pragma once

/**
 * @brief Displayed information verbosity level.
 */
typedef enum {
    NfcProtocolFormatTypeShort, /**< Short format, terse info. */
    NfcProtocolFormatTypeFull, /**< Full format, verbose info. */
} NfcProtocolFormatType;
