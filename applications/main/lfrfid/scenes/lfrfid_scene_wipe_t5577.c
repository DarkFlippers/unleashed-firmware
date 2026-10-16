#include "../lfrfid_i.h"
#include "tools/t5577.h"

// Proxmark3 "lf t55xx wipe" default config block 0: ASK/Manchester, RF/32, 7 data blocks,
// password disabled. Leaves the tag in a known-good, openly re-writable blank state, and matching
// Proxmark3 means a tag wiped by either tool looks the same to the other.
//
// RF/32 only reads back because the decoder corrects for the comparator offset: a half-bit run at
// this rate comes back around 195 us against a 128 us nominal, which no fixed window can hold.
#define WIPE_T5577_CONFIG_BLOCK0                                       \
    (LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x000880E0

#define WIPE_T5577_DATA_BLOCKS    0b11111110
#define WIPE_T5577_CONFIG_BLOCK   0b00000001
#define WIPE_T5577_RESULT_TIMEOUT 3000

// A step of the wipe, one per timer tick, so the screen can be redrawn between them. The work is
// synchronous on this thread: done in one go it is about a second of frozen display, and a
// progress bar driven by a timer could never repaint because the events would queue behind it.
//
// Blocks 1..7 carry the zeros, block 0 the configuration, and the configuration goes last.
// Writing it first proved less: the read-back then answers for a tag that drifted away
// immediately afterwards, where last means the tag was still coupled at the end of the sequence.
typedef enum {
    WipeStepData1, // blocks 1..7, one per step
    WipeStepConfig = WipeStepData1 + 7,
    WipeStepVerify,
    WipeStepDone,
} WipeStep;

#define WIPE_T5577_STEP_PERIOD_MS 20

typedef struct {
    FuriTimer* timer;
    WipeStep step;
    T5577BlockResult result;
} LfRfidWipeState;

// Destructive wipe (Proxmark3 style): zeros to every data block, then the factory config. No
// password (v1), so a protected tag rejects the writes - and cannot slip past the read-back
// either, because PWD is bit 4 of the very word being compared: a tag still enforcing a password
// can never read back as this config.
static void lfrfid_wipe_t5577_write_block(uint8_t block) {
    LFRFIDT5577 data = {
        .block[0] = WIPE_T5577_CONFIG_BLOCK0, // blocks 1..7 stay zero-initialised
        .mask = (uint8_t)(1u << block),
    };

    t5577_write_with_mask(&data, 0, false, 0);
}

static T5577BlockResult lfrfid_wipe_t5577_verify(void) {
    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(WIPE_T5577_CONFIG_BLOCK0),
    };

    return t5577_block_holds(&request, WIPE_T5577_CONFIG_BLOCK0);
}

// Progress-bar geometry (outline plus inset fill).
#define WIPE_BAR_X      14
#define WIPE_BAR_Y      30
#define WIPE_BAR_W      100
#define WIPE_BAR_H      11
#define WIPE_BAR_FILL_W (WIPE_BAR_W - 4) // inset 2 px each side

static void lfrfid_wipe_t5577_draw(LfRfid* app, WipeStep step) {
    Widget* widget = app->widget;
    widget_reset(widget);

    widget_add_string_element(widget, 64, 7, AlignCenter, AlignTop, FontPrimary, "Wiping T5577");
    widget_add_rect_element(widget, WIPE_BAR_X, WIPE_BAR_Y, WIPE_BAR_W, WIPE_BAR_H, 2, false);

    const uint8_t fill = (uint8_t)(((uint32_t)WIPE_BAR_FILL_W * step) / WipeStepDone);
    if(fill > 0) {
        widget_add_rect_element(
            widget, WIPE_BAR_X + 2, WIPE_BAR_Y + 2, fill, WIPE_BAR_H - 4, 1, true);
    }

    // The verify is one unsplittable read, and the longest step by far, so it gets said out loud
    // rather than looking like the bar has stalled.
    widget_add_string_element(
        widget,
        64,
        47,
        AlignCenter,
        AlignTop,
        FontSecondary,
        (step >= WipeStepVerify) ? "Checking tag" : "Keep tag still");

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
}

