#include "../ibutton_i.h"
#include <core/log.h>
#include <toolbox/path.h>

#define EMULATE_TIMEOUT_TICKS 10

static void ibutton_scene_emulate_callback(void* context, bool emulated) {
    iButton* ibutton = context;
    if(emulated) {
        view_dispatcher_send_custom_event(
            ibutton->view_dispatcher, iButtonCustomEventWorkerEmulated);
    }
}

void ibutton_scene_emulate_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;

    Widget* widget = ibutton->widget;
    FuriString* tmp = furi_string_alloc();

    widget_add_icon_element(widget, 3, 10, &I_iButtonKey_49x44);

    if(furi_string_empty(ibutton->file_path)) {
        furi_string_printf(
            tmp,
            "Unsaved\n%s",
            ibutton_protocols_get_name(ibutton->protocols, ibutton_key_get_protocol_id(key)));
    } else {
        furi_string_printf(tmp, "%s", ibutton->key_name);
    }

    widget_add_text_box_element(
        widget, 52, 23, 75, 26, AlignCenter, AlignTop, furi_string_get_cstr(tmp), false);

    widget_add_string_multiline_element(
        widget, 88, 10, AlignCenter, AlignTop, FontPrimary, "Emulating");

    ibutton_worker_emulate_set_callback(ibutton->worker, ibutton_scene_emulate_callback, ibutton);
    ibutton_worker_emulate_start(ibutton->worker, key);

    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    furi_string_free(tmp);
}

bool ibutton_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        uint32_t cnt = scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneEmulate);
        if(cnt > 0) {
            if(--cnt == 0) {
                ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateBlink);
            }
            scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneEmulate, cnt);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventWorkerEmulated) {
            if(scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneEmulate) == 0) {
                ibutton_notification_message(ibutton, iButtonNotificationMessageYellowBlink);
            }
            scene_manager_set_scene_state(
                ibutton->scene_manager, iButtonSceneEmulate, EMULATE_TIMEOUT_TICKS);
        }
    }

    return consumed;
}

void ibutton_scene_emulate_on_exit(void* context) {
    iButton* ibutton = context;
    ibutton_worker_stop(ibutton->worker);
    widget_reset(ibutton->widget);
    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
