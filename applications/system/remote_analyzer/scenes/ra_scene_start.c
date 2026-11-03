#include "../remote_analyzer_i.h"

typedef enum {
    RaSceneStartIndexRead,
    RaSceneStartIndexDecodeRaw,
    RaSceneStartIndexFrequencyAnalyzer,
    RaSceneStartIndexAbout,
} RaSceneStartIndex;

static void ra_scene_start_callback(void* context, uint32_t index) {
    furi_assert(context);
    RemoteAnalyzer* app = context;
    scene_manager_set_scene_state(app->scene_manager, RaSceneStart, index);

    switch(index) {
    case RaSceneStartIndexDecodeRaw:
        scene_manager_next_scene(app->scene_manager, RaSceneDecodeRaw);
        break;
    case RaSceneStartIndexFrequencyAnalyzer:
        scene_manager_next_scene(app->scene_manager, RaSceneFrequencyAnalyzer);
        break;
    case RaSceneStartIndexAbout:
        scene_manager_next_scene(app->scene_manager, RaSceneAbout);
        break;
    default:
        scene_manager_next_scene(app->scene_manager, RaSceneRead);
        break;
    }
}

void ra_scene_start_on_enter(void* context) {
    RemoteAnalyzer* app = context;

    submenu_add_item(app->submenu, "Read", RaSceneStartIndexRead, ra_scene_start_callback, app);
    submenu_add_item(
        app->submenu, "Decode RAW File", RaSceneStartIndexDecodeRaw, ra_scene_start_callback, app);
    submenu_add_item(
        app->submenu,
        "Frequency Analyzer",
        RaSceneStartIndexFrequencyAnalyzer,
        ra_scene_start_callback,
        app);
    submenu_add_item(
        app->submenu, "How to use", RaSceneStartIndexAbout, ra_scene_start_callback, app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, RaSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, RaViewIdSubmenu);
}

bool ra_scene_start_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void ra_scene_start_on_exit(void* context) {
    RemoteAnalyzer* app = context;
    submenu_reset(app->submenu);
}
