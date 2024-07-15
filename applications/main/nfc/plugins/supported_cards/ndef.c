// Parser for NDEF format data
// Supports multiple NDEF messages and records in same tag
// Parsed types: URI (+ Phone, Mail), Text, BT MAC, Contact, WiFi, Empty
// Documentation and sources indicated where relevant
// Made by @Willy-JL

#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

#include <bit_lib.h>

#define TAG "NDEF"

static bool is_text(const uint8_t* buf, size_t len) {
    for(size_t i = 0; i < len; i++) {
        const char c = buf[i];
        if((c < ' ' || c > '~') && c != '\r' && c != '\n') {
            return false;
        }
    }
    return true;
}

static void
    print_data(FuriString* str, const char* prefix, const uint8_t* buf, size_t len, bool force_hex) {
    if(prefix) furi_string_cat_printf(str, "%s: ", prefix);
    if(!force_hex && is_text(buf, len)) {
        char* tmp = malloc(len + 1);
        memcpy(tmp, buf, len);
        tmp[len] = '\0';
        furi_string_cat_printf(str, "%s", tmp);
        free(tmp);
    } else {
        for(uint8_t i = 0; i < len; i++) {
            furi_string_cat_printf(str, "%02X ", buf[i]);
        }
    }
    furi_string_cat(str, "\n");
}

static void parse_ndef_uri(FuriString* str, const uint8_t* payload, uint32_t payload_len) {
    // https://learn.adafruit.com/adafruit-pn532-rfid-nfc/ndef#uri-records-0x55-slash-u-607763
    const char* prepends[] = {
        [0x00] = "",
        [0x01] = "http://www.",
        [0x02] = "https://www.",
        [0x03] = "http://",
        [0x04] = "https://",
        [0x05] = "tel:",
        [0x06] = "mailto:",
        [0x07] = "ftp://anonymous:anonymous@",
        [0x08] = "ftp://ftp.",
        [0x09] = "ftps://",
        [0x0A] = "sftp://",
        [0x0B] = "smb://",
        [0x0C] = "nfs://",
        [0x0D] = "ftp://",
        [0x0E] = "dav://",
        [0x0F] = "news:",
        [0x10] = "telnet://",
        [0x11] = "imap:",
        [0x12] = "rtsp://",
        [0x13] = "urn:",
        [0x14] = "pop:",
        [0x15] = "sip:",
        [0x16] = "sips:",
        [0x17] = "tftp:",
        [0x18] = "btspp://",
        [0x19] = "btl2cap://",
        [0x1A] = "btgoep://",
        [0x1B] = "tcpobex://",
        [0x1C] = "irdaobex://",
        [0x1D] = "file://",
        [0x1E] = "urn:epc:id:",
        [0x1F] = "urn:epc:tag:",
        [0x20] = "urn:epc:pat:",
        [0x21] = "urn:epc:raw:",
        [0x22] = "urn:epc:",
        [0x23] = "urn:nfc:",
    };
    const char* prepend = "";
    uint8_t prepend_type = payload[0];
    if(prepend_type < COUNT_OF(prepends)) {
        prepend = prepends[prepend_type];
    }
    size_t prepend_len = strlen(prepend);

    size_t uri_len = prepend_len + (payload_len - 1);
    char* const uri_buf = malloc(uri_len);
    memcpy(uri_buf, prepend, prepend_len);
    memcpy(uri_buf + prepend_len, payload + 1, payload_len - 1);
    char* uri = uri_buf;

    const char* type = "URI";
    if(strncmp(uri, "http", strlen("http")) == 0) {
        type = "URL";
    } else if(strncmp(uri, "tel:", strlen("tel:")) == 0) {
        type = "Phone";
        uri += strlen("tel:");
        uri_len -= strlen("tel:");
    } else if(strncmp(uri, "mailto:", strlen("mailto:")) == 0) {
        type = "Mail";
        uri += strlen("mailto:");
        uri_len -= strlen("mailto:");
    }

    furi_string_cat_printf(str, "%s\n", type);
    print_data(str, NULL, (uint8_t*)uri, uri_len, false);
    free(uri_buf);
}

