/**
 * @file scene_manager.h
 * GUI: SceneManager API
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Scene Manager events type */
typedef enum {
    SceneManagerEventTypeCustom,
    SceneManagerEventTypeBack,
    SceneManagerEventTypeTick,
} SceneManagerEventType;

/** Scene Manager event
 */
typedef struct {
    SceneManagerEventType type;
    uint32_t event;
} SceneManagerEvent;

/** Prototype for Scene on_enter handler */
typedef void (*AppSceneOnEnterCallback)(void* context);

/** Prototype for Scene on_event handler */
typedef bool (*AppSceneOnEventCallback)(void* context, SceneManagerEvent event);

/** Prototype for Scene on_exit handler */
typedef void (*AppSceneOnExitCallback)(void* context);

/** Scene Manager configuration structure
 * Contains array of Scene handlers
 */
typedef struct {
    const AppSceneOnEnterCallback* on_enter_handlers;
    const AppSceneOnEventCallback* on_event_handlers;
    const AppSceneOnExitCallback* on_exit_handlers;
    const uint32_t scene_num;
} SceneManagerHandlers;

typedef struct SceneManager SceneManager;

/** Set Scene state
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 * @param      state          Scene new state
 */
void scene_manager_set_scene_state(SceneManager* scene_manager, uint32_t scene_id, uint32_t state);

/** Get Scene state
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     Scene state
 */
uint32_t scene_manager_get_scene_state(const SceneManager* scene_manager, uint32_t scene_id);

/** Scene Manager allocation and configuration
 *
 * Scene Manager allocates all scenes internally
 *
 * @param      app_scene_handlers  SceneManagerHandlers instance
 * @param      context             context to be set on Scene handlers calls
 *
 * @return     SceneManager instance
 */
SceneManager* scene_manager_alloc(const SceneManagerHandlers* app_scene_handlers, void* context);

/** Free Scene Manager with allocated Scenes
 *
 * @param      scene_manager  SceneManager instance
 */
void scene_manager_free(SceneManager* scene_manager);

/** Custom event handler
 *
 * Calls Scene event handler with Custom event parameter
 *
 * @param      scene_manager  SceneManager instance
 * @param      custom_event   Custom event code
 *
 * @return     true if event was consumed, false otherwise
 */
bool scene_manager_handle_custom_event(SceneManager* scene_manager, uint32_t custom_event);

/** Back event handler
 *
 * Calls Scene event handler with Back event parameter
 *
 * @param      scene_manager  SceneManager instance
 *
 * @return     true if event was consumed, false otherwise
 */
bool scene_manager_handle_back_event(SceneManager* scene_manager);

/** Tick event handler
 *
 * Calls Scene event handler with Tick event parameter
 *
 * @param      scene_manager  SceneManager instance
 * @return     true if event was consumed, false otherwise
 */
void scene_manager_handle_tick_event(SceneManager* scene_manager);

/** Add and run next Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      next_scene_id  next Scene ID
 */
void scene_manager_next_scene(SceneManager* scene_manager, uint32_t next_scene_id);

/** Run previous Scene
 *
 * @param      scene_manager  SceneManager instance
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_previous_scene(SceneManager* scene_manager);

/** Search previous Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_has_previous_scene(const SceneManager* scene_manager, uint32_t scene_id);

/** Search and switch to previous Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_search_and_switch_to_previous_scene(
    SceneManager* scene_manager,
    uint32_t scene_id);

/** Search and switch to previous Scene, multiple choice
 *
 * @param      scene_manager    SceneManager instance
 * @param      scene_ids        Array of scene IDs
 * @param      scene_ids_size   Array of scene IDs size
 *
 * @return     true if one of previous scenes was found, false otherwise
 */
bool scene_manager_search_and_switch_to_previous_scene_one_of(
    SceneManager* scene_manager,
    const uint32_t* scene_ids,
    size_t scene_ids_size);

/** Clear Scene stack and switch to another Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_search_and_switch_to_another_scene(
    SceneManager* scene_manager,
    uint32_t scene_id);

/** Exit from current scene
 *
 * @param      scene_manager  SceneManager instance
 */
void scene_manager_stop(SceneManager* scene_manager);

#ifdef __cplusplus
}
#endif
