#pragma once

#include <nfc/protocols/iso14443_3a/iso14443_3a.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_iso14443_3a_info(
    const Iso14443_3aData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_iso14443_tech_type(const Iso14443_3aData* data, FuriString* str);

void nfc_render_iso14443_3a_format_bytes(FuriString* str, const uint8_t* data, size_t size);

void nfc_render_iso14443_3a_brief(const Iso14443_3aData* data, FuriString* str);

void nfc_render_iso14443_3a_extra(const Iso14443_3aData* data, FuriString* str);
