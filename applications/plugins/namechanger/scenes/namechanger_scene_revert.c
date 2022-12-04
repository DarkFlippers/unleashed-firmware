#include "../namechanger.h"

static void
    namechanger_scene_revert_widget_callback(GuiButtonType result, InputType type, void* context) {
    NameChanger* namechanger = context;
    FURI_LOG_I(TAG, "revert1");
    if(type == InputTypeShort) {
        FURI_LOG_I(TAG, "revert2");
        view_dispatcher_send_custom_event(namechanger->view_dispatcher, result);
    }
}

void namechanger_scene_revert_on_enter(void* context) {
    NameChanger* namechanger = context;
    Widget* widget = namechanger->widget;
    FURI_LOG_I(TAG, "revert3");
    widget_add_text_box_element(
        widget, 0, 0, 128, 25, AlignCenter, AlignCenter, "\e#Revert Name?\e#", false);
    widget_add_icon_element(widget, 48, 20, &I_MarioBlock);
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Cancel", namechanger_scene_revert_widget_callback, namechanger);
    FURI_LOG_I(TAG, "revert4");
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Revert",
        namechanger_scene_revert_widget_callback,
        namechanger);
    FURI_LOG_I(TAG, "revert5");
    view_dispatcher_switch_to_view(namechanger->view_dispatcher, NameChangerViewWidget);
}

bool namechanger_scene_revert_on_event(void* context, SceneManagerEvent event) {
    NameChanger* namechanger = context;
    bool consumed = false;
    FURI_LOG_I(TAG, "revert6");
    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        FURI_LOG_I(TAG, "revert7");
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        FURI_LOG_I(TAG, "revert8");
        if(event.event == GuiButtonTypeRight) {
            FURI_LOG_I(TAG, "revert9");
            if(namechanger_name_read_write(namechanger, "eraseerase", 3)) {
                FURI_LOG_I(TAG, "revert10");
                scene_manager_next_scene(
                    namechanger->scene_manager, NameChangerSceneRevertSuccess);
            } else {
                FURI_LOG_I(TAG, "revert11");
                scene_manager_search_and_switch_to_previous_scene(
                    namechanger->scene_manager, NameChangerSceneError);
            }
        } else if(event.event == GuiButtonTypeLeft) {
            FURI_LOG_I(TAG, "revert12");
            scene_manager_search_and_switch_to_previous_scene(
                namechanger->scene_manager, NameChangerSceneStart);
        }
    }
    FURI_LOG_I(TAG, "revert13");
    return consumed;
}

void namechanger_scene_revert_on_exit(void* context) {
    NameChanger* namechanger = context;
    FURI_LOG_I(TAG, "revert14");
    widget_reset(namechanger->widget);
}
