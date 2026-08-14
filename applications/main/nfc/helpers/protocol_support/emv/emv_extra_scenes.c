#include "emv_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include "../nfc_protocol_support_gui_common.h"
#include "emv_render.h"

// ---- transactions ------------------------------------------------------
static void emv_scene_transactions_on_enter(NfcApp* nfc) {
    Widget* widget = nfc->widget;
    const EmvData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolEmv);

    FuriString* temp_str = furi_string_alloc();
    nfc_render_emv_transactions(&data->emv_application, temp_str);

    widget_add_text_scroll_element(widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

static bool emv_scene_transactions_on_event(NfcApp* nfc, SceneManagerEvent event) {
    UNUSED(nfc);
    UNUSED(event);
    return false;
}

static void emv_scene_transactions_on_exit(NfcApp* nfc) {
    widget_reset(nfc->widget);
}

const NfcProtocolSupportExtraScene emv_extra_scenes[EmvExtraSceneNum] = {
    [EmvExtraSceneTransactions] =
        {
            .on_enter = emv_scene_transactions_on_enter,
            .on_event = emv_scene_transactions_on_event,
            .on_exit = emv_scene_transactions_on_exit,
        },
};
