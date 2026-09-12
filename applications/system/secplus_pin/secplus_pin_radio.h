#pragma once

#include <furi.h>
#include <lib/subghz/subghz_tx_rx_worker.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/environment.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <flipper_format/flipper_format.h>

typedef struct {
    uint32_t frequency;
    FuriString* preset_name;
    uint8_t* custom_preset;
    size_t custom_preset_size;
} SecPlusPinPreset;

/** Map a "FuriHalSubGhzPreset..." string to its enum.
 * @return false if the name is not a built-in preset, in which case *out is
 *         FuriHalSubGhzPresetCustom and Custom_preset_data must be supplied.
 */
bool secplus_pin_preset_by_name(const char* name, FuriHalSubGhzPreset* out);

typedef enum {
    SecPlusPinTxOk,
    /** the region does not allow transmitting on this frequency */
    SecPlusPinTxNotAllowed,
    /** the radio could not be claimed, or the signal could not be built */
    SecPlusPinTxError,
} SecPlusPinTxStatus;

/** Transmit one signal described by `fff` (an in-memory flipper format). */
SecPlusPinTxStatus secplus_pin_radio_tx(const SecPlusPinPreset* preset, FlipperFormat* fff);
