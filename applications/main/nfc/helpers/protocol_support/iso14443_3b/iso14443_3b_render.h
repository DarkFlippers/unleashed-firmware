#pragma once

#include <nfc/protocols/iso14443_3b/iso14443_3b.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_iso14443_3b_info(
    const Iso14443_3bData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);
