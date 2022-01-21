#include "../irda_app.h"

typedef enum {
    SubmenuIndexUniversalTV,
    SubmenuIndexUniversalAudio,
    SubmenuIndexUniversalAirConditioner,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneUniversal::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "TV's", SubmenuIndexUniversalTV, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);
    submenu_item_selected = 0;

    view_manager->switch_to(IrdaAppViewManager::ViewType::Submenu);
}

bool IrdaAppSceneUniversal::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexUniversalTV:
            app->switch_to_next_scene(IrdaApp::Scene::UniversalTV);
            break;
        case SubmenuIndexUniversalAudio:
            //            app->switch_to_next_scene(IrdaApp::Scene::UniversalAudio);
            break;
        case SubmenuIndexUniversalAirConditioner:
            //            app->switch_to_next_scene(IrdaApp::Scene::UniversalAirConditioner);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneUniversal::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_reset(submenu);
}
