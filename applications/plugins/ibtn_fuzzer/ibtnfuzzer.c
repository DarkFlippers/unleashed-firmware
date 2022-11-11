#include "ibtnfuzzer.h"

#include "scene/ibtnfuzzer_scene_entrypoint.h"
#include "scene/ibtnfuzzer_scene_load_file.h"
#include "scene/ibtnfuzzer_scene_select_field.h"
#include "scene/ibtnfuzzer_scene_run_attack.h"
#include "scene/ibtnfuzzer_scene_load_custom_uids.h"

#define IBTNFUZZER_APP_FOLDER "/ext/ibtnfuzzer"

static void ibtnfuzzer_draw_callback(Canvas* const canvas, void* ctx) {
    iBtnFuzzerState* ibtnfuzzer_state = (iBtnFuzzerState*)acquire_mutex((ValueMutex*)ctx, 100);

    if(ibtnfuzzer_state == NULL) {
        return;
    }

    // Draw correct Canvas
    switch(ibtnfuzzer_state->current_scene) {
    case NoneScene:
    case SceneEntryPoint:
        ibtnfuzzer_scene_entrypoint_on_draw(canvas, ibtnfuzzer_state);
        break;
    case SceneSelectFile:
        ibtnfuzzer_scene_load_file_on_draw(canvas, ibtnfuzzer_state);
        break;
    case SceneSelectField:
        ibtnfuzzer_scene_select_field_on_draw(canvas, ibtnfuzzer_state);
        break;
    case SceneAttack:
        ibtnfuzzer_scene_run_attack_on_draw(canvas, ibtnfuzzer_state);
        break;
    case SceneLoadCustomUids:
        ibtnfuzzer_scene_load_custom_uids_on_draw(canvas, ibtnfuzzer_state);
        break;
    }

    release_mutex((ValueMutex*)ctx, ibtnfuzzer_state);
}

void ibtnfuzzer_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    iBtnFuzzerEvent event = {
        .evt_type = EventTypeKey, .key = input_event->key, .input_type = input_event->type};
    furi_message_queue_put(event_queue, &event, 25);
}

static void ibtnfuzzer_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    iBtnFuzzerEvent event = {
        .evt_type = EventTypeTick, .key = InputKeyUp, .input_type = InputTypeRelease};
    furi_message_queue_put(event_queue, &event, 25);
}

iBtnFuzzerState* ibtnfuzzer_alloc() {
    iBtnFuzzerState* ibtnfuzzer = malloc(sizeof(iBtnFuzzerState));
    ibtnfuzzer->notification_msg = furi_string_alloc();
    ibtnfuzzer->attack_name = furi_string_alloc();
    ibtnfuzzer->proto_name = furi_string_alloc();
    ibtnfuzzer->data_str = furi_string_alloc();

    ibtnfuzzer->previous_scene = NoneScene;
    ibtnfuzzer->current_scene = SceneEntryPoint;
    ibtnfuzzer->is_running = true;
    ibtnfuzzer->is_attacking = false;
    ibtnfuzzer->key_index = 0;
    ibtnfuzzer->menu_index = 0;
    ibtnfuzzer->menu_proto_index = 0;

    ibtnfuzzer->attack = iBtnFuzzerAttackDefaultValues;
    ibtnfuzzer->notify = furi_record_open(RECORD_NOTIFICATION);

    ibtnfuzzer->data[0] = 0x00;
    ibtnfuzzer->data[1] = 0x00;
    ibtnfuzzer->data[2] = 0x00;
    ibtnfuzzer->data[3] = 0x00;
    ibtnfuzzer->data[4] = 0x00;
    ibtnfuzzer->data[5] = 0x00;
    ibtnfuzzer->data[6] = 0x00;
    ibtnfuzzer->data[7] = 0x00;

    ibtnfuzzer->payload[0] = 0x00;
    ibtnfuzzer->payload[1] = 0x00;
    ibtnfuzzer->payload[2] = 0x00;
    ibtnfuzzer->payload[3] = 0x00;
    ibtnfuzzer->payload[4] = 0x00;
    ibtnfuzzer->payload[5] = 0x00;
    ibtnfuzzer->payload[6] = 0x00;
    ibtnfuzzer->payload[7] = 0x00;

    //Dialog
    ibtnfuzzer->dialogs = furi_record_open(RECORD_DIALOGS);

    return ibtnfuzzer;
}

void ibtnfuzzer_free(iBtnFuzzerState* ibtnfuzzer) {
    //Dialog
    furi_record_close(RECORD_DIALOGS);
    notification_message(ibtnfuzzer->notify, &sequence_blink_stop);

    // Strings
    furi_string_free(ibtnfuzzer->notification_msg);
    furi_string_free(ibtnfuzzer->attack_name);
    furi_string_free(ibtnfuzzer->proto_name);
    furi_string_free(ibtnfuzzer->data_str);

    free(ibtnfuzzer->data);
    free(ibtnfuzzer->payload);

    // The rest
    free(ibtnfuzzer);
}

