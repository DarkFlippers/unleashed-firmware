#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_need_saving_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubghzCustomEventSceneNeedSavingYes);
    } else if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubghzCustomEventSceneNeedSavingNo);
    }
}

void subghz_scene_need_saving_on_enter(void* context) {
    SubGhz* subghz = context;

    widget_add_string_multiline_element(
        subghz->widget,
        64,
        25,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "There is an unsaved data.\nDo you want to save it?");

    widget_add_button_element(
        subghz->widget, GuiButtonTypeRight, "Save", subghz_scene_need_saving_callback, subghz);
    widget_add_button_element(
        subghz->widget, GuiButtonTypeLeft, "Delete", subghz_scene_need_saving_callback, subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewWidget);
}

bool subghz_scene_need_saving_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzCustomEventSceneNeedSavingYes) {
            subghz->txrx->rx_key_state = SubGhzRxKeyStateNeedSave;
            scene_manager_previous_scene(subghz->scene_manager);
            return true;
        } else if(event.event == SubghzCustomEventSceneNeedSavingNo) {
            if(subghz->txrx->rx_key_state == SubGhzRxKeyStateExit) {
                subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            } else {
                subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
                scene_manager_previous_scene(subghz->scene_manager);
            }

            return true;
        }
    }
    return false;
}

void subghz_scene_need_saving_on_exit(void* context) {
    SubGhz* subghz = context;
    widget_clear(subghz->widget);
}
