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
    do {
        if(!flipper_format_file_open_new(file, furi_string_get_cstr(path))) break;

        uint32_t pages = 42;
        size_t size = pages * 4;
        uint8_t* buf = malloc(size);

        if(!flipper_format_write_header_cstr(file, "Flipper NFC device", 3)) break;
        if(!flipper_format_write_string_cstr(file, "Device type", "NTAG203")) break;

        // Serial number
        buf[0] = 0x04;
        furi_hal_random_fill_buf(&buf[1], 8);
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
        if(!flipper_format_write_string_cstr(file, "Mifare version", "00 00 00 00 00 00 00 00"))
            break;

        if(!flipper_format_write_string_cstr(file, "Counter 0", "0")) break;
        if(!flipper_format_write_string_cstr(file, "Tearing 0", "00")) break;
        if(!flipper_format_write_string_cstr(file, "Counter 1", "0")) break;
        if(!flipper_format_write_string_cstr(file, "Tearing 1", "00")) break;
        if(!flipper_format_write_string_cstr(file, "Counter 2", "0")) break;
        if(!flipper_format_write_string_cstr(file, "Tearing 2", "00")) break;
        if(!flipper_format_write_uint32(file, "Pages total", &pages, 1)) break;

        // Static data
        buf[9] = 0x48; // Internal
        buf[10] = 0x00; // Lock bytes
        buf[11] = 0x00; // ...
        buf[12] = 0xE1; // Capability container
        buf[13] = 0x10; // ...
        buf[14] = 0x12; // ...
        buf[15] = 0x00; // ...
        buf[16] = 0x01; // ...
        buf[17] = 0x03; // ...
        buf[18] = 0xA0; // ...
        buf[19] = 0x10; // ...
        buf[20] = 0x44; // ...
        buf[21] = 0x03; // Message flags

        size_t msg_len = 0;
        switch(scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneMenu)) {
        case NfcMakerSceneBluetooth: {
            msg_len = 0x2B;

            buf[23] = 0xD2;
            buf[24] = 0x20;
            buf[25] = 0x08;
            buf[26] = 0x61;
            buf[27] = 0x70;

            buf[28] = 0x70;
            buf[29] = 0x6C;
            buf[30] = 0x69;
            buf[31] = 0x63;

            buf[32] = 0x61;
            buf[33] = 0x74;
            buf[34] = 0x69;
            buf[35] = 0x6F;

            buf[36] = 0x6E;
            buf[37] = 0x2F;
            buf[38] = 0x76;
            buf[39] = 0x6E;

            buf[40] = 0x64;
            buf[41] = 0x2E;
            buf[42] = 0x62;
            buf[43] = 0x6C;

            buf[44] = 0x75;
            buf[45] = 0x65;
            buf[46] = 0x74;
            buf[47] = 0x6F;

            buf[48] = 0x6F;
            buf[49] = 0x74;
            buf[50] = 0x68;
            buf[51] = 0x2E;

            buf[52] = 0x65;
            buf[53] = 0x70;
            buf[54] = 0x2E;
            buf[55] = 0x6F;

            buf[56] = 0x6F;
            buf[57] = 0x62;
            buf[58] = 0x08;
            buf[59] = 0x00;

            memcpy(&buf[60], app->mac_buf, GAP_MAC_ADDR_SIZE);
            break;
        }
        case NfcMakerSceneHttps: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);
            msg_len = data_len + 5;

            buf[23] = 0xD1;
            buf[24] = 0x01;
            buf[25] = data_len + 1;
            buf[26] = 0x55;

            buf[27] = 0x04; // Prepend "https://"
            memcpy(&buf[28], app->text_buf, data_len);
            break;
        }
        case NfcMakerSceneMail: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);
            msg_len = data_len + 5;

            buf[23] = 0xD1;
            buf[24] = 0x01;
            buf[25] = data_len + 1;
            buf[26] = 0x55;

            buf[27] = 0x06; // Prepend "mailto:"
            memcpy(&buf[28], app->text_buf, data_len);
            break;
        }
        case NfcMakerScenePhone: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);
            msg_len = data_len + 5;

            buf[23] = 0xD1;
            buf[24] = 0x01;
            buf[25] = data_len + 1;
            buf[26] = 0x55;

            buf[27] = 0x05; // Prepend "tel:"
            memcpy(&buf[28], app->text_buf, data_len);
            break;
        }
        case NfcMakerSceneText: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);
            msg_len = data_len + 7;

            buf[23] = 0xD1;
            buf[24] = 0x01;
            buf[25] = data_len + 3;
            buf[26] = 0x54;

            buf[27] = 0x02;
            buf[28] = 0x65; // e
            buf[29] = 0x6E; // n
            memcpy(&buf[30], app->text_buf, data_len);
            break;
        }
        case NfcMakerSceneUrl: {
            uint8_t data_len = strnlen(app->text_buf, TEXT_INPUT_LEN);
            msg_len = data_len + 5;

            buf[23] = 0xD1;
            buf[24] = 0x01;
            buf[25] = data_len + 1;
            buf[26] = 0x55;

            buf[27] = 0x00; // No prepend
            memcpy(&buf[28], app->text_buf, data_len);
            break;
        }
        case NfcMakerSceneWifi: {
            uint8_t ssid_len = strnlen(app->text_buf, WIFI_INPUT_LEN);
            uint8_t pass_len = strnlen(app->pass_buf, WIFI_INPUT_LEN);
            uint8_t data_len = ssid_len + pass_len;
            msg_len = data_len + 73;

            buf[23] = 0xD2;
            buf[24] = 0x17;
            buf[25] = data_len + 47;
            buf[26] = 0x61;
            buf[27] = 0x70;

            buf[28] = 0x70;
            buf[29] = 0x6C;
            buf[30] = 0x69;
            buf[31] = 0x63;

            buf[32] = 0x61;
            buf[33] = 0x74;
            buf[34] = 0x69;
            buf[35] = 0x6F;

            buf[36] = 0x6E;
            buf[37] = 0x2F;
            buf[38] = 0x76;
            buf[39] = 0x6E;

            buf[40] = 0x64;
            buf[41] = 0x2E;
            buf[42] = 0x77;
            buf[43] = 0x66;

            buf[44] = 0x61;
            buf[45] = 0x2E;
            buf[46] = 0x77;
            buf[47] = 0x73;

            buf[48] = 0x63;
            buf[49] = 0x10;
            buf[50] = 0x0E;
            buf[51] = 0x00;

            buf[52] = data_len + 43;
            buf[53] = 0x10;
            buf[54] = 0x26;
            buf[55] = 0x00;

            buf[56] = 0x01;
            buf[57] = 0x01;
            buf[58] = 0x10;
            buf[59] = 0x45;

            buf[60] = 0x00;
            buf[61] = ssid_len;
            memcpy(&buf[62], app->text_buf, ssid_len);
            size_t ssid = 62 + ssid_len;
            buf[ssid + 0] = 0x10;
            buf[ssid + 1] = 0x03;

            buf[ssid + 2] = 0x00;
            buf[ssid + 3] = 0x02;
            buf[ssid + 4] = 0x00;
            buf[ssid + 5] =
                scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiAuth);

            buf[ssid + 6] = 0x10;
            buf[ssid + 7] = 0x0F;
            buf[ssid + 8] = 0x00;
            buf[ssid + 9] = 0x02;

            buf[ssid + 10] = 0x00;
            buf[ssid + 11] =
                scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiEncr);
            buf[ssid + 12] = 0x10;
            buf[ssid + 13] = 0x27;

            buf[ssid + 14] = 0x00;
            buf[ssid + 15] = pass_len;
            memcpy(&buf[ssid + 16], app->pass_buf, pass_len);
            size_t pass = ssid + 16 + pass_len;
            buf[pass + 0] = 0x10;
            buf[pass + 1] = 0x20;

            buf[pass + 2] = 0x00;
            buf[pass + 3] = 0x06;
            buf[pass + 4] = 0xFF;
            buf[pass + 5] = 0xFF;

            buf[pass + 6] = 0xFF;
            buf[pass + 7] = 0xFF;
            buf[pass + 8] = 0xFF;
            buf[pass + 9] = 0xFF;

            break;
        }
        default:
            break;
        }

        // Message length and terminator
        buf[22] = msg_len;
        size_t msg_end = 23 + msg_len;
        buf[msg_end] = 0xFE;

        // Padding
        for(size_t i = msg_end + 1; i < size; i++) {
            buf[i] = 0x00;
        }

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

        free(buf);
        success = true;

    } while(false);
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
