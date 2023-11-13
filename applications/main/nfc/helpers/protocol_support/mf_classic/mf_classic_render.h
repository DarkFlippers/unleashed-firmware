#pragma once

#include <nfc/protocols/mf_classic/mf_classic.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_mf_classic_info(
    const MfClassicData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_mf_classic_dump(const MfClassicData* data, FuriString* str);
