#pragma once
#include <map>
#include <list>
#include "accessor_view_manager.h"
#include "scene/accessor_scene_start.h"
#include "helpers/wiegand.h"
#include <one_wire/one_wire_host.h>
#include <notification/notification_messages.h>

class AccessorApp {
public:
    void run(void);

    AccessorApp();
    ~AccessorApp();

    enum class Scene : uint8_t {
        Exit,
        Start,
    };

    AccessorAppViewManager* get_view_manager();
    void switch_to_next_scene(Scene index);
    void search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list);
    bool switch_to_previous_scene(uint8_t count = 1);
    Scene get_previous_scene();

    void notify_green_blink();
    void notify_success();

    char* get_text_store();
    uint8_t get_text_store_size();
    void set_text_store(const char* text...);

    WIEGAND* get_wiegand();
    OneWireHost* get_one_wire();

private:
    std::list<Scene> previous_scenes_list = {Scene::Exit};
    Scene current_scene = Scene::Start;
    AccessorAppViewManager view;

    std::map<Scene, AccessorScene*> scenes = {
        {Scene::Start, new AccessorSceneStart()},
    };

    static const uint8_t text_store_size = 128;
    char text_store[text_store_size + 1];

    WIEGAND wiegand;
    OneWireHost* onewire_host;

    NotificationApp* notification;
};