static void parse_ndef_text(FuriString* str, const uint8_t* payload, uint32_t payload_len) {
    furi_string_cat(str, "Text\n");
    print_data(str, NULL, payload + 3, payload_len - 3, false);
}

static void parse_ndef_bt(FuriString* str, const uint8_t* payload, uint32_t payload_len) {
    furi_string_cat(str, "BT MAC\n");
    print_data(str, NULL, payload + 2, payload_len - 2, true);
}

static void parse_ndef_vcard(FuriString* str, const uint8_t* payload, uint32_t payload_len) {
    char* tmp = malloc(payload_len + 1);
    memcpy(tmp, payload, payload_len);
    tmp[payload_len] = '\0';
    FuriString* fmt = furi_string_alloc_set(tmp);
    free(tmp);

    furi_string_trim(fmt);
    if(furi_string_start_with(fmt, "BEGIN:VCARD")) {
        furi_string_right(fmt, furi_string_search_char(fmt, '\n'));
        if(furi_string_end_with(fmt, "END:VCARD")) {
            furi_string_left(fmt, furi_string_search_rchar(fmt, '\n'));
        }
        furi_string_trim(fmt);
        if(furi_string_start_with(fmt, "VERSION:")) {
            furi_string_right(fmt, furi_string_search_char(fmt, '\n'));
            furi_string_trim(fmt);
        }
    }

    furi_string_cat(str, "Contact\n");
    print_data(str, NULL, (uint8_t*)furi_string_get_cstr(fmt), furi_string_size(fmt), false);
    furi_string_free(fmt);
}

static void parse_ndef_wifi(FuriString* str, const uint8_t* payload, uint32_t payload_len) {
// https://android.googlesource.com/platform/packages/apps/Nfc/+/refs/heads/main/src/com/android/nfc/NfcWifiProtectedSetup.java
#define CREDENTIAL_FIELD_ID        (0x100E)
#define SSID_FIELD_ID              (0x1045)
#define NETWORK_KEY_FIELD_ID       (0x1027)
#define AUTH_TYPE_FIELD_ID         (0x1003)
#define AUTH_TYPE_EXPECTED_SIZE    (2)
#define AUTH_TYPE_OPEN             (0x0001)
#define AUTH_TYPE_WPA_PSK          (0x0002)
#define AUTH_TYPE_WPA_EAP          (0x0008)
#define AUTH_TYPE_WPA2_EAP         (0x0010)
#define AUTH_TYPE_WPA2_PSK         (0x0020)
#define AUTH_TYPE_WPA_AND_WPA2_PSK (0x0022)
#define MAX_NETWORK_KEY_SIZE_BYTES (64)

    size_t i = 0;
    while(i < payload_len) {
        uint16_t field_id = bit_lib_bytes_to_num_be(payload + i, 2);
        i += 2;
        uint16_t field_len = bit_lib_bytes_to_num_be(payload + i, 2);
        i += 2;

        if(field_id == CREDENTIAL_FIELD_ID) {
            furi_string_cat(str, "WiFi\n");
            size_t start_position = i;
            while(i < start_position + field_len) {
                uint16_t cfg_id = bit_lib_bytes_to_num_be(payload + i, 2);
                i += 2;
                uint16_t cfg_len = bit_lib_bytes_to_num_be(payload + i, 2);
                i += 2;

                if(i + cfg_len > start_position + field_len) {
                    return;
                }

                switch(cfg_id) {
                case SSID_FIELD_ID:
                    print_data(str, "SSID", payload + i, cfg_len, false);
                    i += cfg_len;
                    break;
                case NETWORK_KEY_FIELD_ID:
                    if(cfg_len > MAX_NETWORK_KEY_SIZE_BYTES) {
                        return;
                    }
                    print_data(str, "PWD", payload + i, cfg_len, false);
                    i += cfg_len;
                    break;
                case AUTH_TYPE_FIELD_ID:
                    if(cfg_len != AUTH_TYPE_EXPECTED_SIZE) {
                        return;
                    }
                    short auth_type = bit_lib_bytes_to_num_be(payload + i, 2);
                    i += 2;
                    const char* auth;
                    switch(auth_type) {
                    case AUTH_TYPE_OPEN:
                        auth = "Open";
                        break;
                    case AUTH_TYPE_WPA_PSK:
                        auth = "WPA Personal";
                        break;
                    case AUTH_TYPE_WPA_EAP:
                        auth = "WPA Enterprise";
                        break;
                    case AUTH_TYPE_WPA2_EAP:
                        auth = "WPA2 Enterprise";
                        break;
                    case AUTH_TYPE_WPA2_PSK:
                        auth = "WPA2 Personal";
                        break;
                    case AUTH_TYPE_WPA_AND_WPA2_PSK:
                        auth = "WPA/WPA2 Personal";
                        break;
                    default:
                        auth = "Unknown";
                        break;
                    }
                    print_data(str, "AUTH", (uint8_t*)auth, strlen(auth), false);
                    break;
                default:
                    i += cfg_len;
                    break;
                }
            }
            return;
        }
        i += field_len;
    }
}

