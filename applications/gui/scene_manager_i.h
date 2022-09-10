/**
 * @file scene_manager_i.h
 * GUI: internal SceneManager API
 */

#pragma once

#include "scene_manager.h"
#include <m-array.h>

ARRAY_DEF(SceneManagerIdStack, uint32_t, M_DEFAULT_OPLIST);

typedef struct {
    uint32_t state;
} AppScene;

struct SceneManager {
    SceneManagerIdStack_t scene_id_stack;
    const SceneManagerHandlers* scene_handlers;
    AppScene* scene;
    void* context;
};
