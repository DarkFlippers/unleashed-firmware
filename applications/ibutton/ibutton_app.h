#pragma once
#include <map>
#include <list>

#include "ibutton_view_manager.h"
#include "scene/ibutton_scene_generic.h"
#include "scene/ibutton_scene_start.h"
#include "scene/ibutton_scene_read.h"
#include "scene/ibutton_scene_read_crc_error.h"
#include "scene/ibutton_scene_read_not_key_error.h"
#include "scene/ibutton_scene_read_success.h"
#include "scene/ibutton_scene_retry_confirm.h"
#include "scene/ibutton_scene_exit_confirm.h"
#include "scene/ibutton_scene_read_key_menu.h"
#include "scene/ibutton_scene_write.h"
#include "scene/ibutton_scene_write_success.h"
#include "scene/ibutton_scene_saved_key_menu.h"
#include "scene/ibutton_scene_delete_confirm.h"
#include "scene/ibutton_scene_delete_success.h"
#include "scene/ibutton_scene_emulate.h"
#include "scene/ibutton_scene_save_name.h"
#include "scene/ibutton_scene_save_success.h"
#include "scene/ibutton_scene_info.h"
#include "scene/ibutton_scene_select_key.h"
#include "scene/ibutton_scene_add_type.h"
#include "scene/ibutton_scene_add_value.h"
#include <one_wire/ibutton/ibutton_worker.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <record_controller.hpp>

class iButtonApp {
public:
    void run(void* args);

    iButtonApp();
    ~iButtonApp();

    enum class Scene : uint8_t {
        SceneExit,
        SceneStart,
        SceneRead,
        SceneReadNotKeyError,
        SceneReadCRCError,
        SceneReadSuccess,
        SceneRetryConfirm,
        SceneExitConfirm,
        SceneReadKeyMenu,
        SceneWrite,
        SceneWriteSuccess,
        SceneEmulate,
        SceneSavedKeyMenu,
        SceneDeleteConfirm,
        SceneDeleteSuccess,
        SceneSaveName,
        SceneSaveSuccess,
        SceneInfo,
        SceneSelectKey,
        SceneAddType,
        SceneAddValue,
    };

    static const char* app_folder;
    static const char* app_extension;
    static const char* app_filetype;

    iButtonAppViewManager* get_view_manager();
    void switch_to_next_scene(Scene index);
    void search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list);
    bool switch_to_previous_scene(uint8_t count = 1);
    Scene get_previous_scene();

    const GpioPin* get_ibutton_pin();
    iButtonWorker* get_key_worker();
    iButtonKey* get_key();

    void notify_green_blink();
    void notify_yellow_blink();
    void notify_red_blink();

    void notify_error();
    void notify_success();
    void notify_green_on();
    void notify_green_off();
    void notify_red_on();
    void notify_red_off();

    void set_text_store(const char* text...);
    char* get_text_store();
    uint8_t get_text_store_size();

    char* get_file_name();
    uint8_t get_file_name_size();

    bool save_key(const char* key_name);
    bool load_key();
    bool load_key(const char* key_name);
    bool delete_key();

private:
    std::list<Scene> previous_scenes_list = {Scene::SceneExit};
    Scene current_scene = Scene::SceneStart;
    iButtonAppViewManager view;

    std::map<Scene, iButtonScene*> scenes = {
        {Scene::SceneStart, new iButtonSceneStart()},
        {Scene::SceneRead, new iButtonSceneRead()},
        {Scene::SceneReadCRCError, new iButtonSceneReadCRCError()},
        {Scene::SceneReadNotKeyError, new iButtonSceneReadNotKeyError()},
        {Scene::SceneReadSuccess, new iButtonSceneReadSuccess()},
        {Scene::SceneRetryConfirm, new iButtonSceneRetryConfirm()},
        {Scene::SceneExitConfirm, new iButtonSceneExitConfirm()},
        {Scene::SceneReadKeyMenu, new iButtonSceneReadKeyMenu()},
        {Scene::SceneWrite, new iButtonSceneWrite()},
        {Scene::SceneWriteSuccess, new iButtonSceneWriteSuccess()},
        {Scene::SceneEmulate, new iButtonSceneEmulate()},
        {Scene::SceneSavedKeyMenu, new iButtonSceneSavedKeyMenu()},
        {Scene::SceneDeleteConfirm, new iButtonSceneDeleteConfirm()},
        {Scene::SceneDeleteSuccess, new iButtonSceneDeleteSuccess()},
        {Scene::SceneSaveName, new iButtonSceneSaveName()},
        {Scene::SceneSaveSuccess, new iButtonSceneSaveSuccess()},
        {Scene::SceneInfo, new iButtonSceneInfo()},
        {Scene::SceneSelectKey, new iButtonSceneSelectKey()},
        {Scene::SceneAddType, new iButtonSceneAddType()},
        {Scene::SceneAddValue, new iButtonSceneAddValue()},
    };

    iButtonWorker* key_worker;
    iButtonKey* key;

    RecordController<NotificationApp> notification;
    RecordController<Storage> storage;
    RecordController<DialogsApp> dialogs;

    static const uint8_t file_name_size = 100;
    char file_name[file_name_size];

    static const uint8_t text_store_size = 128;
    char text_store[text_store_size + 1];

    bool load_key_data(string_t key_path);
    void make_app_folder();
};