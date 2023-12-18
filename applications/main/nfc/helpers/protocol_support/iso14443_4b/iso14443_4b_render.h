#pragma once

#include <nfc/protocols/iso14443_4b/iso14443_4b.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_iso14443_4b_info(
    const Iso14443_4bData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);
