#pragma once

#include <nfc/protocols/iso14443_4a/iso14443_4a.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_iso14443_4a_info(
    const Iso14443_4aData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_iso14443_4a_brief(const Iso14443_4aData* data, FuriString* str);

void nfc_render_iso14443_4a_extra(const Iso14443_4aData* data, FuriString* str);
