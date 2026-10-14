#pragma once

#include <nfc/protocols/texkom/texkom.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_texkom_info(
    const TexkomData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);
