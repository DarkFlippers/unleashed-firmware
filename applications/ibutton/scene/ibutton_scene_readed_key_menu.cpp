#include "ibutton_scene_readed_key_menu.h"
#include "../ibutton_app.h"

typedef enum {
    SubmenuIndexWrite,
    SubmenuIndexEmulate,
    SubmenuIndexSave,
    SubmenuIndexReadNewKey,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void iButtonSceneReadedKeyMenu::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    if(ibutton_key_get_type(app->get_key()) == iButtonKeyDS1990) {
        submenu_add_item(submenu, "Write", SubmenuIndexWrite, submenu_callback, app);
    }
    submenu_add_item(submenu, "Save", SubmenuIndexSave, submenu_callback, app);
    submenu_add_item(submenu, "Emulate", SubmenuIndexEmulate, submenu_callback, app);
    submenu_add_item(submenu, "Read new key", SubmenuIndexReadNewKey, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneReadedKeyMenu::on_event(iButtonApp* app, iButtonEvent* event) {
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
        case SubmenuIndexSave:
            app->switch_to_next_scene(iButtonApp::Scene::SceneSaveName);
            break;
        case SubmenuIndexReadNewKey:
            app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneRead});
            break;
        }
        consumed = true;
    } else if(event->type == iButtonEvent::Type::EventTypeBack) {
        app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
        consumed = true;
    }

    return consumed;
}

void iButtonSceneReadedKeyMenu::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_reset(submenu);
}