#include "../../infrared_app_i.h"

#include <dolphin/dolphin.h>

#pragma pack(push, 1)
typedef union {
    uint32_t packed_value;
    struct {
        bool is_paused;
        uint8_t padding;
        uint16_t signal_index;
    };
} InfraredSceneState;
#pragma pack(pop)

void infrared_scene_universal_common_item_callback(void* context, uint32_t index, InputType type) {
    InfraredApp* infrared = context;
    if(type == InputTypeShort) {
        uint32_t event = infrared_custom_event_pack(InfraredCustomEventTypeButtonSelected, index);
        view_dispatcher_send_custom_event(infrared->view_dispatcher, event);
    }
}

static void infrared_scene_universal_common_progress_input_callback(
    void* context,
    InfraredProgressViewInput input) {
    InfraredApp* infrared = context;
    uint32_t event = infrared_custom_event_pack(InfraredCustomEventTypePopupInput, input);
    view_dispatcher_send_custom_event(infrared->view_dispatcher, event);
}

static void
    infrared_scene_universal_common_show_popup(InfraredApp* infrared, uint32_t record_count) {
    ViewStack* view_stack = infrared->view_stack;
    InfraredProgressView* progress = infrared->progress;
    infrared_progress_view_set_progress_total(progress, record_count);
    infrared_progress_view_set_input_callback(
        progress, infrared_scene_universal_common_progress_input_callback, infrared);
    view_stack_add_view(view_stack, infrared_progress_view_get_view(progress));
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartSend);
}

static void infrared_scene_universal_common_hide_popup(InfraredApp* infrared) {
    ViewStack* view_stack = infrared->view_stack;
    InfraredProgressView* progress = infrared->progress;
    view_stack_remove_view(view_stack, infrared_progress_view_get_view(progress));
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);
}

static int32_t infrared_scene_universal_common_task_callback(void* context) {
    InfraredApp* infrared = context;
    const InfraredErrorCode error = infrared_brute_force_calculate_messages(infrared->brute_force);
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher,
        infrared_custom_event_pack(InfraredCustomEventTypeTaskFinished, 0));

    return error;
}

void infrared_scene_universal_common_on_enter(void* context) {
    InfraredApp* infrared = context;
    view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationVertical);
    view_stack_add_view(infrared->view_stack, button_panel_get_view(infrared->button_panel));

    // Load universal remote data in background
    infrared_blocking_task_start(infrared, infrared_scene_universal_common_task_callback);
}

static void infrared_scene_universal_common_handle_popup_input(
    InfraredApp* infrared,
    InfraredProgressViewInput input) {
    InfraredBruteForce* brute_force = infrared->brute_force;
    SceneManager* scene_manager = infrared->scene_manager;
    uint32_t scene_id = scene_manager_get_current_scene(infrared->scene_manager);
    switch(input) {
    case InfraredProgressViewInputStop: {
        infrared_brute_force_stop(brute_force);
        infrared_scene_universal_common_hide_popup(infrared);
        break;
    }

    case InfraredProgressViewInputPause: {
        infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);
        infrared_progress_view_set_paused(infrared->progress, true);
        InfraredSceneState scene_state = {
            .packed_value = scene_manager_get_scene_state(scene_manager, scene_id)};
        scene_state.is_paused = true;
        if(scene_state.signal_index)
            scene_state.signal_index--; // when running, the state stores the next index
        scene_manager_set_scene_state(scene_manager, scene_id, scene_state.packed_value);
        break;
    }

    case InfraredProgressViewInputResume: {
        infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartSend);
        infrared_progress_view_set_paused(infrared->progress, false);
        InfraredSceneState scene_state = {
            .packed_value = scene_manager_get_scene_state(scene_manager, scene_id)};
        scene_state.is_paused = false;
        scene_manager_set_scene_state(scene_manager, scene_id, scene_state.packed_value);
        break;
    }

    case InfraredProgressViewInputNextSignal: {
        InfraredSceneState scene_state = {
            .packed_value = scene_manager_get_scene_state(scene_manager, scene_id)};
        scene_state.signal_index++;
        if(infrared_progress_view_set_progress(infrared->progress, scene_state.signal_index + 1))
            scene_manager_set_scene_state(scene_manager, scene_id, scene_state.packed_value);
        break;
    }

    case InfraredProgressViewInputPreviousSignal: {
        InfraredSceneState scene_state = {
            .packed_value = scene_manager_get_scene_state(scene_manager, scene_id)};
        if(scene_state.signal_index) {
            scene_state.signal_index--;
            if(infrared_progress_view_set_progress(
                   infrared->progress, scene_state.signal_index + 1))
                scene_manager_set_scene_state(scene_manager, scene_id, scene_state.packed_value);
        }
        break;
    }

    case InfraredProgressViewInputSendSingle: {
        InfraredSceneState scene_state = {
            .packed_value = scene_manager_get_scene_state(scene_manager, scene_id)};
        infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartSend);
        infrared_brute_force_send(infrared->brute_force, scene_state.signal_index);
        infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);
        break;
    }

    default:
        furi_crash();
    }
}

