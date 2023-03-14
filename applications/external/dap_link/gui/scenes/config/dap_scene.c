#include "dap_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const dap_scene_on_enter_handlers[])(void*) = {
#include "dap_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const dap_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "dap_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const dap_scene_on_exit_handlers[])(void* context) = {
#include "dap_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers dap_scene_handlers = {
    .on_enter_handlers = dap_scene_on_enter_handlers,
    .on_event_handlers = dap_scene_on_event_handlers,
    .on_exit_handlers = dap_scene_on_exit_handlers,
    .scene_num = DapSceneNum,
};
