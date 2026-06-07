#include "../lfrfid_i.h"
#include "tools/t5577.h"

// Proxmark3 "lf t55xx wipe" default config block 0: ASK/Manchester, RF/32, 7 data blocks,
// password disabled. Leaves the tag in a known-good, openly re-writable blank state.
#define WIPE_T5577_CONFIG_BLOCK0                                       \
    (LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x000880E0

// Read-back window. A wiped tag decodes as nothing; a still-readable tag means the wipe did not
// take. Auto read alternates ASK<->PSK on a 2 s cadence, so wait at least one full ASK+PSK cycle
// (plus margin) before declaring success - a shorter window risks a false "wiped".
#define WIPE_T5577_VERIFY_TIME_MS (4500UL)

typedef struct {
    FuriTimer* timer;
    bool finished; // result shown; a Back press now returns to the menu
} LfRfidWipeState;

// Destructive wipe (Proxmark3 style): factory config to block 0, zeros to all other page-0
// blocks. No password (v1): works on unprotected tags; a protected tag rejects the write and is
// caught by the read-back verification.
static void lfrfid_wipe_t5577(void) {
    LFRFIDT5577 data = {
        .block[0] = WIPE_T5577_CONFIG_BLOCK0,
        .blocks_to_write = LFRFID_T5577_BLOCK_COUNT, // blocks 1..7 stay zero-initialised
    };
    t5577_write(&data);
}

static void lfrfid_wipe_t5577_read_callback(
    LFRFIDWorkerReadResult result,
    ProtocolId protocol,
    void* context) {
    LfRfid* app = context;
    // Only a confirmed decode matters here - the tag is still readable, so the wipe failed.
    if(result == LFRFIDWorkerReadDone) {
        app->protocol_id_next = protocol;
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventReadDone);
    }
}

static void lfrfid_wipe_t5577_timer_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventWipeVerifyTimeout);
}

void lfrfid_scene_wipe_t5577_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    LfRfidWipeState* state = malloc(sizeof(LfRfidWipeState));
    state->timer = furi_timer_alloc(lfrfid_wipe_t5577_timer_callback, FuriTimerTypeOnce, app);
    state->finished = false;
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneWipeT5577, (uint32_t)state);

    popup_set_header(popup, "Wiping\nT5577", 90, 36, AlignCenter, AlignCenter);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    notification_message(app->notifications, &sequence_blink_start_cyan);

    // Synchronous, direct-HAL destructive write, then start a read-back to verify it took.
    lfrfid_wipe_t5577();

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_read_start(
        app->lfworker, LFRFIDWorkerReadTypeAuto, lfrfid_wipe_t5577_read_callback, app);
    furi_timer_start(state->timer, WIPE_T5577_VERIFY_TIME_MS);
}

bool lfrfid_scene_wipe_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    Popup* popup = app->popup;
    LfRfidWipeState* state =
        (LfRfidWipeState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneWipeT5577);
    bool consumed = false;

    furi_assert(state);

    if(event.type == SceneManagerEventTypeBack) {
        // Ignore Back while wiping/verifying; once a result is shown, Back returns to the menu.
        if(state->finished) {
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, LfRfidSceneExtraActions);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventReadDone) {
            // Tag still decodes -> wipe did not take (tag is likely password-protected).
            furi_timer_stop(state->timer);
            lfrfid_worker_stop(app->lfworker);
            notification_message(app->notifications, &sequence_blink_stop);

            popup_set_header(popup, "Not Wiped", 64, 3, AlignCenter, AlignTop);
            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_text(popup, "Card may be\npassword\nprotected", 3, 19, AlignLeft, AlignTop);
            notification_message(app->notifications, &sequence_double_vibro);

            state->finished = true;
            consumed = true;
        } else if(event.event == LfRfidEventWipeVerifyTimeout) {
            // Nothing decoded within the window -> tag reads as blank -> wipe succeeded.
            lfrfid_worker_stop(app->lfworker);
            notification_message(app->notifications, &sequence_blink_stop);

            popup_set_header(popup, "Wiped!", 75, 10, AlignLeft, AlignTop);
            popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
            notification_message(app->notifications, &sequence_single_vibro);

            state->finished = true;
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_wipe_t5577_on_exit(void* context) {
    LfRfid* app = context;
    LfRfidWipeState* state =
        (LfRfidWipeState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneWipeT5577);

    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);

    furi_timer_stop(state->timer);
    furi_timer_free(state->timer);
    free(state);

    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
}
