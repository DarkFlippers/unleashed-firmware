#include "camera_suite_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const camera_suite_on_enter_handlers[])(void*) = {
#include "camera_suite_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const camera_suite_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "camera_suite_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const camera_suite_on_exit_handlers[])(void* context) = {
#include "camera_suite_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers camera_suite_scene_handlers = {
    .on_enter_handlers = camera_suite_on_enter_handlers,
    .on_event_handlers = camera_suite_on_event_handlers,
    .on_exit_handlers = camera_suite_on_exit_handlers,
    .scene_num = CameraSuiteSceneNum,
};
