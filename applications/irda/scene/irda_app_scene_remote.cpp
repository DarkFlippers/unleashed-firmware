#include "../irda_app.h"
#include "gui/modules/button_menu.h"
#include "input/input.h"
#include "irda_worker.h"
#include <dolphin/dolphin.h>

typedef enum {
    ButtonIndexPlus = -2,
    ButtonIndexEdit = -1,
    ButtonIndexNA = 0,
} ButtonIndex;

static void button_menu_callback(void* context, int32_t index, InputType type) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    if(type == InputTypePress) {
        event.type = IrdaAppEvent::Type::MenuSelectedPress;
    } else if(type == InputTypeRelease) {
        event.type = IrdaAppEvent::Type::MenuSelectedRelease;
    } else if(type == InputTypeShort) {
        event.type = IrdaAppEvent::Type::MenuSelected;
    } else {
        furi_assert(0);
    }

    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneRemote::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();
    auto remote_manager = app->get_remote_manager();
    int i = 0;
    button_pressed = false;

    irda_worker_tx_set_get_signal_callback(
        app->get_irda_worker(), irda_worker_tx_get_signal_steady_callback, app);
    irda_worker_tx_set_signal_sent_callback(
        app->get_irda_worker(), IrdaApp::signal_sent_callback, app);
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
    view_manager->switch_to(IrdaAppViewManager::ViewType::ButtonMenu);
}

bool IrdaAppSceneRemote::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = true;

    if((event->type == IrdaAppEvent::Type::MenuSelected) ||
       (event->type == IrdaAppEvent::Type::MenuSelectedPress) ||
       (event->type == IrdaAppEvent::Type::MenuSelectedRelease)) {
        switch(event->payload.menu_index) {
        case ButtonIndexPlus:
            furi_assert(event->type == IrdaAppEvent::Type::MenuSelected);
            app->notify_click();
            buttonmenu_item_selected = event->payload.menu_index;
            app->set_learn_new_remote(false);
            app->switch_to_next_scene(IrdaApp::Scene::Learn);
            break;
        case ButtonIndexEdit:
            furi_assert(event->type == IrdaAppEvent::Type::MenuSelected);
            app->notify_click();
            buttonmenu_item_selected = event->payload.menu_index;
            app->switch_to_next_scene(IrdaApp::Scene::Edit);
            break;
        default:
            furi_assert(event->type != IrdaAppEvent::Type::MenuSelected);
            bool pressed = (event->type == IrdaAppEvent::Type::MenuSelectedPress);

            if(pressed && !button_pressed) {
                button_pressed = true;
                app->notify_click_and_green_blink();

                auto button_signal =
                    app->get_remote_manager()->get_button_data(event->payload.menu_index);
                if(button_signal.is_raw()) {
                    irda_worker_set_raw_signal(
                        app->get_irda_worker(),
                        button_signal.get_raw_signal().timings,
                        button_signal.get_raw_signal().timings_cnt);
                } else {
                    irda_worker_set_decoded_signal(
                        app->get_irda_worker(), &button_signal.get_message());
                }

                DOLPHIN_DEED(DolphinDeedIrSend);
                irda_worker_tx_start(app->get_irda_worker());
            } else if(!pressed && button_pressed) {
                button_pressed = false;
                irda_worker_tx_stop(app->get_irda_worker());
                app->notify_green_off();
            }
            break;
        }
    } else if(event->type == IrdaAppEvent::Type::Back) {
        if(!button_pressed) {
            app->search_and_switch_to_previous_scene(
                {IrdaApp::Scene::Start, IrdaApp::Scene::RemoteList});
        }
    } else {
        consumed = false;
    }

    return consumed;
}

void IrdaAppSceneRemote::on_exit(IrdaApp* app) {
    irda_worker_tx_set_get_signal_callback(app->get_irda_worker(), nullptr, nullptr);
    irda_worker_tx_set_signal_sent_callback(app->get_irda_worker(), nullptr, nullptr);
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();

    button_menu_reset(button_menu);
}
