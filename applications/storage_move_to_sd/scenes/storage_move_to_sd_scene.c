#include "storage_move_to_sd_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const storage_move_to_sd_on_enter_handlers[])(void*) = {
#include "storage_move_to_sd_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const storage_move_to_sd_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "storage_move_to_sd_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const storage_move_to_sd_on_exit_handlers[])(void* context) = {
#include "storage_move_to_sd_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers storage_move_to_sd_scene_handlers = {
    .on_enter_handlers = storage_move_to_sd_on_enter_handlers,
    .on_event_handlers = storage_move_to_sd_on_event_handlers,
    .on_exit_handlers = storage_move_to_sd_on_exit_handlers,
    .scene_num = StorageMoveToSdSceneNum,
};
