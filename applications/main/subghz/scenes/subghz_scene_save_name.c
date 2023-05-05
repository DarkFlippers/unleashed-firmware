#include "../subghz_i.h"
#include "subghz/types.h"
#include <lib/toolbox/random_name.h>
#include "../helpers/subghz_custom_event.h"
#include <lib/subghz/protocols/raw.h>
#include <gui/modules/validators.h>
#include <dolphin/dolphin.h>

#define MAX_TEXT_INPUT_LEN 23

void subghz_scene_save_name_text_input_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneSaveName);
}

void subghz_scene_save_name_get_timefilename(
    FuriString* name,
    const char* proto_name,
    bool fulldate) {
    FuriHalRtcDateTime datetime = {0};
    furi_hal_rtc_get_datetime(&datetime);
    if(fulldate) {
        furi_string_printf(
            name,
            "%s_%.4d%.2d%.2d-%.2d%.2d%.2d",
            proto_name,
            datetime.year,
            datetime.month,
            datetime.day,
            datetime.hour,
            datetime.minute,
            datetime.second);
    } else {
        furi_string_printf(
            name,
            "%s_%.2d%.2d-%.2d%.2d%.2d",
            proto_name,
            datetime.month,
            datetime.day,
            datetime.hour,
            datetime.minute,
            datetime.second);
    }
}

void subghz_scene_save_name_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    TextInput* text_input = subghz->text_input;
    bool dev_name_empty = false;

    FuriString* file_name = furi_string_alloc();
    FuriString* dir_name = furi_string_alloc();

    if(!subghz_path_is_file(subghz->file_path)) {
        char file_name_buf[SUBGHZ_MAX_LEN_NAME] = {0};
        if(furi_hal_subghz_get_timestamp_file_names()) {
            if(subghz->txrx->decoder_result != 0x0) {
                if(subghz->txrx->decoder_result != NULL) {
                    if(strlen(subghz->txrx->decoder_result->protocol->name) != 0) {
                        if(scene_manager_has_previous_scene(
                               subghz->scene_manager, SubGhzSceneSetType)) {
                            subghz_scene_save_name_get_timefilename(file_name, "S", true);
                        } else {
                            subghz_scene_save_name_get_timefilename(
                                file_name, subghz->txrx->decoder_result->protocol->name, false);
                        }

                    } else {
                        subghz_scene_save_name_get_timefilename(file_name, "S", true);
                    }
                } else {
                    subghz_scene_save_name_get_timefilename(file_name, "S", true);
                }
            } else {
                subghz_scene_save_name_get_timefilename(file_name, "S", true);
            }
        } else {
            set_random_name(file_name_buf, SUBGHZ_MAX_LEN_NAME);
            furi_string_set(file_name, file_name_buf);
        }
        furi_string_set(subghz->file_path, SUBGHZ_APP_FOLDER);
        //highlighting the entire filename by default
        dev_name_empty = true;
    } else {
        furi_string_reset(subghz->file_path_tmp);
        furi_string_set(subghz->file_path_tmp, subghz->file_path);
        path_extract_dirname(furi_string_get_cstr(subghz->file_path), dir_name);
        path_extract_filename(subghz->file_path, file_name, true);
        if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) !=
           SubGhzCustomEventManagerNoSet) {
            if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) ==
               SubGhzCustomEventManagerSetRAW) {
                dev_name_empty = true;
                subghz_scene_save_name_get_timefilename(file_name, "RAW", true);
            }
        }
        furi_string_set(subghz->file_path, dir_name);
    }

    strncpy(subghz->file_name_tmp, furi_string_get_cstr(file_name), SUBGHZ_MAX_LEN_NAME);
    text_input_set_header_text(text_input, "Name signal");
    text_input_set_result_callback(
        text_input,
        subghz_scene_save_name_text_input_callback,
        subghz,
        subghz->file_name_tmp,
        MAX_TEXT_INPUT_LEN,
        dev_name_empty);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(subghz->file_path), SUBGHZ_APP_EXTENSION, "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    furi_string_free(file_name);
    furi_string_free(dir_name);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdTextInput);
}

bool subghz_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        if(!(strcmp(subghz->file_name_tmp, "") == 0) ||
           scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) !=
               SubGhzCustomEventManagerNoSet) {
            if(!subghz->in_decoder_scene) {
                furi_string_set(subghz->file_path, subghz->file_path_tmp);
            }
        }
        scene_manager_previous_scene(subghz->scene_manager);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneSaveName) {
            if(strcmp(subghz->file_name_tmp, "") != 0) {
                furi_string_cat_printf(
                    subghz->file_path, "/%s%s", subghz->file_name_tmp, SUBGHZ_APP_EXTENSION);
                if(subghz_path_is_file(subghz->file_path_tmp)) {
                    if(!subghz_rename_file(subghz)) {
                        return false;
                    }
                } else {
                    if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType) !=
                       SubGhzCustomEventManagerNoSet) {
                        subghz_save_protocol_to_file(
                            subghz,
                            subghz->txrx->fff_data,
                            furi_string_get_cstr(subghz->file_path));
                        scene_manager_set_scene_state(
                            subghz->scene_manager,
                            SubGhzSceneSetType,
                            SubGhzCustomEventManagerNoSet);
                    } else {
                        subghz_save_protocol_to_file(
                            subghz,
                            subghz_history_get_raw_data(
                                subghz->txrx->history, subghz->txrx->idx_menu_chosen),
                            furi_string_get_cstr(subghz->file_path));
                    }
                }

                if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) !=
                   SubGhzCustomEventManagerNoSet) {
                    subghz_protocol_raw_gen_fff_data(
                        subghz->txrx->fff_data, furi_string_get_cstr(subghz->file_path));
                    scene_manager_set_scene_state(
                        subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerNoSet);
                } else {
                    subghz_file_name_clear(subghz);
                }

                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveSuccess);
                if(scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneSavedMenu)) {
                    // Nothing, do not count editing as saving
                } else if(scene_manager_has_previous_scene(
                              subghz->scene_manager, SubGhzSceneMoreRAW)) {
                    // Ditto, for RAW signals
                } else if(scene_manager_has_previous_scene(
                              subghz->scene_manager, SubGhzSceneSetType)) {
                    DOLPHIN_DEED(DolphinDeedSubGhzAddManually);
                } else {
                    DOLPHIN_DEED(DolphinDeedSubGhzSave);
                }
                return true;
            } else {
                furi_string_set(subghz->error_str, "No name file");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
                return true;
            }
        }
    }
    return false;
}

void subghz_scene_save_name_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear validator
    void* validator_context = text_input_get_validator_callback_context(subghz->text_input);
    text_input_set_validator(subghz->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    // Clear view
    text_input_reset(subghz->text_input);
}