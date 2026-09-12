#pragma once

#include <gui/scene_manager.h>

void infrared_scene_universal_common_on_enter(void* context);
bool infrared_scene_universal_common_on_event(void* context, SceneManagerEvent event);
void infrared_scene_universal_common_on_exit(void* context);
void infrared_scene_universal_common_item_callback(void* context, uint32_t index, InputType type);

/**
 * @brief Go back to whichever Universal Remote scene the save flow was started from.
 *
 * The scene is entered anew, so the brute force starts over from the button panel.
 */
void infrared_scene_universal_common_return(void* context);
