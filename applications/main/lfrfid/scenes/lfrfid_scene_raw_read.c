#include "../lfrfid_i.h"

#define RAW_READ_TIME_MS (5000UL)

typedef struct {
    FuriString* string_file_name;
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

    popup_set_icon(popup, 0, 0, &I_NFC_dolphin_emulation_51x64);
    popup_set_header(popup, "Reading ASK", 91, 16, AlignCenter, AlignTop);
    popup_set_text(popup, "Don't move\nfor 5 sec.", 91, 29, AlignCenter, AlignTop);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);

    LfRfidReadRawState* state = malloc(sizeof(LfRfidReadRawState));
    state->string_file_name = furi_string_alloc();
    state->timer = furi_timer_alloc(timer_callback, FuriTimerTypeOnce, app);

    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneRawRead, (uint32_t)state);

    furi_string_printf(
        state->string_file_name,
        "%s/%s%s",
        LFRFID_SD_FOLDER,
        furi_string_get_cstr(app->raw_file_name),
        LFRFID_APP_RAW_ASK_EXTENSION);

    lfrfid_make_app_folder(app);

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_read_raw_start(
        app->lfworker,
        furi_string_get_cstr(state->string_file_name),
        LFRFIDWorkerReadTypeASKOnly,
        lfrfid_read_callback,
        app);

    furi_timer_start(state->timer, RAW_READ_TIME_MS);
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
        if(event.event == LfRfidEventReadError || event.event == LfRfidEventReadOverrun) {
            furi_timer_stop(state->timer);

            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_header(popup, "RAW Reading error!", 64, 0, AlignCenter, AlignTop);
            popup_set_text(
                popup, "This may be\ncaused by SD\ncard issues", 0, 13, AlignLeft, AlignTop);

            notification_message(app->notifications, &sequence_blink_start_red);
            state->error = true;

        } else if(event.event == LfRfidEventReadDone) {
            if(!state->error) {
                if(state->is_psk) {
                    notification_message(app->notifications, &sequence_success);
                    scene_manager_next_scene(app->scene_manager, LfRfidSceneRawSuccess);

                } else {
                    lfrfid_worker_stop(app->lfworker);
                    lfrfid_worker_stop_thread(app->lfworker);

                    state->is_psk = true;

                    furi_string_printf(
                        state->string_file_name,
                        "%s/%s%s",
                        LFRFID_SD_FOLDER,
                        furi_string_get_cstr(app->raw_file_name),
                        LFRFID_APP_RAW_PSK_EXTENSION);

                    lfrfid_worker_start_thread(app->lfworker);
                    lfrfid_worker_read_raw_start(
                        app->lfworker,
                        furi_string_get_cstr(state->string_file_name),
                        LFRFIDWorkerReadTypePSKOnly,
                        lfrfid_read_callback,
                        app);

                    furi_timer_start(state->timer, RAW_READ_TIME_MS);

                    popup_set_header(popup, "Reading PSK", 91, 16, AlignCenter, AlignTop);
                    notification_message(app->notifications, &sequence_blink_start_yellow);
                }
            }
        }

        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_raw_read_on_exit(void* context) {
    LfRfid* app = context;
    LfRfidReadRawState* state =
        (LfRfidReadRawState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneRawRead);

    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);

    furi_timer_free(state->timer);
    furi_string_free(state->string_file_name);
    free(state);

    popup_reset(app->popup);
    notification_message(app->notifications, &sequence_blink_stop);
}
