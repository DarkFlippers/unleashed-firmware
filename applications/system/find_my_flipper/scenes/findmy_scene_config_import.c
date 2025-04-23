#include "../findmy_i.h"

enum VarItemListIndex {
    VarItemListIndexNrfConnect,
    VarItemListIndexOpenHaystack,
    VarItemListIndexRegisterTagManually,
};

static const char* parse_nrf_connect(FindMy* app, const char* path) {
    const char* error = NULL;

    Stream* stream = file_stream_alloc(app->storage);
    FuriString* line = furi_string_alloc();
    do {
        // XX-XX-XX-XX-XX-XX_YYYY-MM-DD HH_MM_SS.txt
        error = "Filename must\nhave MAC\naddress";
        uint8_t mac[EXTRA_BEACON_MAC_ADDR_SIZE];
        path_extract_filename_no_ext(path, line);
        if(furi_string_size(line) < sizeof(mac) * 3 - 1) break;
        error = NULL;
        for(size_t i = 0; i < sizeof(mac); i++) {
            char a = furi_string_get_char(line, i * 3);
            char b = furi_string_get_char(line, i * 3 + 1);
            if((a < 'A' && a > 'F') || (a < '0' && a > '9') || (b < 'A' && b > 'F') ||
               (b < '0' && b > '9') || !hex_char_to_uint8(a, b, &mac[i])) {
                error = "Filename must\nhave MAC\naddress";
                break;
            }
        }
        if(error) break;
        findmy_reverse_mac_addr(mac);

        error = "Can't open file";
        if(!file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) break;

        // YYYY-MM-DD HH:MM:SS.ms, XX dBm, 0xXXXXX
        error = "Wrong file format";
        if(!stream_read_line(stream, line)) break;
        const char* marker = " dBm, 0x";
        size_t pos = furi_string_search(line, marker);
        if(pos == FURI_STRING_FAILURE) break;
        furi_string_right(line, pos + strlen(marker));
        furi_string_trim(line);

        error = "Wrong payload size";
        size_t line_size = furi_string_size(line);
        uint8_t data_size = findmy_state_data_size(app->state.tag_type);
        FURI_LOG_I("ImportPayload", "Line Size: %d", line_size);
        FURI_LOG_I("ImportPayload", "Data Size: %d", data_size * 2);
        if(line_size != data_size * 2) break;
        // Initialize full data to 0's, then fill only first data_size bytes
        uint8_t data[EXTRA_BEACON_MAX_DATA_SIZE] = {0};
        error = NULL;
        for(size_t i = 0; i < data_size; i++) {
            char a = furi_string_get_char(line, i * 2);
            char b = furi_string_get_char(line, i * 2 + 1);
            if((a < 'A' && a > 'F') || (a < '0' && a > '9') || (b < 'A' && b > 'F') ||
               (b < '0' && b > '9') || !hex_char_to_uint8(a, b, &data[i])) {
                error = "Invalid payload";
                break;
            }
        }
        if(error) break;

        memcpy(app->state.mac, mac, sizeof(app->state.mac));
        memcpy(app->state.data, data, sizeof(app->state.data));
        findmy_state_save_and_apply(&app->state);

        error = NULL;

    } while(false);
    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);

    return error;
}

static const char* parse_open_haystack(FindMy* app, const char* path) {
    const char* error = NULL;

    Stream* stream = file_stream_alloc(app->storage);
    FuriString* line = furi_string_alloc();
    do {
        error = "Can't open file";
        if(!file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) break;

        error = "Wrong file format";
        while(stream_read_line(stream, line)) {
            if(furi_string_start_with(line, "Public key: ") ||
               furi_string_start_with(line, "Advertisement key: ")) {
                error = NULL;
                break;
            }
        }
        if(error) break;

        furi_string_right(line, furi_string_search_char(line, ':') + 2);
        furi_string_trim(line);

        error = "Base64 failed";
        size_t decoded_len;
        uint8_t* public_key = base64_decode(
            (uint8_t*)furi_string_get_cstr(line), furi_string_size(line), &decoded_len);
        if(decoded_len != 28) {
            free(public_key);
            break;
        }

        memcpy(app->state.mac, public_key, sizeof(app->state.mac));
        app->state.mac[0] |= 0b11000000;
        findmy_reverse_mac_addr(app->state.mac);

        uint8_t advertisement_template[EXTRA_BEACON_MAX_DATA_SIZE] = {
            0x1e, // length (30)
            0xff, // manufacturer specific data
            0x4c, 0x00, // company ID (Apple)
            0x12, 0x19, // offline finding type and length
            0x00, //state
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, // first two bits of key[0]
            0x00, // hint
        };
        memcpy(app->state.data, advertisement_template, sizeof(app->state.data));
        memcpy(&app->state.data[7], &public_key[6], decoded_len - 6);
        app->state.data[29] = public_key[0] >> 6;
        findmy_state_save_and_apply(&app->state);

        free(public_key);
        error = NULL;

    } while(false);
    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);

    return error;
}

void findmy_scene_config_import_callback(void* context, uint32_t index) {
    furi_assert(context);
    FindMy* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void findmy_scene_config_import_on_enter(void* context) {
    FindMy* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    VariableItem* item;

    //variable_item_list_set_header(var_item_list, "Choose file type");

    item = variable_item_list_add(var_item_list, "nRF Connect (.txt)", 0, NULL, NULL);

    item = variable_item_list_add(var_item_list, "OpenHaystack (.keys)", 0, NULL, NULL);

    item = variable_item_list_add(var_item_list, "Register Tag Manually", 0, NULL, NULL);

    // This scene acts more like a submenu than a var item list tbh
    UNUSED(item);

    variable_item_list_set_enter_callback(var_item_list, findmy_scene_config_import_callback, app);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, FindMySceneConfigImport));

    view_dispatcher_switch_to_view(app->view_dispatcher, FindMyViewVarItemList);
}

bool findmy_scene_config_import_on_event(void* context, SceneManagerEvent event) {
    FindMy* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, FindMySceneConfigImport, event.event);
        consumed = true;

        const char* extension = NULL;
        switch(event.event) {
        case VarItemListIndexNrfConnect:
            extension = ".txt";
            break;
        case VarItemListIndexOpenHaystack:
            extension = ".keys";
            break;
        case VarItemListIndexRegisterTagManually:
            scene_manager_next_scene(app->scene_manager, FindMySceneConfigMac);
            break;
        default:
            break;
        }
        if(!extension) {
            return consumed;
        }

        const DialogsFileBrowserOptions browser_options = {
            .extension = extension,
            .icon = &I_text_10px,
            .base_path = FINDMY_STATE_DIR,
        };
        storage_simply_mkdir(app->storage, browser_options.base_path);
        FuriString* path = furi_string_alloc_set_str(browser_options.base_path);
        if(dialog_file_browser_show(app->dialogs, path, path, &browser_options)) {
            // The parse functions return the error text, or NULL for success
            // Used in result to show success or error message
            const char* error = NULL;
            switch(event.event) {
            case VarItemListIndexNrfConnect:
                error = parse_nrf_connect(app, furi_string_get_cstr(path));
                break;
            case VarItemListIndexOpenHaystack:
                error = parse_open_haystack(app, furi_string_get_cstr(path));
                break;
            }
            scene_manager_set_scene_state(
                app->scene_manager, FindMySceneConfigImportResult, (uint32_t)error);
            scene_manager_next_scene(app->scene_manager, FindMySceneConfigImportResult);
        }
        furi_string_free(path);
    }

    return consumed;
}

void findmy_scene_config_import_on_exit(void* context) {
    FindMy* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_reset(var_item_list);
}
