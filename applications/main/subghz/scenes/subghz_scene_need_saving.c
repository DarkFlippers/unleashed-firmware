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

    widget_add_text_box_element(
        subghz->widget,
        0,
        0,
        128,
        54,
        AlignCenter,
        AlignTop,
        "\e#Exit to Sub-GHz Menu?\e#\nAll unsaved data will be lost",
        false);

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
                subghz_set_default_preset(subghz);
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
