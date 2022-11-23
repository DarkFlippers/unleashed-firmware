#include "../dtmf_dolphin_i.h"
// #include "../dtmf_dolphin_data.h"
// #include "../dtmf_dolphin_audio.h"

void dtmf_dolphin_scene_dialer_on_enter(void* context) {
    DTMFDolphinApp* app = context;
    DTMFDolphinScene scene_id = DTMFDolphinSceneDialer;
    enum DTMFDolphinSceneState state = scene_manager_get_scene_state(app->scene_manager, scene_id);

    switch(state) {
    case DTMFDolphinSceneStateBluebox:
        dtmf_dolphin_data_set_current_section(DTMF_DOLPHIN_TONE_BLOCK_BLUEBOX);
        break;
    case DTMFDolphinSceneStateRedboxUS:
        dtmf_dolphin_data_set_current_section(DTMF_DOLPHIN_TONE_BLOCK_REDBOX_US);
        break;
    case DTMFDolphinSceneStateRedboxUK:
        dtmf_dolphin_data_set_current_section(DTMF_DOLPHIN_TONE_BLOCK_REDBOX_UK);
        break;
    case DTMFDolphinSceneStateRedboxCA:
        dtmf_dolphin_data_set_current_section(DTMF_DOLPHIN_TONE_BLOCK_REDBOX_CA);
        break;
    case DTMFDolphinSceneStateMisc:
        dtmf_dolphin_data_set_current_section(DTMF_DOLPHIN_TONE_BLOCK_MISC);
        break;
    default:
        dtmf_dolphin_data_set_current_section(DTMF_DOLPHIN_TONE_BLOCK_DIALER);
        break;
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, DTMFDolphinViewDialer);
}

bool dtmf_dolphin_scene_dialer_on_event(void* context, SceneManagerEvent event) {
    DTMFDolphinApp* app = context;
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;

    // if(event.type == SceneManagerEventTypeTick) {
    //     consumed = true;
    // }

    return consumed;
}

void dtmf_dolphin_scene_dialer_on_exit(void* context) {
    UNUSED(context);
}
