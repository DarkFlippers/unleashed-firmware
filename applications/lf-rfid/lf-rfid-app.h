#pragma once
#include <map>
#include <list>
#include "lf-rfid-view-manager.h"

#include "scene/lf-rfid-scene-start.h"
#include "scene/lf-rfid-scene-emulate-indala.h"
#include "scene/lf-rfid-scene-emulate-hid.h"
#include "scene/lf-rfid-scene-emulate-emmarine.h"
#include "scene/lf-rfid-scene-read-normal.h"
#include "scene/lf-rfid-scene-read-indala.h"
#include "scene/lf-rfid-scene-tune.h"

#include "helpers/rfid-reader.h"
#include "helpers/rfid-timer-emulator.h"

class LfrfidApp {
public:
    void run(void);

    LfrfidApp();
    ~LfrfidApp();

    enum class Scene : uint8_t {
        Exit,
        Start,
        ReadNormal,
        ReadIndala,
        EmulateIndala,
        EmulateHID,
        EmulateEM,
        Tune,
    };

    LfrfidAppViewManager* get_view_manager();
    void switch_to_next_scene(Scene index);
    void search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list);
    bool switch_to_previous_scene(uint8_t count = 1);
    Scene get_previous_scene();

    void notify_init();
    void notify_green_blink();
    void notify_green_on();
    void notify_green_off();

    char* get_text_store();
    uint8_t get_text_store_size();
    void set_text_store(const char* text...);

    RfidReader* get_reader();
    RfidTimerEmulator* get_emulator();

private:
    std::list<Scene> previous_scenes_list = {Scene::Exit};
    Scene current_scene = Scene::Start;
    LfrfidAppViewManager view;

    std::map<Scene, LfrfidScene*> scenes = {
        {Scene::Start, new LfrfidSceneStart()},
        {Scene::ReadNormal, new LfrfidSceneReadNormal()},
        {Scene::ReadIndala, new LfrfidSceneReadIndala()},
        {Scene::EmulateIndala, new LfrfidSceneEmulateIndala()},
        {Scene::EmulateHID, new LfrfidSceneEmulateHID()},
        {Scene::EmulateEM, new LfrfidSceneEmulateEMMarine()},
        {Scene::Tune, new LfrfidSceneTune()},
    };

    static const uint8_t text_store_size = 128;
    char text_store[text_store_size + 1];

    RfidReader reader;
    RfidTimerEmulator emulator;
};