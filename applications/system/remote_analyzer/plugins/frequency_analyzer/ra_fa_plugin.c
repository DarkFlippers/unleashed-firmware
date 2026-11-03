/**
 * @file ra_fa_plugin.c
 * @brief Entry point of the Frequency Analyzer .fal.
 */
#include "ra_fa_view.h"

#include <flipper_application/flipper_application.h>
#include <furi.h>

/** One analyzer at a time, and each scene entry maps a fresh copy of the plugin, so a file
 * static is enough. */
static RaFaView* ra_fa_view = NULL;

static void ra_fa_plugin_on_enter(RaFaEnv* env) {
    furi_assert(env);
    furi_assert(!ra_fa_view);

    ra_fa_view = ra_fa_view_alloc(env);
    view_dispatcher_add_view(env->view_dispatcher, env->view_id, ra_fa_view_get_view(ra_fa_view));
    view_dispatcher_switch_to_view(env->view_dispatcher, env->view_id);
}

static void ra_fa_plugin_on_exit(RaFaEnv* env) {
    furi_assert(env);
    furi_assert(ra_fa_view);

    ra_fa_view_store(ra_fa_view);

    /* Park on an app-owned view before dropping ours. Removing the current view leaves the
     * dispatcher with none, which stops the event loop for good. The switch also runs our
     * view's exit handler, which joins the scan worker - that thread runs plugin code, so it
     * has to be gone before the caller unmaps us. */
    view_dispatcher_switch_to_view(env->view_dispatcher, env->park_view_id);
    view_dispatcher_remove_view(env->view_dispatcher, env->view_id);

    ra_fa_view_free(ra_fa_view);
    ra_fa_view = NULL;
}

static const RaFaPlugin ra_fa_plugin = {
    .on_enter = ra_fa_plugin_on_enter,
    .on_exit = ra_fa_plugin_on_exit,
};

static const FlipperAppPluginDescriptor ra_fa_plugin_descriptor = {
    .appid = RA_FA_PLUGIN_APP_ID,
    .ep_api_version = RA_FA_PLUGIN_API_VERSION,
    .entry_point = &ra_fa_plugin,
};

const FlipperAppPluginDescriptor* remote_analyzer_fa_ep(void) {
    return &ra_fa_plugin_descriptor;
}
