#include <gui/modules/button_menu.h>
#include <input/input.h>
#include <infrared_worker.h>
#include <dolphin/dolphin.h>
#include "../infrared_app.h"
#include "../infrared_app_view_manager.h"

typedef enum {
    ButtonIndexPlus = -2,
    ButtonIndexEdit = -1,
    ButtonIndexNA = 0,
} ButtonIndex;

static void button_menu_callback(void* context, int32_t index, InputType type) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    if(type == InputTypePress) {
        event.type = InfraredAppEvent::Type::MenuSelectedPress;
    } else if(type == InputTypeRelease) {
        event.type = InfraredAppEvent::Type::MenuSelectedRelease;
    } else if(type == InputTypeShort) {
        event.type = InfraredAppEvent::Type::MenuSelected;
    } else {
        furi_assert(0);
    }

    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void InfraredAppSceneRemote::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();
    auto remote_manager = app->get_remote_manager();
    int i = 0;
    button_pressed = false;

    infrared_worker_tx_set_get_signal_callback(
        app->get_infrared_worker(), infrared_worker_tx_get_signal_steady_callback, app);
    infrared_worker_tx_set_signal_sent_callback(
        app->get_infrared_worker(), InfraredApp::signal_sent_callback, app);
    buttons_names = remote_manager->get_button_list();

    i = 0;
    for(auto& name : buttons_names) {
        button_menu_add_item(
            button_menu, name.c_str(), i++, button_menu_callback, ButtonMenuItemTypeCommon, app);
    }

    button_menu_add_item(
        button_menu, "+", ButtonIndexPlus, button_menu_callback, ButtonMenuItemTypeControl, app);
    button_menu_add_item(
        button_menu, "Edit", ButtonIndexEdit, button_menu_callback, ButtonMenuItemTypeControl, app);

    app->set_text_store(0, "%s", remote_manager->get_remote_name().c_str());
    button_menu_set_header(button_menu, app->get_text_store(0));
    if(buttonmenu_item_selected != ButtonIndexNA) {
        button_menu_set_selected_item(button_menu, buttonmenu_item_selected);
        buttonmenu_item_selected = ButtonIndexNA;
    }
    view_manager->switch_to(InfraredAppViewManager::ViewId::ButtonMenu);
}

bool InfraredAppSceneRemote::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = true;

    if((event->type == InfraredAppEvent::Type::MenuSelected) ||
       (event->type == InfraredAppEvent::Type::MenuSelectedPress) ||
       (event->type == InfraredAppEvent::Type::MenuSelectedRelease)) {
        switch(event->payload.menu_index) {
        case ButtonIndexPlus:
            furi_assert(event->type == InfraredAppEvent::Type::MenuSelected);
            buttonmenu_item_selected = event->payload.menu_index;
            app->set_learn_new_remote(false);
            app->switch_to_next_scene(InfraredApp::Scene::Learn);
            break;
        case ButtonIndexEdit:
            furi_assert(event->type == InfraredAppEvent::Type::MenuSelected);
            buttonmenu_item_selected = event->payload.menu_index;
            app->switch_to_next_scene(InfraredApp::Scene::Edit);
            break;
        default:
            furi_assert(event->type != InfraredAppEvent::Type::MenuSelected);
            bool pressed = (event->type == InfraredAppEvent::Type::MenuSelectedPress);

            if(pressed && !button_pressed) {
                button_pressed = true;

                auto button_signal =
                    app->get_remote_manager()->get_button_data(event->payload.menu_index);
                if(button_signal.is_raw()) {
                    infrared_worker_set_raw_signal(
                        app->get_infrared_worker(),
                        button_signal.get_raw_signal().timings,
                        button_signal.get_raw_signal().timings_cnt);
                } else {
                    infrared_worker_set_decoded_signal(
                        app->get_infrared_worker(), &button_signal.get_message());
                }

                DOLPHIN_DEED(DolphinDeedIrSend);
                infrared_worker_tx_start(app->get_infrared_worker());
            } else if(!pressed && button_pressed) {
                button_pressed = false;
                infrared_worker_tx_stop(app->get_infrared_worker());
                app->notify_green_off();
            }
            break;
        }
    } else if(event->type == InfraredAppEvent::Type::Back) {
        if(!button_pressed) {
            app->search_and_switch_to_previous_scene(
                {InfraredApp::Scene::Start, InfraredApp::Scene::RemoteList});
        }
    } else {
        consumed = false;
    }

    return consumed;
}

void InfraredAppSceneRemote::on_exit(InfraredApp* app) {
    infrared_worker_tx_set_get_signal_callback(app->get_infrared_worker(), nullptr, nullptr);
    infrared_worker_tx_set_signal_sent_callback(app->get_infrared_worker(), nullptr, nullptr);
    InfraredAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();

    button_menu_reset(button_menu);
}
