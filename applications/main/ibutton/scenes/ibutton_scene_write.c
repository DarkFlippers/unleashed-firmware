#include "../ibutton_i.h"
#include "m-string.h"
#include "toolbox/path.h"

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
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;
    iButtonWorker* worker = ibutton->key_worker;

    const uint8_t* key_data = ibutton_key_get_data_p(key);

    string_t key_name;
    string_init(key_name);
    if(string_end_with_str_p(ibutton->file_path, IBUTTON_APP_EXTENSION)) {
        path_extract_filename(ibutton->file_path, key_name, true);
    }

    // check that stored key has name
    if(!string_empty_p(key_name)) {
        ibutton_text_store_set(ibutton, "%s", string_get_cstr(key_name));
    } else {
        // if not, show key data
        switch(ibutton_key_get_type(key)) {
        case iButtonKeyDS1990:
            ibutton_text_store_set(
                ibutton,
                "%02X %02X %02X %02X\n%02X %02X %02X %02X",
                key_data[0],
                key_data[1],
                key_data[2],
                key_data[3],
                key_data[4],
                key_data[5],
                key_data[6],
                key_data[7]);
            break;
        case iButtonKeyCyfral:
            ibutton_text_store_set(ibutton, "%02X %02X", key_data[0], key_data[1]);
            break;
        case iButtonKeyMetakom:
            ibutton_text_store_set(
                ibutton, "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
            break;
        }
    }

    widget_add_string_multiline_element(
        widget, 90, 10, AlignCenter, AlignTop, FontPrimary, "iButton\nwriting");
    widget_add_icon_element(widget, 3, 10, &I_iButtonKey_49x44);
    widget_add_text_box_element(
        widget, 54, 39, 75, 22, AlignCenter, AlignCenter, ibutton->text_store, true);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    ibutton_worker_write_set_callback(worker, ibutton_scene_write_callback, ibutton);
    ibutton_worker_write_start(worker, key);

    string_clear(key_name);

    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
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
            ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateBlink);
        } else if(event.event == iButtonWorkerWriteCannotWrite) {
            ibutton_notification_message(ibutton, iButtonNotificationMessageYellowBlink);
        }

    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void ibutton_scene_write_on_exit(void* context) {
    iButton* ibutton = context;
    ibutton_worker_stop(ibutton->key_worker);
    widget_reset(ibutton->widget);

    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
