#include "../ibutton_i.h"

typedef enum {
    iButtonSceneWriteStateDefault,
    iButtonSceneWriteStateBlinkYellow,
} iButtonSceneWriteState;

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
    default:
        furi_crash();
    }
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
    const iButtonProtocolId protocol_id = ibutton_key_get_protocol_id(key);

    Widget* widget = ibutton->widget;
    FuriString* tmp = furi_string_alloc();

    widget_add_icon_element(widget, 3, 10, &I_iButtonKey_49x44);

    if(furi_string_empty(ibutton->file_path)) {
        furi_string_printf(
            tmp, "Unsaved\n%s", ibutton_protocols_get_name(ibutton->protocols, protocol_id));
    } else {
        furi_string_printf(tmp, "%s", ibutton->key_name);
    }

    widget_add_text_box_element(
        widget, 52, 23, 75, 26, AlignCenter, AlignTop, furi_string_get_cstr(tmp), false);

    ibutton_worker_write_set_callback(worker, ibutton_scene_write_callback, ibutton);

    if(ibutton->write_mode == iButtonWriteModeId) {
        furi_string_set(tmp, "Writing ID");
        ibutton_worker_write_id_start(worker, key);

    } else if(ibutton->write_mode == iButtonWriteModeCopy) {
        furi_string_set(tmp, "Full Writing");
        ibutton_worker_write_copy_start(worker, key);
    }

    widget_add_string_multiline_element(
        widget, 88, 10, AlignCenter, AlignTop, FontPrimary, furi_string_get_cstr(tmp));

    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    furi_string_free(tmp);
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
