#include "../updater_i.h"
#include "updater_scene.h"

#include <update_util/update_operation.h>
#include <furi_hal.h>

void updater_scene_loadcfg_apply_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    Updater* updater = context;
    if(type != InputTypeShort) {
        return;
    }

    if(result == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(updater->view_dispatcher, UpdaterCustomEventStartUpdate);
    } else if(result == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(
            updater->view_dispatcher, UpdaterCustomEventCancelUpdate);
    }
}

void updater_scene_loadcfg_on_enter(void* context) {
    Updater* updater = (Updater*)context;
    UpdateManifest* loaded_manifest = updater->loaded_manifest = update_manifest_alloc();

    if(update_manifest_init(loaded_manifest, furi_string_get_cstr(updater->startup_arg))) {
        widget_add_string_element(
            updater->widget, 64, 12, AlignCenter, AlignCenter, FontPrimary, "Update");

        widget_add_text_box_element(
            updater->widget,
            5,
            20,
            118,
            32,
            AlignCenter,
            AlignCenter,
            furi_string_get_cstr(loaded_manifest->version),
            true);

        widget_add_button_element(
            updater->widget,
            GuiButtonTypeRight,
            "Install",
            updater_scene_loadcfg_apply_callback,
            updater);
    } else {
        widget_add_string_element(
            updater->widget, 64, 24, AlignCenter, AlignCenter, FontPrimary, "Invalid manifest");
    }

    widget_add_button_element(
        updater->widget,
        GuiButtonTypeLeft,
        "Cancel",
        updater_scene_loadcfg_apply_callback,
        updater);

    view_dispatcher_switch_to_view(updater->view_dispatcher, UpdaterViewWidget);
}

bool updater_scene_loadcfg_on_event(void* context, SceneManagerEvent event) {
    Updater* updater = (Updater*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        view_dispatcher_stop(updater->view_dispatcher);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case UpdaterCustomEventStartUpdate:
            updater->preparation_result =
                update_operation_prepare(furi_string_get_cstr(updater->startup_arg));
            if(updater->preparation_result == UpdatePrepareResultOK) {
                furi_hal_power_reset();
            } else {
#ifndef FURI_RAM_EXEC
                scene_manager_next_scene(updater->scene_manager, UpdaterSceneError);
#endif
            }
            consumed = true;
            break;
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

void updater_scene_loadcfg_on_exit(void* context) {
    furi_assert(context);
    Updater* updater = (Updater*)context;

    widget_reset(updater->widget);

    if(updater->loaded_manifest) {
        update_manifest_free(updater->loaded_manifest);
    }
}
