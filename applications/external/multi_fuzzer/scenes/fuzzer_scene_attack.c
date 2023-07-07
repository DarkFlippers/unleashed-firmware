#include "../fuzzer_i.h"
#include "../helpers/fuzzer_custom_event.h"

const NotificationSequence sequence_one_green_50_on_blink_blue = {
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_blink_start_10,
    &message_blink_set_color_blue,
    &message_do_not_reset,
    NULL,
};

static void fuzzer_scene_attack_update_uid(PacsFuzzerApp* app) {
    furi_assert(app);
    furi_assert(app->worker);
    furi_assert(app->attack_view);

    fuzzer_worker_get_current_key(app->worker, app->payload);

    fuzzer_view_attack_set_uid(app->attack_view, app->payload);
}

static void fuzzer_scene_attack_set_state(PacsFuzzerApp* app, FuzzerAttackState state) {
    furi_assert(app);

    scene_manager_set_scene_state(app->scene_manager, FuzzerSceneAttack, state);
    switch(state) {
    case FuzzerAttackStateIdle:
        notification_message(app->notifications, &sequence_blink_stop);
        fuzzer_view_attack_pause(app->attack_view);
        break;

    case FuzzerAttackStateRunning:
        notification_message(app->notifications, &sequence_blink_start_blue);
        fuzzer_view_attack_start(app->attack_view);
        break;

    case FuzzerAttackStateEnd:
        notification_message(app->notifications, &sequence_blink_stop);
        notification_message(app->notifications, &sequence_single_vibro);
        fuzzer_view_attack_end(app->attack_view);
        break;

    case FuzzerAttackStateOff:
        notification_message(app->notifications, &sequence_blink_stop);
        fuzzer_view_attack_stop(app->attack_view);
        break;
    }
}

void fuzzer_scene_attack_worker_tick_callback(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    notification_message(app->notifications, &sequence_one_green_50_on_blink_blue);
    fuzzer_scene_attack_update_uid(app);

    // view_dispatcher_send_custom_event(app->view_dispatcher, FuzzerCustomEventViewAttackTick);
}

void fuzzer_scene_attack_worker_end_callback(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, FuzzerCustomEventViewAttackEnd);
}

void fuzzer_scene_attack_callback(FuzzerCustomEvent event, void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fuzzer_scene_attack_on_enter(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    fuzzer_view_attack_set_callback(app->attack_view, fuzzer_scene_attack_callback, app);

    fuzzer_worker_set_uid_chaged_callback(
        app->worker, fuzzer_scene_attack_worker_tick_callback, app);

    fuzzer_worker_set_end_callback(app->worker, fuzzer_scene_attack_worker_end_callback, app);

    fuzzer_view_attack_reset_data(
        app->attack_view,
        fuzzer_proto_get_menu_label(app->fuzzer_state.menu_index),
        fuzzer_proto_get_name(app->fuzzer_state.proto_index));

    fuzzer_scene_attack_update_uid(app);

    scene_manager_set_scene_state(app->scene_manager, FuzzerSceneAttack, FuzzerAttackStateIdle);

    view_dispatcher_switch_to_view(app->view_dispatcher, FuzzerViewIDAttack);
}

bool fuzzer_scene_attack_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FuzzerCustomEventViewAttackBack) {
            if(scene_manager_get_scene_state(app->scene_manager, FuzzerSceneAttack) ==
               FuzzerAttackStateRunning) {
                // Pause if attack running
                fuzzer_worker_pause(app->worker);

                fuzzer_scene_attack_set_state(app, FuzzerAttackStateIdle);
            } else {
                // Exit
                fuzzer_worker_stop(app->worker);

                fuzzer_scene_attack_set_state(app, FuzzerAttackStateOff);
                if(!scene_manager_previous_scene(app->scene_manager)) {
                    scene_manager_stop(app->scene_manager);
                    view_dispatcher_stop(app->view_dispatcher);
                }
            }
            consumed = true;
        } else if(event.event == FuzzerCustomEventViewAttackOk) {
            if(scene_manager_get_scene_state(app->scene_manager, FuzzerSceneAttack) ==
               FuzzerAttackStateIdle) {
                // Start or Continue Attack
                if(fuzzer_worker_start(
                       app->worker,
                       fuzzer_view_attack_get_time_delay(app->attack_view),
                       fuzzer_view_attack_get_emu_time(app->attack_view))) {
                    fuzzer_scene_attack_set_state(app, FuzzerAttackStateRunning);
                } else {
                    // Error?
                }
            } else if(
                scene_manager_get_scene_state(app->scene_manager, FuzzerSceneAttack) ==
                FuzzerAttackStateRunning) {
                // Pause if attack running
                fuzzer_worker_pause(app->worker);

                fuzzer_scene_attack_set_state(app, FuzzerAttackStateIdle);
            }
            consumed = true;
            // } else if(event.event == FuzzerCustomEventViewAttackTick) {
            //     consumed = true;
        } else if(event.event == FuzzerCustomEventViewAttackEnd) {
            fuzzer_scene_attack_set_state(app, FuzzerAttackStateEnd);
            consumed = true;
        }
    }

    return consumed;
}

void fuzzer_scene_attack_on_exit(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    // XXX the scene has no descendants, and the return will be processed in on_event
    // fuzzer_worker_stop();

    fuzzer_worker_set_uid_chaged_callback(app->worker, NULL, NULL);
    fuzzer_worker_set_end_callback(app->worker, NULL, NULL);
}
