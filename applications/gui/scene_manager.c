#include "scene_manager_i.h"
#include <furi.h>

SceneManager* scene_manager_alloc(const SceneManagerHandlers* app_scene_handlers, void* context) {
    furi_assert(context);

    SceneManager* scene_manager = furi_alloc(sizeof(SceneManager));
    // Set SceneManager context and scene handlers
    scene_manager->context = context;
    scene_manager->scene_handlers = app_scene_handlers;
    // Allocate all scenes
    scene_manager->scene = furi_alloc(sizeof(AppScene) * app_scene_handlers->scene_num);
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
    uint32_t scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
    return scene_manager->scene_handlers->on_event_handlers[scene_id](
        scene_manager->context, event);
}

bool scene_manager_handle_back_event(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    SceneManagerEvent event = {
        .type = SceneManagerEventTypeBack,
    };
    uint32_t scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
    bool consumed =
        scene_manager->scene_handlers->on_event_handlers[scene_id](scene_manager->context, event);
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
    uint32_t scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
    scene_manager->scene_handlers->on_event_handlers[scene_id](scene_manager->context, event);
}

void scene_manager_next_scene(SceneManager* scene_manager, uint32_t next_scene_id) {
    furi_assert(scene_manager);
    furi_assert(next_scene_id < scene_manager->scene_handlers->scene_num);

    // Check if it is not the first scene
    if(SceneManagerIdStack_size(scene_manager->scene_id_stack)) {
        uint32_t cur_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
    }
    // Add next scene and run on_enter
    SceneManagerIdStack_push_back(scene_manager->scene_id_stack, next_scene_id);
    scene_manager->scene_handlers->on_enter_handlers[next_scene_id](scene_manager->context);
}

bool scene_manager_previous_scene(SceneManager* scene_manager) {
    furi_assert(scene_manager);

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
}

bool scene_manager_search_and_switch_to_previous_scene(
    SceneManager* scene_manager,
    uint32_t scene_id) {
    furi_assert(scene_manager);

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
}

bool scene_manager_has_previous_scene(SceneManager* scene_manager, uint32_t scene_id) {
    furi_assert(scene_manager);
    bool scene_found = false;
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
    return scene_found;
}

bool scene_manager_search_and_switch_to_another_scene(
    SceneManager* scene_manager,
    uint32_t scene_id) {
    furi_assert(scene_manager);
    furi_assert(scene_id < scene_manager->scene_handlers->scene_num);

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
}

void scene_manager_stop(SceneManager* scene_manager) {
    furi_assert(scene_manager);

    if(SceneManagerIdStack_size(scene_manager->scene_id_stack)) {
        uint32_t cur_scene_id = *SceneManagerIdStack_back(scene_manager->scene_id_stack);
        scene_manager->scene_handlers->on_exit_handlers[cur_scene_id](scene_manager->context);
    }
}
