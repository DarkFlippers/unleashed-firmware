#pragma once

#include <nfc/protocols/felica/felica.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_felica_info(
    const FelicaData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);
