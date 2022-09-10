#include "picopass_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const picopass_on_enter_handlers[])(void*) = {
#include "picopass_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const picopass_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "picopass_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const picopass_on_exit_handlers[])(void* context) = {
#include "picopass_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers picopass_scene_handlers = {
    .on_enter_handlers = picopass_on_enter_handlers,
    .on_event_handlers = picopass_on_event_handlers,
    .on_exit_handlers = picopass_on_exit_handlers,
    .scene_num = PicopassSceneNum,
};
