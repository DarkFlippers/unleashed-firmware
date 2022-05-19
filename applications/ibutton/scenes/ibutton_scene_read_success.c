#include "../ibutton_i.h"
#include <dolphin/dolphin.h>

static void ibutton_scene_read_success_dialog_ex_callback(DialogExResult result, void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, result);
}

void ibutton_scene_read_success_on_enter(void* context) {
    iButton* ibutton = context;
    DialogEx* dialog_ex = ibutton->dialog_ex;
    iButtonKey* key = ibutton->key;
    const uint8_t* key_data = ibutton_key_get_data_p(key);

    switch(ibutton_key_get_type(key)) {
    case iButtonKeyDS1990:
        ibutton_text_store_set(
            ibutton,
            "Dallas\n%02X %02X %02X %02X\n%02X %02X %02X %02X",
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
        ibutton_text_store_set(ibutton, "Cyfral\n%02X %02X", key_data[0], key_data[1]);
        break;
    case iButtonKeyMetakom:
        ibutton_text_store_set(
            ibutton,
            "Metakom\n%02X %02X %02X %02X",
            key_data[0],
            key_data[1],
            key_data[2],
            key_data[3]);
        break;
    }

    dialog_ex_set_text(dialog_ex, ibutton->text_store, 95, 30, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_icon(dialog_ex, 0, 1, &I_DolphinReadingSuccess_59x63);
    dialog_ex_set_result_callback(dialog_ex, ibutton_scene_read_success_dialog_ex_callback);
    dialog_ex_set_context(dialog_ex, ibutton);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewDialogEx);
}

bool ibutton_scene_read_success_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        scene_manager_next_scene(scene_manager, iButtonSceneExitConfirm);
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == DialogExResultRight) {
            scene_manager_next_scene(scene_manager, iButtonSceneReadKeyMenu);
        } else if(event.event == DialogExResultLeft) {
            scene_manager_next_scene(scene_manager, iButtonSceneRetryConfirm);
        }
    }

    return consumed;
}

void ibutton_scene_read_success_on_exit(void* context) {
    iButton* ibutton = context;
    DialogEx* dialog_ex = ibutton->dialog_ex;

    ibutton_text_store_clear(ibutton);

    dialog_ex_reset(dialog_ex);

    ibutton_notification_message(ibutton, iButtonNotificationMessageGreenOff);
}
