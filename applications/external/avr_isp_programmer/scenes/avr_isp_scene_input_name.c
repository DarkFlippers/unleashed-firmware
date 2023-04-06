#include "../avr_isp_app_i.h"
#include <gui/modules/validators.h>

#define MAX_TEXT_INPUT_LEN 22

void avr_isp_scene_input_name_text_callback(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, AvrIspCustomEventSceneInputName);
}

void avr_isp_scene_input_name_get_timefilename(FuriString* name) {
    FuriHalRtcDateTime datetime = {0};
    furi_hal_rtc_get_datetime(&datetime);
    furi_string_printf(
        name,
        "AVR_dump-%.4d%.2d%.2d-%.2d%.2d%.2d",
        datetime.year,
        datetime.month,
        datetime.day,
        datetime.hour,
        datetime.minute,
        datetime.second);
}

void avr_isp_scene_input_name_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    // Setup view
    TextInput* text_input = app->text_input;
    bool dev_name_empty = false;

    FuriString* file_name = furi_string_alloc();

    avr_isp_scene_input_name_get_timefilename(file_name);
    furi_string_set(app->file_path, STORAGE_APP_DATA_PATH_PREFIX);
    //highlighting the entire filename by default
    dev_name_empty = true;

    strncpy(app->file_name_tmp, furi_string_get_cstr(file_name), AVR_ISP_MAX_LEN_NAME);
    text_input_set_header_text(text_input, "Name dump");
    text_input_set_result_callback(
        text_input,
        avr_isp_scene_input_name_text_callback,
        app,
        app->file_name_tmp,
        MAX_TEXT_INPUT_LEN, // buffer size
        dev_name_empty);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(STORAGE_APP_DATA_PATH_PREFIX, AVR_ISP_APP_EXTENSION, "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    furi_string_free(file_name);

    view_dispatcher_switch_to_view(app->view_dispatcher, AvrIspViewTextInput);
}

bool avr_isp_scene_input_name_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    AvrIspApp* app = context;
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == AvrIspCustomEventSceneInputName) {
            if(strcmp(app->file_name_tmp, "") != 0) {
                scene_manager_next_scene(app->scene_manager, AvrIspSceneReader);
            } else {
            }
        }
    }
    return false;
}

void avr_isp_scene_input_name_on_exit(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    // Clear validator
    void* validator_context = text_input_get_validator_callback_context(app->text_input);
    text_input_set_validator(app->text_input, NULL, NULL);
    validator_is_file_free(validator_context);
    // Clear view
    text_input_reset(app->text_input);
}
