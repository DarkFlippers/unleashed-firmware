#include "ibutton_scene_emulate.h"
#include "../ibutton_app.h"
#include <dolphin/dolphin.h>

static void emulate_callback(void* context, bool emulated) {
    furi_assert(context);
    if(emulated) {
        iButtonApp* app = static_cast<iButtonApp*>(context);
        iButtonEvent event = {.type = iButtonEvent::Type::EventTypeWorkerEmulated};
        app->get_view_manager()->send_event(&event);
    }
}

void iButtonSceneEmulate::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    iButtonKey* key = app->get_key();
    const uint8_t* key_data = ibutton_key_get_data_p(key);
    const char* key_name = ibutton_key_get_name_p(key);
    uint8_t line_count = 2;
    DOLPHIN_DEED(DolphinDeedIbuttonEmulate);

    // check that stored key has name
    if(strcmp(key_name, "") != 0) {
        app->set_text_store("emulating\n%s", key_name);
        line_count = 2;
    } else {
        // if not, show key data
        switch(ibutton_key_get_type(key)) {
        case iButtonKeyDS1990:
            app->set_text_store(
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
            app->set_text_store("emulating\n%02X %02X", key_data[0], key_data[1]);
            line_count = 2;
            break;
        case iButtonKeyMetakom:
            app->set_text_store(
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
        popup_set_text(popup, app->get_text_store(), 82, 22, AlignCenter, AlignTop);
        break;

    default:
        popup_set_header(popup, "iButton", 82, 24, AlignCenter, AlignBottom);
        popup_set_text(popup, app->get_text_store(), 82, 28, AlignCenter, AlignTop);
        break;
    }

    popup_set_icon(popup, 2, 10, &I_iButtonKey_49x44);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);

    ibutton_worker_emulate_set_callback(app->get_key_worker(), emulate_callback, app);
    ibutton_worker_emulate_start(app->get_key_worker(), key);
}

bool iButtonSceneEmulate::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeWorkerEmulated) {
        app->notify_yellow_blink();
        consumed = true;
    } else if(event->type == iButtonEvent::Type::EventTypeTick) {
        app->notify_red_blink();
        consumed = true;
    }

    return consumed;
}

void iButtonSceneEmulate::on_exit(iButtonApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();
    ibutton_worker_stop(app->get_key_worker());
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
