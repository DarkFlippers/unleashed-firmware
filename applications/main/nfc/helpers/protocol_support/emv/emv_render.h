#pragma once

#include <nfc/protocols/emv/emv.h>

#include "../nfc_protocol_support_render_common.h"
#include <stdint.h>

void nfc_render_emv_info(const EmvData* data, NfcProtocolFormatType format_type, FuriString* str);

void nfc_render_emv_data(const EmvData* data, FuriString* str);

void nfc_render_emv_pan(const uint8_t* data, const uint8_t len, FuriString* str);

void nfc_render_emv_name(const char* data, FuriString* str);

void nfc_render_emv_application(const EmvApplication* data, FuriString* str);

void nfc_render_emv_application_interchange_profile(const EmvApplication* apl, FuriString* str);

void nfc_render_emv_extra(const EmvData* data, FuriString* str);

void nfc_render_emv_country(uint16_t country_code, FuriString* str);

void nfc_render_emv_currency(uint16_t cur_code, FuriString* str);

void nfc_render_emv_transactions(const EmvApplication* data, FuriString* str);

void nfc_render_emv_uid(const uint8_t* uid, const uint8_t uid_len, FuriString* str);

void nfc_render_emv_header(FuriString* str);
