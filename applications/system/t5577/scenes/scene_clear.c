#include "../t5577_app.h"
#include <bit_lib.h>

#define TAG "T5577Clear"

// Only block 0 goes out, carrying LFRFID_T5577_CONFIG_OPEN: clearing the password bit is the
// whole of removing a password, and Proxmark3 does it with the same single write.
//
// Writing block 7 as well cannot work in one pass: block 0 opens the tag first, so the block 7
// frame would still carry a password the tag has just stopped expecting, and a password frame an
// open tag does not expect is read off the wrong bit positions.

// Attempts at the unprotected case before any password goes out. Sending a password to an open
// tag is the one thing this scene cannot take back - see the warning on t5577_detect_with_pass()
// for how a password frame is read by a chip that wants none, and the shipped list's own header
// for how many of its words have the bit that lands on a lock bit. So one flaky verify must not
// be what starts the sweep.
#define T5577_APP_CLEAR_OPEN_TRIES 3

// What one attempt came to. "No word" is not a rejection: the dictionary stopped handing words
// out, which has to be told apart from a word the tag refused, or a card pulled mid-sweep reads
// as a hundred passwords tried and found wanting.
typedef enum {
    ClearAttemptRejected,
    ClearAttemptOpened,
    ClearAttemptNoWord,
} ClearAttempt;

// Everything the sweep will try: the open case, the user's own password when there is one, and
// the dictionary.
static uint16_t t5577_app_clear_attempts(const T5577App* app) {
    return T5577_APP_CLEAR_OPEN_TRIES + (app->password_entered ? 1 : 0) + app->candidates;
}

// Where the dictionary starts in the attempt order.
static uint16_t t5577_app_clear_first_word(const T5577App* app) {
    return T5577_APP_CLEAR_OPEN_TRIES + (app->password_entered ? 1 : 0);
}

void t5577_app_scene_clear_on_enter(void* context) {
    T5577App* app = context;

    app->attempt = 0;
    app->cleared = false;
    app->dict_failed = false;
    // Read as the sweep goes, one word per attempt and in the order the file holds them, so the
    // list never has to be in RAM. A missing file opens as an empty dictionary, which leaves the
    // sweep with the open case and the user's own password - and the result screen says so.
    //
    // The user's own list first: a word somebody kept is likelier to open the tag in front of
    // them than one of a hundred the vendor ships, and every attempt before the right one is a
    // password frame sent to a tag that may not want it.
    app->dict_user = true;
    app->dict =
        keys_dict_alloc(T5577_DICT_USER_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
    app->candidates = (uint16_t)keys_dict_get_total_keys(app->dict);

    // Both totals up front, because the bar and the "of N" on the status line must not change
    // under the user when the sweep hands over. The shipped list is opened and closed again
    // just to be counted; it is a hundred-odd lines.
    KeysDict* shipped =
        keys_dict_alloc(T5577_DICT_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);

    app->candidates += (uint16_t)keys_dict_get_total_keys(shipped);
    keys_dict_free(shipped);

    // The bar counts the dictionary, not every attempt: the status line names the word it is on,
    // and a bar counting four more things than that line does is a bar nobody can read. With no
    // dictionary there is no such line, so it counts the handful of attempts that are left.
    t5577_work_view_start(
        app->work_view,
        "Clearing",
        app->candidates > 0 ? app->candidates : t5577_app_clear_attempts(app));
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
static bool t5577_app_scene_clear_is_open(void) {
    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(LFRFID_T5577_CONFIG_OPEN),
    };

    return t5577_block_holds(&request, LFRFID_T5577_CONFIG_OPEN) == T5577BlockHeld;
}

static bool t5577_app_scene_clear_attempt(bool with_pass, uint32_t password) {
    t5577_write_block(0, 0, LFRFID_T5577_CONFIG_OPEN, with_pass, password);

    return t5577_app_scene_clear_is_open();
}

// Attempt number to what it tries: the unprotected case first and more than once, then the
// password the user typed in if there is one, then the dictionary.
static ClearAttempt t5577_app_scene_clear_try(T5577App* app) {
    const uint16_t attempt = app->attempt;
    bool opened = false;

    if(attempt < T5577_APP_CLEAR_OPEN_TRIES) {
        opened = t5577_app_scene_clear_attempt(false, 0);
    } else if(attempt < t5577_app_clear_first_word(app)) {
        opened = t5577_app_scene_clear_attempt(true, app->password);
    } else {
        // Sequential, because the sweep is: the next attempt wants the next word, and the
        // counter the screen shows is the same walk.
        uint8_t key[T5577_DICT_KEY_SIZE];

        // Runs at most twice: dict_user goes true to false once, and a miss on the shipped
        // list returns. Handing over on the first list RUNNING OUT rather than on a word count
        // reaching it is what keeps a list that stops early - a truncated file, a card pulled -
        // from taking the shipped words down with it, and those are the ones most likely to
        // open a tag.
        while(!keys_dict_get_next_key(app->dict, key, sizeof(key))) {
            if(!app->dict_user) {
                FURI_LOG_W(TAG, "dictionary stopped after %u of %u", attempt, app->candidates);
                return ClearAttemptNoWord;
            }

            keys_dict_free(app->dict);
            app->dict_user = false;
            app->dict =
                keys_dict_alloc(T5577_DICT_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
        }

        opened = t5577_app_scene_clear_attempt(true, bit_lib_bytes_to_num_be(key, sizeof(key)));
    }

    return opened ? ClearAttemptOpened : ClearAttemptRejected;
}

// Said after an attempt, so it names the one about to go out: the word on the line and the word
// the bar is filled to are the same number.
static void t5577_app_scene_clear_say(T5577App* app) {
    const uint16_t first = t5577_app_clear_first_word(app);
    char status[24];
    uint16_t word = 0;

    if(app->attempt < T5577_APP_CLEAR_OPEN_TRIES) {
        strlcpy(status, "No password", sizeof(status));
    } else if(app->attempt < first) {
        strlcpy(status, "Your password", sizeof(status));
    } else {
        word = app->attempt - first + 1;
        snprintf(status, sizeof(status), "Pass %u of %u", word, app->candidates);
    }

    t5577_work_view_set_status(app->work_view, status, word);
}

bool t5577_app_scene_clear_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Stopping mid-sweep is safe: an attempt either landed or was rejected, and there is no half
    // written state to leave behind. So Back leaves at once, unlike a test or a wipe - but to the
    // menu, not to the password entry this was reached through, which has nothing left to ask.
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    const ClearAttempt outcome = t5577_app_scene_clear_try(app);

    if(outcome == ClearAttemptNoWord) {
        // Told apart from a list that was never there: the words already tried were really
        // tried, and a card pulled mid-sweep is a different thing to go and fix.
        app->dict_failed = true;
    } else {
        app->attempt++;
    }

    if(outcome != ClearAttemptRejected || app->attempt >= t5577_app_clear_attempts(app)) {
        // One last look before saying no, on every way of ending but the one that already knows.
        // The sweep stops at the word that opens the tag, and that stop rests on a single read:
        // if it flaked on the attempt that worked, every word after it went out at an open tag
        // and the user would be told nothing happened. It is a mitigation rather than a
        // guarantee - a later frame may have left block 0 holding something else again.
        app->cleared = outcome == ClearAttemptOpened || t5577_app_scene_clear_is_open();

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

    keys_dict_free(app->dict);
    app->dict = NULL;
}