// ENTRYPOINT
int32_t ibtnfuzzer_start(void* p) {
    UNUSED(p);
    // Input
    FURI_LOG_I(TAG, "Initializing input");
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(iBtnFuzzerEvent));
    iBtnFuzzerState* ibtnfuzzer_state = ibtnfuzzer_alloc();
    ValueMutex ibtnfuzzer_state_mutex;

    // Mutex
    FURI_LOG_I(TAG, "Initializing ibtnfuzzer mutex");
    if(!init_mutex(&ibtnfuzzer_state_mutex, ibtnfuzzer_state, sizeof(iBtnFuzzerState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        furi_record_close(RECORD_NOTIFICATION);
        ibtnfuzzer_free(ibtnfuzzer_state);
        return 255;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, IBTNFUZZER_APP_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", IBTNFUZZER_APP_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    // Configure view port
    FURI_LOG_I(TAG, "Initializing viewport");
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, ibtnfuzzer_draw_callback, &ibtnfuzzer_state_mutex);
    view_port_input_callback_set(view_port, ibtnfuzzer_input_callback, event_queue);

    // Configure timer
    FURI_LOG_I(TAG, "Initializing timer");
    FuriTimer* timer =
        furi_timer_alloc(ibtnfuzzer_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 10); // 10 times per second

    // Register view port in GUI
    FURI_LOG_I(TAG, "Initializing gui");
    Gui* gui = (Gui*)furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Init values
    iBtnFuzzerEvent event;
    while(ibtnfuzzer_state->is_running) {
        // Get next event
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 25);
        if(event_status == FuriStatusOk) {
            if(event.evt_type == EventTypeKey) {
                //Handle event key
                switch(ibtnfuzzer_state->current_scene) {
                case NoneScene:
                case SceneEntryPoint:
                    ibtnfuzzer_scene_entrypoint_on_event(event, ibtnfuzzer_state);
                    break;
                case SceneSelectFile:
                    ibtnfuzzer_scene_load_file_on_event(event, ibtnfuzzer_state);
                    break;
                case SceneSelectField:
                    ibtnfuzzer_scene_select_field_on_event(event, ibtnfuzzer_state);
                    break;
                case SceneAttack:
                    ibtnfuzzer_scene_run_attack_on_event(event, ibtnfuzzer_state);
                    break;
                case SceneLoadCustomUids:
                    ibtnfuzzer_scene_load_custom_uids_on_event(event, ibtnfuzzer_state);
                    break;
                }

            } else if(event.evt_type == EventTypeTick) {
                //Handle event tick
                if(ibtnfuzzer_state->current_scene != ibtnfuzzer_state->previous_scene) {
                    // Trigger Exit Scene
                    switch(ibtnfuzzer_state->previous_scene) {
                    case SceneEntryPoint:
                        ibtnfuzzer_scene_entrypoint_on_exit(ibtnfuzzer_state);
                        break;
                    case SceneSelectFile:
                        ibtnfuzzer_scene_load_file_on_exit(ibtnfuzzer_state);
                        break;
                    case SceneSelectField:
                        ibtnfuzzer_scene_select_field_on_exit(ibtnfuzzer_state);
                        break;
                    case SceneAttack:
                        ibtnfuzzer_scene_run_attack_on_exit(ibtnfuzzer_state);
                        break;
                    case SceneLoadCustomUids:
                        ibtnfuzzer_scene_load_custom_uids_on_exit(ibtnfuzzer_state);
                        break;
                    case NoneScene:
                        break;
                    }

                    // Trigger Entry Scene
                    switch(ibtnfuzzer_state->current_scene) {
                    case NoneScene:
                    case SceneEntryPoint:
                        ibtnfuzzer_scene_entrypoint_on_enter(ibtnfuzzer_state);
                        break;
                    case SceneSelectFile:
                        ibtnfuzzer_scene_load_file_on_enter(ibtnfuzzer_state);
                        break;
                    case SceneSelectField:
                        ibtnfuzzer_scene_select_field_on_enter(ibtnfuzzer_state);
                        break;
                    case SceneAttack:
                        ibtnfuzzer_scene_run_attack_on_enter(ibtnfuzzer_state);
                        break;
                    case SceneLoadCustomUids:
                        ibtnfuzzer_scene_load_custom_uids_on_enter(ibtnfuzzer_state);
                        break;
                    }
                    ibtnfuzzer_state->previous_scene = ibtnfuzzer_state->current_scene;
                }

                // Trigger Tick Scene
                switch(ibtnfuzzer_state->current_scene) {
                case NoneScene:
                case SceneEntryPoint:
                    ibtnfuzzer_scene_entrypoint_on_tick(ibtnfuzzer_state);
                    break;
                case SceneSelectFile:
                    ibtnfuzzer_scene_load_file_on_tick(ibtnfuzzer_state);
                    break;
                case SceneSelectField:
                    ibtnfuzzer_scene_select_field_on_tick(ibtnfuzzer_state);
                    break;
                case SceneAttack:
                    ibtnfuzzer_scene_run_attack_on_tick(ibtnfuzzer_state);
                    break;
                case SceneLoadCustomUids:
                    ibtnfuzzer_scene_load_custom_uids_on_tick(ibtnfuzzer_state);
                    break;
                }
                view_port_update(view_port);
            }
        }
    }

    // Cleanup
    furi_timer_stop(timer);
    furi_timer_free(timer);

    FURI_LOG_I(TAG, "Cleaning up");
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    ibtnfuzzer_free(ibtnfuzzer_state);

    return 0;
}