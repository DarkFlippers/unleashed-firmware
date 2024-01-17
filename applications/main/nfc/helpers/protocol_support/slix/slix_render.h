#pragma once

#include <nfc/protocols/slix/slix.h>

#include "../nfc_protocol_support_render_common.h"
#include "../iso15693_3/iso15693_3_render.h"

void nfc_render_slix_info(const SlixData* data, NfcProtocolFormatType format_type, FuriString* str);
