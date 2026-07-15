#include "../nfc_app_i.h"

#include "../helpers/protocol_support/mf_plus/mf_plus_render.h"

// GetVersion page, reached from the "ISO14443-4 Data" page's "More" button: the GetVersion fields,
// or a note when the card doesn't answer GetVersion (e.g. an SL3 EV0 that only speaks the Plus
// command set). Kept off the ISO14443-4 Data page so that view stays a clean protocol summary.
void nfc_scene_mf_plus_version_on_enter(void* context) {
    NfcApp* instance = context;
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    furi_string_reset(instance->text_box_store);
    nfc_render_mf_plus_version_info(data, instance->text_box_store);

    text_box_set_font(instance->text_box, TextBoxFontText);
    text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
}

bool nfc_scene_mf_plus_version_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_mf_plus_version_on_exit(void* context) {
    NfcApp* instance = context;
    text_box_reset(instance->text_box);
    furi_string_reset(instance->text_box_store);
}
