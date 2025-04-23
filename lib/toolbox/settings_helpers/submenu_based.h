#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SubmenuSettingsHelpherCallback)(void* context, uint32_t index);

typedef struct {
    const char* name;
    uint32_t scene_id;
} SubmenuSettingsHelperOption;

typedef struct {
    const char* app_name;
    size_t options_cnt;
    SubmenuSettingsHelperOption options[];
} SubmenuSettingsHelperDescriptor;

typedef struct SubmenuSettingsHelper SubmenuSettingsHelper;

/**
 * @brief Allocates a submenu-based settings helper
 * @param descriptor settings descriptor
 */
SubmenuSettingsHelper*
    submenu_settings_helpers_alloc(const SubmenuSettingsHelperDescriptor* descriptor);

/**
 * @brief Assigns dynamic objects to the submenu-based settings helper
 * @param helper          helper object
 * @param view_dispatcher ViewDispatcher
 * @param scene_manager   SceneManager
 * @param submenu         Submenu
 * @param submenu_view_id Submenu view id in the ViewDispatcher
 * @param main_scene_id   Main scene id in the SceneManager
 */
void submenu_settings_helpers_assign_objects(
    SubmenuSettingsHelper* helper,
    ViewDispatcher* view_dispatcher,
    SceneManager* scene_manager,
    Submenu* submenu,
    uint32_t submenu_view_id,
    uint32_t main_scene_id);

/**
 * @brief Frees a submenu-based settings helper
 * @param helper helper object
 */
void submenu_settings_helpers_free(SubmenuSettingsHelper* helper);

/**
 * @brief App start callback for the submenu-based settings helper
 * 
 * If an argument containing one of the options was provided, launches the
 * corresponding scene.
 * 
 * @param helper helper object
 * @param arg app argument, may be NULL
 * @returns true if a setting name was provided in the argument, false if normal
 *          app operation shall commence
 */
bool submenu_settings_helpers_app_start(SubmenuSettingsHelper* helper, void* arg);

/**
 * @brief Main scene enter callback for the submenu-based settings helper
 * @param helper helper object
 */
void submenu_settings_helpers_scene_enter(SubmenuSettingsHelper* helper);

/**
 * @brief Main scene event callback for the submenu-based settings helper
 * @param helper helper object
 * @param event  event data
 * @returns true if the event was consumed, false otherwise
 */
bool submenu_settings_helpers_scene_event(SubmenuSettingsHelper* helper, SceneManagerEvent event);

/**
 * @brief Main scene exit callback for the submenu-based settings helper
 * @param helper helper object
 */
void submenu_settings_helpers_scene_exit(SubmenuSettingsHelper* helper);

#ifdef __cplusplus
}
#endif
