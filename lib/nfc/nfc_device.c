#include "nfc_device_i.h"

#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#include "nfc_common.h"
#include "protocols/nfc_device_defs.h"

#define NFC_FILE_HEADER    "Flipper NFC device"
#define NFC_DEV_TYPE_ERROR "Protocol type mismatch"

#define NFC_DEVICE_UID_KEY  "UID"
#define NFC_DEVICE_TYPE_KEY "Device type"

#define NFC_DEVICE_UID_MAX_LEN (10U)

NfcDevice* nfc_device_alloc(void) {
    NfcDevice* instance = malloc(sizeof(NfcDevice));
    instance->protocol = NfcProtocolInvalid;

    return instance;
}

void nfc_device_free(NfcDevice* instance) {
    furi_check(instance);

    nfc_device_clear(instance);
    free(instance);
}

void nfc_device_clear(NfcDevice* instance) {
    furi_check(instance);

    if(instance->protocol == NfcProtocolInvalid) {
        furi_assert(instance->protocol_data == NULL);
    } else if(instance->protocol < NfcProtocolNum) {
        if(instance->protocol_data) {
            nfc_devices[instance->protocol]->free(instance->protocol_data);
            instance->protocol_data = NULL;
        }
        instance->protocol = NfcProtocolInvalid;
    }
}

void nfc_device_reset(NfcDevice* instance) {
    furi_check(instance);
    furi_check(instance->protocol < NfcProtocolNum);

    if(instance->protocol_data) {
        nfc_devices[instance->protocol]->reset(instance->protocol_data);
    }
}

NfcProtocol nfc_device_get_protocol(const NfcDevice* instance) {
    furi_check(instance);
    return instance->protocol;
}

const NfcDeviceData* nfc_device_get_data(const NfcDevice* instance, NfcProtocol protocol) {
    furi_check(instance);
    return nfc_device_get_data_ptr(instance, protocol);
}

const char* nfc_device_get_protocol_name(NfcProtocol protocol) {
    furi_check(protocol < NfcProtocolNum);

    return nfc_devices[protocol]->protocol_name;
}

const char* nfc_device_get_name(const NfcDevice* instance, NfcDeviceNameType name_type) {
    furi_check(instance);
    furi_check(instance->protocol < NfcProtocolNum);

    return nfc_devices[instance->protocol]->get_name(instance->protocol_data, name_type);
}

const uint8_t* nfc_device_get_uid(const NfcDevice* instance, size_t* uid_len) {
    furi_check(instance);
    furi_check(uid_len);
    furi_check(instance->protocol < NfcProtocolNum);

    return nfc_devices[instance->protocol]->get_uid(instance->protocol_data, uid_len);
}

bool nfc_device_set_uid(NfcDevice* instance, const uint8_t* uid, size_t uid_len) {
    furi_check(instance);
    furi_check(uid);
    furi_check(instance->protocol < NfcProtocolNum);

    return nfc_devices[instance->protocol]->set_uid(instance->protocol_data, uid, uid_len);
}

void nfc_device_set_data(
    NfcDevice* instance,
    NfcProtocol protocol,
    const NfcDeviceData* protocol_data) {
    furi_check(instance);
    furi_check(protocol_data);
    furi_check(protocol < NfcProtocolNum);

    nfc_device_clear(instance);

    instance->protocol = protocol;
    instance->protocol_data = nfc_devices[protocol]->alloc();

    nfc_devices[protocol]->copy(instance->protocol_data, protocol_data);
}

void nfc_device_copy_data(
    const NfcDevice* instance,
    NfcProtocol protocol,
    NfcDeviceData* protocol_data) {
    furi_check(instance);
    furi_check(protocol < NfcProtocolNum);
    furi_check(protocol_data);

    if(instance->protocol != protocol) {
        furi_crash(NFC_DEV_TYPE_ERROR);
    }

    nfc_devices[protocol]->copy(protocol_data, instance->protocol_data);
}

bool nfc_device_is_equal_data(
    const NfcDevice* instance,
    NfcProtocol protocol,
    const NfcDeviceData* protocol_data) {
    furi_check(instance);
    furi_check(protocol < NfcProtocolNum);
    furi_check(protocol_data);

    return instance->protocol == protocol &&
           nfc_devices[protocol]->is_equal(instance->protocol_data, protocol_data);
}

bool nfc_device_is_equal(const NfcDevice* instance, const NfcDevice* other) {
    furi_check(instance);
    furi_check(other);

    return nfc_device_is_equal_data(instance, other->protocol, other->protocol_data);
}

void nfc_device_set_loading_callback(
    NfcDevice* instance,
    NfcLoadingCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);

    instance->loading_callback = callback;
    instance->loading_callback_context = context;
}

