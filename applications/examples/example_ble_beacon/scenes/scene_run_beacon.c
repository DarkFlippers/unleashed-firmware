#include "../ble_beacon_app.h"
#include <example_ble_beacon_icons.h>

static void
    ble_beacon_app_scene_run_beacon_confirm_dialog_callback(DialogExResult result, void* context) {
    BleBeaconApp* ble_beacon = context;

    view_dispatcher_send_custom_event(ble_beacon->view_dispatcher, result);
}

static void update_status_text(BleBeaconApp* ble_beacon) {
    DialogEx* dialog_ex = ble_beacon->dialog_ex;

    dialog_ex_set_header(dialog_ex, "BLE Beacon Demo", 64, 0, AlignCenter, AlignTop);

    FuriString* status = ble_beacon->status_string;

    furi_string_reset(status);

    furi_string_cat_str(status, "Status: ");
    if(ble_beacon->is_beacon_active) {
        furi_string_cat_str(status, "Running\n");
    } else {
        furi_string_cat_str(status, "Stopped\n");
    }

    // Output MAC in reverse order
    for(int i = sizeof(ble_beacon->beacon_config.address) - 1; i >= 0; i--) {
        furi_string_cat_printf(status, "%02X", ble_beacon->beacon_config.address[i]);
        if(i > 0) {
            furi_string_cat_str(status, ":");
        }
    }

    furi_string_cat_printf(status, "\nData length: %d", ble_beacon->beacon_data_len);

    dialog_ex_set_text(dialog_ex, furi_string_get_cstr(status), 0, 29, AlignLeft, AlignCenter);

    dialog_ex_set_icon(dialog_ex, 93, 20, &I_lighthouse_35x44);

    dialog_ex_set_left_button_text(dialog_ex, "Config");

    dialog_ex_set_center_button_text(dialog_ex, ble_beacon->is_beacon_active ? "Stop" : "Start");

    dialog_ex_set_result_callback(
        dialog_ex, ble_beacon_app_scene_run_beacon_confirm_dialog_callback);
    dialog_ex_set_context(dialog_ex, ble_beacon);
}

void ble_beacon_app_scene_run_beacon_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;

    update_status_text(ble_beacon);

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewDialog);
}

bool ble_beacon_app_scene_run_beacon_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneMenu);
            return true;
        } else if(event.event == DialogExResultCenter) {
            ble_beacon->is_beacon_active = !ble_beacon->is_beacon_active;
            ble_beacon_app_update_state(ble_beacon);
            update_status_text(ble_beacon);
            return true;
        }
    }
    return false;
}

void ble_beacon_app_scene_run_beacon_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;
    UNUSED(ble_beacon);
}
