#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/mf_plus/mf_plus_render.h"

// "ISO14443-4 Data" page: the ISO14443-4 protocol details as a scrollable widget, with a "More"
// button to the GetVersion page. The ISO14443-4 text is small (a screen or two), so the text-scroll
// widget's per-line copy is harmless here -- unlike the multi-KB block dump, which uses a text_box.
void nfc_scene_mf_plus_iso4_info_on_enter(void* context) {
    NfcApp* instance = context;
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    furi_string_reset(instance->text_box_store);
    nfc_render_mf_plus_iso14443_4(data, instance->text_box_store);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(instance->text_box_store));
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeRight,
        "More",
        nfc_protocol_support_common_widget_callback,
        instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_plus_iso4_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom && event.event == GuiButtonTypeRight) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusVersion);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_mf_plus_iso4_info_on_exit(void* context) {
    NfcApp* instance = context;
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}
