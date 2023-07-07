#include "../subghz_test_app_i.h"

void subghz_test_scene_packet_callback(SubGhzTestPacketEvent event, void* context) {
    furi_assert(context);
    SubGhzTestApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void subghz_test_scene_packet_on_enter(void* context) {
    SubGhzTestApp* app = context;
    subghz_test_packet_set_callback(
        app->subghz_test_packet, subghz_test_scene_packet_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzTestViewPacket);
}

bool subghz_test_scene_packet_on_event(void* context, SceneManagerEvent event) {
    SubGhzTestApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzTestPacketEventOnlyRx) {
            scene_manager_next_scene(app->scene_manager, SubGhzTestSceneShowOnlyRx);
            return true;
        }
    }
    return false;
}

void subghz_test_scene_packet_on_exit(void* context) {
    UNUSED(context);
}
