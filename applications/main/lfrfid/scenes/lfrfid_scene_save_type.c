#include "../lfrfid_i.h"

typedef struct {
    uint32_t line_sel;
} SaveTypeCtx;

static void lfrfid_scene_save_type_submenu_callback(void* context, uint32_t index) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_scene_save_type_on_enter(void* context) {
    LfRfid* app = context;
    Submenu* submenu = app->submenu;

    SaveTypeCtx* state = malloc(sizeof(SaveTypeCtx));
    FuriString* protocol_string = furi_string_alloc();
    for(uint8_t i = 0; i < LFRFIDProtocolMax; i++) {
        lfrfid_manual_format_get_label(i, protocol_string);
        submenu_add_item(
            submenu,
            furi_string_get_cstr(protocol_string),
            i,
            lfrfid_scene_save_type_submenu_callback,
            app);

        // the Casi-Rusco badge is an EM4100 frame, so it sits with those
        if(i == LFRFIDProtocolEM4100_16) {
            lfrfid_manual_format_get_label(LFRFID_MANUAL_FORMAT_CASI, protocol_string);
            submenu_add_item(
                submenu,
                furi_string_get_cstr(protocol_string),
                LFRFID_MANUAL_FORMAT_CASI,
                lfrfid_scene_save_type_submenu_callback,
                app);
        }

        // the HID Proximity formats saved as Generic HIDProx sit with H10301
        if(i == LFRFIDProtocolH10301) {
            for(size_t format_index = 0; format_index < LFRFID_HID_FORMAT_COUNT; format_index++) {
                lfrfid_manual_format_get_label(
                    LFRFID_MANUAL_FORMAT_HID + format_index, protocol_string);
                submenu_add_item(
                    submenu,
                    furi_string_get_cstr(protocol_string),
                    LFRFID_MANUAL_FORMAT_HID + format_index,
                    lfrfid_scene_save_type_submenu_callback,
                    app);
            }
        }
    }
    furi_string_free(protocol_string);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, LfRfidSceneSaveType));

    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneSaveType, (uint32_t)state);

    // clear key name
    furi_string_reset(app->file_name);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewSubmenu);
}

bool lfrfid_scene_save_type_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    SaveTypeCtx* state =
        (SaveTypeCtx*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneSaveType);
    furi_check(state);

    if(event.type == SceneManagerEventTypeCustom) {
        app->manual_format = event.event;
        app->protocol_id = lfrfid_manual_format_protocol(event.event);
        state->line_sel = event.event;
        if(lfrfid_manual_format_fields_count(event.event) > 0) {
            // formats with a facility code / card number layout offer that besides hex
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveMethod);
        } else {
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveData);
        }
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_save_type_on_exit(void* context) {
    LfRfid* app = context;
    SaveTypeCtx* state =
        (SaveTypeCtx*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneSaveType);
    furi_check(state);

    submenu_reset(app->submenu);

    uint32_t line_sel = state->line_sel;
    free(state);
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneSaveType, line_sel);
}
