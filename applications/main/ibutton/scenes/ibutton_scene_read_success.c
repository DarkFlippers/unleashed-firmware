#include "../ibutton_i.h"

#include <dolphin/dolphin.h>

void ibutton_scene_read_success_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;

    FuriString* tmp = furi_string_alloc();

    const iButtonProtocolId protocol_id = ibutton_key_get_protocol_id(key);

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", ibutton_widget_callback, context);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", ibutton_widget_callback, context);

    furi_string_printf(
        tmp,
        "%s[%s]",
        ibutton_protocols_get_name(ibutton->protocols, protocol_id),
        ibutton_protocols_get_manufacturer(ibutton->protocols, protocol_id));

    widget_add_string_element(
        widget, 0, 2, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(tmp));

    furi_string_reset(tmp);
    ibutton_protocols_render_brief_data(ibutton->protocols, key, tmp);

    widget_add_string_multiline_element(
        widget, 0, 16, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(tmp));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    ibutton_notification_message(ibutton, iButtonNotificationMessageGreenOn);

    furi_string_free(tmp);
}

bool ibutton_scene_read_success_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        scene_manager_next_scene(scene_manager, iButtonSceneExitConfirm);
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(scene_manager, iButtonSceneReadKeyMenu);
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(scene_manager, iButtonSceneRetryConfirm);
        }
    }

    return consumed;
}

void ibutton_scene_read_success_on_exit(void* context) {
    iButton* ibutton = context;

    widget_reset(ibutton->widget);

    ibutton_notification_message(ibutton, iButtonNotificationMessageGreenOff);
}
