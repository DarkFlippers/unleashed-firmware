#include "../ibutton_i.h"
#include <toolbox/path.h>

void ibutton_scene_rpc_on_enter(void* context) {
    iButton* ibutton = context;
    Widget* widget = ibutton->widget;

    widget_add_text_box_element(
        widget, 0, 0, 128, 28, AlignCenter, AlignCenter, "RPC mode", false);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    notification_message(ibutton->notifications, &sequence_display_backlight_on);
}

bool ibutton_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    iButton* ibutton = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventRpcExit) {
            view_dispatcher_stop(ibutton->view_dispatcher);
        }
    }

    return consumed;
}

void ibutton_scene_rpc_on_exit(void* context) {
    iButton* ibutton = context;
    widget_reset(ibutton->widget);
}
