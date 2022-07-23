#include "chip8_scene.h"

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const chip8_scene_on_enter_handlers[])(void*) = {
#include "chip8_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const chip8_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "chip8_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const chip8_scene_on_exit_handlers[])(void* context) = {
#include "chip8_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers chip8_scene_handlers = {
    .on_enter_handlers = chip8_scene_on_enter_handlers,
    .on_event_handlers = chip8_scene_on_event_handlers,
    .on_exit_handlers = chip8_scene_on_exit_handlers,
    .scene_num = Chip8SceneNum,
};