static void parse_ndef_payload(
    FuriString* str,
    uint8_t tnf,
    const char* type,
    uint8_t type_len,
    const uint8_t* payload,
    uint32_t payload_len) {
    if(!payload_len) {
        furi_string_cat(str, "Empty\n");
        return;
    }
    switch(tnf) {
    case 0x01: // NFC Forum well-known type [NFC RTD]
        if(strncmp("U", type, type_len) == 0) {
            parse_ndef_uri(str, payload, payload_len);
        } else if(strncmp("T", type, type_len) == 0) {
            parse_ndef_text(str, payload, payload_len);
        } else {
            print_data(str, "Well-known type", (uint8_t*)type, type_len, false);
            print_data(str, "Payload", payload, payload_len, false);
        }
        break;
    case 0x02: // Media-type [RFC 2046]
        if(strncmp("application/vnd.bluetooth.ep.oob", type, type_len) == 0) {
            parse_ndef_bt(str, payload, payload_len);
        } else if(strncmp("text/vcard", type, type_len) == 0) {
            parse_ndef_vcard(str, payload, payload_len);
        } else if(strncmp("application/vnd.wfa.wsc", type, type_len) == 0) {
            parse_ndef_wifi(str, payload, payload_len);
        } else {
            print_data(str, "Media Type", (uint8_t*)type, type_len, false);
            print_data(str, "Payload", payload, payload_len, false);
        }
        break;
    case 0x00: // Empty
    case 0x03: // Absolute URI [RFC 3986]
    case 0x04: // NFC Forum external type [NFC RTD]
    case 0x05: // Unknown
    case 0x06: // Unchanged
    case 0x07: // Reserved
    default: // Unknown
        // Dump data without parsing
        print_data(str, "Type name format", &tnf, 1, true);
        print_data(str, "Type", (uint8_t*)type, type_len, false);
        print_data(str, "Payload", payload, payload_len, false);
        break;
    }
}

static const uint8_t* parse_ndef_message(
    FuriString* str,
    size_t message_num,
    const uint8_t* cur,
    const uint8_t* message_end) {
    // NDEF message and record documentation:
    // https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/protocols/nfc/index.html#ndef-message-and-record-format
    size_t record_num = 0;
    bool last_record = false;
    while(cur < message_end) {
        // Flags and TNF
        uint8_t flags_tnf = *cur++;
        // Message Begin should only be set on first record
        if(record_num++ && flags_tnf & (1 << 7)) break;
        // Message End should only be set on last record
        if(last_record) break;
        if(flags_tnf & (1 << 6)) last_record = true;
        // Chunked Flag not supported
        if(flags_tnf & (1 << 5)) break;
        // Payload Length field of 1 vs 4 bytes
        bool short_record = flags_tnf & (1 << 4);
        // Is payload ID length and value present
        bool id_present = flags_tnf & (1 << 3);
        // Type Name Format 3 bit value
        uint8_t tnf = flags_tnf & 0b00000111;

        // Type Length
        uint8_t type_len = *cur++;

        // Payload Length
        uint32_t payload_len;
        if(short_record) {
            payload_len = *cur++;
        } else {
            payload_len = bit_lib_bytes_to_num_be(cur, 4);
            cur += 4;
        }

        // ID Length
        uint8_t id_len = 0;
        if(id_present) {
            id_len = *cur++;
        }

        // Payload Type
        char* type = NULL;
        if(type_len) {
            type = malloc(type_len);
            memcpy(type, cur, type_len);
            cur += type_len;
        }

        // Payload ID
        cur += id_len;

        furi_string_cat_printf(str, "\e*> M:%d R:%d - ", message_num, record_num);
        parse_ndef_payload(str, tnf, type, type_len, cur, payload_len);
        cur += payload_len;

        free(type);
        furi_string_trim(str, "\n");
        furi_string_cat(str, "\n\n");
    }
    return cur;
}

