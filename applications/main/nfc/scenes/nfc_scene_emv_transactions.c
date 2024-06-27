#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/emv/emv_render.h"

void nfc_scene_emv_transactions_on_enter(void* context) {
    NfcApp* nfc = context;
    Widget* widget = nfc->widget;
    const EmvData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolEmv);

    FuriString* temp_str = furi_string_alloc();
    nfc_render_emv_transactions(&data->emv_application, temp_str);

    widget_add_text_scroll_element(widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_emv_transactions_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_emv_transactions_on_exit(void* context) {
    NfcApp* nfc = context;

    widget_reset(nfc->widget);
}
