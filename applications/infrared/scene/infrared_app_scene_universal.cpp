#include "../infrared_app.h"

typedef enum {
    SubmenuIndexUniversalTV,
    SubmenuIndexUniversalAudio,
    SubmenuIndexUniversalAirConditioner,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    event.type = InfraredAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void InfraredAppSceneUniversal::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "TVs", SubmenuIndexUniversalTV, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);
    submenu_item_selected = 0;

    view_manager->switch_to(InfraredAppViewManager::ViewId::Submenu);
}

bool InfraredAppSceneUniversal::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexUniversalTV:
            app->switch_to_next_scene(InfraredApp::Scene::UniversalTV);
            break;
        case SubmenuIndexUniversalAudio:
            //            app->switch_to_next_scene(InfraredApp::Scene::UniversalAudio);
            break;
        case SubmenuIndexUniversalAirConditioner:
            //            app->switch_to_next_scene(InfraredApp::Scene::UniversalAirConditioner);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneUniversal::on_exit(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_reset(submenu);
}
