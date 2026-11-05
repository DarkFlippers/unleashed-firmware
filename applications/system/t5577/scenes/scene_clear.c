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
//
// It is also why the result screen credits the word rather than the tag wherever one is named:
// these attempts establish the unprotected case by WRITING and then reading back, and
// LFRFID_T5577_CONFIG_OPEN is the word an EM4100 at the default clock asks for anyway - so an
// unprotected tag of that kind that was not yet coupling for these three hands its credit to
// whichever later attempt first gets a read. Fronting the sweep with a plain t5577_detect(),
// which settles the question instead of sampling it, would be the deeper fix - and about as
// expensive, since one detect is three captures at best and seven at worst.
//
// The look that ends the sweep is given the same number of tries, for the reason spelled out
// where it is taken.
#define T5577_APP_CLEAR_OPEN_TRIES 3

// What one attempt came to. "No word" is not a rejection: the dictionary stopped handing words
// out, which has to be told apart from a word the tag refused, or a card pulled mid-sweep reads
// as a hundred passwords tried and found wanting.
typedef enum {
    ClearAttemptRejected,
    ClearAttemptOpened,
    ClearAttemptNoWord,
} ClearAttempt;

// Whether the user gave this sweep a word of their own. A zero is not one: the password screen
// is on the way in whether or not there is anything to say, so Start on the zero it opens with
// sets password_entered with nothing behind it - and that zero is line 14 of the shipped list,
// so it is still tried, as a dictionary word rather than as a frame sent for nobody.
static bool t5577_app_clear_has_word(const T5577App* app) {
    return app->password_entered && app->password != 0;
}

// Everything the sweep will try: the open case, the user's own password when there is one, and
// the dictionary.
static uint16_t t5577_app_clear_attempts(const T5577App* app) {
    return T5577_APP_CLEAR_OPEN_TRIES + (t5577_app_clear_has_word(app) ? 1 : 0) + app->candidates;
}

// Where the dictionary starts in the attempt order.
static uint16_t t5577_app_clear_first_word(const T5577App* app) {
    return T5577_APP_CLEAR_OPEN_TRIES + (t5577_app_clear_has_word(app) ? 1 : 0);
}

void t5577_app_scene_clear_on_enter(void* context) {
    T5577App* app = context;

    app->attempt = 0;
    app->cleared = false;
    app->clear = (T5577ClearResult){0};
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
    // What opening the tag on this attempt would mean, settled before the frame goes out and
    // committed once at the bottom - so the provenance and the word it names cannot come apart.
    // It starts at the value that names nothing, which the result screen already has a sentence
    // for, so a branch added below without one cannot commit a verdict by default.
    T5577ClearResult result = {0};

    if(attempt < T5577_APP_CLEAR_OPEN_TRIES) {
        result.found = T5577ClearFoundOpen;
    } else if(attempt < t5577_app_clear_first_word(app)) {
        // Told apart from a word out of a list, because this is the one the result screen can
        // offer to keep.
        result = (T5577ClearResult){.found = T5577ClearFoundTyped, .password = app->password};
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

        // Kept now or not at all: `key` is a local, the attempt number does not say which file
        // the word came from (the handover above is on a list running out, not on a count), and
        // on_exit frees the dictionary before the result screen draws.
        result = (T5577ClearResult){
            .found = T5577ClearFoundWord, .password = bit_lib_bytes_to_num_be(key, sizeof(key))};
    }

    // One frame for the three branches above, carrying what the one of them that ran decided.
    // Only the unprotected case goes out without a password, and it is the only branch that
    // leaves the word at zero.
    if(!t5577_app_scene_clear_attempt(result.found != T5577ClearFoundOpen, result.password)) {
        return ClearAttemptRejected;
    }

    app->clear = result;

    // The one record of which word it was outside the screen that is about to show it: where a
    // flaked read-back has credited the wrong attempt, this is what says so afterwards.
    FURI_LOG_I(
        TAG,
        "attempt %u opened the tag, found %u, pass %08lX",
        attempt,
        (unsigned)result.found,
        (unsigned long)result.password);

    return ClearAttemptOpened;
}

// Said after an attempt, so it names the one about to go out: the word on the line and the word
// the bar is filled to are the same number.
// The look that decides between "Cleared!" and sending the user back in for another sweep, at a
// tag that is now open and reads every password frame as a write. Given the same number of tries
// as the unprotected case at the head of the sweep, and for the same reason - one read is not
// enough to hang that on. Free to retry: the sweep is over, the field is still up, and a plain
// read carries no password.
static bool t5577_app_scene_clear_looks_open(void) {
    for(uint8_t look = 0; look < T5577_APP_CLEAR_OPEN_TRIES; look++) {
        if(t5577_app_scene_clear_is_open()) return true;
    }

    return false;
}

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
        // The sweep stops at the word that opens the tag, and that stop rests on a read: if it
        // flaked on the attempt that worked, every word after it went out at an open tag and the
        // user would be told nothing happened. Still a mitigation and not a guarantee - a later
        // frame may have left block 0 holding something else again.
        //
        // Said on the status line first, because those looks are around a second and a half in
        // which nothing else moves.
        t5577_work_view_set_status(app->work_view, "Last look", app->candidates);

        app->cleared = outcome == ClearAttemptOpened || t5577_app_scene_clear_looks_open();

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
