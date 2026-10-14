/** @file ibutton_settings_plugin.h
 *
 * ABI between the app and the settings plugin (ibutton_settings.fal).
 *
 * Settings pages are only reachable from the Settings menu, so they are loaded on demand and
 * unloaded again on the way out instead of sitting in the app image. The app owns everything
 * that outlives the plugin - the list view and the scene stack - and a page fills the list in
 * and persists its own result.
 */

#pragma once

#include <gui/modules/variable_item_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IBUTTON_SETTINGS_PLUGIN_APP_ID "IButtonSettingsPlugin"

/** Plugin ABI version.
 *
 * Guards the page layout and the order of the pages in iButtonSettingsPlugin. Bump it whenever
 * either changes, so a stale .fal left on the SD card is refused instead of being handed the
 * wrong pointers. The shared lib/ibutton types a page also uses are covered by the firmware API
 * version instead.
 */
#define IBUTTON_SETTINGS_PLUGIN_API_VERSION 1

/** One settings page. The app owns the list, and resets it afterwards. */
typedef struct {
    /** Fill the list in. The app shows it. */
    void (*on_enter)(VariableItemList* list);

    /** Persist pending edits.
     *
     * Called on the way out while the page is still the current scene and the plugin is still
     * mapped, so the app can report a failure without unwinding first.
     *
     * @return false if the settings could not be saved
     */
    bool (*on_save)(void);
} iButtonSettingsPluginPage;

typedef struct {
    const iButtonSettingsPluginPage* write_targets;
} iButtonSettingsPlugin;

#ifdef __cplusplus
}
#endif
