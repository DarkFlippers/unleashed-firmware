#include "../lfrfid_i.h"
#include "tools/t5577.h"

// Proxmark3 "lf t55xx wipe" default config block 0: ASK/Manchester, RF/32, 7 data blocks,
// password disabled. Leaves the tag in a known-good, openly re-writable blank state.
#define WIPE_T5577_CONFIG_BLOCK0                                       \
    (LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x000880E0

// Read-back window. A wiped tag decodes as nothing; a still-readable tag means the wipe did not
// take. Auto read dwells on each feature ~2.45 s (450 ms detector stabilize + 2000 ms switch)
// before alternating ASK<->PSK, so one full no-decode ASK+PSK cycle is ~5 s. The window must
// exceed a full cycle (plus worker-startup margin) so a non-wiped PSK tag still gets a complete
// decode pass before we declare success - a shorter window risks a false "wiped".
#define WIPE_T5577_VERIFY_TIME_MS (6000UL)
// Progress-bar refresh cadence; the bar also doubles as the verify clock (full bar == window
// elapsed == success), so the tick count spans exactly the verify window.
#define WIPE_T5577_TICK_MS        (150UL)
#define WIPE_T5577_TICK_COUNT     (WIPE_T5577_VERIFY_TIME_MS / WIPE_T5577_TICK_MS)
_Static_assert(
    (WIPE_T5577_VERIFY_TIME_MS % WIPE_T5577_TICK_MS) == 0,
    "Verify window must be an integer multiple of the tick period, else the real window "
    "(TICK_COUNT * TICK_MS) silently drifts from WIPE_T5577_VERIFY_TIME_MS");

// Progress-bar geometry (outline + inset fill).
#define WIPE_BAR_X      14
#define WIPE_BAR_Y      30
#define WIPE_BAR_W      100
#define WIPE_BAR_H      11
#define WIPE_BAR_FILL_W (WIPE_BAR_W - 4) // inset 2 px each side

typedef struct {
    FuriTimer* timer;
    uint16_t ticks;
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

// Render the "Wiping" screen with a progress bar filled to the given tick.
static void lfrfid_wipe_t5577_draw_progress(LfRfid* app, uint16_t ticks) {
    Widget* widget = app->widget;
    widget_reset(widget);

    widget_add_string_element(widget, 64, 7, AlignCenter, AlignTop, FontPrimary, "Wiping T5577");
    widget_add_rect_element(widget, WIPE_BAR_X, WIPE_BAR_Y, WIPE_BAR_W, WIPE_BAR_H, 2, false);

    uint8_t fill = (uint8_t)(((uint32_t)WIPE_BAR_FILL_W * ticks) / WIPE_T5577_TICK_COUNT);
    if(fill > WIPE_BAR_FILL_W) fill = WIPE_BAR_FILL_W;
    if(fill > 0) {
        widget_add_rect_element(
            widget, WIPE_BAR_X + 2, WIPE_BAR_Y + 2, fill, WIPE_BAR_H - 4, 1, true);
    }

    widget_add_string_element(
        widget, 64, 47, AlignCenter, AlignTop, FontSecondary, "Keep tag still");
}

static void lfrfid_wipe_t5577_show_result(LfRfid* app, bool wiped) {
    Popup* popup = app->popup;
    popup_reset(popup);

    if(wiped) {
        popup_set_header(popup, "Wiped!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
        notification_message(app->notifications, &sequence_single_vibro);
    } else {
        popup_set_header(popup, "Not Wiped", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        popup_set_text(
            popup,
            "Tag still reads.\nNot a T5577, or\nlocked/protected",
            3,
            19,
            AlignLeft,
            AlignTop);
        notification_message(app->notifications, &sequence_double_vibro);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

static void lfrfid_wipe_t5577_read_callback(
    LFRFIDWorkerReadResult result,
    ProtocolId protocol,
    void* context) {
    LfRfid* app = context;
    UNUSED(protocol); // which protocol decoded is irrelevant - any decode means "still readable"
    // Only a confirmed decode matters here - the tag is still readable, so the wipe failed.
    if(result == LFRFIDWorkerReadDone) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventReadDone);
    }
}

static void lfrfid_wipe_t5577_timer_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventWipeProgress);
}

// Stop the verify clock + radio, then show the terminal result. Shared by both outcomes so the
// teardown can never drift between the success and failure branches.
static void lfrfid_wipe_t5577_finish(LfRfid* app, LfRfidWipeState* state, bool wiped) {
    furi_timer_stop(state->timer);
    lfrfid_worker_stop(app->lfworker);
    notification_message(app->notifications, &sequence_blink_stop);
    lfrfid_wipe_t5577_show_result(app, wiped);
    state->finished = true;
}

void lfrfid_scene_wipe_t5577_on_enter(void* context) {
    LfRfid* app = context;

    LfRfidWipeState* state = malloc(sizeof(LfRfidWipeState));
    state->timer = furi_timer_alloc(lfrfid_wipe_t5577_timer_callback, FuriTimerTypePeriodic, app);
    state->ticks = 0;
    state->finished = false;
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneWipeT5577, (uint32_t)state);

    lfrfid_wipe_t5577_draw_progress(app, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    notification_message(app->notifications, &sequence_blink_start_cyan);

    // Synchronous, direct-HAL destructive write, then start a read-back to verify it took.
    lfrfid_wipe_t5577();

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_read_start(
        app->lfworker, LFRFIDWorkerReadTypeAuto, lfrfid_wipe_t5577_read_callback, app);
    furi_timer_start(state->timer, WIPE_T5577_TICK_MS);
}

bool lfrfid_scene_wipe_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
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
    } else if(event.type == SceneManagerEventTypeCustom && !state->finished) {
        if(event.event == LfRfidEventWipeProgress) {
            state->ticks++;
            if(state->ticks >= WIPE_T5577_TICK_COUNT) {
                // Verify window elapsed with no decode -> tag reads as blank -> wipe succeeded.
                lfrfid_wipe_t5577_finish(app, state, true);
            } else {
                lfrfid_wipe_t5577_draw_progress(app, state->ticks);
            }
            consumed = true;
        } else if(event.event == LfRfidEventReadDone) {
            // Tag still decodes -> wipe did not take (locked/protected, poor coupling, or the
            // tag is not actually a T5577).
            lfrfid_wipe_t5577_finish(app, state, false);
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
    widget_reset(app->widget);
    popup_reset(app->popup);
}
