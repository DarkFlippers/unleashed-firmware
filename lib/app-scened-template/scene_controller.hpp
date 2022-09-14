#include <map>
#include <forward_list>
#include <initializer_list>

#define GENERIC_SCENE_ENUM_VALUES Exit, Start
#define GENERIC_EVENT_ENUM_VALUES Tick, Back

/**
 * @brief Controller for scene navigation in application
 * 
 * @tparam TScene generic scene class
 * @tparam TApp application class
 */
template <typename TScene, typename TApp>
class SceneController {
public:
    /**
     * @brief Add scene to scene container
     * 
     * @param scene_index scene index
     * @param scene_pointer scene object pointer
     */
    void add_scene(typename TApp::SceneType scene_index, TScene* scene_pointer) {
        furi_check(scenes.count(scene_index) == 0);
        scenes[scene_index] = scene_pointer;
    }

    /**
     * @brief Switch to next scene and store current scene in previous scenes list
     * 
     * @param scene_index next scene index
     * @param need_restore true, if we want the scene to restore its parameters
     */
    void switch_to_next_scene(typename TApp::SceneType scene_index, bool need_restore = false) {
        previous_scenes_list.push_front(current_scene_index);
        switch_to_scene(scene_index, need_restore);
    }

    /**
     * @brief Switch to next scene without ability to return to current scene
     * 
     * @param scene_index next scene index
     * @param need_restore true, if we want the scene to restore its parameters
     */
    void switch_to_scene(typename TApp::SceneType scene_index, bool need_restore = false) {
        if(scene_index != TApp::SceneType::Exit) {
            scenes[current_scene_index]->on_exit(app);
            current_scene_index = scene_index;
            scenes[current_scene_index]->on_enter(app, need_restore);
        }
    }

    /**
     * @brief Search the scene in the list of previous scenes and switch to it
     * 
     * @param scene_index_list list of scene indexes to which you want to switch
     */
    bool search_and_switch_to_previous_scene(
        const std::initializer_list<typename TApp::SceneType>& scene_index_list) {
        auto previous_scene_index = TApp::SceneType::Exit;
        bool scene_found = false;
        bool result = false;

        while(!scene_found) {
            previous_scene_index = get_previous_scene_index();
            for(const auto& element : scene_index_list) {
                if(previous_scene_index == element) {
                    scene_found = true;
                    result = true;
                    break;
                }

                if(previous_scene_index == TApp::SceneType::Exit) {
                    scene_found = true;
                    break;
                }
            }
        }

        if(result) {
            switch_to_scene(previous_scene_index, true);
        }

        return result;
    }

    bool search_and_switch_to_another_scene(
        const std::initializer_list<typename TApp::SceneType>& scene_index_list,
        typename TApp::SceneType scene_index) {
        auto previous_scene_index = TApp::SceneType::Exit;
        bool scene_found = false;
        bool result = false;

        while(!scene_found) {
            previous_scene_index = get_previous_scene_index();
            for(const auto& element : scene_index_list) {
                if(previous_scene_index == element) {
                    scene_found = true;
                    result = true;
                    break;
                }

                if(previous_scene_index == TApp::SceneType::Exit) {
                    scene_found = true;
                    break;
                }
            }
        }

        if(result) {
            switch_to_scene(scene_index, true);
        }

        return result;
    }

    bool has_previous_scene(
        const std::initializer_list<typename TApp::SceneType>& scene_index_list) {
        bool result = false;

        for(auto const& previous_element : previous_scenes_list) {
            for(const auto& element : scene_index_list) {
                if(previous_element == element) {
                    result = true;
                    break;
                }

                if(previous_element == TApp::SceneType::Exit) {
                    break;
                }
            }

            if(result) break;
        }

        return result;
    }

    /**
     * @brief Start application main cycle
     * 
     * @param tick_length_ms tick event length in milliseconds
     */
    void process(
        uint32_t /* tick_length_ms */ = 100,
        typename TApp::SceneType start_scene_index = TApp::SceneType::Start) {
        typename TApp::Event event;
        bool consumed;
        bool exit = false;

        current_scene_index = start_scene_index;
        scenes[current_scene_index]->on_enter(app, false);

        while(!exit) {
            app->view_controller.receive_event(&event);

            consumed = scenes[current_scene_index]->on_event(app, &event);

            if(!consumed) {
                if(event.type == TApp::EventType::Back) {
                    exit = switch_to_previous_scene();
                }
            }
        };

        scenes[current_scene_index]->on_exit(app);
    }

    /**
     * @brief Switch to previous scene
     * 
     * @param count how many steps back
     * @return true if app need to exit
     */
    bool switch_to_previous_scene(uint8_t count = 1) {
        auto previous_scene_index = TApp::SceneType::Start;

        for(uint8_t i = 0; i < count; i++) previous_scene_index = get_previous_scene_index();

        if(previous_scene_index == TApp::SceneType::Exit) return true;

        switch_to_scene(previous_scene_index, true);
        return false;
    }

    /**
     * @brief Construct a new Scene Controller object
     * 
     * @param app_pointer pointer to application class
     */
    SceneController(TApp* app_pointer) {
        app = app_pointer;
        current_scene_index = TApp::SceneType::Exit;
    }

    /**
     * @brief Destroy the Scene Controller object
     * 
     */
    ~SceneController() {
        for(auto& it : scenes) delete it.second;
    }

private:
    /**
     * @brief Scenes pointers container
     * 
     */
    std::map<typename TApp::SceneType, TScene*> scenes;

    /**
     * @brief List of indexes of previous scenes
     * 
     */
    std::forward_list<typename TApp::SceneType> previous_scenes_list;

    /**
     * @brief Current scene index holder
     * 
     */
    typename TApp::SceneType current_scene_index;

    /**
     * @brief Application pointer holder
     * 
     */
    TApp* app;

    /**
     * @brief Get the previous scene index
     * 
     * @return previous scene index
     */
    typename TApp::SceneType get_previous_scene_index() {
        auto scene_index = TApp::SceneType::Exit;

        if(!previous_scenes_list.empty()) {
            scene_index = previous_scenes_list.front();
            previous_scenes_list.pop_front();
        }

        return scene_index;
    }
};
