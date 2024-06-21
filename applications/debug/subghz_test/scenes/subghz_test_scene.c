#include "../subghz_test_app_i.h" // IWYU pragma: keep

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const subghz_test_scene_on_enter_handlers[])(void*) = {
#include "subghz_test_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const subghz_test_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "subghz_test_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const subghz_test_scene_on_exit_handlers[])(void* context) = {
#include "subghz_test_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers subghz_test_scene_handlers = {
    .on_enter_handlers = subghz_test_scene_on_enter_handlers,
    .on_event_handlers = subghz_test_scene_on_event_handlers,
    .on_exit_handlers = subghz_test_scene_on_exit_handlers,
    .scene_num = SubGhzTestSceneNum,
};
