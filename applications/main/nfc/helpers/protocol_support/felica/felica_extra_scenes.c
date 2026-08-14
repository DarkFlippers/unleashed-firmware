#include "felica_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include "../nfc_protocol_support_gui_common.h"
#include "felica_render.h"

enum {
    FelicaMoreInfoStateMenu,
    FelicaMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

// ---- more_info ---------------------------------------------------------
#undef TAG
enum FelicaMoreInfoSubmenuIndex {
    FelicaMoreInfoSubmenuIndexDynamic, // dynamic indices start here
};

static void felica_scene_more_info_on_enter(NfcApp* nfc) {
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
                i + FelicaMoreInfoSubmenuIndexDynamic,
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
            nfc->submenu, state - FelicaMoreInfoStateItem + FelicaMoreInfoSubmenuIndexDynamic);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

static bool felica_scene_more_info_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaMoreInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t index = event.event - FelicaMoreInfoSubmenuIndexDynamic;

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

static void felica_scene_more_info_on_exit(NfcApp* nfc) {
    // Clear views
    widget_reset(nfc->widget);
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}

// ---- system ------------------------------------------------------------
#undef TAG
enum FelicaSystemSubmenuIndex {
    FelicaSystemSubmenuIndexDirectory,
    FelicaSystemSubmenuIndexDynamic, // dynamic indices start here
};

static void felica_scene_system_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

static void felica_scene_system_on_enter(NfcApp* nfc) {
    Submenu* submenu = nfc->submenu;
    submenu_reset(nfc->submenu);

    const uint32_t system_index =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaSystem) >> 4;
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    submenu_add_item(
        submenu,
        "Directory",
        FelicaSystemSubmenuIndexDirectory,
        felica_scene_system_submenu_callback,
        nfc);

    FuriString* label = furi_string_alloc();

    const FelicaSystem* system = simple_array_cget(data->systems, system_index);
    for(uint32_t i = 0; i < simple_array_get_count(system->services); ++i) {
        const FelicaService* service = simple_array_cget(system->services, i);
        bool is_public = (service->attr & FELICA_SERVICE_ATTRIBUTE_UNAUTH_READ) == 1;
        if(!is_public) {
            continue;
        }
        furi_string_printf(label, "Readable serv %04X", service->code);
        submenu_add_item(
            submenu,
            furi_string_get_cstr(label),
            i + FelicaSystemSubmenuIndexDynamic,
            nfc_protocol_support_common_submenu_callback,
            nfc);
    }

    furi_string_free(label);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

static bool felica_scene_system_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    const uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaSystem);
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    const uint32_t system_index = state >> 4;
    const FelicaSystem* system = simple_array_cget(data->systems, system_index);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else {
            if(event.event == FelicaSystemSubmenuIndexDirectory) {
                FuriString* temp_str = furi_string_alloc();
                nfc_more_info_render_felica_dir(system, temp_str);

                widget_add_text_scroll_element(
                    nfc->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
                furi_string_free(temp_str);

                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            } else {
                const uint32_t service_ind =
                    event.event - FelicaSystemSubmenuIndexDynamic; // offset the three enums above

                text_box_reset(nfc->text_box);
                furi_string_reset(nfc->text_box_store);

                const FelicaService* service = simple_array_cget(system->services, service_ind);
                furi_string_cat_printf(nfc->text_box_store, "Service 0x%04X\n", service->code);
                nfc_more_info_render_felica_blocks(
                    data, system, nfc->text_box_store, service->code);

                text_box_set_font(nfc->text_box, TextBoxFontHex);
                text_box_set_text(nfc->text_box, furi_string_get_cstr(nfc->text_box_store));
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            }
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneFelicaSystem, state | 1);
            consumed = true;
        }

    } else if(event.type == SceneManagerEventTypeBack) {
        if(state & 1) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneFelicaSystem, state & ~1);
            consumed = true;
        }
    }

    return consumed;
}

static void felica_scene_system_on_exit(NfcApp* nfc) {
    // Clear views
    widget_reset(nfc->widget);
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}

const NfcProtocolSupportExtraScene felica_extra_scenes[FelicaExtraSceneNum] = {
    [FelicaExtraSceneMoreInfo] =
        {
            .on_enter = felica_scene_more_info_on_enter,
            .on_event = felica_scene_more_info_on_event,
            .on_exit = felica_scene_more_info_on_exit,
        },
    [FelicaExtraSceneSystem] =
        {
            .on_enter = felica_scene_system_on_enter,
            .on_event = felica_scene_system_on_event,
            .on_exit = felica_scene_system_on_exit,
        },
};
