#include "../ibutton_i.h"

#include <ibutton/ibutton_settings.h>

static inline iButtonCustomEvent
    ibutton_scene_write_to_custom_event(iButtonWorkerWriteResult result) {
    switch(result) {
    case iButtonWorkerWriteOK:
        return iButtonCustomEventWorkerWriteOK;
    case iButtonWorkerWriteSameKey:
        return iButtonCustomEventWorkerWriteSameKey;
    case iButtonWorkerWriteNoDetect:
        return iButtonCustomEventWorkerWriteNoDetect;
    case iButtonWorkerWriteCannotWrite:
        return iButtonCustomEventWorkerWriteCannotWrite;
    case iButtonWorkerWriteStartTarget:
        return iButtonCustomEventWorkerWriteStartTarget;
    case iButtonWorkerWriteNoEnabledTarget:
        return iButtonCustomEventWorkerWriteNoEnabledTarget;
    default:
        furi_crash();
    }
}

// Redraw the write screen: "[<proto>]\n<source>", plus "\n(<blank>)" once a specific blank
// type is being attempted.
static void ibutton_scene_write_draw(iButton* ibutton, const char* target) {
    Widget* widget = ibutton->widget;
    const char* proto =
        ibutton_protocols_get_name(ibutton->protocols, ibutton_key_get_protocol_id(ibutton->key));
    const char* source = furi_string_empty(ibutton->file_path) ? "Unsaved Key" : ibutton->key_name;

    if(target) {
        snprintf(
            ibutton->text_store, IBUTTON_TEXT_STORE_SIZE, "[%s]\n%s\n(%s)", proto, source, target);
    } else {
        snprintf(ibutton->text_store, IBUTTON_TEXT_STORE_SIZE, "[%s]\n%s", proto, source);
    }

    widget_reset(widget);
    widget_add_icon_element(widget, 3, 10, &I_iButtonKey_49x44);
    widget_add_text_box_element(
        widget, 52, 24, 75, 40, AlignCenter, AlignTop, ibutton->text_store, true);
    widget_add_string_multiline_element(
        widget,
        88,
        5,
        AlignCenter,
        AlignTop,
        FontPrimary,
        ibutton->write_mode == iButtonWriteModeCopy ? "Full Writing" : "Writing ID");
}

// Replaces the writing screen once the worker has given up, so it stays until the user
// backs out.
static void ibutton_scene_write_show_error(iButton* ibutton, const char* text) {
    widget_reset(ibutton->widget);
    widget_add_string_element(
        ibutton->widget, 64, 4, AlignCenter, AlignTop, FontPrimary, "Cannot Write");
    widget_add_text_box_element(
        ibutton->widget, 0, 20, 128, 44, AlignCenter, AlignTop, text, false);
}

static void ibutton_scene_write_callback(void* context, iButtonWorkerWriteResult result) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(
        ibutton->view_dispatcher, ibutton_scene_write_to_custom_event(result));
}

void ibutton_scene_write_on_enter(void* context) {
    iButton* ibutton = context;
    furi_assert(ibutton->write_mode != iButtonWriteModeInvalid);

    iButtonKey* key = ibutton->key;
    iButtonWorker* worker = ibutton->worker;
    ibutton_scene_write_draw(ibutton, NULL);

    ibutton_worker_write_set_callback(worker, ibutton_scene_write_callback, ibutton);

    // Kept as scene state so the no-target message can tell "none enabled at all" from "none
    // that fit this key" without re-reading the settings file.
    const iButtonWriteTargetMask enabled = ibutton_settings_get_write_targets();
    scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneWrite, enabled);
    ibutton_worker_set_write_targets(worker, enabled);

    if(ibutton->write_mode == iButtonWriteModeId) {
        ibutton_worker_write_id_start(worker, key);
    } else if(ibutton->write_mode == iButtonWriteModeCopy) {
        ibutton_worker_write_copy_start(worker, key);
    }

    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
}

bool ibutton_scene_write_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if((event.event == iButtonCustomEventWorkerWriteOK) ||
           (event.event == iButtonCustomEventWorkerWriteSameKey)) {
            scene_manager_next_scene(scene_manager, iButtonSceneWriteSuccess);
        } else if(event.event == iButtonCustomEventWorkerWriteNoDetect) {
            ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateBlink);
        } else if(event.event == iButtonCustomEventWorkerWriteCannotWrite) {
            ibutton_notification_message(ibutton, iButtonNotificationMessageYellowBlink);
        } else if(event.event == iButtonCustomEventWorkerWriteStartTarget) {
            ibutton_scene_write_draw(ibutton, ibutton_worker_get_write_chip_name(ibutton->worker));
        } else if(event.event == iButtonCustomEventWorkerWriteNoEnabledTarget) {
            // Same remedy either way, but naming the emptier case saves a puzzled trip to a
            // settings screen the user may have switched fully off on purpose.
            const bool none_enabled =
                scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneWrite) == 0;
            ibutton_scene_write_show_error(
                ibutton,
                none_enabled ? "No blanks enabled.\nEnable one in\nSettings" :
                               "No enabled blank\ncan write this key");
            ibutton_notification_message(ibutton, iButtonNotificationMessageYellowBlink);
        }
    }

    return consumed;
}

void ibutton_scene_write_on_exit(void* context) {
    iButton* ibutton = context;
    ibutton->write_mode = iButtonWriteModeInvalid;

    ibutton_worker_stop(ibutton->worker);
    widget_reset(ibutton->widget);

    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
