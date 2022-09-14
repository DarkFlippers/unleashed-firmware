#include "../accessor_app.h"
#include "../accessor_view_manager.h"
#include "../accessor_event.h"
#include <callback-connector.h>
#include "accessor_scene_start.h"

void AccessorSceneStart::on_enter(AccessorApp* app) {
    AccessorAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_header(popup, "Accessor App", 64, 16, AlignCenter, AlignBottom);
    app->set_text_store("[??????]");
    popup_set_text(popup, app->get_text_store(), 64, 22, AlignCenter, AlignTop);

    view_manager->switch_to(AccessorAppViewManager::ViewType::Popup);
}

bool AccessorSceneStart::on_event(AccessorApp* app, AccessorEvent* event) {
    bool consumed = false;

    if(event->type == AccessorEvent::Type::Tick) {
        WIEGAND* wiegand = app->get_wiegand();
        Popup* popup = app->get_view_manager()->get_popup();
        OneWireHost* onewire_host = app->get_one_wire();

        uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t type = 0;

        if(wiegand->available()) {
            type = wiegand->getWiegandType();

            for(uint8_t i = 0; i < 4; i++) {
                data[i] = wiegand->getCode() >> (i * 8);
            }

            for(uint8_t i = 4; i < 8; i++) {
                data[i] = wiegand->getCodeHigh() >> ((i - 4) * 8);
            }
        } else {
            FURI_CRITICAL_ENTER();
            if(onewire_host_reset(onewire_host)) {
                type = 255;
                onewire_host_write(onewire_host, 0x33);
                for(uint8_t i = 0; i < 8; i++) {
                    data[i] = onewire_host_read(onewire_host);
                }

                for(uint8_t i = 0; i < 7; i++) {
                    data[i] = data[i + 1];
                }
            }
            FURI_CRITICAL_EXIT();
        }

        if(type > 0) {
            if(type == 255) {
                app->set_text_store(
                    "[%02X %02X %02X %02X %02X %02X DS]",
                    data[5],
                    data[4],
                    data[3],
                    data[2],
                    data[1],
                    data[0]);
            } else {
                app->set_text_store(
                    "[%02X %02X %02X %02X %02X %02X W%u]",
                    data[5],
                    data[4],
                    data[3],
                    data[2],
                    data[1],
                    data[0],
                    type);
            }
            popup_set_text(popup, app->get_text_store(), 64, 22, AlignCenter, AlignTop);
            app->notify_success();
        }
    }

    return consumed;
}

void AccessorSceneStart::on_exit(AccessorApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
}