static bool ndef_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    bool parsed = false;

    do {
        // Memory layout documentation:
        // https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrfxlib/nfc/doc/type_2_tag.html#id2

        // Check card type can contain NDEF
        if(data->type != MfUltralightTypeNTAG203 && data->type != MfUltralightTypeNTAG213 &&
           data->type != MfUltralightTypeNTAG215 && data->type != MfUltralightTypeNTAG216 &&
           data->type != MfUltralightTypeNTAGI2C1K && data->type != MfUltralightTypeNTAGI2C2K) {
            break;
        }

        // Double check Capability Container (CC) and find data area bounds
        struct {
            uint8_t nfc_magic_number;
            uint8_t document_version_number;
            uint8_t data_area_size;
            uint8_t read_write_access;
        }* cc = (void*)&data->page[3].data[0];
        if(cc->nfc_magic_number != 0xE1) break;
        if(cc->document_version_number != 0x10) break;
        const uint8_t* cur = &data->page[4].data[0];
        const uint8_t* end = cur + (cc->data_area_size * 2 * MF_ULTRALIGHT_PAGE_SIZE);
        size_t max_size = mf_ultralight_get_pages_total(data->type) * MF_ULTRALIGHT_PAGE_SIZE;
        end = MIN(end, &data->page[0].data[0] + max_size);
        size_t message_num = 0;

        // Parse as TLV (see docs above)
        while(cur < end) {
            switch(*cur++) {
            case 0x03: { // NDEF message
                if(cur >= end) break;
                uint16_t len;
                if(*cur < 0xFF) { // 1 byte length
                    len = *cur++;
                } else { // 3 byte length (0xFF marker + 2 byte integer)
                    if(cur + 2 >= end) {
                        cur = end;
                        break;
                    }
                    len = bit_lib_bytes_to_num_be(++cur, 2);
                    cur += 2;
                }
                if(cur + len >= end) {
                    cur = end;
                    break;
                }

                if(message_num++ == 0) {
                    furi_string_printf(
                        parsed_data,
                        "\e#NDEF Format Data\nCard type: %s\n",
                        mf_ultralight_get_device_name(data, NfcDeviceNameTypeFull));
                }

                const uint8_t* message_end = cur + len;
                cur = parse_ndef_message(parsed_data, message_num, cur, message_end);
                if(cur != message_end) cur = end;

                break;
            }

            case 0xFE: // TLV end
                cur = end;
                if(message_num != 0) parsed = true;
                break;

            case 0x00: // Padding, has no length, skip
                break;

            case 0x01: // Lock control
            case 0x02: // Memory control
            case 0xFD: // Proprietary
                // We don't care, skip this TLV block
                if(cur >= end) break;
                if(*cur < 0xFF) { // 1 byte length
                    cur += *cur + 1; // Shift by TLV length
                } else { // 3 byte length (0xFF marker + 2 byte integer)
                    if(cur + 2 >= end) {
                        cur = end;
                        break;
                    }
                    cur += bit_lib_bytes_to_num_be(cur + 1, 2) + 3; // Shift by TLV length
                }
                break;

            default: // Unknown, bail to avoid problems
                cur = end;
                break;
            }
        }

        if(parsed) {
            furi_string_trim(parsed_data, "\n");
            furi_string_cat(parsed_data, "\n");
        } else {
            furi_string_reset(parsed_data);
        }
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin ndef_plugin = {
    .protocol = NfcProtocolMfUltralight,
    .verify = NULL,
    .read = NULL,
    .parse = ndef_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor ndef_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &ndef_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* ndef_plugin_ep() {
    return &ndef_plugin_descriptor;
}
