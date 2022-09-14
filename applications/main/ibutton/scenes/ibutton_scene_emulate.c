#include "../ibutton_i.h"
#include <core/log.h>
#include <dolphin/dolphin.h>
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
    Popup* popup = ibutton->popup;
    iButtonKey* key = ibutton->key;

    const uint8_t* key_data = ibutton_key_get_data_p(key);

    string_t key_name;
    string_init(key_name);
    if(string_end_with_str_p(ibutton->file_path, IBUTTON_APP_EXTENSION)) {
        path_extract_filename(ibutton->file_path, key_name, true);
    }

    uint8_t line_count = 2;
    DOLPHIN_DEED(DolphinDeedIbuttonEmulate);

    // check that stored key has name
    if(!string_empty_p(key_name)) {
        ibutton_text_store_set(ibutton, "emulating\n%s", string_get_cstr(key_name));
        line_count = 2;
    } else {
        // if not, show key data
        switch(ibutton_key_get_type(key)) {
        case iButtonKeyDS1990:
            ibutton_text_store_set(
                ibutton,
                "emulating\n%02X %02X %02X %02X\n%02X %02X %02X %02X",
                key_data[0],
                key_data[1],
                key_data[2],
                key_data[3],
                key_data[4],
                key_data[5],
                key_data[6],
                key_data[7]);
            line_count = 3;
            break;
        case iButtonKeyCyfral:
            ibutton_text_store_set(ibutton, "emulating\n%02X %02X", key_data[0], key_data[1]);
            line_count = 2;
            break;
        case iButtonKeyMetakom:
            ibutton_text_store_set(
                ibutton,
                "emulating\n%02X %02X %02X %02X",
                key_data[0],
                key_data[1],
                key_data[2],
                key_data[3]);
            line_count = 2;
            break;
        }
    }

    switch(line_count) {
    case 3:
        popup_set_header(popup, "iButton", 82, 18, AlignCenter, AlignBottom);
        popup_set_text(popup, ibutton->text_store, 82, 22, AlignCenter, AlignTop);
        break;

    default:
        popup_set_header(popup, "iButton", 82, 24, AlignCenter, AlignBottom);
        popup_set_text(popup, ibutton->text_store, 82, 28, AlignCenter, AlignTop);
        break;
    }

    popup_set_icon(popup, 2, 10, &I_iButtonKey_49x44);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);

    ibutton_worker_emulate_set_callback(
        ibutton->key_worker, ibutton_scene_emulate_callback, ibutton);
    ibutton_worker_emulate_start(ibutton->key_worker, key);

    string_clear(key_name);

    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
}

bool ibutton_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        uint32_t cnt = scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneEmulate);
        if(cnt > 0) {
            cnt--;
            if(cnt == 0) {
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
    Popup* popup = ibutton->popup;
    ibutton_worker_stop(ibutton->key_worker);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
