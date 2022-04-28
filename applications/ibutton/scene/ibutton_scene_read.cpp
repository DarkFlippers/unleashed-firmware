#include "ibutton_scene_read.h"
#include "../ibutton_app.h"
#include <dolphin/dolphin.h>

static void read_callback(void* context) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event = {.type = iButtonEvent::Type::EventTypeWorkerRead};
    app->get_view_manager()->send_event(&event);
}

void iButtonSceneRead::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    iButtonKey* key = app->get_key();
    iButtonWorker* worker = app->get_key_worker();
    DOLPHIN_DEED(DolphinDeedIbuttonRead);

    popup_set_header(popup, "iButton", 95, 26, AlignCenter, AlignBottom);
    popup_set_text(popup, "waiting\nfor key ...", 95, 30, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 5, &I_DolphinWait_61x59);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);
    ibutton_key_set_name(key, "");

    ibutton_worker_read_set_callback(worker, read_callback, app);
    ibutton_worker_read_start(worker, key);
}

bool iButtonSceneRead::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeWorkerRead) {
        consumed = true;

        iButtonKey* key = app->get_key();
        bool success = false;
        if(ibutton_key_get_type(key) == iButtonKeyDS1990) {
            if(!ibutton_key_dallas_crc_is_valid(key)) {
                app->switch_to_next_scene(iButtonApp::Scene::SceneReadCRCError);
            } else if(!ibutton_key_dallas_is_1990_key(key)) {
                app->switch_to_next_scene(iButtonApp::Scene::SceneReadNotKeyError);
            } else {
                success = true;
            }
        } else {
            success = true;
        }
        if(success) {
            app->notify_success();
            app->notify_green_on();
            DOLPHIN_DEED(DolphinDeedIbuttonReadSuccess);
            app->switch_to_next_scene(iButtonApp::Scene::SceneReadSuccess);
        }
    } else if(event->type == iButtonEvent::Type::EventTypeTick) {
        consumed = true;
        app->notify_read();
    }

    return consumed;
}

void iButtonSceneRead::on_exit(iButtonApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();
    ibutton_worker_stop(app->get_key_worker());
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
