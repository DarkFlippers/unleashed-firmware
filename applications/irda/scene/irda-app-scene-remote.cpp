#include "../irda-app.hpp"
#include "gui/modules/button_menu.h"

typedef enum {
    ButtonIndexPlus = -2,
    ButtonIndexEdit = -1,
    ButtonIndexNA = 0,
} ButtonIndex;

static void button_menu_callback(void* context, int32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneRemote::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();
    auto remote_manager = app->get_remote_manager();
    int i = 0;

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

    if(event->type == IrdaAppEvent::Type::MenuSelected) {
        switch(event->payload.menu_index) {
        case ButtonIndexPlus:
            app->notify_click();
            buttonmenu_item_selected = event->payload.menu_index;
            app->set_learn_new_remote(false);
            app->switch_to_next_scene(IrdaApp::Scene::Learn);
            break;
        case ButtonIndexEdit:
            app->notify_click();
            buttonmenu_item_selected = event->payload.menu_index;
            app->switch_to_next_scene(IrdaApp::Scene::Edit);
            break;
        default:
            app->notify_click_and_blink();
            auto remote_manager = app->get_remote_manager();
            auto message = remote_manager->get_button_data(event->payload.menu_index);
            app->get_transceiver()->send_message(message);
            break;
        }
    } else if(event->type == IrdaAppEvent::Type::Back) {
        app->search_and_switch_to_previous_scene(
            {IrdaApp::Scene::Start, IrdaApp::Scene::RemoteList});
    } else {
        consumed = false;
    }

    return consumed;
}

void IrdaAppSceneRemote::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();

    button_menu_clean(button_menu);
}
