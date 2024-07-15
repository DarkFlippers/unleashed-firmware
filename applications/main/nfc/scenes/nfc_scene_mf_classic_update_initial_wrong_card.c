#include "../nfc_app_i.h"

void nfc_scene_mf_classic_update_initial_wrong_card_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_update_initial_wrong_card_on_enter(void* context) {
    NfcApp* instance = context;
    Widget* widget = instance->widget;

    notification_message(instance->notifications, &sequence_error);

    widget_add_icon_element(widget, 83, 22, &I_WarningDolphinFlip_45x42);
    widget_add_string_element(widget, 3, 4, AlignLeft, AlignTop, FontPrimary, "Wrong Card!");
    widget_add_string_multiline_element(
        widget,
        4,
        17,
        AlignLeft,
        AlignTop,
        FontSecondary,
        "Data management\nis only possible\nwith source card");
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Retry",
        nfc_scene_mf_classic_update_initial_wrong_card_widget_callback,
        instance);

    // Setup and start worker
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_update_initial_wrong_card_on_event(
    void* context,
    SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(instance->scene_manager);
        }
    }
    return consumed;
}

void nfc_scene_mf_classic_update_initial_wrong_card_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
