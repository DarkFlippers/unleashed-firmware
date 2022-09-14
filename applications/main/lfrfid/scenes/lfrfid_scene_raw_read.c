#include "../lfrfid_i.h"

#define RAW_READ_TIME 5000

typedef struct {
    string_t string_file_name;
    FuriTimer* timer;
    bool is_psk;
    bool error;
} LfRfidReadRawState;

static void lfrfid_read_callback(LFRFIDWorkerReadRawResult result, void* context) {
    LfRfid* app = context;

    if(result == LFRFIDWorkerReadRawFileError) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventReadError);
    } else if(result == LFRFIDWorkerReadRawOverrun) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventReadOverrun);
    }
}

static void timer_callback(void* context) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventReadDone);
}

void lfrfid_scene_raw_read_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    LfRfidReadRawState* state = malloc(sizeof(LfRfidReadRawState));
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneRawRead, (uint32_t)state);
    string_init(state->string_file_name);

    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_make_app_folder(app);

    state->timer = furi_timer_alloc(timer_callback, FuriTimerTypeOnce, app);
    furi_timer_start(state->timer, RAW_READ_TIME);
    string_printf(
        state->string_file_name,
        "%s/%s%s",
        LFRFID_SD_FOLDER,
        string_get_cstr(app->raw_file_name),
        LFRFID_APP_RAW_ASK_EXTENSION);
    popup_set_header(popup, "Reading\nRAW RFID\nASK", 89, 30, AlignCenter, AlignTop);
    lfrfid_worker_read_raw_start(
        app->lfworker,
        string_get_cstr(state->string_file_name),
        LFRFIDWorkerReadTypeASKOnly,
        lfrfid_read_callback,
        app);

    notification_message(app->notifications, &sequence_blink_start_cyan);

    state->is_psk = false;
    state->error = false;
}

bool lfrfid_scene_raw_read_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    Popup* popup = app->popup;
    LfRfidReadRawState* state =
        (LfRfidReadRawState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneRawRead);
    bool consumed = false;

    furi_assert(state);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventReadError) {
            consumed = true;
            state->error = true;
            popup_set_header(
                popup, "Reading\nRAW RFID\nFile error", 89, 30, AlignCenter, AlignTop);
            notification_message(app->notifications, &sequence_blink_start_red);
            furi_timer_stop(state->timer);
        } else if(event.event == LfRfidEventReadDone) {
            consumed = true;
            if(!state->error) {
                if(state->is_psk) {
                    notification_message(app->notifications, &sequence_success);
                    scene_manager_next_scene(app->scene_manager, LfRfidSceneRawSuccess);
                } else {
                    popup_set_header(
                        popup, "Reading\nRAW RFID\nPSK", 89, 30, AlignCenter, AlignTop);
                    notification_message(app->notifications, &sequence_blink_start_yellow);
                    lfrfid_worker_stop(app->lfworker);
                    string_printf(
                        state->string_file_name,
                        "%s/%s%s",
                        LFRFID_SD_FOLDER,
                        string_get_cstr(app->raw_file_name),
                        LFRFID_APP_RAW_PSK_EXTENSION);
                    lfrfid_worker_read_raw_start(
                        app->lfworker,
                        string_get_cstr(state->string_file_name),
                        LFRFIDWorkerReadTypePSKOnly,
                        lfrfid_read_callback,
                        app);
                    furi_timer_start(state->timer, RAW_READ_TIME);
                    state->is_psk = true;
                }
            }
        }
    }

    return consumed;
}

void lfrfid_scene_raw_read_on_exit(void* context) {
    LfRfid* app = context;
    LfRfidReadRawState* state =
        (LfRfidReadRawState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneRawRead);

    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
    furi_timer_free(state->timer);

    string_clear(state->string_file_name);
    free(state);
}
