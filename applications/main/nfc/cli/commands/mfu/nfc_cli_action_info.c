
#include "nfc_cli_action_info.h"

#include "../../../helpers/protocol_support/mf_ultralight/mf_ultralight_render.h"
#include "../helpers/nfc_cli_format.h"

#include <furi.h>
#include <flipper_format/flipper_format.h>
#include <storage/storage.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_poller_sync.h>

#define TAG "INFO"

typedef struct {
    uint8_t magic;
    union {
        uint8_t value;
        struct {
            uint8_t minor : 4;
            uint8_t major : 4;
        };
    } version;

    uint8_t size;

    union {
        uint8_t value;
        struct {
            uint8_t write : 4;
            uint8_t read  : 4;
        };
    } access;
} FURI_PACKED MfUltralightCapabilityContainer;

typedef struct {
    Nfc* nfc;
    MfUltralightData* data;
} NfcCliMfuContext;

static void nfc_cli_mfu_info_get_vendor(const uint8_t vendor_key, FuriString* output) {
    furi_assert(output);

    FuriString* buf = furi_string_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_existing(ff, EXT_PATH("nfc/assets/vendors.nfc"))) {
            FURI_LOG_W(TAG, "NFC Vendors dict not found");
            break;
        }

        char uid_str[5];
        snprintf(uid_str, sizeof(uid_str), "%d", vendor_key);

        if(flipper_format_read_string(ff, uid_str, buf))
            furi_string_printf(output, "%s, %s", uid_str, furi_string_get_cstr(buf));
        else
            furi_string_printf(output, "unknown");
    } while(false);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(buf);
}

const char*
    nfc_cli_mfu_capability_container_get_access_description(const uint8_t value, bool read) {
    const char* description = "RFU"; //value 0x01 - 0x07, and 0xF when read
    if(value == 0x00)
        description = "access fully granted";
    else if(value >= 0x08 && value <= 0x0E)
        description = "proprietary";
    else if(value == 0x0F && !read)
        description = "no access granted at all";

    return description;
}

static void nfc_cli_mfu_info_print_common(const MfUltralightData* data) {
    FuriString* str = furi_string_alloc();

    printf(ANSI_FG_GREEN "\r\n\tTag information\r\n" ANSI_RESET);
    printf(
        "Type: " ANSI_FG_YELLOW "%s\r\n" ANSI_RESET,
        mf_ultralight_get_device_name(data, NfcDeviceNameTypeFull));

    nfc_cli_mfu_info_get_vendor(data->iso14443_3a_data->uid[0], str);
    printf("Vendor ID: %s\r\n", furi_string_get_cstr(str));

    furi_string_reset(str);
    nfc_render_mf_ultralight_info(data, NfcProtocolFormatTypeFull, str);
    printf("%s\r\n", furi_string_get_cstr(str));
    printf("BCC0: %02X\r\nBCC1: %02X\r\n", data->page[0].data[3], data->page[2].data[0]);

    furi_string_free(str);
}

static void nfc_cli_mfu_info_print_ndef(const MfUltralightData* data) {
    const MfUltralightCapabilityContainer* cc =
        (const MfUltralightCapabilityContainer*)data->page[3].data;
    if(cc->magic == 0xE1) {
        printf(ANSI_FG_GREEN "\r\n\tNDEF Message\r\n" ANSI_RESET);
        nfc_cli_printf_array(data->page[3].data, 4, "Capability container: ");
        printf(
            "\r\nMagic number: %02X\r\nVersion %d.%d\r\nSize: [%02X] - %d bytes\r\n",
            cc->magic,
            cc->version.major,
            cc->version.minor,
            cc->size,
            cc->size * 8);
        printf(
            "Access read: [%02X] - %s",
            cc->access.read,
            nfc_cli_mfu_capability_container_get_access_description(cc->access.read, true));
        printf(
            "Access write: [%02X] - %s",
            cc->access.write,
            nfc_cli_mfu_capability_container_get_access_description(cc->access.write, false));
    }
}

