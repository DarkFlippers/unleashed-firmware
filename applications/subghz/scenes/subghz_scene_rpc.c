#include "../subghz_i.h"

void subghz_scene_rpc_on_enter(void* context) {
    SubGhz* subghz = context;
    Widget* widget = subghz->widget;

    widget_add_text_box_element(
        widget, 0, 0, 128, 28, AlignCenter, AlignCenter, "RPC mode", false);

    notification_message(subghz->notifications, &sequence_display_backlight_on);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
}

bool subghz_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == SubGhzCustomEventSceneExit) {
            view_dispatcher_stop(subghz->view_dispatcher);
        }
    }
    return consumed;
}

void subghz_scene_rpc_on_exit(void* context) {
    SubGhz* subghz = context;

    //subghz_rpc_exit_callback(subghz);

    widget_reset(subghz->widget);
}