static void lfrfid_wipe_t5577_show_result(LfRfid* app, T5577BlockResult result) {
    Popup* popup = app->popup;
    popup_reset(popup);

    if(result == T5577BlockHeld) {
        popup_set_header(popup, "Wiped!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
        notification_message(app->notifications, &sequence_single_vibro);
    } else {
        popup_set_header(popup, "Not Wiped", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        // Each line says what was measured, not what it means. Silence is the commonest symptom
        // of a password, because the read carries none and a protected tag cannot answer it - so
        // that branch points at the action that would fix it. An answer that does not match is a
        // tag talking in some other configuration, which is what a refused write looks like.
        popup_set_text(
            popup,
            (result == T5577BlockCannotVerify) ? "Wrote, but\ncannot read\nthis config" :
            (result == T5577BlockNoAnswer)     ? "No answer.\nProtected? Try\nClear Password" :
                                                 "Answered, but\nnot as wiped.\nLocked tag?",
            3,
            19,
            AlignLeft,
            AlignTop);
        notification_message(app->notifications, &sequence_double_vibro);
    }

    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(popup, WIPE_T5577_RESULT_TIMEOUT);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

static void lfrfid_wipe_t5577_timer_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventWipeProgress);
}

void lfrfid_scene_wipe_t5577_on_enter(void* context) {
    LfRfid* app = context;

    LfRfidWipeState* state = malloc(sizeof(LfRfidWipeState));
    state->timer = furi_timer_alloc(lfrfid_wipe_t5577_timer_callback, FuriTimerTypePeriodic, app);
    state->step = WipeStepData1;
    state->result = T5577BlockNoAnswer;
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneWipeT5577, (uint32_t)state);

    lfrfid_wipe_t5577_draw(app, WipeStepData1);
    notification_message(app->notifications, &sequence_blink_start_cyan);

    // One field for the whole sequence, held across the steps, so the tag is not power-cycled
    // between blocks and the detector settles once.
    t5577_field_start();

    furi_timer_start(state->timer, furi_ms_to_ticks(WIPE_T5577_STEP_PERIOD_MS));
}

bool lfrfid_scene_wipe_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    LfRfidWipeState* state =
        (LfRfidWipeState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneWipeT5577);
    bool consumed = false;

    furi_assert(state);

    if(event.type == SceneManagerEventTypeBack) {
        // Back does nothing while the tag is being written: stopping half way through would leave
        // it in a state no message could describe. Once the result is up, its timeout leaves.
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventWipeProgress) {
            if(state->step < WipeStepConfig) {
                lfrfid_wipe_t5577_write_block((uint8_t)(state->step - WipeStepData1) + 1);
            } else if(state->step == WipeStepConfig) {
                lfrfid_wipe_t5577_write_block(0);
            } else if(state->step == WipeStepVerify) {
                state->result = lfrfid_wipe_t5577_verify();
            }

            state->step++;

            if(state->step == WipeStepDone) {
                furi_timer_stop(state->timer);
                t5577_field_stop();
                notification_message(app->notifications, &sequence_blink_stop);
                lfrfid_wipe_t5577_show_result(app, state->result);
            } else {
                lfrfid_wipe_t5577_draw(app, state->step);
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

void lfrfid_scene_wipe_t5577_on_exit(void* context) {
    LfRfid* app = context;
    LfRfidWipeState* state =
        (LfRfidWipeState*)scene_manager_get_scene_state(app->scene_manager, LfRfidSceneWipeT5577);

    // Leaving before the last step would otherwise strand the field raised.
    if(state->step < WipeStepDone) {
        furi_timer_stop(state->timer);
        t5577_field_stop();
    }

    furi_timer_free(state->timer);
    free(state);

    notification_message(app->notifications, &sequence_blink_stop);
    widget_reset(app->widget);
    popup_reset(app->popup);
}