static void nfc_cli_mfu_info_print_counter(const MfUltralightData* data) {
    uint32_t features = mf_ultralight_get_feature_support_set(data->type);
    if(!mf_ultralight_support_feature(features, MfUltralightFeatureSupportReadCounter)) return;

    printf(ANSI_FG_GREEN "\r\n\n\tTag counters\r\n" ANSI_RESET);
    uint8_t i =
        mf_ultralight_support_feature(features, MfUltralightFeatureSupportSingleCounter) ? 2 : 0;

    for(; i < MF_ULTRALIGHT_COUNTER_NUM; i++) {
        printf("Counter [%d]: ", i);
        nfc_cli_printf_array(data->counter[i].data, MF_ULTRALIGHT_COUNTER_SIZE, "");
        printf(" Value: %lu\r\n", data->counter[i].counter);

        const uint8_t tf = data->tearing_flag[i].data;
        printf(
            "Tearing [%d]: [%02X] %s",
            i,
            tf,
            tf == MF_ULTRALIGHT_TEARING_FLAG_DEFAULT ? "(ok)" : "");
    }
}

static void nfc_cli_mfu_info_print_signature(const MfUltralightData* data) {
    uint32_t features = mf_ultralight_get_feature_support_set(data->type);
    if(!mf_ultralight_support_feature(features, MfUltralightFeatureSupportReadSignature)) return;

    const MfUltralightSignature* signature = &data->signature;
    printf(ANSI_FG_GREEN "\r\n\n\tTag signature\r\n" ANSI_RESET);
    nfc_cli_printf_array(signature->data, sizeof(signature->data), "ECC signature: ");
}

static void nfc_cli_mfu_info_print_version_storage_size(uint8_t storage_size) {
    uint16_t max_size = 1 << ((storage_size >> 1) + 1);
    uint16_t min_exact_size = 1 << (storage_size >> 1);

    bool exact_size = !(storage_size & 0x01);
    if(exact_size)
        printf("[%02X], (%u bytes)", storage_size, min_exact_size);
    else
        printf("[%02X], (%u <-> %u bytes)", storage_size, min_exact_size, max_size);
}

static void nfc_cli_mfu_info_print_version(const MfUltralightData* data) {
    uint32_t features = mf_ultralight_get_feature_support_set(data->type);
    if(!mf_ultralight_support_feature(features, MfUltralightFeatureSupportReadVersion)) return;

    const MfUltralightVersion* version = &data->version;
    printf(ANSI_FG_GREEN "\r\n\n\tTag Version\r\n" ANSI_RESET);
    nfc_cli_printf_array((uint8_t*)version, sizeof(MfUltralightVersion), "Raw bytes: ");

    FuriString* str = furi_string_alloc();
    nfc_cli_mfu_info_get_vendor(version->vendor_id, str);
    printf("\r\nVendor ID: %s\r\n", furi_string_get_cstr(str));
    furi_string_free(str);

    printf("Product type: %02X\r\n", version->prod_type);

    printf(
        "Protocol type: %02X%s\r\n",
        version->protocol_type,
        (version->protocol_type == 0x3) ? ", ISO14443-3 Compliant" : "");

    printf(
        "Product subtype: [%02X], %s\r\n",
        version->prod_subtype,
        (version->prod_subtype == 1) ? "17 pF" : "50pF");
    printf(
        "Major version: %02X\r\nMinor version: %02X\r\nSize: ",
        version->prod_ver_major,
        version->prod_ver_minor);
    nfc_cli_mfu_info_print_version_storage_size(version->storage_size);
}

NfcCliActionContext* nfc_cli_mfu_info_alloc_ctx(Nfc* nfc) {
    NfcCliMfuContext* instance = malloc(sizeof(NfcCliMfuContext));
    instance->nfc = nfc;
    instance->data = mf_ultralight_alloc();
    return instance;
}

void nfc_cli_mfu_info_free_ctx(NfcCliActionContext* ctx) {
    NfcCliMfuContext* instance = ctx;
    mf_ultralight_free(instance->data);
    free(instance);
}

void nfc_cli_mfu_info_execute(PipeSide* pipe, NfcCliActionContext* ctx) {
    furi_assert(pipe);
    furi_assert(ctx);

    NfcCliMfuContext* instance = ctx;

    MfUltralightError error =
        mf_ultralight_poller_sync_read_card(instance->nfc, instance->data, NULL);
    if(error == MfUltralightErrorNone) {
        const MfUltralightData* data = instance->data;
        nfc_cli_mfu_info_print_common(data);
        nfc_cli_mfu_info_print_ndef(data);
        nfc_cli_mfu_info_print_counter(data);
        nfc_cli_mfu_info_print_signature(data);
        nfc_cli_mfu_info_print_version(data);
    } else {
        printf(ANSI_FG_RED "Error: %s" ANSI_RESET, nfc_cli_mf_ultralight_get_error(error));
    }
}
