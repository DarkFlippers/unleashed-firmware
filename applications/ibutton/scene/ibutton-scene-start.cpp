#include "ibutton-scene-start.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>
#include <filesystem-api.h>

typedef enum {
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexAdd,
} SubmenuIndex;

void iButtonSceneStart::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &iButtonSceneStart::submenu_callback);

    submenu_add_item(submenu, "Read", SubmenuIndexRead, callback, app);
    submenu_add_item(submenu, "Saved", SubmenuIndexSaved, callback, app);
    submenu_add_item(submenu, "Add manually", SubmenuIndexAdd, callback, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneStart::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        switch(event->payload.menu_index) {
        case SubmenuIndexRead:
            app->switch_to_next_scene(iButtonApp::Scene::SceneRead);
            break;
        case SubmenuIndexSaved: {
            bool res = app->get_sd_ex_api()->file_select(
                app->get_sd_ex_api()->context,
                "ibutton",
                "*",
                app->get_file_name(),
                app->get_file_name_size());
            if(res) {
                string_t key_str;
                string_init_set_str(key_str, "ibutton/");
                string_cat_str(key_str, app->get_file_name());
                File key_file;
                uint8_t key_data[IBUTTON_KEY_DATA_SIZE + 1] = {};
                // Read data from file
                // TODO handle false return
                res = app->get_fs_api()->file.open(
                    &key_file, string_get_cstr(key_str), FSAM_READ, FSOM_OPEN_EXISTING);
                res = app->get_fs_api()->file.read(&key_file, key_data, IBUTTON_KEY_DATA_SIZE + 1);
                res = app->get_fs_api()->file.close(&key_file);
                string_clear(key_str);
                // Set key
                iButtonKeyType key_type = static_cast<iButtonKeyType>(key_data[0]);
                if(key_type > iButtonKeyType::KeyMetakom) {
                    app->switch_to_next_scene(iButtonApp::Scene::SceneStart);
                }
                app->get_key()->set_name(app->get_file_name());
                app->get_key()->set_type(key_type);
                app->get_key()->set_data(key_data + 1, IBUTTON_KEY_DATA_SIZE);
                app->switch_to_next_scene(iButtonApp::Scene::SceneSavedKeyMenu);
            } else {
                // TODO add error scene
                app->switch_to_next_scene(iButtonApp::Scene::SceneStart);
            }
        }; break;
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

    submenu_clean(submenu);
}

void iButtonSceneStart::submenu_callback(void* context, uint32_t index) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}