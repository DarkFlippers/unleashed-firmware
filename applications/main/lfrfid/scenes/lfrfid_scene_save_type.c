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
        if(strcmp(
               protocol_dict_get_manufacturer(app->dict, i),
               protocol_dict_get_name(app->dict, i)) != 0) {
            furi_string_printf(
                protocol_string,
                "%s %s",
                protocol_dict_get_manufacturer(app->dict, i),
                protocol_dict_get_name(app->dict, i));
        } else {
            furi_string_printf(protocol_string, "%s", protocol_dict_get_name(app->dict, i));
        }
        submenu_add_item(
            submenu,
            furi_string_get_cstr(protocol_string),
            i,
            lfrfid_scene_save_type_submenu_callback,
            app);
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
        app->protocol_id = event.event;
        state->line_sel = event.event;
        scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveData);
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
