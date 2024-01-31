#include "../subghz_i.h"
#include "subghz/types.h"
#include "../helpers/subghz_custom_event.h"
#include <lib/subghz/protocols/raw.h>
#include <gui/modules/validators.h>
#include <dolphin/dolphin.h>
#include <toolbox/name_generator.h>

void subghz_scene_save_name_text_input_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneSaveName);
}

void subghz_scene_save_name_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    TextInput* text_input = subghz->text_input;
    bool dev_name_empty = false;

    FuriString* file_name = furi_string_alloc();
    FuriString* dir_name = furi_string_alloc();

    char file_name_buf[SUBGHZ_MAX_LEN_NAME] = {0};
    FuriHalRtcDateTime* datetime = subghz->save_datetime_set ? &subghz->save_datetime : NULL;
    subghz->save_datetime_set = false;
    if(!subghz_path_is_file(subghz->file_path)) {
        SubGhzProtocolDecoderBase* decoder_result = subghz_txrx_get_decoder(subghz->txrx);

        bool skip_dec_is_present = false;
        if(decoder_result != 0x0) {
            if(decoder_result != NULL) {
                if(strlen(decoder_result->protocol->name) != 0 &&
                   subghz->last_settings->timestamp_file_names) {
                    if(!scene_manager_has_previous_scene(
                           subghz->scene_manager, SubGhzSceneSetType)) {
                        name_generator_make_auto_datetime(
                            file_name_buf,
                            SUBGHZ_MAX_LEN_NAME,
                            decoder_result->protocol->name,
                            datetime);
                        skip_dec_is_present = true;
                    }
                }
            }
        }
        if(!skip_dec_is_present) {
            name_generator_make_auto_datetime(file_name_buf, SUBGHZ_MAX_LEN_NAME, NULL, datetime);
        }
        furi_string_set(file_name, file_name_buf);
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
                name_generator_make_auto_datetime(
                    file_name_buf, SUBGHZ_MAX_LEN_NAME, "RAW", datetime);
                furi_string_set(file_name, file_name_buf);
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
        SUBGHZ_MAX_LEN_NAME,
        dev_name_empty);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(subghz->file_path), SUBGHZ_APP_FILENAME_EXTENSION, "");
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
            if(!scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneDecodeRAW)) {
                furi_string_set(subghz->file_path, subghz->file_path_tmp);
            }
        }
        if(scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneSetSeed)) {
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneSetType);
        } else {
            scene_manager_previous_scene(subghz->scene_manager);
        }
        // Set file path to default
        furi_string_set(subghz->file_path, SUBGHZ_APP_FOLDER);

        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneSaveName) {
            if(strcmp(subghz->file_name_tmp, "") != 0) {
                furi_string_cat_printf(
                    subghz->file_path,
                    "/%s%s",
                    subghz->file_name_tmp,
                    SUBGHZ_APP_FILENAME_EXTENSION);
                if(subghz_path_is_file(subghz->file_path_tmp)) {
                    if(!subghz_rename_file(subghz)) {
                        return false;
                    }
                } else {
                    if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType) !=
                       SubGhzCustomEventManagerNoSet) {
                        subghz_save_protocol_to_file(
                            subghz,
                            subghz_txrx_get_fff_data(subghz->txrx),
                            furi_string_get_cstr(subghz->file_path));
                        scene_manager_set_scene_state(
                            subghz->scene_manager,
                            SubGhzSceneSetType,
                            SubGhzCustomEventManagerNoSet);
                    } else {
                        subghz_save_protocol_to_file(
                            subghz,
                            subghz_history_get_raw_data(subghz->history, subghz->idx_menu_chosen),
                            furi_string_get_cstr(subghz->file_path));
                    }
                }

                if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) !=
                   SubGhzCustomEventManagerNoSet) {
                    subghz_protocol_raw_gen_fff_data(
                        subghz_txrx_get_fff_data(subghz->txrx),
                        furi_string_get_cstr(subghz->file_path),
                        subghz_txrx_radio_device_get_name(subghz->txrx));
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
                    dolphin_deed(DolphinDeedSubGhzAddManually);
                } else {
                    dolphin_deed(DolphinDeedSubGhzSave);
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