bool nfc_device_save(NfcDevice* instance, const char* path) {
    furi_check(instance);
    furi_check(instance->protocol < NfcProtocolNum);
    furi_check(path);

    bool saved = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);
    FuriString* temp_str = furi_string_alloc();

    if(instance->loading_callback) {
        instance->loading_callback(instance->loading_callback_context, true);
    }

    do {
        // Open file
        if(!flipper_format_buffered_file_open_always(ff, path)) break;

        // Write header
        if(!flipper_format_write_header_cstr(ff, NFC_FILE_HEADER, NFC_CURRENT_FORMAT_VERSION))
            break;

        // Write allowed device types
        furi_string_printf(temp_str, "%s can be ", NFC_DEVICE_TYPE_KEY);
        for(NfcProtocol protocol = 0; protocol < NfcProtocolNum; ++protocol) {
            furi_string_cat(temp_str, nfc_devices[protocol]->protocol_name);
            if(protocol < NfcProtocolNum - 1) {
                furi_string_cat(temp_str, ", ");
            }
        }

        if(!flipper_format_write_comment(ff, temp_str)) break;

        // Write device type
        if(!flipper_format_write_string_cstr(
               ff, NFC_DEVICE_TYPE_KEY, nfc_devices[instance->protocol]->protocol_name))
            break;

        // Write UID
        furi_string_printf(temp_str, "%s is common for all formats", NFC_DEVICE_UID_KEY);
        if(!flipper_format_write_comment(ff, temp_str)) break;

        size_t uid_len;
        const uint8_t* uid = nfc_device_get_uid(instance, &uid_len);
        if(!flipper_format_write_hex(ff, NFC_DEVICE_UID_KEY, uid, uid_len)) break;

        // Write protocol-dependent data
        if(!nfc_devices[instance->protocol]->save(instance->protocol_data, ff)) break;

        saved = true;
    } while(false);

    if(instance->loading_callback) {
        instance->loading_callback(instance->loading_callback_context, false);
    }

    furi_string_free(temp_str);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    return saved;
}

static bool nfc_device_load_uid(
    FlipperFormat* ff,
    uint8_t* uid,
    uint32_t* uid_len,
    const uint32_t uid_maxlen) {
    bool loaded = false;

    do {
        uint32_t uid_len_current;
        if(!flipper_format_get_value_count(ff, NFC_DEVICE_UID_KEY, &uid_len_current)) break;
        if(uid_len_current > uid_maxlen) break;
        if(!flipper_format_read_hex(ff, NFC_DEVICE_UID_KEY, uid, uid_len_current)) break;

        *uid_len = uid_len_current;
        loaded = true;
    } while(false);

    return loaded;
}

static bool nfc_device_load_unified(NfcDevice* instance, FlipperFormat* ff, uint32_t version) {
    bool loaded = false;

    FuriString* temp_str = furi_string_alloc();

    do {
        // Read Nfc device type
        if(!flipper_format_read_string(ff, NFC_DEVICE_TYPE_KEY, temp_str)) break;

        // Detect protocol
        NfcProtocol protocol;
        for(protocol = 0; protocol < NfcProtocolNum; ++protocol) {
            if(furi_string_equal(temp_str, nfc_devices[protocol]->protocol_name)) {
                break;
            }
        }

        if(protocol == NfcProtocolNum) break;

        nfc_device_clear(instance);

        instance->protocol = protocol;
        instance->protocol_data = nfc_devices[protocol]->alloc();

        // Load UID
        uint8_t uid[NFC_DEVICE_UID_MAX_LEN];
        uint32_t uid_len;

        if(!nfc_device_load_uid(ff, uid, &uid_len, NFC_DEVICE_UID_MAX_LEN)) break;
        if(!nfc_device_set_uid(instance, uid, uid_len)) break;

        // Load data
        if(!nfc_devices[protocol]->load(instance->protocol_data, ff, version)) break;

        loaded = true;
    } while(false);

    if(!loaded) {
        nfc_device_clear(instance);
    }

    furi_string_free(temp_str);
    return loaded;
}

static bool nfc_device_load_legacy(NfcDevice* instance, FlipperFormat* ff, uint32_t version) {
    bool loaded = false;

    FuriString* temp_str = furi_string_alloc();

    do {
        // Read Nfc device type
        if(!flipper_format_read_string(ff, NFC_DEVICE_TYPE_KEY, temp_str)) break;

        nfc_device_clear(instance);

        // Detect protocol
        for(NfcProtocol protocol = 0; protocol < NfcProtocolNum; protocol++) {
            instance->protocol = protocol;
            instance->protocol_data = nfc_devices[protocol]->alloc();

            // Verify protocol
            if(nfc_devices[protocol]->verify(instance->protocol_data, temp_str)) {
                uint8_t uid[NFC_DEVICE_UID_MAX_LEN];
                uint32_t uid_len;

                // Load data
                loaded = nfc_device_load_uid(ff, uid, &uid_len, NFC_DEVICE_UID_MAX_LEN) &&
                         nfc_device_set_uid(instance, uid, uid_len) &&
                         nfc_devices[protocol]->load(instance->protocol_data, ff, version);
                break;
            }

            nfc_device_clear(instance);
        }

    } while(false);

    furi_string_free(temp_str);
    return loaded;
}

bool nfc_device_load(NfcDevice* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    bool loaded = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    if(instance->loading_callback) {
        instance->loading_callback(instance->loading_callback_context, true);
    }

    do {
        if(!flipper_format_buffered_file_open_existing(ff, path)) break;

        // Read and verify file header
        uint32_t version = 0;
        if(!flipper_format_read_header(ff, temp_str, &version)) break;

        if(furi_string_cmp_str(temp_str, NFC_FILE_HEADER)) break;
        if(version < NFC_MINIMUM_SUPPORTED_FORMAT_VERSION) break;

        // Select loading method
        loaded = (version < NFC_UNIFIED_FORMAT_VERSION) ?
                     nfc_device_load_legacy(instance, ff, version) :
                     nfc_device_load_unified(instance, ff, version);

    } while(false);

    if(instance->loading_callback) {
        instance->loading_callback(instance->loading_callback_context, false);
    }

    furi_string_free(temp_str);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    return loaded;
}
