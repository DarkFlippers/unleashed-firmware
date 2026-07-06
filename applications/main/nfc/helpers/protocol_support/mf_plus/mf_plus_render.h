#pragma once

#include <nfc/protocols/mf_plus/mf_plus.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_mf_plus_info(
    const MfPlusData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

// Recovered SL3 content for the first "More info" page: a MIFARE-Classic-style block dump (the
// sector trailer shows the recovered Key A + access bits + Key B), plus admin keys, config and
// signature.
void nfc_render_mf_plus_dump(const MfPlusData* data, FuriString* str);

// Hardware/protocol details for the second "More" page: GetVersion fields + ISO14443-4 data.
void nfc_render_mf_plus_iso4(const MfPlusData* data, FuriString* str);

void nfc_render_mf_plus_version(const MfPlusVersion* data, FuriString* str);
