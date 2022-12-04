#include "flipfrid.h"

#include "scene/flipfrid_scene_entrypoint.h"
#include "scene/flipfrid_scene_load_file.h"
#include "scene/flipfrid_scene_select_field.h"
#include "scene/flipfrid_scene_run_attack.h"
#include "scene/flipfrid_scene_load_custom_uids.h"

#define RFIDFUZZER_APP_FOLDER "/ext/lrfid/rfidfuzzer"

static void flipfrid_draw_callback(Canvas* const canvas, void* ctx) {
    FlipFridState* flipfrid_state = (FlipFridState*)acquire_mutex((ValueMutex*)ctx, 100);

    if(flipfrid_state == NULL) {
        return;
    }

    // Draw correct Canvas
    switch(flipfrid_state->current_scene) {
    case NoneScene:
    case SceneEntryPoint:
        flipfrid_scene_entrypoint_on_draw(canvas, flipfrid_state);
        break;
    case SceneSelectFile:
        flipfrid_scene_load_file_on_draw(canvas, flipfrid_state);
        break;
    case SceneSelectField:
        flipfrid_scene_select_field_on_draw(canvas, flipfrid_state);
        break;
    case SceneAttack:
        flipfrid_scene_run_attack_on_draw(canvas, flipfrid_state);
        break;
    case SceneLoadCustomUids:
        flipfrid_scene_load_custom_uids_on_draw(canvas, flipfrid_state);
        break;
    default:
        break;
    }

    release_mutex((ValueMutex*)ctx, flipfrid_state);
}

void flipfrid_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    FlipFridEvent event = {
        .evt_type = EventTypeKey, .key = input_event->key, .input_type = input_event->type};
    furi_message_queue_put(event_queue, &event, 25);
}

static void flipfrid_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    FlipFridEvent event = {
        .evt_type = EventTypeTick, .key = InputKeyUp, .input_type = InputTypeRelease};
    furi_message_queue_put(event_queue, &event, 25);
}

FlipFridState* flipfrid_alloc() {
    FlipFridState* flipfrid = malloc(sizeof(FlipFridState));
    flipfrid->notification_msg = furi_string_alloc();
    flipfrid->attack_name = furi_string_alloc();
    flipfrid->proto_name = furi_string_alloc();
    flipfrid->data_str = furi_string_alloc();

    flipfrid->previous_scene = NoneScene;
    flipfrid->current_scene = SceneEntryPoint;
    flipfrid->is_running = true;
    flipfrid->is_attacking = false;
    flipfrid->key_index = 0;
    flipfrid->menu_index = 0;
    flipfrid->menu_proto_index = 0;

    flipfrid->attack = FlipFridAttackDefaultValues;
    flipfrid->notify = furi_record_open(RECORD_NOTIFICATION);

    flipfrid->data[0] = 0x00;
    flipfrid->data[1] = 0x00;
    flipfrid->data[2] = 0x00;
    flipfrid->data[3] = 0x00;
    flipfrid->data[4] = 0x00;
    flipfrid->data[5] = 0x00;

    flipfrid->payload[0] = 0x00;
    flipfrid->payload[1] = 0x00;
    flipfrid->payload[2] = 0x00;
    flipfrid->payload[3] = 0x00;
    flipfrid->payload[4] = 0x00;
    flipfrid->payload[5] = 0x00;

    //Dialog
    flipfrid->dialogs = furi_record_open(RECORD_DIALOGS);

    return flipfrid;
}

void flipfrid_free(FlipFridState* flipfrid) {
    //Dialog
    furi_record_close(RECORD_DIALOGS);
    notification_message(flipfrid->notify, &sequence_blink_stop);

    // Strings
    furi_string_free(flipfrid->notification_msg);
    furi_string_free(flipfrid->attack_name);
    furi_string_free(flipfrid->proto_name);
    furi_string_free(flipfrid->data_str);

    free(flipfrid->data);
    free(flipfrid->payload);

    // The rest
    free(flipfrid);
}

