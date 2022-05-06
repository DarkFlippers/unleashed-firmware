#include "../ibutton_i.h"

typedef enum {
    iButtonSceneWriteStateDefault,
    iButtonSceneWriteStateBlinkYellow,
} iButtonSceneWriteState;

static void ibutton_scene_write_callback(void* context, iButtonWorkerWriteResult result) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, result);
}

void ibutton_scene_write_on_enter(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    iButtonKey* key = ibutton->key;
    iButtonWorker* worker = ibutton->key_worker;

    const uint8_t* key_data = ibutton_key_get_data_p(key);
    const char* key_name = ibutton_key_get_name_p(key);

    uint8_t line_count = 2;

    // check that stored key has name
    if(strcmp(key_name, "") != 0) {
        ibutton_text_store_set(ibutton, "writing\n%s", key_name);
        line_count = 2;
    } else {
        // if not, show key data
        switch(ibutton_key_get_type(key)) {
        case iButtonKeyDS1990:
            ibutton_text_store_set(
                ibutton,
                "writing\n%02X %02X %02X %02X\n%02X %02X %02X %02X",
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
            ibutton_text_store_set(ibutton, "writing\n%02X %02X", key_data[0], key_data[1]);
            line_count = 2;
            break;
        case iButtonKeyMetakom:
            ibutton_text_store_set(
                ibutton,
                "writing\n%02X %02X %02X %02X",
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

    scene_manager_set_scene_state(
        ibutton->scene_manager, iButtonSceneWrite, iButtonSceneWriteStateDefault);
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);

    ibutton_worker_write_set_callback(worker, ibutton_scene_write_callback, ibutton);
    ibutton_worker_write_start(worker, key);
}

bool ibutton_scene_write_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if((event.event == iButtonWorkerWriteOK) || (event.event == iButtonWorkerWriteSameKey)) {
            scene_manager_next_scene(scene_manager, iButtonSceneWriteSuccess);
        } else if(event.event == iButtonWorkerWriteNoDetect) {
            scene_manager_set_scene_state(
                scene_manager, iButtonSceneWrite, iButtonSceneWriteStateDefault);
        } else if(event.event == iButtonWorkerWriteCannotWrite) {
            scene_manager_set_scene_state(
                scene_manager, iButtonSceneWrite, iButtonSceneWriteStateBlinkYellow);
        }

    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
        if(scene_manager_get_scene_state(scene_manager, iButtonSceneWrite) ==
           iButtonSceneWriteStateBlinkYellow) {
            ibutton_notification_message(ibutton, iButtonNotificationMessageYellowBlink);
        } else {
            ibutton_notification_message(ibutton, iButtonNotificationMessageEmulate);
        }
    }

    return consumed;
}

void ibutton_scene_write_on_exit(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    ibutton_worker_stop(ibutton->key_worker);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
