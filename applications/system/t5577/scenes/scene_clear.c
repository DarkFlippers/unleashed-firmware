#include "../t5577_app.h"

// Config written to block 0 while clearing: ASK/Manchester, RF/64, 2 data blocks, and - the part
// that matters - LFRFID_T5577_PWD clear. Clearing that bit is what opens the tag; block 7 goes on
// holding the old password and stops being consulted. Proxmark3 removes a password the same way,
// with a single `lf t55xx write -b 0 -d <config> -p <pass>`.
//
// Writing block 7 as well cannot work in one pass: block 0 opens the tag first, so the block 7
// frame would still carry a password the tag has just stopped expecting, and a password frame an
// open tag does not expect is read off the wrong bit positions.
#define T5577_APP_CLEAR_BLOCK0                                         \
    (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (2UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x00148040

#define T5577_APP_CLEAR_MASK 0b00000001

// Attempts at the unprotected case before any password goes out. A password frame reaching an
// open tag is the one irreversible thing this scene can do - the address it lands on is block 0
// and the lock bit it writes is the password's own top bit, which 44 of the shipped passwords
// have set - so one flaky verify must not be what starts the sweep.
#define T5577_APP_CLEAR_OPEN_TRIES 3

// Everything the sweep will try: the open case, the user's own password when there is one, and
// the shipped list. A password that was never typed in is not tried as a zero - that word is the
// first entry of the list anyway.
static uint16_t t5577_app_clear_attempts(const T5577App* app) {
    return T5577_APP_CLEAR_OPEN_TRIES + (app->password_entered ? 1 : 0) + app->candidates;
}

void t5577_app_scene_clear_on_enter(void* context) {
    T5577App* app = context;

    app->attempt = 0;
    app->cleared = false;
    t5577_default_passwords(&app->candidates);

    t5577_work_view_start(app->work_view, "Clearing", (uint8_t)t5577_app_clear_attempts(app));
    t5577_work_view_set_status(app->work_view, "No password", 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWork);

    notification_message(app->notifications, &sequence_blink_start_magenta);

    // One field across the whole sweep, so the carrier is not dropped between attempts.
    t5577_field_start();

    t5577_app_step_later(app, 50);
}

// One attempt: write block 0, then ask the tag to read it back.
//
// The read carries no password, so it can only answer with this word once the tag is open - and a
// tag still enforcing one cannot match it by accident either, because LFRFID_T5577_PWD is bit 4
// of the very word being compared. Verifying after each attempt is what makes the sweep stop at
// the password that works, and stopping is the safety property: every attempt after the tag is
// already open would be a password frame that an open tag misreads.
static bool t5577_app_scene_clear_attempt(bool with_pass, uint32_t password) {
    LFRFIDT5577 data = {
        .block[0] = T5577_APP_CLEAR_BLOCK0,
        .mask = T5577_APP_CLEAR_MASK,
    };

    t5577_write_with_mask(&data, 0, with_pass, password);

    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(T5577_APP_CLEAR_BLOCK0),
    };

    return t5577_block_holds(&request, T5577_APP_CLEAR_BLOCK0) == T5577BlockHeld;
}

// Attempt number to what it tries: the unprotected case first and more than once, then the
// password the user typed in if there is one, then the shipped list.
static bool t5577_app_scene_clear_try(T5577App* app) {
    const uint16_t attempt = app->attempt;

    if(attempt < T5577_APP_CLEAR_OPEN_TRIES) {
        return t5577_app_scene_clear_attempt(false, 0);
    }

    const uint16_t own = app->password_entered ? 1 : 0;

    if(own && attempt == T5577_APP_CLEAR_OPEN_TRIES) {
        return t5577_app_scene_clear_attempt(true, app->password);
    }

    uint8_t count = 0;
    const uint32_t* list = t5577_default_passwords(&count);

    return t5577_app_scene_clear_attempt(true, list[attempt - T5577_APP_CLEAR_OPEN_TRIES - own]);
}

static void t5577_app_scene_clear_say(T5577App* app) {
    char status[24];

    if(app->attempt < T5577_APP_CLEAR_OPEN_TRIES) {
        strlcpy(status, "No password", sizeof(status));
    } else if(app->password_entered && app->attempt == T5577_APP_CLEAR_OPEN_TRIES) {
        strlcpy(status, "Your password", sizeof(status));
    } else {
        const uint16_t own = app->password_entered ? 1 : 0;
        snprintf(
            status,
            sizeof(status),
            "Pass %u of %u",
            app->attempt - T5577_APP_CLEAR_OPEN_TRIES - own + 1,
            app->candidates);
    }

    t5577_work_view_set_status(app->work_view, status, (uint8_t)app->attempt);
}

bool t5577_app_scene_clear_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Stopping mid-sweep is safe: an attempt either landed or was rejected, and there is no half
    // written state to leave behind. So Back leaves at once, unlike a test or a wipe.
    if(event.type == SceneManagerEventTypeBack) return false;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    const bool cleared = t5577_app_scene_clear_try(app);
    app->attempt++;

    if(cleared || app->attempt >= t5577_app_clear_attempts(app)) {
        app->cleared = cleared;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneClearResult);
        return true;
    }

    t5577_app_scene_clear_say(app);
    t5577_app_step_later(app, 10);

    return true;
}

void t5577_app_scene_clear_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    t5577_field_stop();
    notification_message(app->notifications, &sequence_blink_stop);
}