bool infrared_scene_universal_common_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    InfraredBruteForce* brute_force = infrared->brute_force;
    uint32_t scene_id = scene_manager_get_current_scene(infrared->scene_manager);
    bool consumed = false;

    if(infrared_brute_force_is_started(brute_force)) {
        if(event.type == SceneManagerEventTypeTick) {
            InfraredSceneState scene_state = {
                .packed_value = scene_manager_get_scene_state(scene_manager, scene_id)};

            if(!scene_state.is_paused) {
                bool success = infrared_brute_force_send(brute_force, scene_state.signal_index);
                if(success) {
                    success = infrared_progress_view_set_progress(
                        infrared->progress, scene_state.signal_index + 1);
                    scene_state.signal_index++;
                    scene_manager_set_scene_state(
                        scene_manager, scene_id, scene_state.packed_value);
                }
                if(!success) {
                    infrared_brute_force_stop(brute_force);
                    infrared_scene_universal_common_hide_popup(infrared);
                }
                consumed = true;
            }
        } else if(event.type == SceneManagerEventTypeCustom) {
            uint16_t event_type;
            int16_t event_value;
            infrared_custom_event_unpack(event.event, &event_type, &event_value);
            if(event_type == InfraredCustomEventTypePopupInput) {
                infrared_scene_universal_common_handle_popup_input(infrared, event_value);
                consumed = true;
            }
        }
    } else {
        if(event.type == SceneManagerEventTypeBack) {
            scene_manager_previous_scene(scene_manager);
            consumed = true;
        } else if(event.type == SceneManagerEventTypeCustom) {
            uint16_t event_type;
            int16_t event_value;
            infrared_custom_event_unpack(event.event, &event_type, &event_value);

            if(event_type == InfraredCustomEventTypeButtonSelected) {
                uint32_t record_count;
                if(infrared_brute_force_start(brute_force, event_value, &record_count)) {
                    scene_manager_set_scene_state(infrared->scene_manager, scene_id, 0);
                    dolphin_deed(DolphinDeedIrSend);
                    infrared_scene_universal_common_show_popup(infrared, record_count);
                } else {
                    scene_manager_next_scene(scene_manager, InfraredSceneErrorDatabases);
                }
            } else if(event_type == InfraredCustomEventTypeTaskFinished) {
                const InfraredErrorCode task_error = infrared_blocking_task_finalize(infrared);

                if(INFRARED_ERROR_PRESENT(task_error)) {
                    scene_manager_next_scene(infrared->scene_manager, InfraredSceneErrorDatabases);
                } else {
                    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewStack);
                }
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_universal_common_on_exit(void* context) {
    InfraredApp* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    view_stack_remove_view(infrared->view_stack, button_panel_get_view(button_panel));
    infrared_brute_force_reset(infrared->brute_force);
    button_panel_reset(button_panel);
}
