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
#include "scene/lfrfid_app_scene_extra_actions.h"
#include "scene/lfrfid_app_scene_raw_info.h"
#include "scene/lfrfid_app_scene_raw_name.h"
#include "scene/lfrfid_app_scene_raw_read.h"
#include "scene/lfrfid_app_scene_raw_success.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

#include <rpc/rpc_app.h>

const char* LfRfidApp::app_folder = ANY_PATH("lfrfid");
const char* LfRfidApp::app_sd_folder = EXT_PATH("lfrfid");
const char* LfRfidApp::app_extension = ".rfid";
const char* LfRfidApp::app_filetype = "Flipper RFID key";

LfRfidApp::LfRfidApp()
    : scene_controller{this}
    , notification{RECORD_NOTIFICATION}
    , storage{RECORD_STORAGE}
    , dialogs{RECORD_DIALOGS}
    , text_store(40) {
    string_init(file_name);
    string_init(raw_file_name);
    string_init_set_str(file_path, app_folder);

    dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    size_t size = protocol_dict_get_max_data_size(dict);
    new_key_data = (uint8_t*)malloc(size);
    old_key_data = (uint8_t*)malloc(size);

    lfworker = lfrfid_worker_alloc(dict);
}

LfRfidApp::~LfRfidApp() {
    string_clear(raw_file_name);
    string_clear(file_name);
    string_clear(file_path);
    protocol_dict_free(dict);

    lfrfid_worker_free(lfworker);

    if(rpc_ctx) {
        rpc_system_app_set_callback(rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(rpc_ctx);
    }

    free(new_key_data);
    free(old_key_data);
}

static void rpc_command_callback(RpcAppSystemEvent rpc_event, void* context) {
    furi_assert(context);
    LfRfidApp* app = static_cast<LfRfidApp*>(context);

    if(rpc_event == RpcAppEventSessionClose) {
        LfRfidApp::Event event;
        event.type = LfRfidApp::EventType::RpcSessionClose;
        app->view_controller.send_event(&event);
        // Detach RPC
        rpc_system_app_set_callback(app->rpc_ctx, NULL, NULL);
        app->rpc_ctx = NULL;
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
            view_controller.attach_to_gui(ViewDispatcherTypeDesktop);
            scene_controller.add_scene(SceneType::Rpc, new LfRfidAppSceneRpc());
            scene_controller.process(100, SceneType::Rpc);
        } else {
            string_set_str(file_path, args);
            load_key_data(file_path, true);
            view_controller.attach_to_gui(ViewDispatcherTypeFullscreen);
            scene_controller.add_scene(SceneType::Emulate, new LfRfidAppSceneEmulate());
            scene_controller.process(100, SceneType::Emulate);
        }

    } else {
        view_controller.attach_to_gui(ViewDispatcherTypeFullscreen);
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
        scene_controller.add_scene(SceneType::ExtraActions, new LfRfidAppSceneExtraActions());
        scene_controller.add_scene(SceneType::RawInfo, new LfRfidAppSceneRawInfo());
        scene_controller.add_scene(SceneType::RawName, new LfRfidAppSceneRawName());
        scene_controller.add_scene(SceneType::RawRead, new LfRfidAppSceneRawRead());
        scene_controller.add_scene(SceneType::RawSuccess, new LfRfidAppSceneRawSuccess());
        scene_controller.process(100);
    }
}

bool LfRfidApp::save_key() {
    bool result = false;

    make_app_folder();

    if(string_end_with_str_p(file_path, app_extension)) {
        size_t filename_start = string_search_rchar(file_path, '/');
        string_left(file_path, filename_start);
    }

    string_cat_printf(file_path, "/%s%s", string_get_cstr(file_name), app_extension);

    result = save_key_data(file_path);
    return result;
}

bool LfRfidApp::load_key_from_file_select(bool need_restore) {
    if(!need_restore) {
        string_set_str(file_path, app_folder);
    }

    bool result = dialog_file_browser_show(
        dialogs, file_path, file_path, app_extension, true, &I_125_10px, true);

    if(result) {
        result = load_key_data(file_path, true);
    }

    return result;
}

bool LfRfidApp::delete_key() {
    return storage_simply_remove(storage, string_get_cstr(file_path));
}

bool LfRfidApp::load_key_data(string_t path, bool show_dialog) {
    bool result = false;

    do {
        protocol_id = lfrfid_dict_file_load(dict, string_get_cstr(path));
        if(protocol_id == PROTOCOL_NO) break;

        path_extract_filename(path, file_name, true);
        result = true;
    } while(0);

    if((!result) && (show_dialog)) {
        dialog_message_show_storage_error(dialogs, "Cannot load\nkey file");
    }

    return result;
}

bool LfRfidApp::save_key_data(string_t path) {
    bool result = lfrfid_dict_file_save(dict, protocol_id, string_get_cstr(path));

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
