#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_need_saving_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneStay);
    } else if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneExit);
    }
}

void subghz_scene_need_saving_on_enter(void* context) {
    SubGhz* subghz = context;

    widget_add_string_multiline_element(
        subghz->widget, 64, 13, AlignCenter, AlignCenter, FontPrimary, "Exit to Sub-GHz Menu?");
    widget_add_string_multiline_element(
        subghz->widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "All unsaved data\nwill be lost!");

    widget_add_button_element(
        subghz->widget, GuiButtonTypeRight, "Stay", subghz_scene_need_saving_callback, subghz);
    widget_add_button_element(
        subghz->widget, GuiButtonTypeLeft, "Exit", subghz_scene_need_saving_callback, subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
}

bool subghz_scene_need_saving_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateBack);
        scene_manager_previous_scene(subghz->scene_manager);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneStay) {
            subghz_rx_key_state_set(subghz, SubGhzRxKeyStateBack);
            scene_manager_previous_scene(subghz->scene_manager);
            return true;
        } else if(event.event == SubGhzCustomEventSceneExit) {
            SubGhzRxKeyState state = subghz_rx_key_state_get(subghz);
            subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);

            if(state == SubGhzRxKeyStateExit) {
                if(scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneReadRAW)) {
                    if(!furi_string_empty(subghz->file_path_tmp)) {
                        subghz_delete_file(subghz);
                    }
                }

                subghz_txrx_set_preset(
                    subghz->txrx, "AM650", subghz->last_settings->frequency, NULL, 0);
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            } else {
                scene_manager_previous_scene(subghz->scene_manager);
            }

            return true;
        }
    }
    return false;
}

void subghz_scene_need_saving_on_exit(void* context) {
    SubGhz* subghz = context;
    widget_reset(subghz->widget);
}
