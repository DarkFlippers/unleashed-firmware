#include "lf-rfid-scene-read-normal.h"

#include "../lf-rfid-app.h"
#include "../lf-rfid-view-manager.h"
#include "../lf-rfid-event.h"
#include "../helpers/key-info.h"

void LfrfidSceneReadNormal::on_enter(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, "LF-RFID read EM & HID", 64, 16, AlignCenter, AlignBottom);
    app->set_text_store("waiting...");
    popup_set_text(popup, app->get_text_store(), 64, 22, AlignCenter, AlignTop);

    view_manager->switch_to(LfrfidAppViewManager::ViewType::Popup);
    app->get_reader()->start(RfidReader::Type::Normal);
}

bool LfrfidSceneReadNormal::on_event(LfrfidApp* app, LfrfidEvent* event) {
    bool consumed = false;

    if(event->type == LfrfidEvent::Type::Tick) {
        uint8_t data[data_size];
        LfrfidKeyType type;

        if(app->get_reader()->read(&type, data, data_size)) {
            app->notify_green_blink();

            if(memcmp(last_data, data, data_size) == 0) {
                success_reads++;
            } else {
                success_reads = 1;
                memcpy(last_data, data, data_size);
            }

            switch(type) {
            case LfrfidKeyType::KeyEmarine:
                app->set_text_store(
                    "[EM] %02X %02X %02X %02X %02X\n"
                    "count: %u",
                    data[0],
                    data[1],
                    data[2],
                    data[3],
                    data[4],
                    success_reads);
                break;
            case LfrfidKeyType::KeyHID:
                app->set_text_store(
                    "[HID26] %02X %02X %02X\n"
                    "count: %u",
                    data[0],
                    data[1],
                    data[2],
                    success_reads);
                break;
            }
            popup_set_text(
                app->get_view_manager()->get_popup(),
                app->get_text_store(),
                64,
                22,
                AlignCenter,
                AlignTop);
        }
    }

    return consumed;
}

void LfrfidSceneReadNormal::on_exit(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);

    app->get_reader()->stop();
}