#pragma once

#include <nfc/protocols/ntag4xx/ntag4xx.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_ntag4xx_info(
    const Ntag4xxData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_ntag4xx_data(const Ntag4xxData* data, FuriString* str);

void nfc_render_ntag4xx_version(const Ntag4xxVersion* data, FuriString* str);
