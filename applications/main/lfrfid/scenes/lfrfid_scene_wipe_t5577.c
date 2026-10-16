#include "../lfrfid_i.h"
#include "tools/t5577.h"

// Blank state to leave behind: ASK/Manchester, RF/64, 7 data blocks, password disabled.
//
// Proxmark3's "lf t55xx wipe" uses RF/32 and this followed it, but the rate is arbitrary for a
// blank tag and RF/64 is the one that can be read back. The comparator's duty cycle is skewed by
// a roughly fixed 70-100 us, which is a quarter of an RF/64 half-bit and most of an RF/32 one:
// measured on a tag configured RF/32, half-bit runs come back at 195 us against a 128 us nominal
// and 313 of 400 runs classify as nothing. The same bias is why the stock EM4100/32 and
// EM4100/16 decoders fail on that tag too. RF/64 is also what EM4100 - the usual next thing
// written to one of these - already uses.
#define WIPE_T5577_CONFIG_BLOCK0                                       \
    (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x001480E0

#define WIPE_T5577_DATA_BLOCKS    0b11111110
#define WIPE_T5577_CONFIG_BLOCK   0b00000001
#define WIPE_T5577_RESULT_TIMEOUT 3000

// Destructive wipe (Proxmark3 style): zeros to every data block, then the factory config. No
// password (v1), so a protected tag rejects the writes - and cannot slip past the read-back
// either, because PWD is bit 4 of the very word being compared: a tag still enforcing a password
// can never read back as this config.
//
// Config last, not first. The read-back then proves the config block landed, which is the part
// that decides how the tag talks and the part the user sees. It does not prove the data blocks
// landed - coupling comes and goes, and lock bits are per block - but config first proved even
// less, since it answered for a tag that drifted away immediately afterwards.
static void lfrfid_wipe_t5577(void) {
    LFRFIDT5577 data = {
        .block[0] = WIPE_T5577_CONFIG_BLOCK0, // blocks 1..7 stay zero-initialised
        .mask = WIPE_T5577_DATA_BLOCKS,
    };

    t5577_write_with_mask(&data, 0, false, 0);

    data.mask = WIPE_T5577_CONFIG_BLOCK;
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

static void lfrfid_wipe_t5577_draw_progress(LfRfid* app) {
    Widget* widget = app->widget;
    widget_reset(widget);

    widget_add_string_element(widget, 64, 7, AlignCenter, AlignTop, FontPrimary, "Wiping T5577");
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignTop, FontSecondary, "Keep tag still");

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

void lfrfid_scene_wipe_t5577_on_enter(void* context) {
    LfRfid* app = context;

    lfrfid_wipe_t5577_draw_progress(app);
    notification_message(app->notifications, &sequence_blink_start_cyan);

    // One field for the whole transaction, so the detector settles once rather than per write.
    t5577_field_start();
    lfrfid_wipe_t5577();
    const T5577BlockResult result = lfrfid_wipe_t5577_verify();
    t5577_field_stop();

    notification_message(app->notifications, &sequence_blink_stop);
    lfrfid_wipe_t5577_show_result(app, result);
}

bool lfrfid_scene_wipe_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        // The wipe runs synchronously in on_enter, so a Back pressed during it is only delivered
        // afterwards - and acting on it then would snatch away the result before it was read.
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom && event.event == LfRfidEventPopupClosed) {
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
