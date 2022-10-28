#include "../nfc_i.h"

void nfc_scene_mf_classic_write_fail_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_write_fail_on_enter(void* context) {
    Nfc* nfc = context;
    Widget* widget = nfc->widget;

    notification_message(nfc->notifications, &sequence_error);

    widget_add_icon_element(widget, 72, 17, &I_DolphinCommon_56x48);
    widget_add_string_element(
        widget, 7, 4, AlignLeft, AlignTop, FontPrimary, "Writing gone wrong!");
    widget_add_string_multiline_element(
        widget,
        7,
        17,
        AlignLeft,
        AlignTop,
        FontSecondary,
        "Not all sectors\nwere written\ncorrectly.");

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Finish", nfc_scene_mf_classic_write_fail_widget_callback, nfc);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_write_fail_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc->scene_manager, NfcSceneFileSelect);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            nfc->scene_manager, NfcSceneSavedMenu);
    }
    return consumed;
}

void nfc_scene_mf_classic_write_fail_on_exit(void* context) {
    Nfc* nfc = context;

    widget_reset(nfc->widget);
}
