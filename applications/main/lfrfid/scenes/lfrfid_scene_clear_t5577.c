#include "../lfrfid_i.h"
#include <bit_lib.h>
#include "tools/t5577.h"
#define TAG "Clear T5577"

// Config written once the password is known: ASK/Manchester, RF/64, 2 data blocks, password
// disabled. An EM4100 carrier, and clearing LFRFID_T5577_PWD in it is what actually opens the tag.
#define CLEAR_T5577_CONFIG_BLOCK0                                      \
    (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (2UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x00148040

#define CLEAR_T5577_MASK_CONFIG   0b00000001
#define CLEAR_T5577_MASK_PASSWORD 0b10000000

#define CLEAR_T5577_STEP_PERIOD_MS  20
#define CLEAR_T5577_RESULT_TIMEOUT  1500
#define CLEAR_T5577_FAILURE_TIMEOUT 3000

// Progress-bar geometry (outline plus inset fill).
#define CLEAR_BAR_X      14
#define CLEAR_BAR_Y      30
#define CLEAR_BAR_W      100
#define CLEAR_BAR_H      11
#define CLEAR_BAR_FILL_W (CLEAR_BAR_W - 4)

typedef enum {
    ClearOutcomeWorking,
    ClearOutcomeNoPassword, // nothing was protecting the tag, and nothing was written
    ClearOutcomeCleared, // a password was found and taken off
    ClearOutcomeFailed,
} ClearOutcome;

typedef enum {
    ClearPhaseProbeOpen,
    ClearPhaseProbeCustom,
    ClearPhaseProbeDict,
    ClearPhaseWrite,
    ClearPhaseVerify,
    ClearPhaseDone,
} ClearPhase;

typedef struct {
    FuriTimer* timer;
    ClearPhase phase;
    uint8_t candidate; // index into the default password list
    uint8_t candidates; // how many there are
    uint32_t password; // the one that answered
    ClearOutcome outcome;
} LfRfidClearState;

// Testing a password by reading, never by writing.
//
// A T5577 enforcing a password ignores any command that does not carry the right one, so a read
// that comes back at all is the test - which is how Proxmark3's `lf t55xx chk` does it. Writing a
// candidate instead would be both slower and dangerous: the moment one of them opens the tag,
// every later password frame is read from the wrong bit positions, and the address those land on
// is block 0 with the lock bit taken from the password's own top bit. A third of the shipped list
// has that bit set, and a locked block 0 can never be written again. Reading cannot do that, and
// a search that finds nothing leaves the tag exactly as it was.
static bool lfrfid_clear_t5577_answers(bool with_pass, uint32_t password) {
    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .with_pass = with_pass,
        .password = password,
        .clock = t5577_read_clock_from_config(CLEAR_T5577_CONFIG_BLOCK0),
    };

    return t5577_block_answers(&request) == T5577AnswerPresent;
}

// Config block first, with the password that worked: clearing the PWD bit is what opens the tag.
// Block 7 holds the password itself, so it is only worth tidying once the tag is open, and then
// with a frame that carries no password and cannot be misread.
static void lfrfid_clear_t5577_write(uint32_t password) {
    LFRFIDT5577 data = {
        .block[0] = CLEAR_T5577_CONFIG_BLOCK0,
        .block[7] = 0,
        .mask = CLEAR_T5577_MASK_CONFIG,
    };

    t5577_write_with_mask(&data, 0, true, password);

    data.mask = CLEAR_T5577_MASK_PASSWORD;
    t5577_write_with_mask(&data, 0, false, 0);
}

static bool lfrfid_clear_t5577_verify(void) {
    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(CLEAR_T5577_CONFIG_BLOCK0),
    };

    // No password on the read, so it can only answer with this word once the tag is open. A tag
    // still enforcing one cannot match it either way: PWD is bit 4 of the very word compared.
    return t5577_block_holds(&request, CLEAR_T5577_CONFIG_BLOCK0) == T5577BlockHeld;
}

static void lfrfid_clear_t5577_draw(LfRfid* app, const LfRfidClearState* state) {
    Widget* widget = app->widget;
    char label[32];

    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 7, AlignCenter, AlignTop, FontPrimary, "Clearing Password");
    widget_add_rect_element(widget, CLEAR_BAR_X, CLEAR_BAR_Y, CLEAR_BAR_W, CLEAR_BAR_H, 2, false);

    // Every candidate costs one read, so the bar tracks the list rather than the clock.
    const uint16_t total = (uint16_t)state->candidates + 2;
    const uint16_t done = (state->phase == ClearPhaseProbeOpen)   ? 0 :
                          (state->phase == ClearPhaseProbeCustom) ? 1 :
                          (state->phase == ClearPhaseProbeDict)   ? 2u + state->candidate :
                                                                    total;
    const uint8_t fill = (uint8_t)(((uint32_t)CLEAR_BAR_FILL_W * done) / total);
    if(fill > 0) {
        widget_add_rect_element(
            widget, CLEAR_BAR_X + 2, CLEAR_BAR_Y + 2, fill, CLEAR_BAR_H - 4, 1, true);
    }

    if(state->phase == ClearPhaseProbeOpen) {
        snprintf(label, sizeof(label), "No password");
    } else if(state->phase == ClearPhaseProbeCustom) {
        snprintf(label, sizeof(label), "Custom password");
    } else if(state->phase == ClearPhaseProbeDict) {
        snprintf(label, sizeof(label), "Pass %d of %d", state->candidate + 1, state->candidates);
    } else {
        snprintf(label, sizeof(label), "Removing password");
    }
    widget_add_string_element(widget, 64, 47, AlignCenter, AlignTop, FontSecondary, label);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
}

