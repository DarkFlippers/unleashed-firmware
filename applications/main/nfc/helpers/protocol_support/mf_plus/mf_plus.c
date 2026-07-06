#include "mf_plus.h"
#include "mf_plus_render.h"

#include <nfc/protocols/mf_plus/mf_plus_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_4a/iso14443_4a_i.h"

enum {
    SubmenuIndexShowKeys = SubmenuIndexCommonMax,
};

// SL3 is the only level with recovered AES content: SL0/SL1/SL2 have no sector keys/blocks here
// (SL1 is read as MIFARE Classic and never reaches this handler), so only SL3 gets full emulation,
// More Info, and the Show Keys view. The dictionary attack runs automatically on read (see the read
// on_event), so there is no manual "Unlock with Dictionary" menu entry.
static bool nfc_scene_mf_plus_is_sl3(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    return data->security_level == MfPlusSecurityLevel3;
}

// "Show Keys" lists the recovered SL3 sector and admin keys. Offered on both the read and saved
// menus for SL3 cards; always present (even with no keys yet) so the view is always reachable.
static void nfc_scene_mf_plus_menu_on_enter(NfcApp* instance) {
    if(!nfc_scene_mf_plus_is_sl3(instance)) return;
    submenu_add_item(
        instance->submenu,
        "Show Keys",
        SubmenuIndexShowKeys,
        nfc_protocol_support_common_submenu_callback,
        instance);
}

static bool nfc_scene_mf_plus_menu_on_event(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom && event.event == SubmenuIndexShowKeys) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusShowKeys);
        return true;
    }
    return false;
}

static void nfc_scene_info_on_enter_mf_plus(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfPlusData* data = nfc_device_get_data(device, NfcProtocolMfPlus);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");
    nfc_render_mf_plus_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_mf_plus(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfPlusData* data = nfc_device_get_data(device, NfcProtocolMfPlus);

    furi_string_reset(instance->text_box_store);
    nfc_render_mf_plus_data(data, instance->text_box_store);

    text_box_set_font(instance->text_box, TextBoxFontHex);
    text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
}

static NfcCommand nfc_scene_read_poller_callback_mf_plus(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolMfPlus);
    furi_assert(event.event_data);

    NfcApp* instance = context;
    const MfPlusPollerEvent* mf_plus_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(mf_plus_event->type == MfPlusPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfPlus, nfc_poller_get_data(instance->poller));
        // The identity scan is done. An SL3 card can be dictionary-attacked to recover its keys
        // and blocks, so continue straight into the dictionary attack (like MIFARE Classic auto-
        // runs its dict). SL0/SL1/SL2 have nothing further to read here, so finish.
        const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
        const NfcCustomEvent custom_event = (data->security_level == MfPlusSecurityLevel3) ?
                                                NfcCustomEventPollerIncomplete :
                                                NfcCustomEventPollerSuccess;
        view_dispatcher_send_custom_event(instance->view_dispatcher, custom_event);
        command = NfcCommandStop;
    } else if(mf_plus_event->type == MfPlusPollerEventTypeReadFailed) {
        command = NfcCommandReset;
    }

    return command;
}

static void nfc_scene_read_on_enter_mf_plus(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_mf_plus, instance);
}

static bool nfc_scene_read_on_event_mf_plus(NfcApp* instance, SceneManagerEvent event) {
    // Auto-continue an SL3 identity read into the dictionary attack (mirrors MIFARE Classic).
    if(event.type == SceneManagerEventTypeCustom &&
       event.event == NfcCustomEventPollerIncomplete) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusDictAttack);
        return true;
    }
    return false;
}

static void nfc_scene_read_success_on_enter_mf_plus(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfPlusData* data = nfc_device_get_data(device, NfcProtocolMfPlus);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");
    nfc_render_mf_plus_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_mf_plus(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    if(data->security_level == MfPlusSecurityLevel3) {
        // SL3 has recovered keys/blocks/config: emulate the full native card, so a reader can
        // authenticate and read it. (Reader-write handling / .shd writeback comes with the write
        // handler in a later step.)
        instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolMfPlus, data);
        nfc_listener_start(instance->listener, NULL, NULL);
    } else {
        // SL0/SL1/SL2 have no recovered SL3 memory to emulate: fall back to UID-only, like UL-AES.
        const Iso14443_4aData* iso14443_4a_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_4a);
        instance->listener =
            nfc_listener_alloc(instance->nfc, NfcProtocolIso14443_4a, iso14443_4a_data);
        nfc_listener_start(
            instance->listener, nfc_scene_emulate_listener_callback_iso14443_4a, instance);
    }
}

// SL3 exposes full native emulation (a reader can authenticate and read the recovered card);
// SL0/SL1/SL2 have no recovered memory, so they only emulate the UID. Evaluated at runtime per
// loaded card -- a static .features field would wrongly offer full emulation for every level.
// (Write-to-card is a separate later feature and is intentionally not advertised here yet.)
#define MF_PLUS_SL3_FEATURES (NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo)
#define MF_PLUS_UID_FEATURES (NfcProtocolFeatureEmulateUid | NfcProtocolFeatureMoreInfo)

static uint32_t nfc_mf_plus_get_features(NfcApp* instance) {
    return nfc_scene_mf_plus_is_sl3(instance) ? MF_PLUS_SL3_FEATURES : MF_PLUS_UID_FEATURES;
}

const NfcProtocolSupportBase nfc_protocol_support_mf_plus = {
    .features = MF_PLUS_UID_FEATURES,
    .get_features = nfc_mf_plus_get_features,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_mf_plus,
            .on_event = nfc_scene_read_on_event_mf_plus,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_scene_mf_plus_menu_on_enter,
            .on_event = nfc_scene_mf_plus_menu_on_event,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_scene_mf_plus_menu_on_enter,
            .on_event = nfc_scene_mf_plus_menu_on_event,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};

NFC_PROTOCOL_SUPPORT_PLUGIN(mf_plus, NfcProtocolMfPlus);
