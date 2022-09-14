#include "scene_manager_i.h"
#include <furi.h>

SceneManager* scene_manager_alloc(const SceneManagerHandlers* app_scene_handlers, void* context) {
    furi_assert(context);

    SceneManager* scene_manager = malloc(sizeof(SceneManager));
    // Set SceneManager context and scene handlers
    scene_manager->context = context;
    scene_manager->scene_handlers = app_scene_handlers;
    // Allocate all scenes
    scene_manager->scene = malloc(sizeof(AppScene) * app_scene_handlers->scene_num);
    // Initialize ScaneManager array for navigation
    SceneManagerIdStack_init(scene_manager->scene_id_stack);

    return scene_manager;
}

void scene_manager_free(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    // Clear ScaneManager array
    SceneManagerIdStack_clear(scene_manager->scene_id_stack);
    // Clear allocated scenes
    free(scene_manager->scene);
    // Free SceneManager structure
    free(scene_manager);
}

void scene_manager_set_scene_state(SceneManager* scene_manager, uint32_t scene_id, uint32_t state) {
    furi_assert(scene_manager);
    furi_assert(scene_id < scene_manager->scene_handlers->scene_num);

    scene_manager->scene[scene_id].state = state;
}

uint32_t scene_manager_get_scene_state(SceneManager* scene_manager, uint32_t scene_id) {
    furi_assert(scene_manager);
    furi_assert(scene_id < scene_manager->scene_handlers->scene_num);

    return scene_manager->scene[scene_id].state;
}

bool scene_manager_handle_custom_event(SceneManager* scene_manager, uint32_t custom_event) {
    furi_assert(scene_manager);

    SceneManagerEvent event = {
        .type = SceneManagerEventTypeCustom,
        .event = custom_event,
    };

    bool result = false;
    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t* scene_id_p = SceneManagerIdStack_back(scene_manager->scene_id_stack);
        uint32_t scene_id = *scene_id_p;
        result = scene_manager->scene_handlers->on_event_handlers[scene_id](
            scene_manager->context, event);
    }

    return result;
}

bool scene_manager_handle_back_event(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    SceneManagerEvent event = {
        .type = SceneManagerEventTypeBack,
    };

    bool consumed = false;
    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t* scene_id_p = SceneManagerIdStack_back(scene_manager->scene_id_stack);
        uint32_t scene_id = *scene_id_p;
        consumed = scene_manager->scene_handlers->on_event_handlers[scene_id](
            scene_manager->context, event);
    }

    if(!consumed) {
        consumed = scene_manager_previous_scene(scene_manager);
    }
    return consumed;
}

void scene_manager_handle_tick_event(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    SceneManagerEvent event = {
        .type = SceneManagerEventTypeTick,
    };

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t* scene_id_p = SceneManagerIdStack_back(scene_manager->scene_id_stack);
        uint32_t scene_id = *scene_id_p;
        scene_manager->scene_handlers->on_event_handlers[scene_id](scene_manager->context, event);
    }
}

void scene_manager_next_scene(SceneManager* scene_manager, uint32_t next_scene_id) {
    furi_assert(scene_manager);
    furi_assert(next_scene_id < scene_manager->scene_handlers->scene_num);

    // Check if it is not the first scene
    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t cur_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
    }
    // Add next scene and run on_enter
    SceneManagerIdStack_push_back(scene_manager->scene_id_stack, next_scene_id);
    scene_manager->scene_handlers->on_enter_handlers[next_scene_id](scene_manager->context);
}

bool scene_manager_previous_scene(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t cur_scene_id = 0;
        SceneManagerIdStack_pop_back(&cur_scene_id, scene_manager->scene_id_stack);

        // Handle exit from start scene separately
        if(SceneManagerIdStack_size(scene_manager->scene_id_stack) == 0) {
            scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
            return false;
        }
        uint32_t prev_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
        scene_manager->scene_handlers->on_enter_handlers[prev_scene_id](scene_manager->context);
        return true;
    } else {
        return false;
    }
}

bool scene_manager_search_and_switch_to_previous_scene(
    SceneManager* scene_manager,
    uint32_t scene_id) {
    furi_assert(scene_manager);

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t prev_scene_id = 0;
        uint32_t cur_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        SceneManagerIdStack_it_t scene_it;
        SceneManagerIdStack_it_last(scene_it, scene_manager->scene_id_stack);
        // Search scene with given id in navigation stack
        bool scene_found = false;
        while(!scene_found) {
            SceneManagerIdStack_previous(scene_it);
            if(SceneManagerIdStack_end_p(scene_it)) {
                return false;
            }
            prev_scene_id = *SceneManagerIdStack_ref(scene_it);
            if(prev_scene_id == scene_id) {
                scene_found = true;
            }
        }
        // Remove all scene id from navigation stack
        SceneManagerIdStack_next(scene_it);
        SceneManagerIdStack_pop_until(scene_manager->scene_id_stack, scene_it);

        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
        scene_manager->scene_handlers->on_enter_handlers[prev_scene_id](scene_manager->context);

        return true;
    } else {
        return false;
    }
}

bool scene_manager_search_and_switch_to_previous_scene_one_of(
    SceneManager* scene_manager,
    const uint32_t* scene_ids,
    size_t scene_ids_size) {
    furi_assert(scene_manager);
    furi_assert(scene_ids);
    bool scene_found = false;

    for(size_t i = 0; i < scene_ids_size; ++i) {
        const uint32_t scene_id = scene_ids[i];
        if(scene_manager_has_previous_scene(scene_manager, scene_id)) {
            scene_manager_search_and_switch_to_previous_scene(scene_manager, scene_id);
            scene_found = true;
            break;
        }
    }
    return scene_found;
}

bool scene_manager_has_previous_scene(SceneManager* scene_manager, uint32_t scene_id) {
    furi_assert(scene_manager);
    bool scene_found = false;

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t prev_scene_id;
        SceneManagerIdStack_it_t scene_it;
        SceneManagerIdStack_it_last(scene_it, scene_manager->scene_id_stack);

        // Perform search in scene stack
        while(!scene_found) {
            SceneManagerIdStack_previous(scene_it);
            if(SceneManagerIdStack_end_p(scene_it)) {
                break;
            }
            prev_scene_id = *SceneManagerIdStack_ref(scene_it);
            if(prev_scene_id == scene_id) {
                scene_found = true;
            }
        }
    }
    return scene_found;
}

bool scene_manager_search_and_switch_to_another_scene(
    SceneManager* scene_manager,
    uint32_t scene_id) {
    furi_assert(scene_manager);
    furi_assert(scene_id < scene_manager->scene_handlers->scene_num);

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t cur_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        SceneManagerIdStack_it_t scene_it;
        SceneManagerIdStack_it(scene_it, scene_manager->scene_id_stack);
        SceneManagerIdStack_next(scene_it);
        // Remove all scene id from navigation stack until first scene
        SceneManagerIdStack_pop_until(scene_manager->scene_id_stack, scene_it);
        // Add next scene
        SceneManagerIdStack_push_back(scene_manager->scene_id_stack, scene_id);

        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
        scene_manager->scene_handlers->on_enter_handlers[scene_id](scene_manager->context);

        return true;
    } else {
        return false;
    }
}

void scene_manager_stop(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack) > 0) {
        uint32_t cur_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
    }
}
