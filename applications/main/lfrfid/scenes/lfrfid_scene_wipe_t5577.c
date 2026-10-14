#include "../lfrfid_i.h"
#include "tools/t5577.h"

// Proxmark3 "lf t55xx wipe" default config block 0: ASK/Manchester, RF/32, 7 data blocks,
// password disabled. Leaves the tag in a known-good, openly re-writable blank state.
#define WIPE_T5577_CONFIG_BLOCK0                                       \
    (LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x000880E0

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

// Address the two blocks the wipe defines and ask each to confirm what it holds. The config
// block proves the tag took the write and now talks ASK/Manchester RF/32; block 1 proves the
// data blocks really were zeroed, and reads as a second, independent confirmation.
static bool lfrfid_wipe_t5577_verify(void) {
    T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(WIPE_T5577_CONFIG_BLOCK0),
    };

    if(!t5577_block_holds(&request, WIPE_T5577_CONFIG_BLOCK0)) return false;

    request.block = 1;
    return t5577_block_holds(&request, 0);
}

static void lfrfid_wipe_t5577_draw_progress(LfRfid* app) {
    Widget* widget = app->widget;
    widget_reset(widget);

    widget_add_string_element(widget, 64, 7, AlignCenter, AlignTop, FontPrimary, "Wiping T5577");
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignTop, FontSecondary, "Keep tag still");

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
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
            "Tag did not\nconfirm. Not a\nT5577, locked,\nor moved",
            3,
            19,
            AlignLeft,
            AlignTop);
        notification_message(app->notifications, &sequence_double_vibro);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

void lfrfid_scene_wipe_t5577_on_enter(void* context) {
    LfRfid* app = context;

    lfrfid_wipe_t5577_draw_progress(app);
    notification_message(app->notifications, &sequence_blink_start_cyan);

    // Both halves are synchronous direct-HAL work on this thread, and together they take well
    // under a second - the wipe is eight blind block writes, the verify two addressed reads.
    // There is no window for a Back press in between, so the scene needs no state of its own.
    lfrfid_wipe_t5577();
    const bool wiped = lfrfid_wipe_t5577_verify();

    notification_message(app->notifications, &sequence_blink_stop);
    lfrfid_wipe_t5577_show_result(app, wiped);
}

bool lfrfid_scene_wipe_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, LfRfidSceneExtraActions);
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_wipe_t5577_on_exit(void* context) {
    LfRfid* app = context;

    notification_message(app->notifications, &sequence_blink_stop);
    widget_reset(app->widget);
    popup_reset(app->popup);
}
