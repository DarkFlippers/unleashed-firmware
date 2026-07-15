#pragma once

#include <nfc/protocols/mf_plus/mf_plus.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_mf_plus_info(
    const MfPlusData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

// Recovered SL3 content for the "View Dump" page: a MIFARE-Classic-style block dump (the sector
// trailer shows the recovered Key A + access bits + Key B), plus admin keys, config and signature.
void nfc_render_mf_plus_dump(const MfPlusData* data, FuriString* str);

// ISO14443-4 protocol details ("ISO14443-4 Data" page).
void nfc_render_mf_plus_iso14443_4(const MfPlusData* data, FuriString* str);

// GetVersion page (the "More" page reached from "ISO14443-4 Data"): the GetVersion fields, or a note
// when the card doesn't answer GetVersion.
void nfc_render_mf_plus_version_info(const MfPlusData* data, FuriString* str);

void nfc_render_mf_plus_version(const MfPlusVersion* data, FuriString* str);
