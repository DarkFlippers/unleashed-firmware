#pragma once

#include <nfc/protocols/mf_desfire/mf_desfire.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_mf_desfire_info(
    const MfDesfireData* data,
    NfcProtocolFormatType format_type,
    FuriString* str);

void nfc_render_mf_desfire_data(const MfDesfireData* data, FuriString* str);

void nfc_render_mf_desfire_version(const MfDesfireVersion* data, FuriString* str);

void nfc_render_mf_desfire_free_memory(const MfDesfireFreeMemory* data, FuriString* str);

void nfc_render_mf_desfire_key_settings(const MfDesfireKeySettings* data, FuriString* str);

void nfc_render_mf_desfire_key_version(
    const MfDesfireKeyVersion* data,
    uint32_t index,
    FuriString* str);

void nfc_render_mf_desfire_application_id(const MfDesfireApplicationId* data, FuriString* str);

void nfc_render_mf_desfire_application(const MfDesfireApplication* data, FuriString* str);

void nfc_render_mf_desfire_file_id(const MfDesfireFileId* data, FuriString* str);

void nfc_render_mf_desfire_file_settings_data(
    const MfDesfireFileSettings* settings,
    const MfDesfireFileData* data,
    FuriString* str);
