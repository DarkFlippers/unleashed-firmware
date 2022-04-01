#include "ibutton_scene_saved_key_menu.h"
#include "../ibutton_app.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexEmulate,
    SubmenuIndexWrite,
    SubmenuIndexEdit,
    SubmenuIndexDelete,
    SubmenuIndexInfo,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void iButtonSceneSavedKeyMenu::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "Emulate", SubmenuIndexEmulate, submenu_callback, app);
    if(ibutton_key_get_type(app->get_key()) == iButtonKeyDS1990) {
        submenu_add_item(submenu, "Write", SubmenuIndexWrite, submenu_callback, app);
    }
    submenu_add_item(submenu, "Edit", SubmenuIndexEdit, submenu_callback, app);
    submenu_add_item(submenu, "Delete", SubmenuIndexDelete, submenu_callback, app);
    submenu_add_item(submenu, "Info", SubmenuIndexInfo, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneSavedKeyMenu::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexWrite:
            app->switch_to_next_scene(iButtonApp::Scene::SceneWrite);
            break;
        case SubmenuIndexEmulate:
            app->switch_to_next_scene(iButtonApp::Scene::SceneEmulate);
            break;
        case SubmenuIndexEdit:
            app->switch_to_next_scene(iButtonApp::Scene::SceneAddValue);
            break;
        case SubmenuIndexDelete:
            app->switch_to_next_scene(iButtonApp::Scene::SceneDeleteConfirm);
            break;
        case SubmenuIndexInfo:
            app->switch_to_next_scene(iButtonApp::Scene::SceneInfo);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void iButtonSceneSavedKeyMenu::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_reset(submenu);
}
