#include "lf-rfid-scene-write.h"

#include "../lf-rfid-app.h"
#include "../lf-rfid-view-manager.h"
#include "../lf-rfid-event.h"
#include "../helpers/key-info.h"

void LfrfidSceneWrite::on_enter(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, "LF-RFID", 64, 16, AlignCenter, AlignBottom);
    app->set_text_store("Writing...");
    popup_set_text(popup, app->get_text_store(), 64, 22, AlignCenter, AlignTop);

    view_manager->switch_to(LfrfidAppViewManager::ViewType::Popup);

    timing_index = 0;
}

bool LfrfidSceneWrite::on_event(LfrfidApp* app, LfrfidEvent* event) {
    bool consumed = false;

    // TODO move read\write logic to key worker

    bool readed = false;
    uint8_t em_data[5] = {0x1A, 0x2B, 0xC3, 0xD4, 0xE5};

    if(timing_index == 0) {
        app->get_reader()->stop();
        app->get_writer()->start();
        app->get_writer()->write_em(em_data);
        app->get_writer()->stop();
        delay(200);
        app->get_reader()->start(RfidReader::Type::Normal);
    } else {
        uint8_t data[LFRFID_KEY_SIZE];
        LfrfidKeyType type;

        app->get_reader()->read(&type, data, LFRFID_KEY_SIZE);
        if(type == LfrfidKeyType::KeyEM4100) {
            if(memcmp(em_data, data, 5) == 0) {
                readed = true;
            }
        }
    }

    if(readed) {
        app->set_text_store("Writed!");
        app->notify_green_blink();
    } else {
        app->set_text_store("Writing [1A 2B C3 D4 E5]");
        timing_index++;
        if(timing_index == 4) {
            timing_index = 0;
        }
    }
    popup_set_text(
        app->get_view_manager()->get_popup(), app->get_text_store(), 64, 22, AlignCenter, AlignTop);

    return consumed;
}

void LfrfidSceneWrite::on_exit(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
}