static void lfrfid_clear_t5577_show_result(LfRfid* app, const LfRfidClearState* state) {
    Popup* popup = app->popup;
    popup_reset(popup);

    switch(state->outcome) {
    case ClearOutcomeCleared:
        notification_message(app->notifications, &sequence_success);
        popup_set_header(popup, "Success!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
        break;
    case ClearOutcomeNoPassword:
        notification_message(app->notifications, &sequence_success);
        popup_set_header(popup, "Tag Is Open", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
        popup_set_text(
            popup, "Tag answers\nunprotected.\nNothing written", 3, 19, AlignLeft, AlignTop);
        break;
    default:
        notification_message(app->notifications, &sequence_error);
        popup_set_header(popup, "Not Cleared", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        popup_set_text(
            popup, "No password\nmatched. Tag\nleft untouched", 3, 19, AlignLeft, AlignTop);
        break;
    }

    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(
        popup,
        (state->outcome == ClearOutcomeFailed) ? CLEAR_T5577_FAILURE_TIMEOUT :
                                                 CLEAR_T5577_RESULT_TIMEOUT);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

// One candidate per tick, so the bar can be redrawn and a Back press can be heard between them.
// Each read takes about half a second, nearly all of it waiting for the detector to settle, and
// the whole list is a minute - which is why this has to be both visible and interruptible.
static void lfrfid_clear_t5577_step(LfRfid* app, LfRfidClearState* state) {
    switch(state->phase) {
    case ClearPhaseProbeOpen:
        if(lfrfid_clear_t5577_answers(false, 0)) {
            state->outcome = ClearOutcomeNoPassword;
            state->phase = ClearPhaseDone;
        } else {
            state->phase = ClearPhaseProbeCustom;
        }
        break;

    case ClearPhaseProbeCustom: {
        const uint32_t custom = bit_lib_bytes_to_num_be(app->password, 4);
        if(lfrfid_clear_t5577_answers(true, custom)) {
            state->password = custom;
            state->phase = ClearPhaseWrite;
        } else {
            state->phase = ClearPhaseProbeDict;
        }
        break;
    }

    case ClearPhaseProbeDict: {
        uint8_t len;
        const uint32_t* list = lfrfid_get_t5577_default_passwords(&len);
        if(lfrfid_clear_t5577_answers(true, list[state->candidate])) {
            state->password = list[state->candidate];
            state->phase = ClearPhaseWrite;
        } else if(++state->candidate >= len) {
            state->outcome = ClearOutcomeFailed;
            state->phase = ClearPhaseDone;
        }
        break;
    }

    case ClearPhaseWrite:
        lfrfid_clear_t5577_write(state->password);
        state->phase = ClearPhaseVerify;
        break;

    case ClearPhaseVerify:
        state->outcome = lfrfid_clear_t5577_verify() ? ClearOutcomeCleared : ClearOutcomeFailed;
        state->phase = ClearPhaseDone;
        break;

    case ClearPhaseDone:
        break;
    }
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
    state->timer = furi_timer_alloc(lfrfid_clear_t5577_timer_callback, FuriTimerTypePeriodic, app);
    state->phase = ClearPhaseProbeOpen;
    state->candidate = 0;
    state->candidates = len;
    state->password = 0;
    state->outcome = ClearOutcomeWorking;
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneClearT5577, (uint32_t)state);

    lfrfid_clear_t5577_draw(app, state);
    notification_message(app->notifications, &sequence_blink_start_magenta);

    // One field for the whole search, so the detector is not restarted per candidate.
    t5577_field_start();

    furi_timer_start(state->timer, furi_ms_to_ticks(CLEAR_T5577_STEP_PERIOD_MS));
}

bool lfrfid_scene_clear_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    LfRfidClearState* state = (LfRfidClearState*)scene_manager_get_scene_state(
        app->scene_manager, LfRfidSceneClearT5577);
    bool consumed = false;

    furi_assert(state);

    if(event.type == SceneManagerEventTypeBack) {
        // Searching writes nothing, so giving up is free - but once a password has been found,
        // the two writes that follow have to finish.
        if(state->phase < ClearPhaseWrite) {
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, LfRfidSceneExtraActions);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventWipeProgress) {
            lfrfid_clear_t5577_step(app, state);

            if(state->phase == ClearPhaseDone) {
                furi_timer_stop(state->timer);
                t5577_field_stop();
                notification_message(app->notifications, &sequence_blink_stop);

                FURI_LOG_I(
                    TAG,
                    "%s",
                    (state->outcome == ClearOutcomeCleared)    ? "password found and cleared" :
                    (state->outcome == ClearOutcomeNoPassword) ? "tag already open" :
                                                                 "no password matched");

                lfrfid_clear_t5577_show_result(app, state);
            } else {
                lfrfid_clear_t5577_draw(app, state);
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

    // Leaving before the last step would otherwise strand the field raised.
    if(state->phase != ClearPhaseDone) {
        furi_timer_stop(state->timer);
        t5577_field_stop();
    }

    furi_timer_free(state->timer);
    free(state);

    notification_message(app->notifications, &sequence_blink_stop);
    widget_reset(app->widget);
    popup_reset(app->popup);
}
