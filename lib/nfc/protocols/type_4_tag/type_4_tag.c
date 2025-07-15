#include "type_4_tag_i.h"

#define TYPE_4_TAG_PROTOCOL_NAME "Type 4 Tag"

const NfcDeviceBase nfc_device_type_4_tag = {
    .protocol_name = TYPE_4_TAG_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)type_4_tag_alloc,
    .free = (NfcDeviceFree)type_4_tag_free,
    .reset = (NfcDeviceReset)type_4_tag_reset,
    .copy = (NfcDeviceCopy)type_4_tag_copy,
    .verify = (NfcDeviceVerify)type_4_tag_verify,
    .load = (NfcDeviceLoad)type_4_tag_load,
    .save = (NfcDeviceSave)type_4_tag_save,
    .is_equal = (NfcDeviceEqual)type_4_tag_is_equal,
    .get_name = (NfcDeviceGetName)type_4_tag_get_device_name,
    .get_uid = (NfcDeviceGetUid)type_4_tag_get_uid,
    .set_uid = (NfcDeviceSetUid)type_4_tag_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)type_4_tag_get_base_data,
};

Type4TagData* type_4_tag_alloc(void) {
    Type4TagData* data = malloc(sizeof(Type4TagData));
    data->iso14443_4a_data = iso14443_4a_alloc();
    data->device_name = furi_string_alloc();
    data->platform_name = furi_string_alloc();
    data->ndef_data = simple_array_alloc(&simple_array_config_uint8_t);
    return data;
}

void type_4_tag_free(Type4TagData* data) {
    furi_check(data);

    type_4_tag_reset(data);
    simple_array_free(data->ndef_data);
    furi_string_free(data->platform_name);
    furi_string_free(data->device_name);
    iso14443_4a_free(data->iso14443_4a_data);
    free(data);
}

void type_4_tag_reset(Type4TagData* data) {
    furi_check(data);

    iso14443_4a_reset(data->iso14443_4a_data);

    data->is_tag_specific = false;
    furi_string_reset(data->device_name);
    furi_string_reset(data->platform_name);
    data->t4t_version.value = 0;
    data->chunk_max_read = 0;
    data->chunk_max_write = 0;
    data->ndef_file_id = 0;
    data->ndef_max_len = 0;
    data->ndef_read_lock = 0;
    data->ndef_write_lock = 0;

    simple_array_reset(data->ndef_data);
}

void type_4_tag_copy(Type4TagData* data, const Type4TagData* other) {
    furi_check(data);
    furi_check(other);

    type_4_tag_reset(data);

    iso14443_4a_copy(data->iso14443_4a_data, other->iso14443_4a_data);

    data->is_tag_specific = other->is_tag_specific;
    furi_string_set(data->device_name, other->device_name);
    furi_string_set(data->platform_name, other->platform_name);
    data->t4t_version.value = other->t4t_version.value;
    data->chunk_max_read = other->chunk_max_read;
    data->chunk_max_write = other->chunk_max_write;
    data->ndef_file_id = other->ndef_file_id;
    data->ndef_max_len = other->ndef_max_len;
    data->ndef_read_lock = other->ndef_read_lock;
    data->ndef_write_lock = other->ndef_write_lock;

    simple_array_copy(data->ndef_data, other->ndef_data);
}

bool type_4_tag_verify(Type4TagData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    return false;
}

bool type_4_tag_load(Type4TagData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    FuriString* prefix = furi_string_alloc();

    bool success = false;

    do {
        if(!iso14443_4a_load(data->iso14443_4a_data, ff, version)) break;

        if(!type_4_tag_ndef_data_load(data, ff)) break;

        success = true;
    } while(false);

    furi_string_free(prefix);
    return success;
}

bool type_4_tag_save(const Type4TagData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    FuriString* prefix = furi_string_alloc();

    bool success = false;

    do {
        if(!iso14443_4a_save(data->iso14443_4a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, TYPE_4_TAG_PROTOCOL_NAME " specific data"))
            break;
        if(!type_4_tag_ndef_data_save(data, ff)) break;

        success = true;
    } while(false);

    furi_string_free(prefix);
    return success;
}

bool type_4_tag_is_equal(const Type4TagData* data, const Type4TagData* other) {
    furi_check(data);
    furi_check(other);

    return iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data) &&
           data->is_tag_specific == other->is_tag_specific &&
           data->t4t_version.value == other->t4t_version.value &&
           data->chunk_max_read == other->chunk_max_read &&
           data->chunk_max_write == other->chunk_max_write &&
           data->ndef_file_id == other->ndef_file_id &&
           data->ndef_max_len == other->ndef_max_len &&
           data->ndef_read_lock == other->ndef_read_lock &&
           data->ndef_write_lock == other->ndef_write_lock &&
           simple_array_is_equal(data->ndef_data, other->ndef_data);
}

const char* type_4_tag_get_device_name(const Type4TagData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);
    return TYPE_4_TAG_PROTOCOL_NAME;
}

const uint8_t* type_4_tag_get_uid(const Type4TagData* data, size_t* uid_len) {
    furi_check(data);
    furi_check(uid_len);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool type_4_tag_set_uid(Type4TagData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}

Iso14443_4aData* type_4_tag_get_base_data(const Type4TagData* data) {
    furi_check(data);

    return data->iso14443_4a_data;
}
