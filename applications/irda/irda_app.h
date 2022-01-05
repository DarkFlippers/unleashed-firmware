#pragma once
#include <map>
#include <irda.h>
#include <furi.h>
#include "scene/irda_app_scene.h"
#include "irda_app_event.h"
#include "scene/irda_app_scene.h"
#include "irda_app_view_manager.h"
#include "irda_app_remote_manager.h"
#include <forward_list>
#include <stdint.h>
#include <notification/notification_messages.h>
#include <irda_worker.h>

class IrdaApp {
public:
    enum class EditElement : uint8_t {
        Button,
        Remote,
    };
    enum class EditAction : uint8_t {
        Rename,
        Delete,
    };
    enum class Scene : uint8_t {
        Exit,
        Start,
        Universal,
        UniversalTV,
        UniversalAudio,
        UniversalAirConditioner,
        Learn,
        LearnSuccess,
        LearnEnterName,
        LearnDone,
        Remote,
        RemoteList,
        Edit,
        EditKeySelect,
        EditRename,
        EditDelete,
        EditRenameDone,
        EditDeleteDone,
    };

    int32_t run(void* args);
    void switch_to_next_scene(Scene index);
    void switch_to_next_scene_without_saving(Scene index);
    bool switch_to_previous_scene(uint8_t count = 1);
    Scene get_previous_scene();
    IrdaAppViewManager* get_view_manager();
    void set_text_store(uint8_t index, const char* text...);
    char* get_text_store(uint8_t index);
    uint8_t get_text_store_size();
    IrdaAppRemoteManager* get_remote_manager();

    IrdaWorker* get_irda_worker();
    const IrdaAppSignal& get_received_signal() const;
    void set_received_signal(const IrdaAppSignal& signal);

    void search_and_switch_to_previous_scene(const std::initializer_list<Scene>& scenes_list);

    void set_edit_element(EditElement value);
    EditElement get_edit_element(void);

    void set_edit_action(EditAction value);
    EditAction get_edit_action(void);

    bool get_learn_new_remote();
    void set_learn_new_remote(bool value);

    enum : int {
        ButtonNA = -1,
    };
    int get_current_button();
    void set_current_button(int value);

    void notify_success();
    void notify_red_blink();
    void notify_sent_just_learnt();
    void notify_green_on();
    void notify_green_off();
    void notify_click();
    void notify_click_and_green_blink();
    void notify_blink_green();

    static void text_input_callback(void* context);
    static void popup_callback(void* context);

    IrdaApp();
    ~IrdaApp();

private:
    static inline const uint8_t text_store_size = 128;
    static inline const uint8_t text_store_max = 2;
    char text_store[text_store_max][text_store_size + 1];
    bool learn_new_remote;
    EditElement element;
    EditAction action;
    uint32_t current_button;

    NotificationApp* notification;
    IrdaAppViewManager view_manager;
    IrdaAppRemoteManager remote_manager;
    IrdaWorker* irda_worker;
    IrdaAppSignal received_signal;

    std::forward_list<Scene> previous_scenes_list;
    Scene current_scene = Scene::Start;

    std::map<Scene, IrdaAppScene*> scenes = {
        {Scene::Start, new IrdaAppSceneStart()},
        {Scene::Universal, new IrdaAppSceneUniversal()},
        {Scene::UniversalTV, new IrdaAppSceneUniversalTV()},
        //        {Scene::UniversalAudio, new IrdaAppSceneUniversalAudio()},
        {Scene::Learn, new IrdaAppSceneLearn()},
        {Scene::LearnSuccess, new IrdaAppSceneLearnSuccess()},
        {Scene::LearnEnterName, new IrdaAppSceneLearnEnterName()},
        {Scene::LearnDone, new IrdaAppSceneLearnDone()},
        {Scene::Remote, new IrdaAppSceneRemote()},
        {Scene::RemoteList, new IrdaAppSceneRemoteList()},
        {Scene::Edit, new IrdaAppSceneEdit()},
        {Scene::EditKeySelect, new IrdaAppSceneEditKeySelect()},
        {Scene::EditRename, new IrdaAppSceneEditRename()},
        {Scene::EditDelete, new IrdaAppSceneEditDelete()},
        {Scene::EditRenameDone, new IrdaAppSceneEditRenameDone()},
        {Scene::EditDeleteDone, new IrdaAppSceneEditDeleteDone()},
    };
};
