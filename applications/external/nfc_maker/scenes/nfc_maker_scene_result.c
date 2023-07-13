#include "../nfc_maker.h"

enum PopupEvent {
    PopupEventExit,
};

static void nfc_maker_scene_result_popup_callback(void* context) {
    NfcMaker* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, PopupEventExit);
}

void nfc_maker_scene_result_on_enter(void* context) {
    NfcMaker* app = context;
    Popup* popup = app->popup;
    bool success = false;

    FlipperFormat* file = flipper_format_file_alloc(furi_record_open(RECORD_STORAGE));
    FuriString* path = furi_string_alloc();
    furi_string_printf(path, NFC_APP_FOLDER "/%s" NFC_APP_EXTENSION, app->save_buf);

    uint32_t pages = 135;
    size_t size = pages * 4;
    uint8_t* buf = malloc(size);
    do {
        if(!flipper_format_file_open_new(file, furi_string_get_cstr(path))) break;

        if(!flipper_format_write_header_cstr(file, "Flipper NFC device", 3)) break;
        if(!flipper_format_write_string_cstr(file, "Device type", "NTAG215")) break;

        // Serial number
        size_t i = 0;
        buf[i++] = 0x04;
        furi_hal_random_fill_buf(&buf[i], 8);
        i += 8;
        uint8_t uid[7];
        memcpy(&uid[0], &buf[0], 3);
        memcpy(&uid[3], &buf[4], 4);

        if(!flipper_format_write_hex(file, "UID", uid, sizeof(uid))) break;
        if(!flipper_format_write_string_cstr(file, "ATQA", "00 44")) break;
        if(!flipper_format_write_string_cstr(file, "SAK", "00")) break;
        // TODO: Maybe randomize?
        if(!flipper_format_write_string_cstr(
               file,
               "Signature",
               "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"))
            break;
        if(!flipper_format_write_string_cstr(file, "Mifare version", "00 04 04 02 01 00 11 03"))
            break;

        if(!flipper_format_write_string_cstr(file, "Counter 0", "0")) break;
        if(!flipper_format_write_string_cstr(file, "Tearing 0", "00")) break;
        if(!flipper_format_write_string_cstr(file, "Counter 1", "0")) break;
        if(!flipper_format_write_string_cstr(file, "Tearing 1", "00")) break;
        if(!flipper_format_write_string_cstr(file, "Counter 2", "0")) break;
        if(!flipper_format_write_string_cstr(file, "Tearing 2", "00")) break;
        if(!flipper_format_write_uint32(file, "Pages total", &pages, 1)) break;

        // Static data
        buf[i++] = 0x48; // Internal
        buf[i++] = 0x00; // Lock bytes
        buf[i++] = 0x00; // ...

        buf[i++] = 0xE1; // Capability container
        buf[i++] = 0x10; // ...
        buf[i++] = 0x3E; // ...
        buf[i++] = 0x00; // ...

        buf[i++] = 0x03; // Container flags

        // NDEF Docs: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/protocols/nfc/index.html#nfc-data-exchange-format-ndef
        uint8_t tnf = 0x00;
        const char* type = "";
        uint8_t* payload = NULL;
        size_t payload_len = 0;

        size_t data_len = 0;
        size_t j = 0;
        switch(scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneStart)) {
        case NfcMakerSceneBluetooth: {
            tnf = 0x02; // Media-type [RFC 2046]
            type = "application/vnd.bluetooth.ep.oob";

            data_len = MAC_INPUT_LEN;
            payload_len = data_len + 2;
            payload = malloc(payload_len);

            payload[j++] = 0x08;
            payload[j++] = 0x00;
            memcpy(&payload[j], app->mac_buf, data_len);
            j += data_len;
            break;
        }
        case NfcMakerSceneContact: {
            tnf = 0x02; // Media-type [RFC 2046]
            type = "text/vcard";

            FuriString* vcard = furi_string_alloc_set("BEGIN:VCARD\r\nVERSION:3.0\r\n");
            furi_string_cat_printf(
                vcard, "PRODID:-//Flipper Xtreme//%s//EN\r\n", version_get_version(NULL));
            furi_string_cat_printf(vcard, "N:%s;%s;;;\r\n", app->small_buf2, app->small_buf1);
            furi_string_cat_printf(
                vcard,
                "FN:%s%s%s\r\n",
                app->small_buf1,
                strnlen(app->small_buf2, SMALL_INPUT_LEN) ? " " : "",
                app->small_buf2);
            if(strnlen(app->mail_buf, MAIL_INPUT_LEN)) {
                furi_string_cat_printf(vcard, "EMAIL:%s\r\n", app->mail_buf);
            }
            if(strnlen(app->phone_buf, PHONE_INPUT_LEN)) {
                furi_string_cat_printf(vcard, "TEL:%s\r\n", app->phone_buf);
            }
            if(strnlen(app->big_buf, BIG_INPUT_LEN)) {
                furi_string_cat_printf(vcard, "URL:%s\r\n", app->big_buf);
            }
            furi_string_cat_printf(vcard, "END:VCARD\r\n");

            payload_len = furi_string_size(vcard);
            payload = malloc(payload_len);
            memcpy(payload, furi_string_get_cstr(vcard), payload_len);
            furi_string_free(vcard);
            break;
        }
        case NfcMakerSceneHttps: {
            tnf = 0x01; // NFC Forum well-known type [NFC RTD]
            type = "\x55";

            data_len = strnlen(app->big_buf, BIG_INPUT_LEN);
            payload_len = data_len + 1;
            payload = malloc(payload_len);

            payload[j++] = 0x04; // Prepend "https://"
            memcpy(&payload[j], app->big_buf, data_len);
            j += data_len;
            break;
        }
        case NfcMakerSceneMail: {
            tnf = 0x01; // NFC Forum well-known type [NFC RTD]
            type = "\x55";

            data_len = strnlen(app->mail_buf, MAIL_INPUT_LEN);
            payload_len = data_len + 1;
            payload = malloc(payload_len);

            payload[j++] = 0x06; // Prepend "mailto:"
            memcpy(&payload[j], app->mail_buf, data_len);
            j += data_len;
            break;
        }
        case NfcMakerScenePhone: {
            tnf = 0x01; // NFC Forum well-known type [NFC RTD]
            type = "\x55";

            data_len = strnlen(app->phone_buf, PHONE_INPUT_LEN);
            payload_len = data_len + 1;
            payload = malloc(payload_len);

            payload[j++] = 0x05; // Prepend "tel:"
            memcpy(&payload[j], app->phone_buf, data_len);
            j += data_len;
            break;
        }
        case NfcMakerSceneText: {
            tnf = 0x01; // NFC Forum well-known type [NFC RTD]
            type = "\x54";

            data_len = strnlen(app->big_buf, BIG_INPUT_LEN);
            payload_len = data_len + 3;
            payload = malloc(payload_len);

            payload[j++] = 0x02;
            payload[j++] = 0x65; // e
            payload[j++] = 0x6E; // n
            memcpy(&payload[j], app->big_buf, data_len);
            j += data_len;
            break;
        }
        case NfcMakerSceneUrl: {
            tnf = 0x01; // NFC Forum well-known type [NFC RTD]
            type = "\x55";

            data_len = strnlen(app->big_buf, BIG_INPUT_LEN);
            payload_len = data_len + 1;
            payload = malloc(payload_len);

            payload[j++] = 0x00; // No prepend
            memcpy(&payload[j], app->big_buf, data_len);
            j += data_len;
            break;
        }
        case NfcMakerSceneWifi: {
            tnf = 0x02; // Media-type [RFC 2046]
            type = "application/vnd.wfa.wsc";

            uint8_t ssid_len = strnlen(app->small_buf1, SMALL_INPUT_LEN);
            uint8_t pass_len = strnlen(app->small_buf2, SMALL_INPUT_LEN);
            uint8_t data_len = ssid_len + pass_len;
            payload_len = data_len + 39;
            payload = malloc(payload_len);

            payload[j++] = 0x10;
            payload[j++] = 0x0E;
            payload[j++] = 0x00;

            payload[j++] = data_len + 43;
            payload[j++] = 0x10;
            payload[j++] = 0x26;
            payload[j++] = 0x00;

            payload[j++] = 0x01;
            payload[j++] = 0x01;
            payload[j++] = 0x10;
            payload[j++] = 0x45;

            payload[j++] = 0x00;
            payload[j++] = ssid_len;
            memcpy(&payload[j], app->small_buf1, ssid_len);
            j += ssid_len;
            payload[j++] = 0x10;
            payload[j++] = 0x03;

            payload[j++] = 0x00;
            payload[j++] = 0x02;
            payload[j++] = 0x00;
            payload[j++] =
                scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiAuth);

            payload[j++] = 0x10;
            payload[j++] = 0x0F;
            payload[j++] = 0x00;
            payload[j++] = 0x02;

            payload[j++] = 0x00;
            payload[j++] =
                scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiEncr);
            payload[j++] = 0x10;
            payload[j++] = 0x27;

            payload[j++] = 0x00;
            payload[j++] = pass_len;
            memcpy(&payload[j], app->small_buf2, pass_len);
            j += pass_len;
            payload[j++] = 0x10;
            payload[j++] = 0x20;

            payload[j++] = 0x00;
            payload[j++] = 0x06;
            payload[j++] = 0xFF;
            payload[j++] = 0xFF;

            payload[j++] = 0xFF;
            payload[j++] = 0xFF;
            payload[j++] = 0xFF;
            payload[j++] = 0xFF;

            break;
        }
        default:
            break;
        }

        // Record header
        uint8_t flags = 0;
        flags |= 1 << 7; // MB (Message Begin)
        flags |= 1 << 6; // ME (Message End)
        flags |= tnf; // TNF (Type Name Format)
        size_t type_len = strlen(type);

        size_t header_len = 0;
        header_len += 1; // Flags and TNF
        header_len += 1; // Type length
        if(payload_len < 0xFF) {
            flags |= 1 << 4; // SR (Short Record)
            header_len += 1; // Payload length
        } else {
            header_len += 4; // Payload length
        }
        header_len += type_len; // Payload type

        size_t record_len = header_len + payload_len;
        if(record_len < 0xFF) {
            buf[i++] = record_len; // Record length
        } else {
            buf[i++] = 0xFF; // Record length
            buf[i++] = record_len >> 8; // ...
            buf[i++] = record_len & 0xFF; // ...
        }
        buf[i++] = flags; // Flags and TNF
        buf[i++] = type_len; // Type length
        if(flags & 1 << 4) { // SR (Short Record)
            buf[i++] = payload_len; // Payload length
        } else {
            buf[i++] = 0x00; // Payload length
            buf[i++] = 0x00; // ...
            buf[i++] = payload_len >> 8; // ...
            buf[i++] = payload_len & 0xFF; // ...
        }
        memcpy(&buf[i], type, type_len); // Payload type
        i += type_len;

        // Record payload
        memcpy(&buf[i], payload, payload_len);
        i += payload_len;
        free(payload);

        // Record terminator
        buf[i++] = 0xFE;

        // Padding until last 5 pages
        for(; i < size - 20; i++) {
            buf[i] = 0x00;
        }

        // Last 5 static pages
        buf[i++] = 0x00;
        buf[i++] = 0x00;
        buf[i++] = 0x00;
        buf[i++] = 0xBD;

        buf[i++] = 0x04;
        buf[i++] = 0x00;
        buf[i++] = 0x00;
        buf[i++] = 0xFF;

        buf[i++] = 0x00;
        buf[i++] = 0x05;
        buf[i++] = 0x00;
        buf[i++] = 0x00;

        buf[i++] = 0xFF;
        buf[i++] = 0xFF;
        buf[i++] = 0xFF;
        buf[i++] = 0xFF;

        buf[i++] = 0x00;
        buf[i++] = 0x00;
        buf[i++] = 0x00;
        buf[i++] = 0x00;

        // Write pages
        char str[16];
        bool ok = true;
        for(size_t page = 0; page < pages; page++) {
            snprintf(str, sizeof(str), "Page %u", page);
            if(!flipper_format_write_hex(file, str, &buf[page * 4], 4)) {
                ok = false;
                break;
            }
        }
        if(!ok) break;

        success = true;

    } while(false);
    free(buf);

    furi_string_free(path);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    if(success) {
        popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
        popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    } else {
        popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
        popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    }
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, nfc_maker_scene_result_popup_callback);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMakerViewPopup);
}

bool nfc_maker_scene_result_on_event(void* context, SceneManagerEvent event) {
    NfcMaker* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case PopupEventExit:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, NfcMakerSceneStart);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void nfc_maker_scene_result_on_exit(void* context) {
    NfcMaker* app = context;
    popup_reset(app->popup);
}
