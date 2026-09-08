#include "subghz_feature_plugin.h"

#include "../subghz_i.h"
#include "../api/subghz_app_api_interface.h"

#include <loader/firmware_api/firmware_api.h>

#define TAG "SubGhzFeaturePlugin"

/** Every feature plugin resolves against the same two tables. Refcounted rather than owned by
 * whichever plugin loaded first, so it survives a second one being mapped alongside. */
static const ElfApiInterface* subghz_feature_plugin_resolver_acquire(SubGhz* subghz) {
    if(!subghz->api_resolver) {
        subghz->api_resolver = composite_api_resolver_alloc();
        composite_api_resolver_add(subghz->api_resolver, firmware_api_interface);
        composite_api_resolver_add(subghz->api_resolver, subghz_application_api_interface);
    }
    subghz->api_resolver_refs++;
    return composite_api_resolver_get(subghz->api_resolver);
}

static void subghz_feature_plugin_resolver_release(SubGhz* subghz) {
    furi_assert(subghz->api_resolver_refs);
    if(--subghz->api_resolver_refs == 0) {
        composite_api_resolver_free(subghz->api_resolver);
        subghz->api_resolver = NULL;
    }
}

static void subghz_feature_plugin_report_missing(SubGhz* subghz, const char* message) {
    furi_string_set(subghz->error_str, message);

    // Off the loading view before anything else: it swallows every key, Back included, so leaving
    // it up would cost the user the app if the event below went unhandled.
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
    // Deferred, because switching scenes inside on_enter would run the entering scene's own
    // on_exit first.
    view_dispatcher_send_custom_event(
        subghz->view_dispatcher, SubGhzCustomEventSceneFeaturePluginMissing);
}

const void* subghz_feature_plugin_load(
    SubGhz* subghz,
    PluginManager** manager,
    const char* app_id,
    const char* path,
    const char* missing_message) {
    furi_assert(!*manager);

    // Mapping is an SD read, and the scene being entered has already reset the menu drawn behind
    // it. The loading view also swallows what is pressed at it, so a Back cannot pop the scene the
    // failure is about to be reported to.
    view_dispatcher_show_loading(subghz->view_dispatcher);

    *manager = plugin_manager_alloc(
        app_id, SUBGHZ_FEATURE_PLUGIN_API_VERSION, subghz_feature_plugin_resolver_acquire(subghz));

    const void* entry_point = NULL;
    PluginManagerError error = plugin_manager_load_single(*manager, path);
    if(error == PluginManagerErrorNone) {
        // Checks the descriptor but not what it points at, so a .fal can still export nothing.
        entry_point = plugin_manager_get_ep(*manager, 0);
    }
    if(entry_point) {
        return entry_point;
    }

    FURI_LOG_E(TAG, "Failed to load %s (error %d)", path, error);
    // Leave nothing half-built behind, so failure and success are the same contract.
    subghz_feature_plugin_unload(subghz, manager);
    subghz_feature_plugin_report_missing(subghz, missing_message);
    return NULL;
}

void subghz_feature_plugin_unload(SubGhz* subghz, PluginManager** manager) {
    if(!*manager) {
        return;
    }
    plugin_manager_free(*manager);
    *manager = NULL;
    subghz_feature_plugin_resolver_release(subghz);
}

bool subghz_feature_plugin_handle_missing(SubGhz* subghz, SceneManagerEvent event) {
    if(event.type != SceneManagerEventTypeCustom ||
       event.event != SubGhzCustomEventSceneFeaturePluginMissing) {
        return false;
    }

    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
    return true;
}
