#include "../storage_move_to_sd.h"
#include <gui/canvas.h>
#include <gui/modules/widget_elements/widget_element_i.h>
#include <storage/storage.h>

static void storage_move_to_sd_scene_confirm_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    StorageMoveToSd* app = context;
    furi_assert(app);
    if(type == InputTypeShort) {
        if(result == GuiButtonTypeRight) {
            view_dispatcher_send_custom_event(app->view_dispatcher, MoveToSdCustomEventConfirm);
        } else if(result == GuiButtonTypeLeft) {
            view_dispatcher_send_custom_event(app->view_dispatcher, MoveToSdCustomEventExit);
        }
    }
}

void storage_move_to_sd_scene_confirm_on_enter(void* context) {
    StorageMoveToSd* app = context;

    widget_add_button_element(
        app->widget,
        GuiButtonTypeLeft,
        "Cancel",
        storage_move_to_sd_scene_confirm_widget_callback,
        app);
    widget_add_button_element(
        app->widget,
        GuiButtonTypeRight,
        "Confirm",
        storage_move_to_sd_scene_confirm_widget_callback,
        app);

    widget_add_string_element(
        app->widget, 64, 10, AlignCenter, AlignCenter, FontPrimary, "SD card inserted");
    widget_add_string_multiline_element(
        app->widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "Move data from\ninternal storage to SD card?");

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageMoveToSdViewWidget);
}

bool storage_move_to_sd_scene_confirm_on_event(void* context, SceneManagerEvent event) {
    StorageMoveToSd* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MoveToSdCustomEventConfirm) {
            scene_manager_next_scene(app->scene_manager, StorageMoveToSdProgress);
            consumed = true;
        } else if(event.event == MoveToSdCustomEventExit) {
            view_dispatcher_stop(app->view_dispatcher);
        }
    }

    return consumed;
}

void storage_move_to_sd_scene_confirm_on_exit(void* context) {
    StorageMoveToSd* app = context;
    widget_reset(app->widget);
}
