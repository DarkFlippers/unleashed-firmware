#include "../../infrared_i.h"

#include <dolphin/dolphin.h>

void infrared_scene_universal_common_item_callback(void* context, uint32_t index) {
    Infrared* infrared = context;
    uint32_t event = infrared_custom_event_pack(InfraredCustomEventTypeButtonSelected, index);
    view_dispatcher_send_custom_event(infrared->view_dispatcher, event);
}

static void infrared_scene_universal_common_progress_back_callback(void* context) {
    Infrared* infrared = context;
    uint32_t event = infrared_custom_event_pack(InfraredCustomEventTypeBackPressed, -1);
    view_dispatcher_send_custom_event(infrared->view_dispatcher, event);
}

static void infrared_scene_universal_common_show_popup(Infrared* infrared, uint32_t record_count) {
    ViewStack* view_stack = infrared->view_stack;
    InfraredProgressView* progress = infrared->progress;
    infrared_progress_view_set_progress_total(progress, record_count);
    infrared_progress_view_set_back_callback(
        progress, infrared_scene_universal_common_progress_back_callback, infrared);
    view_stack_add_view(view_stack, infrared_progress_view_get_view(progress));
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartSend);
}

static void infrared_scene_universal_common_hide_popup(Infrared* infrared) {
    ViewStack* view_stack = infrared->view_stack;
    InfraredProgressView* progress = infrared->progress;
    view_stack_remove_view(view_stack, infrared_progress_view_get_view(progress));
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);
}

void infrared_scene_universal_common_on_enter(void* context) {
    Infrared* infrared = context;
    view_stack_add_view(infrared->view_stack, button_panel_get_view(infrared->button_panel));
}

bool infrared_scene_universal_common_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
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
                consumed = true;
            }
        }
    } else {
        if(event.type == SceneManagerEventTypeBack) {
            scene_manager_previous_scene(scene_manager);
            consumed = true;
        } else if(event.type == SceneManagerEventTypeCustom) {
            if(infrared_custom_event_get_type(event.event) ==
               InfraredCustomEventTypeButtonSelected) {
                uint32_t record_count;
                if(infrared_brute_force_start(
                       brute_force, infrared_custom_event_get_value(event.event), &record_count)) {
                    DOLPHIN_DEED(DolphinDeedIrBruteForce);
                    infrared_scene_universal_common_show_popup(infrared, record_count);
                } else {
                    scene_manager_next_scene(scene_manager, InfraredSceneErrorDatabases);
                }
                consumed = true;
            }
        }
    }

    return consumed;
}

void infrared_scene_universal_common_on_exit(void* context) {
    Infrared* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    view_stack_remove_view(infrared->view_stack, button_panel_get_view(button_panel));
    infrared_brute_force_reset(infrared->brute_force);
    button_panel_reset(button_panel);
}
