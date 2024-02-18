#include "../ble_beacon_app.h"

static void ble_beacon_app_scene_add_type_byte_input_callback(void* context) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(
        ble_beacon->view_dispatcher, BleBeaconAppCustomEventDataEditResult);
}

void ble_beacon_app_scene_input_beacon_data_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    byte_input_set_header_text(ble_beacon->byte_input, "Enter beacon data");

    byte_input_set_result_callback(
        ble_beacon->byte_input,
        ble_beacon_app_scene_add_type_byte_input_callback,
        NULL,
        context,
        ble_beacon->beacon_data,
        sizeof(ble_beacon->beacon_data));

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewByteInput);
}

bool ble_beacon_app_scene_input_beacon_data_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BleBeaconAppCustomEventDataEditResult) {
            ble_beacon_app_update_state(ble_beacon);
            scene_manager_previous_scene(scene_manager);
            return true;
        }
    }

    return false;
}

void ble_beacon_app_scene_input_beacon_data_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;

    byte_input_set_result_callback(ble_beacon->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ble_beacon->byte_input, NULL);
}
