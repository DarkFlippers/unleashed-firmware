#include "subbrute.h"

#include "scene/subbrute_scene_load_file.h"
#include "scene/subbrute_scene_select_field.h"
#include "scene/subbrute_scene_run_attack.h"
#include "scene/subbrute_scene_entrypoint.h"
#include "scene/subbrute_scene_save_name.h"

static void draw_callback(Canvas* const canvas, void* ctx) {
    SubBruteState* subbrute_state = (SubBruteState*)acquire_mutex((ValueMutex*)ctx, 100);

    if(subbrute_state == NULL) {
        return;
    }

    // Draw correct Canvas
    switch(subbrute_state->current_scene) {
    case NoneScene:
    case SceneSelectFile:
        subbrute_scene_load_file_on_draw(canvas, subbrute_state);
        break;
    case SceneSelectField:
        subbrute_scene_select_field_on_draw(canvas, subbrute_state);
        break;
    case SceneAttack:
        subbrute_scene_run_attack_on_draw(canvas, subbrute_state);
        break;
    case SceneEntryPoint:
        subbrute_scene_entrypoint_on_draw(canvas, subbrute_state);
        break;
    case SceneSaveName:
        break;
    }

    release_mutex((ValueMutex*)ctx, subbrute_state);
}

void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    SubBruteEvent event = {
        .evt_type = EventTypeKey, .key = input_event->key, .input_type = input_event->type};
    furi_message_queue_put(event_queue, &event, 100);
}

static void timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    SubBruteEvent event = {
        .evt_type = EventTypeTick, .key = InputKeyUp, .input_type = InputTypeRelease};
    furi_message_queue_put(event_queue, &event, 100);
}

SubBruteState* subbrute_alloc() {
    SubBruteState* subbrute = malloc(sizeof(SubBruteState));

    string_init(subbrute->protocol);
    string_init(subbrute->preset);
    string_init(subbrute->file_path);
    string_init(subbrute->file_path_tmp);
    string_init_set(subbrute->notification_msg, "");
    string_init(subbrute->candidate);
    string_init(subbrute->flipper_format_string);

    subbrute->previous_scene = NoneScene;
    subbrute->current_scene = SceneSelectFile;
    subbrute->is_running = true;
    subbrute->is_attacking = false;
    subbrute->key_index = 7;
    subbrute->notify = furi_record_open(RECORD_NOTIFICATION);

    subbrute->view_dispatcher = view_dispatcher_alloc();

    //Dialog
    subbrute->dialogs = furi_record_open(RECORD_DIALOGS);

    subbrute->preset_def = malloc(sizeof(SubGhzPresetDefinition));

    //subbrute->flipper_format = flipper_format_string_alloc();
    //subbrute->environment = subghz_environment_alloc();

    return subbrute;
}

void subbrute_free(SubBruteState* subbrute) {
    //Dialog
    furi_record_close(RECORD_DIALOGS);

    notification_message(subbrute->notify, &sequence_blink_stop);

    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_free(subbrute->view_dispatcher);

    string_clear(subbrute->preset);
    string_clear(subbrute->candidate);

    // Path strings
    string_clear(subbrute->file_path);
    string_clear(subbrute->file_path_tmp);
    string_clear(subbrute->notification_msg);
    string_clear(subbrute->candidate);
    string_clear(subbrute->flipper_format_string);

    //flipper_format_free(subbrute->flipper_format);
    //subghz_environment_free(subbrute->environment);
    //subghz_receiver_free(subbrute->receiver);

    free(subbrute->preset_def);

    // The rest
    free(subbrute);
}

