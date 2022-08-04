#include "lfrfid_app.h"
#include "assets_icons.h"
#include <core/common_defines.h>
#include "m-string.h"
#include "scene/lfrfid_app_scene_start.h"
#include "scene/lfrfid_app_scene_read.h"
#include "scene/lfrfid_app_scene_read_success.h"
#include "scene/lfrfid_app_scene_retry_confirm.h"
#include "scene/lfrfid_app_scene_exit_confirm.h"
#include "scene/lfrfid_app_scene_read_menu.h"
#include "scene/lfrfid_app_scene_write.h"
#include "scene/lfrfid_app_scene_write_success.h"
#include "scene/lfrfid_app_scene_emulate.h"
#include "scene/lfrfid_app_scene_save_name.h"
#include "scene/lfrfid_app_scene_save_success.h"
#include "scene/lfrfid_app_scene_select_key.h"
#include "scene/lfrfid_app_scene_saved_key_menu.h"
#include "scene/lfrfid_app_scene_save_data.h"
#include "scene/lfrfid_app_scene_save_type.h"
#include "scene/lfrfid_app_scene_saved_info.h"
#include "scene/lfrfid_app_scene_delete_confirm.h"
#include "scene/lfrfid_app_scene_delete_success.h"
#include "scene/lfrfid_app_scene_rpc.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

#include <rpc/rpc_app.h>

const char* LfRfidApp::app_folder = ANY_PATH("lfrfid");
const char* LfRfidApp::app_extension = ".rfid";
const char* LfRfidApp::app_filetype = "Flipper RFID key";

LfRfidApp::LfRfidApp()
    : scene_controller{this}
    , notification{"notification"}
    , storage{"storage"}
    , dialogs{"dialogs"}
    , text_store(40) {
    string_init_set_str(file_path, app_folder);
}

LfRfidApp::~LfRfidApp() {
    string_clear(file_path);
    if(rpc_ctx) {
        rpc_system_app_set_callback(rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(rpc_ctx);
    }
}

static void rpc_command_callback(RpcAppSystemEvent rpc_event, void* context) {
    furi_assert(context);
    LfRfidApp* app = static_cast<LfRfidApp*>(context);

    if(rpc_event == RpcAppEventSessionClose) {
        LfRfidApp::Event event;
        event.type = LfRfidApp::EventType::RpcSessionClose;
        app->view_controller.send_event(&event);
    } else if(rpc_event == RpcAppEventAppExit) {
        LfRfidApp::Event event;
        event.type = LfRfidApp::EventType::Exit;
        app->view_controller.send_event(&event);
    } else if(rpc_event == RpcAppEventLoadFile) {
        LfRfidApp::Event event;
        event.type = LfRfidApp::EventType::RpcLoadFile;
        app->view_controller.send_event(&event);
    } else {
        rpc_system_app_confirm(app->rpc_ctx, rpc_event, false);
    }
}

void LfRfidApp::run(void* _args) {
    const char* args = reinterpret_cast<const char*>(_args);

    make_app_folder();

    if(args && strlen(args)) {
        uint32_t rpc_ctx_ptr = 0;
        if(sscanf(args, "RPC %lX", &rpc_ctx_ptr) == 1) {
            rpc_ctx = (RpcAppSystem*)rpc_ctx_ptr;
            rpc_system_app_set_callback(rpc_ctx, rpc_command_callback, this);
            rpc_system_app_send_started(rpc_ctx);
            scene_controller.add_scene(SceneType::Rpc, new LfRfidAppSceneRpc());
            scene_controller.process(100, SceneType::Rpc);
        } else {
            string_set_str(file_path, args);
            load_key_data(file_path, &worker.key, true);
            scene_controller.add_scene(SceneType::Emulate, new LfRfidAppSceneEmulate());
            scene_controller.process(100, SceneType::Emulate);
        }

    } else {
        scene_controller.add_scene(SceneType::Start, new LfRfidAppSceneStart());
        scene_controller.add_scene(SceneType::Read, new LfRfidAppSceneRead());
        scene_controller.add_scene(SceneType::RetryConfirm, new LfRfidAppSceneRetryConfirm());
        scene_controller.add_scene(SceneType::ExitConfirm, new LfRfidAppSceneExitConfirm());
        scene_controller.add_scene(SceneType::ReadSuccess, new LfRfidAppSceneReadSuccess());
        scene_controller.add_scene(SceneType::ReadKeyMenu, new LfRfidAppSceneReadKeyMenu());
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
    bool result = false;

    make_app_folder();

    if(string_end_with_str_p(file_path, app_extension)) {
        size_t filename_start = string_search_rchar(file_path, '/');
        string_left(file_path, filename_start);
    }

    string_cat_printf(file_path, "/%s%s", key->get_name(), app_extension);

    result = save_key_data(file_path, key);
    return result;
}

bool LfRfidApp::load_key_from_file_select(bool need_restore) {
    if(!need_restore) {
        string_set_str(file_path, app_folder);
    }

    bool result = dialog_file_browser_show(
        dialogs, file_path, file_path, app_extension, true, &I_125_10px, true);

    if(result) {
        result = load_key_data(file_path, &worker.key, true);
    }

    return result;
}

bool LfRfidApp::delete_key(RfidKey* key) {
    UNUSED(key);
    return storage_simply_remove(storage, string_get_cstr(file_path));
}

bool LfRfidApp::load_key_data(string_t path, RfidKey* key, bool show_dialog) {
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool result = false;
    string_t str_result;
    string_init(str_result);

    do {
        if(!flipper_format_file_open_existing(file, string_get_cstr(path))) break;

        // header
        uint32_t version;
        if(!flipper_format_read_header(file, str_result, &version)) break;
        if(string_cmp_str(str_result, app_filetype) != 0) break;
        if(version != 1) break;

        // key type
        LfrfidKeyType type;
        RfidKey loaded_key;

        if(!flipper_format_read_string(file, "Key type", str_result)) break;
        if(!lfrfid_key_get_string_type(string_get_cstr(str_result), &type)) break;
        loaded_key.set_type(type);

        // key data
        uint8_t key_data[loaded_key.get_type_data_count()] = {};
        if(!flipper_format_read_hex(file, "Data", key_data, loaded_key.get_type_data_count()))
            break;
        loaded_key.set_data(key_data, loaded_key.get_type_data_count());

        path_extract_filename(path, str_result, true);
        loaded_key.set_name(string_get_cstr(str_result));

        *key = loaded_key;
        result = true;
    } while(0);

    flipper_format_free(file);
    string_clear(str_result);

    if((!result) && (show_dialog)) {
        dialog_message_show_storage_error(dialogs, "Cannot load\nkey file");
    }

    return result;
}

bool LfRfidApp::save_key_data(string_t path, RfidKey* key) {
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool result = false;

    do {
        if(!flipper_format_file_open_always(file, string_get_cstr(path))) break;
        if(!flipper_format_write_header_cstr(file, app_filetype, 1)) break;
        if(!flipper_format_write_comment_cstr(file, "Key type can be EM4100, H10301 or I40134"))
            break;
        if(!flipper_format_write_string_cstr(
               file, "Key type", lfrfid_key_get_type_string(key->get_type())))
            break;
        if(!flipper_format_write_comment_cstr(
               file, "Data size for EM4100 is 5, for H10301 is 3, for I40134 is 3"))
            break;
        if(!flipper_format_write_hex(file, "Data", key->get_data(), key->get_type_data_count()))
            break;
        result = true;
    } while(0);

    flipper_format_free(file);

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
