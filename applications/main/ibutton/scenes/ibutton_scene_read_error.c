#include "../ibutton_i.h"
#include <one_wire/maxim_crc.h>

void ibutton_scene_read_error_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;

    Widget* widget = ibutton->widget;

    FuriString* tmp = furi_string_alloc();

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", ibutton_widget_callback, context);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", ibutton_widget_callback, context);

    widget_add_string_element(
        widget, 128 / 2, 2, AlignCenter, AlignTop, FontPrimary, "Read Error");

    ibutton_protocols_render_error(ibutton->protocols, key, tmp);

    widget_add_string_multiline_element(
        widget, 128 / 2, 16, AlignCenter, AlignTop, FontSecondary, furi_string_get_cstr(tmp));

    ibutton_notification_message(ibutton, iButtonNotificationMessageError);
    ibutton_notification_message(ibutton, iButtonNotificationMessageRedOn);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    furi_string_free(tmp);
}

bool ibutton_scene_read_error_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        scene_manager_next_scene(scene_manager, iButtonSceneExitConfirm);

    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(scene_manager, iButtonSceneReadKeyMenu);
        }
    }

    return consumed;
}

void ibutton_scene_read_error_on_exit(void* context) {
    iButton* ibutton = context;

    ibutton_notification_message(ibutton, iButtonNotificationMessageRedOff);
    widget_reset(ibutton->widget);
}
