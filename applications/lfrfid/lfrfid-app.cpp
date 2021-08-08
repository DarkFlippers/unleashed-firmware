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

#include <file-worker-cpp.h>
#include <lib/toolbox/path.h>

const char* LfRfidApp::app_folder = "/any/lfrfid";
const char* LfRfidApp::app_extension = ".rfid";

LfRfidApp::LfRfidApp()
    : scene_controller{this}
    , notification{"notification"}
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
    FileWorkerCpp file_worker;
    TextStore* filename_ts = new TextStore(64);
    bool result;

    if(need_restore) {
        result = file_worker.file_select(
            app_folder,
            app_extension,
            filename_ts->text,
            filename_ts->text_size,
            worker.key.get_name());
    } else {
        result = file_worker.file_select(
            app_folder, app_extension, filename_ts->text, filename_ts->text_size, NULL);
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
    FileWorkerCpp file_worker;
    string_t file_name;
    bool result = false;

    string_init_printf(file_name, "%s/%s%s", app_folder, key->get_name(), app_extension);
    result = file_worker.remove(string_get_cstr(file_name));
    string_clear(file_name);

    return result;
}

bool LfRfidApp::load_key_data(const char* path, RfidKey* key) {
    FileWorkerCpp file_worker;
    bool result = false;

    bool res = file_worker.open(path, FSAM_READ, FSOM_OPEN_EXISTING);

    if(res) {
        string_t str_result;
        string_init(str_result);

        do {
            RfidKey loaded_key;
            LfrfidKeyType loaded_type;

            // load type
            if(!file_worker.read_until(str_result, ' ')) break;
            if(!lfrfid_key_get_string_type(string_get_cstr(str_result), &loaded_type)) {
                file_worker.show_error("Cannot parse\nfile");
                break;
            }
            loaded_key.set_type(loaded_type);

            // load data
            uint8_t tmp_data[loaded_key.get_type_data_count()];
            if(!file_worker.read_hex(tmp_data, loaded_key.get_type_data_count())) break;
            loaded_key.set_data(tmp_data, loaded_key.get_type_data_count());

            *key = loaded_key;
            result = true;
        } while(0);

        // load name
        path_extract_filename_no_ext(path, str_result);
        key->set_name(string_get_cstr(str_result));

        string_clear(str_result);
    }

    file_worker.close();

    return result;
}

bool LfRfidApp::save_key_data(const char* path, RfidKey* key) {
    FileWorkerCpp file_worker;
    bool result = false;

    bool res = file_worker.open(path, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(res) {
        do {
            // type header
            const char* key_type = lfrfid_key_get_type_string(key->get_type());
            char delimeter = ' ';

            if(!file_worker.write(key_type, strlen(key_type))) break;
            if(!file_worker.write(&delimeter)) break;
            if(!file_worker.write_hex(key->get_data(), key->get_type_data_count())) break;

            result = true;
        } while(0);
    }

    file_worker.close();

    return result;
}

void LfRfidApp::make_app_folder() {
    FileWorkerCpp file_worker;
    file_worker.mkdir(app_folder);
}