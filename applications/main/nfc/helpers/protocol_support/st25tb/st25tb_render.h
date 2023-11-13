#pragma once

#include <nfc/protocols/st25tb/st25tb.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_st25tb_info(
    const St25tbData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);
