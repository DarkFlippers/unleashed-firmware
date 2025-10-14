#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/felica/felica_render.h"

enum {
    FelicaMoreInfoStateMenu,
    FelicaMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

enum SubmenuIndex {
    SubmenuIndexDynamic, // dynamic indices start here
};

void nfc_scene_felica_more_info_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaMoreInfo);
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    switch(data->workflow_type) {
    case FelicaLite:
        widget_reset(nfc->widget);
        FuriString* temp_str = furi_string_alloc();
        nfc_more_info_render_felica_lite_dump(data, temp_str);
        widget_add_text_scroll_element(nfc->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
        furi_string_free(temp_str);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
        return;
        break;
    case FelicaStandard:
        FuriString* label = furi_string_alloc();

        for(uint32_t i = 0; i < simple_array_get_count(data->systems); ++i) {
            const FelicaSystem* system = simple_array_cget(data->systems, i);
            furi_string_printf(label, "System %04X", system->system_code);
            submenu_add_item(
                submenu,
                furi_string_get_cstr(label),
                i + SubmenuIndexDynamic,
                nfc_protocol_support_common_submenu_callback,
                nfc);
        }
        furi_string_free(label);
        break;
    default:
        break;
    }

    if(state >= FelicaMoreInfoStateItem) {
        submenu_set_selected_item(
            nfc->submenu, state - FelicaMoreInfoStateItem + SubmenuIndexDynamic);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_felica_more_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaMoreInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t index = event.event - SubmenuIndexDynamic;

        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateItem + index);
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneFelicaSystem, index << 4);
        scene_manager_next_scene(nfc->scene_manager, NfcSceneFelicaSystem);
        consumed = true;

    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= FelicaMoreInfoStateItem) {
            widget_reset(nfc->widget);
            text_box_reset(nfc->text_box);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateMenu);
        } else {
            widget_reset(nfc->widget);
            text_box_reset(nfc->text_box);
            // Return directly to the Info scene
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneInfo);
        }
        consumed = true;
    }

    return consumed;
}

void nfc_scene_felica_more_info_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear views
    widget_reset(nfc->widget);
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
