#include "subbrute_scene_load_file.h"
#include "subbrute_scene_entrypoint.h"
#include "../subbrute_utils.h"
#include <lib/subghz/protocols/registry.h>

#define SUBGHZ_APP_PATH_FOLDER "/ext/subghz"

bool subbrute_load(SubBruteState* context, const char* file_path) {
    bool result = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data32;

    do {
        if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
            FURI_LOG_E(TAG, "Error open file %s", file_path);
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Error open file");
            break;
        }
        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect header");
            break;
        }
        // Frequency
        if(flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
            FURI_LOG_I(TAG, "Frequency: %d", temp_data32);
            context->frequency = temp_data32;
            if(!subbrute_is_frequency_allowed(context)) {
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Missing or incorrect Frequency");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect Frequency");
            break;
        }
        // Preset
        if(!flipper_format_read_string(fff_data_file, "Preset", context->preset)) {
            FURI_LOG_E(TAG, "Preset FAIL");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Preset FAIL");
        }
        // Protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", context->protocol)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing Protocol");
            break;
        } else {
            FURI_LOG_I(TAG, "Protocol: %s", string_get_cstr(context->protocol));
        }

        if(strcmp(string_get_cstr(context->protocol), "RAW") == 0) {
            FURI_LOG_E(TAG, "RAW unsupported");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "RAW unsupported");
            break;
        }

        const SubGhzProtocol* registry =
            subghz_protocol_registry_get_by_name(string_get_cstr(context->protocol));

        if(registry && registry->type == SubGhzProtocolTypeDynamic) {
            FURI_LOG_D(TAG, "Protocol is dynamic - not supported");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Dynamic protocol unsupported");
            break;
        }

        context->decoder_result = subghz_receiver_search_decoder_base_by_name(
            context->receiver, string_get_cstr(context->protocol));

        if(context->decoder_result) {
            FURI_LOG_I(TAG, "Found decoder");
        } else {
            FURI_LOG_E(TAG, "Protocol not found");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Protocol not found");
            break;
        }

        // Bit
        if(!flipper_format_read_uint32(fff_data_file, "Bit", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing or incorrect Bit");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect Bit");
            break;
        } else {
            FURI_LOG_I(TAG, "Bit: %d", temp_data32);
            context->bit = temp_data32;
        }

        // Key
        if(!flipper_format_read_string(fff_data_file, "Key", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Key");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect Key");
            break;
        } else {
            FURI_LOG_I(TAG, "Key: %s", string_get_cstr(temp_str));
            string_set(context->key, string_get_cstr(temp_str));
        }

        // TE
        if(!flipper_format_read_uint32(fff_data_file, "TE", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing or incorrect TE");
            //string_reset(context->notification_msg);
            //string_set_str(context->notification_msg, "Missing or incorrect TE");
            //break;
        } else {
            FURI_LOG_I(TAG, "TE: %d", temp_data32);
            context->te = temp_data32;
        }

        // Repeat
        if(flipper_format_read_uint32(fff_data_file, "Repeat", &temp_data32, 1)) {
            FURI_LOG_I(TAG, "Repeat: %d", temp_data32);
            context->repeat = temp_data32;
        } else {
            FURI_LOG_I(TAG, "Repeat: 3 (default)");
            context->repeat = 3;
        }

        result = true;
    } while(0);

    string_clear(temp_str);
    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);
    if(result) {
        FURI_LOG_I(TAG, "Loaded successfully");
        string_reset(context->notification_msg);
        string_set_str(context->notification_msg, "File looks ok.");
    }

    return result;
}

void subbrute_scene_load_file_on_enter(SubBruteState* context) {
    if(subbrute_load_protocol_from_file(context)) {
        context->current_scene = SceneSelectField;
    } else {
        subbrute_scene_entrypoint_on_enter(context);
        context->current_scene = SceneEntryPoint;
    }
}

void subbrute_scene_load_file_on_exit(SubBruteState* context) {
    UNUSED(context);
}

void subbrute_scene_load_file_on_tick(SubBruteState* context) {
    UNUSED(context);
}

void subbrute_scene_load_file_on_event(SubBruteEvent event, SubBruteState* context) {
    UNUSED(context);
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
            case InputKeyUp:
            case InputKeyLeft:
            case InputKeyRight:
            case InputKeyOk:
                break;
            case InputKeyBack:
                context->current_scene = SceneEntryPoint;
                break;
            }
        }
    }
}

void subbrute_scene_load_file_on_draw(Canvas* canvas, SubBruteState* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    //canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignTop, "SubGHz Fuzzer");
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, "Error: Press back");
}

bool subbrute_load_protocol_from_file(SubBruteState* context) {
    string_t file_path;
    string_init(file_path);
    string_set_str(file_path, SUBGHZ_APP_PATH_FOLDER);
    context->environment = subghz_environment_alloc();
    context->receiver = subghz_receiver_alloc_init(context->environment);
    subghz_receiver_set_filter(context->receiver, SubGhzProtocolFlag_Decodable);

    // Input events and views are managed by file_select

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBGHZ_APP_EXTENSION, &I_sub1_10px);

    bool res = dialog_file_browser_show(context->dialogs, file_path, file_path, &browser_options);

    if(res) {
        res = subbrute_load(context, string_get_cstr(file_path));
    }

    subghz_environment_free(context->environment);
    subghz_receiver_free(context->receiver);

    string_clear(file_path);

    return res;
}