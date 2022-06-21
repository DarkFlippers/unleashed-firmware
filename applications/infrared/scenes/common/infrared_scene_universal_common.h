#pragma once

#include <gui/scene_manager.h>

void infrared_scene_universal_common_on_enter(void* context);
bool infrared_scene_universal_common_on_event(void* context, SceneManagerEvent event);
void infrared_scene_universal_common_on_exit(void* context);
void infrared_scene_universal_common_item_callback(void* context, uint32_t index);
