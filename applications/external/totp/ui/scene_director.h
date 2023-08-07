#pragma once

#include <gui/gui.h>
#include "../types/plugin_state.h"
#include "../types/plugin_event.h"
#include "totp_scenes_enum.h"

/**
 * @brief Activates scene
 * @param plugin_state application state
 * @param scene scene to be activated
 * @param context scene context to be passed to the scene activation method
 */
void totp_scene_director_activate_scene(PluginState* const plugin_state, Scene scene);

/**
 * @brief Deactivate current scene
 * @param plugin_state application state
 */
void totp_scene_director_deactivate_active_scene(PluginState* const plugin_state);

/**
 * @brief Renders current scene
 * @param canvas canvas to render at
 * @param plugin_state application state
 */
void totp_scene_director_render(Canvas* const canvas, PluginState* const plugin_state);

/**
 * @brief Handles application event for the current scene
 * @param event event to be handled
 * @param plugin_state application state
 * @return \c true if event handled and applilcation should continue; \c false if application should be closed
 */
bool totp_scene_director_handle_event(PluginEvent* const event, PluginState* const plugin_state);

/**
 * @brief Forces screen to be redraw\updated
 * @param plugin_state application state
 */
void totp_scene_director_force_redraw(PluginState* const plugin_state);