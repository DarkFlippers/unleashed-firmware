#include "../../infrared_app_i.h"

#include <dolphin/dolphin.h>

void infrared_scene_universal_common_item_callback(void* context, uint32_t index) {
    InfraredApp* infrared = context;
    uint32_t event = infrared_custom_event_pack(InfraredCustomEventTypeButtonSelected, index);
    view_dispatcher_send_custom_event(infrared->view_dispatcher, event);
}

static void infrared_scene_universal_common_progress_back_callback(void* context) {
    InfraredApp* infrared = context;
    uint32_t event = infrared_custom_event_pack(InfraredCustomEventTypeBackPressed, -1);
    view_dispatcher_send_custom_event(infrared->view_dispatcher, event);
}

static void
    infrared_scene_universal_common_show_popup(InfraredApp* infrared, uint32_t record_count) {
    ViewStack* view_stack = infrared->view_stack;
    InfraredProgressView* progress = infrared->progress;
    infrared_progress_view_set_progress_total(progress, record_count);
    infrared_progress_view_set_back_callback(
        progress, infrared_scene_universal_common_progress_back_callback, infrared);
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

bool infrared_scene_universal_common_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    InfraredBruteForce* brute_force = infrared->brute_force;
    bool consumed = false;

    if(infrared_brute_force_is_started(brute_force)) {
        if(event.type == SceneManagerEventTypeTick) {
            bool success = infrared_brute_force_send_next(brute_force);
            if(success) {
                success = infrared_progress_view_increase_progress(infrared->progress);
            }
            if(!success) {
                infrared_brute_force_stop(brute_force);
                infrared_scene_universal_common_hide_popup(infrared);
            }
            consumed = true;
        } else if(event.type == SceneManagerEventTypeCustom) {
            if(infrared_custom_event_get_type(event.event) == InfraredCustomEventTypeBackPressed) {
                infrared_brute_force_stop(brute_force);
                infrared_scene_universal_common_hide_popup(infrared);
            }
            consumed = true;
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
