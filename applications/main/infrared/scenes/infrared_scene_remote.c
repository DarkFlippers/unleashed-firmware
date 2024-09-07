#include "../infrared_app_i.h"

typedef enum {
    ButtonIndexLearn = -2,
    ButtonIndexEdit = -1,
    ButtonIndexNA = 0,
} ButtonIndex;

static void
    infrared_scene_remote_button_menu_callback(void* context, int32_t index, InputType type) {
    InfraredApp* infrared = context;

    uint16_t custom_type;
    if(type == InputTypePress) {
        custom_type = InfraredCustomEventTypeTransmitStarted;
    } else if(type == InputTypeRelease) {
        custom_type = InfraredCustomEventTypeTransmitStopped;
    } else if(type == InputTypeShort) {
        custom_type = InfraredCustomEventTypeMenuSelected;
    } else {
        furi_crash("Unexpected input type");
    }

    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, infrared_custom_event_pack(custom_type, index));
}

void infrared_scene_remote_on_enter(void* context) {
    InfraredApp* infrared = context;
    InfraredRemote* remote = infrared->remote;
    ButtonMenu* button_menu = infrared->button_menu;
    SceneManager* scene_manager = infrared->scene_manager;

    for(size_t i = 0; i < infrared_remote_get_signal_count(remote); ++i) {
        button_menu_add_item(
            button_menu,
            infrared_remote_get_signal_name(remote, i),
            i,
            infrared_scene_remote_button_menu_callback,
            ButtonMenuItemTypeCommon,
            context);
    }

    button_menu_add_item(
        button_menu,
        "+",
        ButtonIndexLearn,
        infrared_scene_remote_button_menu_callback,
        ButtonMenuItemTypeControl,
        context);
    button_menu_add_item(
        button_menu,
        "Edit",
        ButtonIndexEdit,
        infrared_scene_remote_button_menu_callback,
        ButtonMenuItemTypeControl,
        context);

    button_menu_set_header(button_menu, infrared_remote_get_name(remote));
    const int16_t button_index =
        (signed)scene_manager_get_scene_state(scene_manager, InfraredSceneRemote);
    button_menu_set_selected_item(button_menu, button_index);
    scene_manager_set_scene_state(scene_manager, InfraredSceneRemote, ButtonIndexNA);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewButtonMenu);
}

bool infrared_scene_remote_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    const bool is_transmitter_idle = !infrared->app_state.is_transmitting;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        if(is_transmitter_idle) {
            const uint32_t possible_scenes[] = {InfraredSceneRemoteList, InfraredSceneStart};
            consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
                scene_manager, possible_scenes, COUNT_OF(possible_scenes));
        } else {
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeCustom) {
        const uint16_t custom_type = infrared_custom_event_get_type(event.event);
        const int16_t button_index = infrared_custom_event_get_value(event.event);

        if(custom_type == InfraredCustomEventTypeTransmitStarted) {
            furi_assert(button_index >= 0);
            InfraredErrorCode error = infrared_tx_start_button_index(infrared, button_index);
            if(INFRARED_ERROR_PRESENT(error)) {
                infrared_show_error_message(
                    infrared,
                    "Failed to load\n\"%s\"",
                    infrared_remote_get_signal_name(infrared->remote, button_index));
            }
            consumed = true;
        } else if(custom_type == InfraredCustomEventTypeTransmitStopped) {
            infrared_tx_stop(infrared);
            consumed = true;
        } else if(custom_type == InfraredCustomEventTypeMenuSelected) {
            furi_assert(button_index < 0);
            if(is_transmitter_idle) {
                scene_manager_set_scene_state(
                    scene_manager, InfraredSceneRemote, (unsigned)button_index);
                if(button_index == ButtonIndexLearn) {
                    infrared->app_state.is_learning_new_remote = false;
                    scene_manager_next_scene(scene_manager, InfraredSceneLearn);
                    consumed = true;
                } else if(button_index == ButtonIndexEdit) {
                    scene_manager_next_scene(scene_manager, InfraredSceneEdit);
                    consumed = true;
                }

            } else {
                consumed = true;
            }
        }
    }

    return consumed;
}

void infrared_scene_remote_on_exit(void* context) {
    InfraredApp* infrared = context;
    button_menu_reset(infrared->button_menu);
}
