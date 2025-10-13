#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/felica/felica_render.h"

enum SubmenuIndex {
    SubmenuIndexDirectory,
    SubmenuIndexDynamic, // dynamic indices start here
};

static void nfc_scene_felica_system_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_felica_system_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;
    submenu_reset(nfc->submenu);

    const uint32_t system_index =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaSystem) >> 4;
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    submenu_add_item(
        submenu, "Directory", SubmenuIndexDirectory, nfc_scene_felica_system_submenu_callback, nfc);

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
            i + SubmenuIndexDynamic,
            nfc_protocol_support_common_submenu_callback,
            nfc);
    }

    furi_string_free(label);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_felica_system_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    const uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaSystem);
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    const uint32_t system_index = state >> 4;
    const FelicaSystem* system = simple_array_cget(data->systems, system_index);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else {
            if(event.event == SubmenuIndexDirectory) {
                FuriString* temp_str = furi_string_alloc();
                nfc_more_info_render_felica_dir(system, temp_str);

                widget_add_text_scroll_element(
                    nfc->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
                furi_string_free(temp_str);

                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            } else {
                const uint32_t service_ind =
                    event.event - SubmenuIndexDynamic; // offset the three enums above

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

void nfc_scene_felica_system_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear views
    widget_reset(nfc->widget);
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
