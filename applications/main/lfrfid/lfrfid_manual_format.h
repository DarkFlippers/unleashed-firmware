/** @file lfrfid_manual_format.h
 *
 * Add Manually by the numbers printed on a card (facility code, card number, ...)
 * rather than by raw hex.
 *
 * The entries on offer are every protocol of the dictionary, the HID Proximity formats
 * of lfrfid_hid_format.h (saved as Generic HIDProx) and the Casi-Rusco badge of
 * lfrfid_casi_format.h (saved as EM4100 at RF/32). One "manual format" id covers them all: the
 * protocol id itself, LFRFID_MANUAL_FORMAT_HID plus the HID format index, or
 * LFRFID_MANUAL_FORMAT_CASI.
 */

#pragma once

#include <furi.h>
#include <toolbox/protocols/protocol_dict.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include "lfrfid_hid_format.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The most fields any format is entered by */
#define LFRFID_MANUAL_FORMAT_FIELDS_MAX (4)

/** First manual format id of the HID Proximity formats */
#define LFRFID_MANUAL_FORMAT_HID  (LFRFIDProtocolMax)
/** Manual format id of the Casi-Rusco C10106 badge */
#define LFRFID_MANUAL_FORMAT_CASI (LFRFID_MANUAL_FORMAT_HID + LFRFID_HID_FORMAT_COUNT)

/** One number a card is entered by. */
typedef struct {
    const char* name; /**< Shown to the user, e.g. "Facility Code" */
    uint64_t min; /**< Smallest value the field can hold */
    uint64_t max; /**< Largest value the field can hold */
} LfRfidManualFormatField;

/** The protocol a manual format's data is saved as, PROTOCOL_NO for an unknown id. */
ProtocolId lfrfid_manual_format_protocol(uint32_t format);

/** The name a manual format is listed under: the protocol's manufacturer and name, or
 * "HID H10304", "Casi-Rusco C10106" and so on for the formats saved as another protocol. */
void lfrfid_manual_format_get_label(uint32_t format, FuriString* label);

/** How many fields a manual format is entered by, 0 when it has no such layout and takes
 * raw data only. */
size_t lfrfid_manual_format_fields_count(uint32_t format);

/** Describe one field of a manual format.
 *
 * @return false when the index is out of range
 */
bool lfrfid_manual_format_field(uint32_t format, size_t index, LfRfidManualFormatField* field);

/** Build protocol data from the field values, one per field in lfrfid_manual_format_field()
 * order.
 *
 * @return false when the format has no fields, a value is outside its field's range, or
 *         the buffer is smaller than the protocol's data
 */
bool lfrfid_manual_format_encode(
    uint32_t format,
    const uint64_t* values,
    size_t values_count,
    uint8_t* data,
    size_t data_size);

/** Append what the app reads out of a protocol's data to the firmware's text: the HID
 * Proximity format lines for Generic HIDProx, the Casi-Rusco badge line for EM4100. */
void lfrfid_manual_format_render(ProtocolId protocol_id, const uint8_t* data, FuriString* result);

#ifdef __cplusplus
}
#endif
