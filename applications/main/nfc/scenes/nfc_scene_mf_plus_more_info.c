#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/mf_plus/mf_plus_render.h"

// "More info" hub, reached from the Info screen's "More" button (mf_plus's scene_more_info jumps
// straight here, DESFire-style). It splits the recovered SL3 block dump from the protocol details:
// "View Dump" is shown in a memory-light text_box within this scene (see nfc_render_mf_plus_dump for
// why a text_box and not a text-scroll widget); "ISO14443-4 Data" opens its own scene.
enum {
    MfPlusMoreInfoStateMenu,
    MfPlusMoreInfoStateItem, // states >= this: an item's sub-view is showing; index = state - Item
};

enum {
    SubmenuIndexViewDump,
    SubmenuIndexIso14443,
};

void nfc_scene_mf_plus_more_info_on_enter(void* context) {
    NfcApp* instance = context;
    Submenu* submenu = instance->submenu;
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    // A dump only exists for SL3 (the only level with recovered blocks/keys); other levels still
    // have ISO14443-4/version details worth showing.
    if(data->security_level == MfPlusSecurityLevel3) {
        submenu_add_item(
            submenu,
            "View Dump",
            SubmenuIndexViewDump,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
    submenu_add_item(
        submenu,
        "ISO14443-4 Data",
        SubmenuIndexIso14443,
        nfc_protocol_support_common_submenu_callback,
        instance);

    // Restore the selection after returning from a sub-view (e.g. the ISO14443-4 Data scene).
    const uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusMoreInfo);
    if(state >= MfPlusMoreInfoStateItem) {
        submenu_set_selected_item(submenu, state - MfPlusMoreInfoStateItem);
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMfPlusMoreInfo, MfPlusMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_plus_more_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusMoreInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexViewDump) {
            const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
            furi_string_reset(instance->text_box_store);
            nfc_render_mf_plus_dump(data, instance->text_box_store);
            text_box_set_font(instance->text_box, TextBoxFontHex);
            text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));
            view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfPlusMoreInfo,
                MfPlusMoreInfoStateItem + SubmenuIndexViewDump);
            consumed = true;
        } else if(event.event == SubmenuIndexIso14443) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfPlusMoreInfo,
                MfPlusMoreInfoStateItem + SubmenuIndexIso14443);
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusIso4Info);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= MfPlusMoreInfoStateItem) {
            // Leaving the in-scene dump text_box: return to the menu, not out of the scene.
            view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                instance->scene_manager, NfcSceneMfPlusMoreInfo, MfPlusMoreInfoStateMenu);
        } else {
            // Skip the generic more_info scene (it auto-jumps back here) and return to Info.
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneInfo);
        }
        consumed = true;
    }

    return consumed;
}

void nfc_scene_mf_plus_more_info_on_exit(void* context) {
    NfcApp* instance = context;
    text_box_reset(instance->text_box);
    furi_string_reset(instance->text_box_store);
    submenu_reset(instance->submenu);
}
