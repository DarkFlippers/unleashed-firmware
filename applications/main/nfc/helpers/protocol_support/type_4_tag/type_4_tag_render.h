#pragma once

#include <nfc/protocols/type_4_tag/type_4_tag.h>

#include "../nfc_protocol_support_render_common.h"

#define TYPE_4_TAG_RENDER_BYTES_PER_LINE (4U)

void nfc_render_type_4_tag_info(
    const Type4TagData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_type_4_tag_dump(const Type4TagData* data, FuriString* str);