// ENTRYPOINT
int32_t flipfrid_start(void* p) {
    UNUSED(p);
    // Input
    FURI_LOG_I(TAG, "Initializing input");
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(FlipFridEvent));
    FlipFridState* flipfrid_state = flipfrid_alloc();
    ValueMutex flipfrid_state_mutex;

    // Mutex
    FURI_LOG_I(TAG, "Initializing flipfrid mutex");
    if(!init_mutex(&flipfrid_state_mutex, flipfrid_state, sizeof(FlipFridState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        furi_record_close(RECORD_NOTIFICATION);
        flipfrid_free(flipfrid_state);
        return 255;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, RFIDFUZZER_APP_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", RFIDFUZZER_APP_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    // Configure view port
    FURI_LOG_I(TAG, "Initializing viewport");
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, flipfrid_draw_callback, &flipfrid_state_mutex);
    view_port_input_callback_set(view_port, flipfrid_input_callback, event_queue);

    // Configure timer
    FURI_LOG_I(TAG, "Initializing timer");
    FuriTimer* timer =
        furi_timer_alloc(flipfrid_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 10); // 10 times per second

    // Register view port in GUI
    FURI_LOG_I(TAG, "Initializing gui");
    Gui* gui = (Gui*)furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Init values
    FlipFridEvent event;
    while(flipfrid_state->is_running) {
        // Get next event
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 25);
        if(event_status == FuriStatusOk) {
            if(event.evt_type == EventTypeKey) {
                //Handle event key
                switch(flipfrid_state->current_scene) {
                case NoneScene:
                case SceneEntryPoint:
                    flipfrid_scene_entrypoint_on_event(event, flipfrid_state);
                    break;
                case SceneSelectFile:
                    flipfrid_scene_load_file_on_event(event, flipfrid_state);
                    break;
                case SceneSelectField:
                    flipfrid_scene_select_field_on_event(event, flipfrid_state);
                    break;
                case SceneAttack:
                    flipfrid_scene_run_attack_on_event(event, flipfrid_state);
                    break;
                case SceneLoadCustomUids:
                    flipfrid_scene_load_custom_uids_on_event(event, flipfrid_state);
                    break;
                default:
                    break;
                }

            } else if(event.evt_type == EventTypeTick) {
                //Handle event tick
                if(flipfrid_state->current_scene != flipfrid_state->previous_scene) {
                    // Trigger Exit Scene
                    switch(flipfrid_state->previous_scene) {
                    case SceneEntryPoint:
                        flipfrid_scene_entrypoint_on_exit(flipfrid_state);
                        break;
                    case SceneSelectFile:
                        flipfrid_scene_load_file_on_exit(flipfrid_state);
                        break;
                    case SceneSelectField:
                        flipfrid_scene_select_field_on_exit(flipfrid_state);
                        break;
                    case SceneAttack:
                        flipfrid_scene_run_attack_on_exit(flipfrid_state);
                        break;
                    case SceneLoadCustomUids:
                        flipfrid_scene_load_custom_uids_on_exit(flipfrid_state);
                        break;
                    case NoneScene:
                        break;
                    default:
                        break;
                    }

                    // Trigger Entry Scene
                    switch(flipfrid_state->current_scene) {
                    case NoneScene:
                    case SceneEntryPoint:
                        flipfrid_scene_entrypoint_on_enter(flipfrid_state);
                        break;
                    case SceneSelectFile:
                        flipfrid_scene_load_file_on_enter(flipfrid_state);
                        break;
                    case SceneSelectField:
                        flipfrid_scene_select_field_on_enter(flipfrid_state);
                        break;
                    case SceneAttack:
                        flipfrid_scene_run_attack_on_enter(flipfrid_state);
                        break;
                    case SceneLoadCustomUids:
                        flipfrid_scene_load_custom_uids_on_enter(flipfrid_state);
                        break;
                    default:
                        break;
                    }
                    flipfrid_state->previous_scene = flipfrid_state->current_scene;
                }

                // Trigger Tick Scene
                switch(flipfrid_state->current_scene) {
                case NoneScene:
                case SceneEntryPoint:
                    flipfrid_scene_entrypoint_on_tick(flipfrid_state);
                    break;
                case SceneSelectFile:
                    flipfrid_scene_load_file_on_tick(flipfrid_state);
                    break;
                case SceneSelectField:
                    flipfrid_scene_select_field_on_tick(flipfrid_state);
                    break;
                case SceneAttack:
                    flipfrid_scene_run_attack_on_tick(flipfrid_state);
                    break;
                case SceneLoadCustomUids:
                    flipfrid_scene_load_custom_uids_on_tick(flipfrid_state);
                    break;
                default:
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
    flipfrid_free(flipfrid_state);

    return 0;
}