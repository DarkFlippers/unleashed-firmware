#pragma once
#include <map>
#include <irda.h>
#include <furi.h>
#include "irda/scene/irda-app-scene.hpp"
#include "irda-app-event.hpp"
#include "scene/irda-app-scene.hpp"
#include "irda-app-view-manager.hpp"
#include "irda-app-remote-manager.hpp"
#include "irda-app-transceiver.hpp"
#include <forward_list>
#include <stdint.h>
#include <notification/notification-messages.h>


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
        LearnDoneAfter,
        Remote,
        RemoteList,
        Edit,
        EditKeySelect,
        EditRename,
        EditDelete,
        EditRenameDone,
        EditDeleteDone,
    };

    void run(void);
    void switch_to_next_scene(Scene index);
    void switch_to_next_scene_without_saving(Scene index);
    bool switch_to_previous_scene(uint8_t count = 1);
    Scene get_previous_scene();
    IrdaAppViewManager* get_view_manager();
    IrdaAppSignalTransceiver* get_transceiver();
    void set_text_store(uint8_t index, const char* text...);
    char* get_text_store(uint8_t index);
    uint8_t get_text_store_size();
    IrdaAppRemoteManager* get_remote_manager();
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
    void notify_space_blink();
    void notify_double_vibro();
    void notify_green_on();
    void notify_green_off();
    void notify_click();
    void notify_click_and_blink();

    static void text_input_callback(void* context);
    static void popup_callback(void* context);

    IrdaApp() {
        notification = static_cast<NotificationApp*>(furi_record_open("notification"));
    }
    ~IrdaApp() {
        furi_record_close("notification");
        for (auto &it : scenes)
            delete it.second;
    }
private:
    static const uint8_t text_store_size = 128;
    static const uint8_t text_store_max = 2;
    char text_store[text_store_max][text_store_size + 1];
    bool learn_new_remote;
    EditElement element;
    EditAction action;
    uint32_t current_button;

    NotificationApp* notification;
    IrdaAppSignalTransceiver transceiver;
    IrdaAppViewManager view_manager;
    IrdaAppRemoteManager remote_manager;

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
        {Scene::LearnDoneAfter, new IrdaAppSceneLearnDoneAfter()},
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
