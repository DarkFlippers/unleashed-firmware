#include "../remote_analyzer_i.h"

/** The manual is text and nothing else, so it waits in its .fal until someone asks for it. */
void ra_scene_about_on_enter(void* context) {
    RemoteAnalyzer* app = context;

    app->about_env.view_dispatcher = app->view_dispatcher;
    app->about_env.view_id = RaViewIdPlugin;
    app->about_env.park_view_id = RaViewIdSubmenu;

    app->plugin_loader = ra_plugin_loader_load(
        RA_ABOUT_PLUGIN_APP_ID,
        RA_ABOUT_PLUGIN_API_VERSION,
        RA_PLUGIN_PATH("remote_analyzer_about"),
        (const void**)&app->about_plugin);
    if(app->about_plugin == NULL) {
        // Back out from on_event, once the scene manager is done with this transition
        view_dispatcher_send_custom_event(app->view_dispatcher, RaCustomEventPluginMissing);
        return;
    }

    app->about_plugin->on_enter(&app->about_env);
}

bool ra_scene_about_on_event(void* context, SceneManagerEvent event) {
    RemoteAnalyzer* app = context;

    if(event.type == SceneManagerEventTypeCustom && event.event == RaCustomEventPluginMissing) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void ra_scene_about_on_exit(void* context) {
    RemoteAnalyzer* app = context;

    if(app->about_plugin != NULL) {
        app->about_plugin->on_exit(&app->about_env);
        app->about_plugin = NULL;
    }

    ra_plugin_loader_unload(app->plugin_loader);
    app->plugin_loader = NULL;
}
