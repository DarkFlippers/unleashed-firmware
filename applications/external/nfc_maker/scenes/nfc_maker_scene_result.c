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
    furi_string_printf(path, NFC_APP_FOLDER "/%s" NFC_APP_EXTENSION, app->name_buf);

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
        buf[i++] = 0x03; // Message flags
        size_t start = i++;

        switch(scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneMenu)) {
        case NfcMakerSceneBluetooth: {
            buf[i++] = 0xD2;
            buf[i++] = 0x20;
            buf[i++] = 0x08;
            buf[i++] = 0x61;
            buf[i++] = 0x70;

            buf[i++] = 0x70;
            buf[i++] = 0x6C;
            buf[i++] = 0x69;
            buf[i++] = 0x63;

            buf[i++] = 0x61;
            buf[i++] = 0x74;
            buf[i++] = 0x69;
            buf[i++] = 0x6F;

            buf[i++] = 0x6E;
            buf[i++] = 0x2F;
            buf[i++] = 0x76;
            buf[i++] = 0x6E;

            buf[i++] = 0x64;
            buf[i++] = 0x2E;
            buf[i++] = 0x62;
            buf[i++] = 0x6C;

            buf[i++] = 0x75;
            buf[i++] = 0x65;
            buf[i++] = 0x74;
            buf[i++] = 0x6F;

            buf[i++] = 0x6F;
            buf[i++] = 0x74;
            buf[i++] = 0x68;
            buf[i++] = 0x2E;

            buf[i++] = 0x65;
            buf[i++] = 0x70;
            buf[i++] = 0x2E;
            buf[i++] = 0x6F;

            buf[i++] = 0x6F;
            buf[i++] = 0x62;
            buf[i++] = 0x08;
            buf[i++] = 0x00;

            memcpy(&buf[i], app->mac_buf, GAP_MAC_ADDR_SIZE);
            i += GAP_MAC_ADDR_SIZE;
            break;
        }
        case NfcMakerSceneHttps: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);

            buf[i++] = 0xD1;
            buf[i++] = 0x01;
            buf[i++] = data_len + 1;
            buf[i++] = 0x55;

            buf[i++] = 0x04; // Prepend "https://"
            memcpy(&buf[i], app->text_buf, data_len);
            i += data_len;
            break;
        }
        case NfcMakerSceneMail: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);

            buf[i++] = 0xD1;
            buf[i++] = 0x01;
            buf[i++] = data_len + 1;
            buf[i++] = 0x55;

            buf[i++] = 0x06; // Prepend "mailto:"
            memcpy(&buf[i], app->text_buf, data_len);
            i += data_len;
            break;
        }
        case NfcMakerScenePhone: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);

            buf[i++] = 0xD1;
            buf[i++] = 0x01;
            buf[i++] = data_len + 1;
            buf[i++] = 0x55;

            buf[i++] = 0x05; // Prepend "tel:"
            memcpy(&buf[i], app->text_buf, data_len);
            i += data_len;
            break;
        }
        case NfcMakerSceneText: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);

            buf[i++] = 0xD1;
            buf[i++] = 0x01;
            buf[i++] = data_len + 3;
            buf[i++] = 0x54;

            buf[i++] = 0x02;
            buf[i++] = 0x65; // e
            buf[i++] = 0x6E; // n
            memcpy(&buf[i], app->text_buf, data_len);
            i += data_len;
            break;
        }
        case NfcMakerSceneUrl: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);

            buf[i++] = 0xD1;
            buf[i++] = 0x01;
            buf[i++] = data_len + 1;
            buf[i++] = 0x55;

            buf[i++] = 0x00; // No prepend
            memcpy(&buf[i], app->text_buf, data_len);
            i += data_len;
            break;
        }
        case NfcMakerSceneWifi: {
            uint8_t ssid_len = strnlen(app->text_buf, WIFI_INPUT_LEN);
            uint8_t pass_len = strnlen(app->pass_buf, WIFI_INPUT_LEN);
            uint8_t data_len = ssid_len + pass_len;

            buf[i++] = 0xD2;
            buf[i++] = 0x17;
            buf[i++] = data_len + 47;
            buf[i++] = 0x61;
            buf[i++] = 0x70;

            buf[i++] = 0x70;
            buf[i++] = 0x6C;
            buf[i++] = 0x69;
            buf[i++] = 0x63;

            buf[i++] = 0x61;
            buf[i++] = 0x74;
            buf[i++] = 0x69;
            buf[i++] = 0x6F;

            buf[i++] = 0x6E;
            buf[i++] = 0x2F;
            buf[i++] = 0x76;
            buf[i++] = 0x6E;

            buf[i++] = 0x64;
            buf[i++] = 0x2E;
            buf[i++] = 0x77;
            buf[i++] = 0x66;

            buf[i++] = 0x61;
            buf[i++] = 0x2E;
            buf[i++] = 0x77;
            buf[i++] = 0x73;

            buf[i++] = 0x63;
            buf[i++] = 0x10;
            buf[i++] = 0x0E;
            buf[i++] = 0x00;

            buf[i++] = data_len + 43;
            buf[i++] = 0x10;
            buf[i++] = 0x26;
            buf[i++] = 0x00;

            buf[i++] = 0x01;
            buf[i++] = 0x01;
            buf[i++] = 0x10;
            buf[i++] = 0x45;

            buf[i++] = 0x00;
            buf[i++] = ssid_len;
            memcpy(&buf[i], app->text_buf, ssid_len);
            i += ssid_len;
            buf[i++] = 0x10;
            buf[i++] = 0x03;

            buf[i++] = 0x00;
            buf[i++] = 0x02;
            buf[i++] = 0x00;
            buf[i++] = scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiAuth);

            buf[i++] = 0x10;
            buf[i++] = 0x0F;
            buf[i++] = 0x00;
            buf[i++] = 0x02;

            buf[i++] = 0x00;
            buf[i++] = scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiEncr);
            buf[i++] = 0x10;
            buf[i++] = 0x27;

            buf[i++] = 0x00;
            buf[i++] = pass_len;
            memcpy(&buf[i], app->pass_buf, pass_len);
            i += pass_len;
            buf[i++] = 0x10;
            buf[i++] = 0x20;

            buf[i++] = 0x00;
            buf[i++] = 0x06;
            buf[i++] = 0xFF;
            buf[i++] = 0xFF;

            buf[i++] = 0xFF;
            buf[i++] = 0xFF;
            buf[i++] = 0xFF;
            buf[i++] = 0xFF;

            break;
        }
        default:
            break;
        }

        // Message length and terminator
        buf[start] = i - start - 1;
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
                app->scene_manager, NfcMakerSceneMenu);
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
