#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_show_error_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubghzCustomEventSceneShowErrorOk);
    } else if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubghzCustomEventSceneShowErrorBack);
    }
}

void subghz_scene_show_error_on_enter(void* context) {
    SubGhz* subghz = context;

    widget_add_icon_element(subghz->widget, 0, 0, &I_SDQuestion_35x43);

    widget_add_string_multiline_element(
        subghz->widget,
        81,
        24,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        string_get_cstr(subghz->error_str));
    if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneShowError) ==
       SubghzCustomEventManagerSet) {
        widget_add_button_element(
            subghz->widget, GuiButtonTypeRight, "Ok", subghz_scene_show_error_callback, subghz);
    }

    widget_add_button_element(
        subghz->widget, GuiButtonTypeLeft, "Back", subghz_scene_show_error_callback, subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewWidget);
}

bool subghz_scene_show_error_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneShowError) ==
           SubghzCustomEventManagerSet) {
            return false;
        } else {
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
        }
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzCustomEventSceneShowErrorOk) {
            if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneShowError) ==
               SubghzCustomEventManagerSet) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStart);
            }
            return true;
        } else if(event.event == SubghzCustomEventSceneShowErrorBack) {
            if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneShowError) ==
               SubghzCustomEventManagerSet) {
                //exit app
                if(!scene_manager_previous_scene(subghz->scene_manager)) {
                    scene_manager_stop(subghz->scene_manager);
                    view_dispatcher_stop(subghz->view_dispatcher);
                }
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            }
            return true;
        }
    }
    return false;
}

void subghz_scene_show_error_on_exit(void* context) {
    SubGhz* subghz = context;
    scene_manager_set_scene_state(
        subghz->scene_manager, SubGhzSceneShowError, SubghzCustomEventManagerNoSet);
    widget_clear(subghz->widget);
    string_reset(subghz->error_str);
}
