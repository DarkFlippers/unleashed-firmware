#pragma once

#include <nfc/protocols/felica/felica.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_felica_blocks_count(
    const FelicaData* data,
    FuriString* str,
    bool render_auth_notification);

void nfc_render_felica_info(
    const FelicaData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_more_info_render_felica_lite_dump(const FelicaData* data, FuriString* str);

void nfc_render_felica_idm(
    const FelicaData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_more_info_render_felica_dir(const FelicaData* data, FuriString* str);

void nfc_more_info_render_felica_blocks(
    const FelicaData* data,
    FuriString* str,
    const uint16_t service_code_key);
