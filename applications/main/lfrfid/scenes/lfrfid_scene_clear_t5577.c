#include "../lfrfid_i.h"
#include <bit_lib.h>
#include "tools/t5577.h"
#define TAG "Clear T5577"

// Config written while clearing: ASK/Manchester, RF/64, 2 data blocks, password disabled - an
// EM4100 carrier, which is also what makes the result verifiable by reading block 0 back.
#define CLEAR_T5577_CONFIG_BLOCK0                                      \
    (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (2UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x00148040

#define CLEAR_T5577_PASSWORD_BLOCK 0b10000000
#define CLEAR_T5577_CONFIG_BLOCK   0b00000001

#define CLEAR_T5577_SETTLE_MS       8
#define CLEAR_T5577_OPEN_TRIES      3
#define CLEAR_T5577_RESULT_TIMEOUT  1500
#define CLEAR_T5577_FAILURE_TIMEOUT 3000

typedef enum {
    ClearT5577NoPassword, // nothing was protecting the tag
    ClearT5577Cleared, // a password was found and taken off
    ClearT5577Failed,
} ClearT5577Outcome;

// One attempt: clear the password bit, then ask the tag to confirm it landed.
//
// Block 0 goes first and, on a failed attempt, alone. Clearing LFRFID_T5577_PWD is what opens the
// tag; the password stored in block 7 is only consulted while that bit is set. Writing block 7 in
// the same breath cannot work, because block 7 *is* the password: zeroing it is accepted and then
// the block 0 frame is still carrying the old one, which the tag has already stopped accepting.
// Proxmark3 lives with that by switching to the new password mid-sequence (t55xxProtect in
// cmdlft55xx.c); here there is no reason to write block 7 before the tag is known to be open.
static bool lfrfid_clear_t5577_attempt(LFRFIDT5577* data, bool with_pass, uint32_t password) {
    data->mask = CLEAR_T5577_CONFIG_BLOCK;
    t5577_write_with_mask(data, 0, with_pass, password);

    furi_delay_ms(CLEAR_T5577_SETTLE_MS);

    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(CLEAR_T5577_CONFIG_BLOCK0),
    };

    // The read carries no password, so it only answers once the tag is open. A tag that still
    // wants one cannot answer it, and in that case the config write did not land either. Nor can
    // a protected tag match by accident: PWD is bit 4 of the word being compared, so a tag still
    // enforcing a password can never read back as this value.
    if(t5577_block_holds(&request, CLEAR_T5577_CONFIG_BLOCK0) != T5577BlockHeld) return false;

    // Open now, so this frame carries no password and cannot be misread. Only worth sending when
    // a password was in use: on a tag that never had one there is nothing to clear, and writing
    // block 7 anyway would destroy a stored value the user still has a use for.
    if(with_pass) {
        data->mask = CLEAR_T5577_PASSWORD_BLOCK;
        t5577_write_with_mask(data, 0, false, 0);
    }

    return true;
}

static void lfrfid_clear_t5577_progress(LfRfid* app, const char* label) {
    popup_set_text(app->popup, label, 90, 56, AlignCenter, AlignCenter);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

static ClearT5577Outcome lfrfid_clear_t5577_password_and_config_to_EM(LfRfid* app) {
    Popup* popup = app->popup;
    char curr_buf[32] = {};

    uint8_t default_passwords_len;
    const uint32_t* default_passwords = lfrfid_get_t5577_default_passwords(&default_passwords_len);

    popup_set_header(popup, "Removing\npassword", 90, 36, AlignCenter, AlignCenter);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    notification_message(app->notifications, &sequence_blink_start_magenta);

    LFRFIDT5577 data = {
        .block[0] = CLEAR_T5577_CONFIG_BLOCK0,
        .block[7] = 0,
    };

    // One field for the whole sweep, so the detector settles once instead of per attempt.
    t5577_field_start();

    // The open case first, and more than once, because the cost of getting it wrong is permanent.
    // Every password attempt that follows sends a frame an open tag misreads from the lock bit
    // onward: the address it lands on is the top three bits of the word being written, which is
    // block 0 for both words here, and the lock bit it writes is the password's own MSB. A third
    // of the dictionary has that bit set, and a locked block 0 can never be written again. So one
    // flaky verify must not be what sends us down that path.
    snprintf(curr_buf, sizeof(curr_buf), "No password");
    lfrfid_clear_t5577_progress(app, curr_buf);

    ClearT5577Outcome outcome = ClearT5577Failed;

    // A tag that was already open reads back the config it was just given, which is also the
    // config most EM4100 clones already carry - so this says the tag is open now, not that it
    // never had a password.
    for(uint8_t attempt = 0; attempt < CLEAR_T5577_OPEN_TRIES && outcome == ClearT5577Failed;
        attempt++) {
        if(lfrfid_clear_t5577_attempt(&data, false, 0)) {
            outcome = ClearT5577NoPassword;
        }
    }

    if(outcome == ClearT5577Failed) {
        snprintf(curr_buf, sizeof(curr_buf), "Custom password");
        lfrfid_clear_t5577_progress(app, curr_buf);

        if(lfrfid_clear_t5577_attempt(&data, true, bit_lib_bytes_to_num_be(app->password, 4))) {
            outcome = ClearT5577Cleared;
        }
    }

    // Stopping at the password that works is not just faster: another attempt after the tag is
    // open is the stray write described above.
    for(uint8_t i = 0; i < default_passwords_len && outcome == ClearT5577Failed; i++) {
        snprintf(curr_buf, sizeof(curr_buf), "Pass %d of %d", i + 1, default_passwords_len);
        lfrfid_clear_t5577_progress(app, curr_buf);

        if(lfrfid_clear_t5577_attempt(&data, true, default_passwords[i])) {
            outcome = ClearT5577Cleared;
        }
    }

    t5577_field_stop();

    // The verdict the user is about to see, so a bug report can be matched against it.
    FURI_LOG_I(
        TAG,
        "%s",
        (outcome == ClearT5577Cleared)    ? "password found and cleared" :
        (outcome == ClearT5577NoPassword) ? "tag already open" :
                                            "no password matched");

    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(popup);

    return outcome;
}

void lfrfid_scene_clear_t5577_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    const ClearT5577Outcome outcome = lfrfid_clear_t5577_password_and_config_to_EM(app);

    switch(outcome) {
    case ClearT5577Cleared:
        notification_message(app->notifications, &sequence_success);
        popup_set_header(popup, "Success!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
        break;
    case ClearT5577NoPassword:
        notification_message(app->notifications, &sequence_success);
        popup_set_header(popup, "Tag Is Open", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
        popup_set_text(popup, "Tag answers\nunprotected", 3, 19, AlignLeft, AlignTop);
        break;
    case ClearT5577Failed:
        notification_message(app->notifications, &sequence_error);
        popup_set_header(popup, "Not Cleared", 64, 3, AlignCenter, AlignTop);
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        popup_set_text(
            popup, "Password not\nfound, or tag\nnot answering", 3, 19, AlignLeft, AlignTop);
        break;
    }

    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(
        popup,
        (outcome == ClearT5577Failed) ? CLEAR_T5577_FAILURE_TIMEOUT : CLEAR_T5577_RESULT_TIMEOUT);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    if(outcome != ClearT5577Failed) {
        notification_message_block(app->notifications, &sequence_set_green_255);
    }
}

bool lfrfid_scene_clear_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true; // Ignore Back button presses
    } else if(event.type == SceneManagerEventTypeCustom && event.event == LfRfidEventPopupClosed) {
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, LfRfidSceneExtraActions);
        consumed = true;
    }
    return consumed;
}

void lfrfid_scene_clear_t5577_on_exit(void* context) {
    LfRfid* app = context;
    popup_reset(app->popup);
    notification_message_block(app->notifications, &sequence_reset_green);
}
