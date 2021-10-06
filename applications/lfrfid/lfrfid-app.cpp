#include "lfrfid-app.h"
#include "scene/lfrfid-app-scene-start.h"
#include "scene/lfrfid-app-scene-read.h"
#include "scene/lfrfid-app-scene-read-success.h"
#include "scene/lfrfid-app-scene-readed-menu.h"
#include "scene/lfrfid-app-scene-write.h"
#include "scene/lfrfid-app-scene-write-success.h"
#include "scene/lfrfid-app-scene-emulate.h"
#include "scene/lfrfid-app-scene-save-name.h"
#include "scene/lfrfid-app-scene-save-success.h"
#include "scene/lfrfid-app-scene-select-key.h"
#include "scene/lfrfid-app-scene-saved-key-menu.h"
#include "scene/lfrfid-app-scene-save-data.h"
#include "scene/lfrfid-app-scene-save-type.h"
#include "scene/lfrfid-app-scene-saved-info.h"
#include "scene/lfrfid-app-scene-delete-confirm.h"
#include "scene/lfrfid-app-scene-delete-success.h"

#include <lib/toolbox/path.h>
#include <lib/toolbox/flipper-file-cpp.h>

const char* LfRfidApp::app_folder = "/any/lfrfid";
const char* LfRfidApp::app_extension = ".rfid";
const char* LfRfidApp::app_filetype = "Flipper RFID key";

LfRfidApp::LfRfidApp()
    : scene_controller{this}
    , notification{"notification"}
    , storage{"storage"}
    , dialogs{"dialogs"}
    , text_store(40) {
    furi_hal_power_insomnia_enter();
}

LfRfidApp::~LfRfidApp() {
    furi_hal_power_insomnia_exit();
}

void LfRfidApp::run(void* _args) {
    const char* args = reinterpret_cast<const char*>(_args);

    make_app_folder();

    if(strlen(args)) {
        load_key_data(args, &worker.key);
        scene_controller.add_scene(SceneType::Emulate, new LfRfidAppSceneEmulate());
        scene_controller.process(100, SceneType::Emulate);
    } else {
        scene_controller.add_scene(SceneType::Start, new LfRfidAppSceneStart());
        scene_controller.add_scene(SceneType::Read, new LfRfidAppSceneRead());
        scene_controller.add_scene(SceneType::ReadSuccess, new LfRfidAppSceneReadSuccess());
        scene_controller.add_scene(SceneType::ReadedMenu, new LfRfidAppSceneReadedMenu());
        scene_controller.add_scene(SceneType::Write, new LfRfidAppSceneWrite());
        scene_controller.add_scene(SceneType::WriteSuccess, new LfRfidAppSceneWriteSuccess());
        scene_controller.add_scene(SceneType::Emulate, new LfRfidAppSceneEmulate());
        scene_controller.add_scene(SceneType::SaveName, new LfRfidAppSceneSaveName());
        scene_controller.add_scene(SceneType::SaveSuccess, new LfRfidAppSceneSaveSuccess());
        scene_controller.add_scene(SceneType::SelectKey, new LfRfidAppSceneSelectKey());
        scene_controller.add_scene(SceneType::SavedKeyMenu, new LfRfidAppSceneSavedKeyMenu());
        scene_controller.add_scene(SceneType::SaveData, new LfRfidAppSceneSaveData());
        scene_controller.add_scene(SceneType::SaveType, new LfRfidAppSceneSaveType());
        scene_controller.add_scene(SceneType::SavedInfo, new LfRfidAppSceneSavedInfo());
        scene_controller.add_scene(SceneType::DeleteConfirm, new LfRfidAppSceneDeleteConfirm());
        scene_controller.add_scene(SceneType::DeleteSuccess, new LfRfidAppSceneDeleteSuccess());
        scene_controller.process(100);
    }
}

bool LfRfidApp::save_key(RfidKey* key) {
    string_t file_name;
    bool result = false;

    make_app_folder();

    string_init_printf(file_name, "%s/%s%s", app_folder, key->get_name(), app_extension);
    result = save_key_data(string_get_cstr(file_name), key);
    string_clear(file_name);

    return result;
}

bool LfRfidApp::load_key_from_file_select(bool need_restore) {
    TextStore* filename_ts = new TextStore(64);
    bool result = false;

    if(need_restore) {
        result = dialog_file_select_show(
            dialogs,
            app_folder,
            app_extension,
            filename_ts->text,
            filename_ts->text_size,
            worker.key.get_name());
    } else {
        result = dialog_file_select_show(
            dialogs, app_folder, app_extension, filename_ts->text, filename_ts->text_size, NULL);
    }

    if(result) {
        string_t key_str;
        string_init_printf(key_str, "%s/%s%s", app_folder, filename_ts->text, app_extension);
        result = load_key_data(string_get_cstr(key_str), &worker.key);
        string_clear(key_str);
    }

    delete filename_ts;
    return result;
}

bool LfRfidApp::delete_key(RfidKey* key) {
    string_t file_name;
    bool result = false;

    string_init_printf(file_name, "%s/%s%s", app_folder, key->get_name(), app_extension);
    result = storage_simply_remove(storage, string_get_cstr(file_name));
    string_clear(file_name);

    return result;
}

bool LfRfidApp::load_key_data(const char* path, RfidKey* key) {
    FlipperFileCpp file(storage);
    bool result = false;
    string_t str_result;
    string_init(str_result);

    do {
        if(!file.open_read(path)) break;

        // header
        uint32_t version;
        if(!file.read_header(str_result, &version)) break;
        if(string_cmp_str(str_result, app_filetype) != 0) break;
        if(version != 1) break;

        // key type
        LfrfidKeyType type;
        RfidKey loaded_key;

        if(!file.read_string("Key type", str_result)) break;
        if(!lfrfid_key_get_string_type(string_get_cstr(str_result), &type)) break;
        loaded_key.set_type(type);

        // key data
        uint8_t key_data[loaded_key.get_type_data_count()] = {};
        if(!file.read_hex_array("Data", key_data, loaded_key.get_type_data_count())) break;
        loaded_key.set_data(key_data, loaded_key.get_type_data_count());

        path_extract_filename_no_ext(path, str_result);
        loaded_key.set_name(string_get_cstr(str_result));

        *key = loaded_key;
        result = true;
    } while(0);

    file.close();
    string_clear(str_result);

    if(!result) {
        dialog_message_show_storage_error(dialogs, "Cannot load\nkey file");
    }

    return result;
}

bool LfRfidApp::save_key_data(const char* path, RfidKey* key) {
    FlipperFileCpp file(storage);
    bool result = false;

    do {
        if(!file.new_write(path)) break;
        if(!file.write_header_cstr(app_filetype, 1)) break;
        if(!file.write_comment_cstr("Key type can be EM4100, H10301 or I40134")) break;
        if(!file.write_string_cstr("Key type", lfrfid_key_get_type_string(key->get_type()))) break;
        if(!file.write_comment_cstr("Data size for EM4100 is 5, for H10301 is 3, for I40134 is 3"))
            break;
        if(!file.write_hex_array("Data", key->get_data(), key->get_type_data_count())) break;
        result = true;
    } while(0);

    file.close();

    if(!result) {
        dialog_message_show_storage_error(dialogs, "Cannot save\nkey file");
    }

    return result;
}

void LfRfidApp::make_app_folder() {
    if(!storage_simply_mkdir(storage, app_folder)) {
        dialog_message_show_storage_error(dialogs, "Cannot create\napp folder");
    }
}