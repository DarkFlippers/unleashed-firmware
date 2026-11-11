#include "ra_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const ra_on_enter_handlers[])(void*) = {
#include "ra_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const ra_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "ra_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const ra_on_exit_handlers[])(void* context) = {
#include "ra_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers ra_scene_handlers = {
    .on_enter_handlers = ra_on_enter_handlers,
    .on_event_handlers = ra_on_event_handlers,
    .on_exit_handlers = ra_on_exit_handlers,
    .scene_num = RaSceneNum,
};
