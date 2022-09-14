#include "../updater_i.h"
#include "updater_scene.h"
#include <update_util/update_operation.h>

void updater_scene_error_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    Updater* updater = context;
    if(type != InputTypeShort) {
        return;
    }

    if(result == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(
            updater->view_dispatcher, UpdaterCustomEventCancelUpdate);
    }
}

void updater_scene_error_on_enter(void* context) {
    Updater* updater = (Updater*)context;

    widget_add_button_element(
        updater->widget, GuiButtonTypeLeft, "Exit", updater_scene_error_callback, updater);

    widget_add_string_multiline_element(
        updater->widget, 64, 13, AlignCenter, AlignCenter, FontPrimary, "Error");

    widget_add_string_multiline_element(
        updater->widget,
        64,
        33,
        AlignCenter,
        AlignCenter,
        FontPrimary,
        update_operation_describe_preparation_result(updater->preparation_result));

    view_dispatcher_switch_to_view(updater->view_dispatcher, UpdaterViewWidget);
}

bool updater_scene_error_on_event(void* context, SceneManagerEvent event) {
    Updater* updater = (Updater*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        view_dispatcher_stop(updater->view_dispatcher);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case UpdaterCustomEventCancelUpdate:
            view_dispatcher_stop(updater->view_dispatcher);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

void updater_scene_error_on_exit(void* context) {
    Updater* updater = (Updater*)context;

    widget_reset(updater->widget);
    free(updater->pending_update);
}
