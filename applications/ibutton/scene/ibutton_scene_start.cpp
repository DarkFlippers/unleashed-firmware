#include "ibutton_scene_start.h"
#include "../ibutton_app.h"

typedef enum {
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexAdd,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void iButtonSceneStart::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "Read", SubmenuIndexRead, submenu_callback, app);
    submenu_add_item(submenu, "Saved", SubmenuIndexSaved, submenu_callback, app);
    submenu_add_item(submenu, "Add Manually", SubmenuIndexAdd, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneStart::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexRead:
            app->switch_to_next_scene(iButtonApp::Scene::SceneRead);
            break;
        case SubmenuIndexSaved:
            app->switch_to_next_scene(iButtonApp::Scene::SceneSelectKey);
            break;
        case SubmenuIndexAdd:
            app->switch_to_next_scene(iButtonApp::Scene::SceneAddType);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void iButtonSceneStart::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_reset(submenu);
}
