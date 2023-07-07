#include "../subghz_remote_app_i.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const subrem_scene_on_enter_handlers[])(void*) = {
#include "subrem_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const subrem_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "subrem_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const subrem_scene_on_exit_handlers[])(void* context) = {
#include "subrem_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers subrem_scene_handlers = {
    .on_enter_handlers = subrem_scene_on_enter_handlers,
    .on_event_handlers = subrem_scene_on_event_handlers,
    .on_exit_handlers = subrem_scene_on_exit_handlers,
    .scene_num = SubRemSceneNum,
};
