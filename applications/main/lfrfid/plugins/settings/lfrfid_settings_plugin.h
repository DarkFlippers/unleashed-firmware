/** @file lfrfid_settings_plugin.h
 *
 * ABI between the app and the settings plugin (lfrfid_settings.fal).
 *
 * Settings pages are only reachable from the Settings menu, so they are loaded on demand and
 * unloaded again on the way out instead of sitting in the app image. The app keeps ownership
 * of everything that outlives the plugin - the view, the scene stack, the settings file - and
 * a page only fills the view in and writes the result back.
 */

#pragma once

#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LFRFID_SETTINGS_PLUGIN_APP_ID "LfRfidSettingsPlugin"

/** Plugin ABI version.
 *
 * Guards everything below: the context and page layouts, and the order of the pages in
 * LfRfidSettingsPlugin. Bump it whenever any of those change, so a stale .fal left on the SD
 * card is refused instead of being handed the wrong pointers. Deliberately independent of the
 * app's own struct layout, which no page ever sees.
 */
#define LFRFID_SETTINGS_PLUGIN_API_VERSION 1

/** What a page is given by the app. Valid only for the duration of the call. */
typedef struct {
    VariableItemList* list; /**< App owned, already registered with the view dispatcher */
    ViewDispatcher* view_dispatcher; /**< For switching to the list on enter */
    uint32_t view_id; /**< Id the list is registered under */
} LfRfidSettingsPluginCtx;

/** One settings page. The app resets the list after on_exit(), so a page never has to. */
typedef struct {
    void (*on_enter)(const LfRfidSettingsPluginCtx* ctx);
    void (*on_exit)(const LfRfidSettingsPluginCtx* ctx);
} LfRfidSettingsPluginPage;

typedef struct {
    const LfRfidSettingsPluginPage* write_targets;
} LfRfidSettingsPlugin;

#ifdef __cplusplus
}
#endif
