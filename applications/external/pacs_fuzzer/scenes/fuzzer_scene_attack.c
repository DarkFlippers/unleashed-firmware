#include "../fuzzer_i.h"
#include "../helpers/fuzzer_custom_event.h"

// TODO simlify callbacks and attack state

void fuzzer_scene_attack_worker_tick_callback(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, FuzzerCustomEventViewAttackTick);
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

static void fuzzer_scene_attack_update_uid(PacsFuzzerApp* app) {
    furi_assert(app);
    furi_assert(app->worker);
    furi_assert(app->attack_view);

    FuzzerPayload uid;
    fuzzer_worker_get_current_key(app->worker, &uid);

    fuzzer_view_attack_set_uid(app->attack_view, uid);

    free(uid.data);
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
                scene_manager_set_scene_state(
                    app->scene_manager, FuzzerSceneAttack, FuzzerAttackStateIdle);
                fuzzer_view_attack_pause(app->attack_view);
            } else {
                // Exit
                fuzzer_worker_stop(app->worker);
                scene_manager_set_scene_state(
                    app->scene_manager, FuzzerSceneAttack, FuzzerAttackStateOff);
                fuzzer_view_attack_stop(app->attack_view);
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
                       app->worker, fuzzer_view_attack_get_time_delay(app->attack_view))) {
                    scene_manager_set_scene_state(
                        app->scene_manager, FuzzerSceneAttack, FuzzerAttackStateRunning);
                    fuzzer_view_attack_start(app->attack_view);
                } else {
                    // Error?
                }
            } else if(
                scene_manager_get_scene_state(app->scene_manager, FuzzerSceneAttack) ==
                FuzzerAttackStateRunning) {
                scene_manager_set_scene_state(
                    app->scene_manager, FuzzerSceneAttack, FuzzerAttackStateIdle);
                fuzzer_view_attack_pause(app->attack_view);
                fuzzer_worker_pause(app->worker); // XXX
            }
            consumed = true;
        } else if(event.event == FuzzerCustomEventViewAttackTick) {
            fuzzer_scene_attack_update_uid(app);
            consumed = true;
        } else if(event.event == FuzzerCustomEventViewAttackEnd) {
            scene_manager_set_scene_state(
                app->scene_manager, FuzzerSceneAttack, FuzzerAttackStateEnd);
            fuzzer_view_attack_end(app->attack_view);
            consumed = true;
        }
    }

    return consumed;
}

void fuzzer_scene_attack_on_exit(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    fuzzer_worker_set_uid_chaged_callback(app->worker, NULL, NULL);
    fuzzer_worker_set_end_callback(app->worker, NULL, NULL);
}
