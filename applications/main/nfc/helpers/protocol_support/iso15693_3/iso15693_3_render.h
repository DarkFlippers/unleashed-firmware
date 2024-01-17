#pragma once

#include <nfc/protocols/iso15693_3/iso15693_3.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_iso15693_3_info(
    const Iso15693_3Data* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_iso15693_3_brief(const Iso15693_3Data* data, FuriString* str);

void nfc_render_iso15693_3_extra(const Iso15693_3Data* data, FuriString* str);

void nfc_render_iso15693_3_system_info(const Iso15693_3Data* data, FuriString* str);
