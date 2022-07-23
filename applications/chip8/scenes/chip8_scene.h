#pragma once

#include <gui/scene_manager.h>

#define ADD_SCENE(prefix, name, id) Chip8Scene##id,
typedef enum {
#include "chip8_scene_config.h"
    Chip8SceneNum,
} Chip8Scene;
#undef ADD_SCENE

extern const SceneManagerHandlers chip8_scene_handlers;

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "chip8_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "chip8_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "chip8_scene_config.h"
#undef ADD_SCENE
