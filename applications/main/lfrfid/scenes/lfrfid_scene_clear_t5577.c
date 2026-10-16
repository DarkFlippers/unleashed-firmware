#include "../lfrfid_i.h"
#include <bit_lib.h>
#include "tools/t5577.h"

#define TAG "ClearT5577"

// Config written to block 0 while clearing: ASK/Manchester, RF/64, 2 data blocks, and - the part
// that matters - LFRFID_T5577_PWD clear. Clearing that bit is what opens the tag; block 7 goes on
// holding the old password and stops being consulted. Proxmark3 removes a password the same way,
// with a single `lf t55xx write -b 0 -d <config> -p <pass>`.
//
// Writing block 7 as well, as this scene used to, cannot work in one pass: block 0 opens the tag
// first, so the block 7 frame is still carrying a password the tag has just stopped expecting,
// and a password frame an open tag does not expect is read off the wrong bit positions.
#define CLEAR_T5577_CONFIG_BLOCK0                                      \
    (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (2UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x00148040

#define CLEAR_T5577_MASK_CONFIG 0b00000001

// Attempts at the unprotected case before any password goes out. A password frame reaching an
// open tag is the one irreversible thing this scene can do - the address it lands on is block 0
// and the lock bit it writes is the password's own top bit, which 44 of the 127 shipped
// passwords have set - so one flaky verify must not be what starts the sweep.
#define CLEAR_T5577_OPEN_TRIES 3

#define CLEAR_T5577_STEP_PERIOD_MS  20
#define CLEAR_T5577_RESULT_TIMEOUT  1500
#define CLEAR_T5577_FAILURE_TIMEOUT 3000

typedef enum {
    ClearOutcomeWorking,
    ClearOutcomeCleared,
    ClearOutcomeFailed,
} ClearOutcome;

typedef struct {
    FuriTimer* timer;
    uint16_t attempt; // unprotected tries, then the custom password, then the dictionary
    uint8_t candidates;
    ClearOutcome outcome;
    bool finished;
    // The popup keeps the pointer it is given rather than copying, so the text it is showing has
    // to outlive the call that set it.
    char label[24];
} LfRfidClearState;

// One attempt: write block 0, then ask the tag to read it back.
//
// The read carries no password, so it can only answer with this word once the tag is open - and a
// tag still enforcing one cannot match it by accident either, because LFRFID_T5577_PWD is bit 4
// of the very word being compared. Verifying after each attempt is what makes the sweep stop at
// the password that works, and stopping is the safety property: every attempt after the tag is
// already open would be a password frame that an open tag misreads.
static bool lfrfid_clear_t5577_attempt(bool with_pass, uint32_t password) {
    LFRFIDT5577 data = {
        .block[0] = CLEAR_T5577_CONFIG_BLOCK0,
        .mask = CLEAR_T5577_MASK_CONFIG,
    };

    t5577_write_with_mask(&data, 0, with_pass, password);

    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(CLEAR_T5577_CONFIG_BLOCK0),
    };

    return t5577_block_holds(&request, CLEAR_T5577_CONFIG_BLOCK0) == T5577BlockHeld;
}

// Attempt number to what it tries: the unprotected case first and more than once, then the
// password the user entered, then the shipped list.
static bool lfrfid_clear_t5577_try(LfRfid* app, uint16_t attempt) {
    if(attempt < CLEAR_T5577_OPEN_TRIES) {
        return lfrfid_clear_t5577_attempt(false, 0);
    }

    if(attempt == CLEAR_T5577_OPEN_TRIES) {
        return lfrfid_clear_t5577_attempt(true, bit_lib_bytes_to_num_be(app->password, 4));
    }

    uint8_t len;
    const uint32_t* list = lfrfid_get_t5577_default_passwords(&len);
    return lfrfid_clear_t5577_attempt(true, list[attempt - CLEAR_T5577_OPEN_TRIES - 1]);
}

static uint16_t lfrfid_clear_t5577_attempts(uint8_t candidates) {
    return CLEAR_T5577_OPEN_TRIES + 1 + candidates;
}

// Progress through the popup rather than a widget: a widget has to be reset and rebuilt element
// by element to change one line, and each of those calls locks the model and requests a redraw of
// its own, so a sweep this long spends its time drawing half-built screens. One text line per
// attempt is all there is to say anyway.
static void lfrfid_clear_t5577_draw(LfRfid* app, LfRfidClearState* state) {
    if(state->attempt < CLEAR_T5577_OPEN_TRIES) {
        snprintf(state->label, sizeof(state->label), "No password");
    } else if(state->attempt == CLEAR_T5577_OPEN_TRIES) {
        snprintf(state->label, sizeof(state->label), "Custom password");
    } else {
        snprintf(
            state->label,
            sizeof(state->label),
            "Pass %u of %u",
            state->attempt - CLEAR_T5577_OPEN_TRIES,
            state->candidates);
    }

    Popup* popup = app->popup;
    popup_set_header(popup, "Removing\npassword", 90, 36, AlignCenter, AlignCenter);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    popup_set_text(popup, state->label, 90, 56, AlignCenter, AlignCenter);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

static void lfrfid_clear_t5577_show_result(LfRfid* app, ClearOutcome outcome) {
    Popup* popup = app->popup;
    popup_reset(popup);

    if(outcome == ClearOutcomeCleared) {
        notification_message(app->notifications, &sequence_success);
        // The dolphin is 91 px wide and sits on the left, so the text goes to its right - the
        // same layout every other success screen in this app uses.
        popup_set_header(popup, "Success!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
    } else {
        notification_message(app->notifications, &sequence_error);
        popup_set_header(popup, "Not Cleared", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        popup_set_text(popup, "No password\nmatched", 3, 19, AlignLeft, AlignTop);
    }

    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(
        popup,
        (outcome == ClearOutcomeCleared) ? CLEAR_T5577_RESULT_TIMEOUT :
                                           CLEAR_T5577_FAILURE_TIMEOUT);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

static void lfrfid_clear_t5577_timer_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventWipeProgress);
}

void lfrfid_scene_clear_t5577_on_enter(void* context) {
    LfRfid* app = context;

    uint8_t len;
    lfrfid_get_t5577_default_passwords(&len);

    LfRfidClearState* state = malloc(sizeof(LfRfidClearState));
    // One shot, re-armed after each attempt. A periodic timer would keep firing from the timer
    // service thread while an attempt blocks this one for about 600 ms, and since posting a
    // custom event waits forever on a 16-deep queue, the backlog would stall every other timer
    // in the system and then arrive all at once at the end.
    state->timer = furi_timer_alloc(lfrfid_clear_t5577_timer_callback, FuriTimerTypeOnce, app);
    state->attempt = 0;
    state->candidates = len;
    state->outcome = ClearOutcomeWorking;
    state->finished = false;
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneClearT5577, (uint32_t)state);

    lfrfid_clear_t5577_draw(app, state);
    notification_message(app->notifications, &sequence_blink_start_magenta);

    // One field across the whole sweep, so the carrier is not dropped between attempts.
    t5577_field_start();

    furi_timer_start(state->timer, furi_ms_to_ticks(CLEAR_T5577_STEP_PERIOD_MS));
}

static void lfrfid_clear_t5577_stop(LfRfid* app, LfRfidClearState* state) {
    furi_timer_stop(state->timer);
    t5577_field_stop();
    notification_message(app->notifications, &sequence_blink_stop);
    state->finished = true;
}

bool lfrfid_scene_clear_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    LfRfidClearState* state = (LfRfidClearState*)scene_manager_get_scene_state(
        app->scene_manager, LfRfidSceneClearT5577);
    bool consumed = false;

    furi_assert(state);

    if(event.type == SceneManagerEventTypeBack) {
        // Stopping mid-sweep is safe: an attempt either landed or was rejected, and there is no
        // half-written state to leave behind. Once the result is up, its timeout leaves.
        if(!state->finished) {
            lfrfid_clear_t5577_stop(app, state);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, LfRfidSceneExtraActions);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventWipeProgress) {
            if(state->finished) return true;

            const bool cleared = lfrfid_clear_t5577_try(app, state->attempt);
            state->attempt++;

            if(cleared) {
                state->outcome = ClearOutcomeCleared;
            } else if(state->attempt >= lfrfid_clear_t5577_attempts(state->candidates)) {
                state->outcome = ClearOutcomeFailed;
            }

            if(state->outcome == ClearOutcomeWorking) {
                lfrfid_clear_t5577_draw(app, state);
                furi_timer_start(state->timer, furi_ms_to_ticks(CLEAR_T5577_STEP_PERIOD_MS));
            } else {
                lfrfid_clear_t5577_stop(app, state);
                FURI_LOG_I(
                    TAG,
                    "%s after %u attempts",
                    (state->outcome == ClearOutcomeCleared) ? "cleared" : "no password matched",
                    state->attempt);
                lfrfid_clear_t5577_show_result(app, state->outcome);
            }
            consumed = true;
        } else if(event.event == LfRfidEventPopupClosed) {
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, LfRfidSceneExtraActions);
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_clear_t5577_on_exit(void* context) {
    LfRfid* app = context;
    LfRfidClearState* state = (LfRfidClearState*)scene_manager_get_scene_state(
        app->scene_manager, LfRfidSceneClearT5577);

    // Leaving before the sweep ended would otherwise strand the field raised.
    if(!state->finished) {
        furi_timer_stop(state->timer);
        t5577_field_stop();
    }

    furi_timer_free(state->timer);
    free(state);

    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
}
