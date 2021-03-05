#pragma once
#include <map>
#include <list>
#include "subghz-view-manager.h"

#include "scene/subghz-scene-start.h"
#include "scene/subghz-scene-spectrum-settings.h"

class SubghzApp {
public:
    void run(void);

    SubghzApp();
    ~SubghzApp();

    enum class Scene : uint8_t {
        SceneExit,
        SceneStart,
        SceneSpectrumSettings,
    };

    SubghzAppViewManager* get_view_manager();
    void switch_to_next_scene(Scene index);
    void search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list);
    bool switch_to_previous_scene(uint8_t count = 1);
    Scene get_previous_scene();

private:
    std::list<Scene> previous_scenes_list = {Scene::SceneExit};
    Scene current_scene = Scene::SceneStart;
    SubghzAppViewManager view;

    std::map<Scene, SubghzScene*> scenes = {
        {Scene::SceneStart, new SubghzSceneStart()},
        {Scene::SceneSpectrumSettings, new SubghzSceneSpectrumSettings()},
    };
};