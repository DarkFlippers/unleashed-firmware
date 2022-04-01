#include "ibutton_scene_add_type.h"
#include "../ibutton_app.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexCyfral,
    SubmenuIndexDallas,
    SubmenuIndexMetakom,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void iButtonSceneAddType::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "Cyfral", SubmenuIndexCyfral, submenu_callback, app);
    submenu_add_item(submenu, "Dallas", SubmenuIndexDallas, submenu_callback, app);
    submenu_add_item(submenu, "Metakom", SubmenuIndexMetakom, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneAddType::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        iButtonKey* key = app->get_key();

        switch(event->payload.menu_index) {
        case SubmenuIndexCyfral:
            ibutton_key_set_type(key, iButtonKeyCyfral);
            break;
        case SubmenuIndexDallas:
            ibutton_key_set_type(key, iButtonKeyDS1990);
            break;
        case SubmenuIndexMetakom:
            ibutton_key_set_type(key, iButtonKeyMetakom);
            break;
        }
        ibutton_key_set_name(key, "");
        ibutton_key_clear_data(key);
        app->switch_to_next_scene(iButtonApp::Scene::SceneAddValue);
        consumed = true;
    }

    return consumed;
}

void iButtonSceneAddType::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_reset(submenu);
}