// ENTRYPOINT
int32_t subbrute_start(void* p) {
    UNUSED(p);
    // Input
    FURI_LOG_I(TAG, "Initializing input");
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(SubBruteEvent));
    SubBruteState* subbrute_state = subbrute_alloc();
    ValueMutex subbrute_state_mutex;

    // Mutex
    FURI_LOG_I(TAG, "Initializing flipfrid mutex");
    if(!init_mutex(&subbrute_state_mutex, subbrute_state, sizeof(SubBruteState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        subbrute_free(subbrute_state);
        return 255;
    }

    furi_hal_power_suppress_charge_enter();

    // Configure view port
    FURI_LOG_I(TAG, "Initializing viewport");
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &subbrute_state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Configure timer
    FURI_LOG_I(TAG, "Initializing timer");
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 10); // 10 times per second

    // Register view port in GUI
    FURI_LOG_I(TAG, "Initializing gui");
    subbrute_state->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(subbrute_state->gui, view_port, GuiLayerFullscreen);

    view_dispatcher_attach_to_gui(
        subbrute_state->view_dispatcher, subbrute_state->gui, ViewDispatcherTypeFullscreen);

    subbrute_state->current_scene = SceneEntryPoint;

    // Init values
    SubBruteEvent event;
    while(subbrute_state->is_running) {
        // Get next event
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 25);
        if(event_status == FuriStatusOk) {
            if(event.evt_type == EventTypeKey) {
                //Handle event key
                FURI_LOG_D(TAG, "EVENT ###");
                switch(subbrute_state->current_scene) {
                case SceneSelectFile:
                    subbrute_scene_load_file_on_event(event, subbrute_state);
                    break;
                case SceneSelectField:
                    subbrute_scene_select_field_on_event(event, subbrute_state);
                    break;
                case SceneSaveName:
                    subbrute_scene_save_name_on_event(event, subbrute_state);
                    break;
                case SceneAttack:
                    subbrute_scene_run_attack_on_event(event, subbrute_state);
                    break;
                case NoneScene:
                case SceneEntryPoint:
                    subbrute_scene_entrypoint_on_event(event, subbrute_state);
                    break;
                }

            } else if(event.evt_type == EventTypeTick) {
                //Handle event tick
                if(subbrute_state->current_scene != subbrute_state->previous_scene) {
                    // Trigger Exit Scene
                    switch(subbrute_state->previous_scene) {
                    case SceneSelectFile:
                        subbrute_scene_load_file_on_exit(subbrute_state);
                        break;
                    case SceneSelectField:
                        subbrute_scene_select_field_on_exit(subbrute_state);
                        break;
                    case SceneAttack:
                        subbrute_scene_run_attack_on_exit(subbrute_state);
                        break;
                    case SceneEntryPoint:
                        subbrute_scene_entrypoint_on_exit(subbrute_state);
                        break;
                    case SceneSaveName:
                        subbrute_scene_save_name_on_exit(subbrute_state);
                        break;
                    case NoneScene:
                        break;
                    }

                    // Trigger Entry Scene
                    switch(subbrute_state->current_scene) {
                    case NoneScene:
                    case SceneSelectFile:
                        subbrute_scene_load_file_on_enter(subbrute_state);
                        break;
                    case SceneSelectField:
                        subbrute_scene_select_field_on_enter(subbrute_state);
                        break;
                    case SceneAttack:
                        subbrute_scene_run_attack_on_enter(subbrute_state);
                        break;
                    case SceneSaveName:
                        subbrute_scene_save_name_on_enter(subbrute_state);
                        break;
                    case SceneEntryPoint:
                        subbrute_scene_entrypoint_on_enter(subbrute_state);
                        break;
                    }
                    subbrute_state->previous_scene = subbrute_state->current_scene;
                }

                // Trigger Tick Scene
                switch(subbrute_state->current_scene) {
                case NoneScene:
                case SceneSelectFile:
                    subbrute_scene_load_file_on_tick(subbrute_state);
                    break;
                case SceneSelectField:
                    subbrute_scene_select_field_on_tick(subbrute_state);
                    break;
                case SceneAttack:
                    //subbrute_scene_run_attack_on_tick(subbrute_state);
                    break;
                case SceneEntryPoint:
                    subbrute_scene_entrypoint_on_tick(subbrute_state);
                    break;
                case SceneSaveName:
                    subbrute_scene_save_name_on_tick(subbrute_state);
                    break;
                }
                view_port_update(view_port);
            }
        }
    }

    // Cleanup
    furi_timer_stop(timer);
    furi_timer_free(timer);

    furi_hal_power_suppress_charge_exit();

    FURI_LOG_I(TAG, "Cleaning up");
    gui_remove_view_port(subbrute_state->gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    subbrute_free(subbrute_state);

    return 0;
}