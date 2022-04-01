#include "ibutton_scene_write.h"
#include "../ibutton_app.h"

static void ibutton_worker_write_cb(void* context, iButtonWorkerWriteResult result) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;
    event.type = iButtonEvent::Type::EventTypeWorkerWrite;
    event.payload.worker_write_result = result;

    app->get_view_manager()->send_event(&event);
}

void iButtonSceneWrite::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    iButtonKey* key = app->get_key();
    iButtonWorker* worker = app->get_key_worker();
    const uint8_t* key_data = ibutton_key_get_data_p(key);
    const char* key_name = ibutton_key_get_name_p(key);
    uint8_t line_count = 2;

    // check that stored key has name
    if(strcmp(key_name, "") != 0) {
        app->set_text_store("writing\n%s", key_name);
        line_count = 2;
    } else {
        // if not, show key data
        switch(ibutton_key_get_type(key)) {
        case iButtonKeyDS1990:
            app->set_text_store(
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
            app->set_text_store("writing\n%02X %02X", key_data[0], key_data[1]);
            line_count = 2;
            break;
        case iButtonKeyMetakom:
            app->set_text_store(
                "writing\n%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
            line_count = 2;
            break;
        }
    }

    switch(line_count) {
    case 3:
        popup_set_header(popup, "iButton", 82, 18, AlignCenter, AlignBottom);
        popup_set_text(popup, app->get_text_store(), 82, 22, AlignCenter, AlignTop);
        break;

    default:
        popup_set_header(popup, "iButton", 82, 24, AlignCenter, AlignBottom);
        popup_set_text(popup, app->get_text_store(), 82, 28, AlignCenter, AlignTop);
        break;
    }

    popup_set_icon(popup, 2, 10, &I_iButtonKey_49x44);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);

    blink_yellow = false;
    ibutton_worker_write_set_callback(worker, ibutton_worker_write_cb, app);
    ibutton_worker_write_start(worker, key);
}

bool iButtonSceneWrite::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeWorkerWrite) {
        consumed = true;

        switch(event->payload.worker_write_result) {
        case iButtonWorkerWriteOK:
        case iButtonWorkerWriteSameKey:
            app->switch_to_next_scene(iButtonApp::Scene::SceneWriteSuccess);
            break;
        case iButtonWorkerWriteNoDetect:
            blink_yellow = false;
            break;
        case iButtonWorkerWriteCannotWrite:
            blink_yellow = true;
            break;
        }
    } else if(event->type == iButtonEvent::Type::EventTypeTick) {
        if(blink_yellow) {
            app->notify_yellow_blink();
        } else {
            app->notify_red_blink();
        }
    }

    return consumed;
}

void iButtonSceneWrite::on_exit(iButtonApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();
    ibutton_worker_stop(app->get_key_worker());
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}