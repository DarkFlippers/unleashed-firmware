#include "ibutton-scene-select-key.h"
#include "../ibutton-app.h"
#include "../ibutton-event.h"
#include "../ibutton-key.h"

void iButtonSceneSelectKey::on_enter(iButtonApp* app) {
    // Input events and views are managed by file_select
    bool res = app->get_sd_ex_api()->file_select(
        app->get_sd_ex_api()->context,
        "ibutton",
        "*",
        app->get_file_name(),
        app->get_file_name_size());

    // Process file_select return
    if(res) {
        // Get key file path
        string_t key_str;
        string_init_set_str(key_str, "ibutton/");
        string_cat_str(key_str, app->get_file_name());

        // Read data from file
        File key_file;
        uint8_t key_data[IBUTTON_KEY_DATA_SIZE + 1] = {};
        // TODO process false result from file system service
        app->get_fs_api()->file.open(
            &key_file, string_get_cstr(key_str), FSAM_READ, FSOM_OPEN_EXISTING);
        app->get_fs_api()->file.read(&key_file, key_data, IBUTTON_KEY_DATA_SIZE + 1);
        app->get_fs_api()->file.close(&key_file);
        string_clear(key_str);

        // Set key data
        iButtonKeyType key_type = static_cast<iButtonKeyType>(key_data[0]);
        if(key_type > iButtonKeyType::KeyMetakom) {
            app->switch_to_next_scene(iButtonApp::Scene::SceneStart);
        }
        app->get_key()->set_name(app->get_file_name());
        app->get_key()->set_type(key_type);
        app->get_key()->set_data(key_data + 1, IBUTTON_KEY_DATA_SIZE);
        app->switch_to_next_scene(iButtonApp::Scene::SceneSavedKeyMenu);
    } else {
        app->switch_to_previous_scene();
    }
}

bool iButtonSceneSelectKey::on_event(iButtonApp* app, iButtonEvent* event) {
    return false;
}

void iButtonSceneSelectKey::on_exit(iButtonApp* app) {
}
