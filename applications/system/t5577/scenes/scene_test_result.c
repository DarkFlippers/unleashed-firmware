#include "../t5577_app.h"
#include <bit_lib.h>

// The grid is still on screen from the run, verdicts and all, so the result is two lines on the
// same view rather than a screen of its own: which blocks took a write is the answer, and it has
// already been drawn, block by block, as it was found out.
#define T5577_APP_TEST_LINE T5577_TEST_VIEW_LINE_MAX

// Both lines and the sound, so that every outcome is decided in one place and the switch stays
// total - a stop reason added later is a build failure rather than a wrong message on screen.
static const NotificationSequence*
    t5577_app_scene_test_result_lines(T5577App* app, char* status, char* hint) {
    const uint8_t block = app->block;

    switch(app->test_stop) {
    case T5577TestStopNone: {
        const uint8_t writeable = bit_lib_get_bit_count(app->test_writeable);

        if(writeable == T5577_APP_TEST_BLOCKS) {
            strlcpy(status, "Full T5577", T5577_APP_TEST_LINE);
        } else if(writeable == 0) {
            strlcpy(status, "Nothing writeable", T5577_APP_TEST_LINE);
        } else {
            snprintf(
                status,
                T5577_APP_TEST_LINE,
                "%u of %u writeable",
                writeable,
                T5577_APP_TEST_BLOCKS);
        }

        // The question at this point is whether the tag is free, which is also the answer to
        // the warning the run started with.
        strlcpy(hint, "Tag can be removed", T5577_APP_TEST_LINE);
        return &sequence_success;
    }
    case T5577TestStopStopped:
        snprintf(status, T5577_APP_TEST_LINE, "Stopped at block %u", block);
        strlcpy(hint, "Tag can be removed", T5577_APP_TEST_LINE);
        // A run the user stopped is not a failure and should not sound like one.
        return &sequence_blink_blue_100;
    case T5577TestStopNoTag:
        // A tag that wants a password refuses the same block 0 read an absent tag never answers,
        // so this cannot tell them apart and does not pretend to.
        strlcpy(status, "No tag found", T5577_APP_TEST_LINE);
        strlcpy(hint, "Or it wants a password", T5577_APP_TEST_LINE);
        return &sequence_error;
    case T5577TestStopPassword:
        strlcpy(status, "Tag has a password", T5577_APP_TEST_LINE);
        strlcpy(hint, "Not supported yet", T5577_APP_TEST_LINE);
        return &sequence_error;
    case T5577TestStopUnreadable:
        snprintf(status, T5577_APP_TEST_LINE, "Block %u did not read", block);
        strlcpy(hint, "Nothing was written", T5577_APP_TEST_LINE);
        return &sequence_error;
    case T5577TestStopNoBackup:
        strlcpy(status, "No backup saved", T5577_APP_TEST_LINE);
        strlcpy(hint, "Check the SD card", T5577_APP_TEST_LINE);
        return &sequence_error;
    case T5577TestStopLost:
        snprintf(status, T5577_APP_TEST_LINE, "Lost tag at block %u", block);
        strlcpy(hint, "Check it, then retry", T5577_APP_TEST_LINE);
        return &sequence_error;
    case T5577TestStopOdd:
        snprintf(status, T5577_APP_TEST_LINE, "Block %u read oddly", block);
        strlcpy(hint, "Check it, then retry", T5577_APP_TEST_LINE);
        return &sequence_error;
    case T5577TestStopNotRestored:
        // The one outcome that leaves the tag wrong, so it is the one that names the way back.
        snprintf(status, T5577_APP_TEST_LINE, "Block %u is not back", block);
        snprintf(hint, T5577_APP_TEST_LINE, "Write %s", T5577_APP_TEST_BACKUP);
        return &sequence_error;
    }

    return &sequence_error;
}

// A backup is the way back from a run, and only a run that leaves a block changed still has
// anywhere to go back to. Kept past that it is a dump of a tag that is already fine, under a name
// every run reuses - indistinguishable, in the list of saved dumps, from a dump of the tag in
// hand, and the obvious thing to write to the next tag.
static void t5577_app_scene_test_result_drop_backup(T5577App* app) {
    if(!app->test_backed_up) return;
    // The one outcome whose hint names the file, and it is kept in step with that line rather
    // than with a list of failures: a stop reason that leaves the tag wrong has to say so there.
    if(app->test_stop == T5577TestStopNotRestored) return;

    FuriString* path = furi_string_alloc();
    t5577_app_test_backup_path(path);
    // Said out loud: the next run reads a surviving file as a tag left changed, so a delete
    // that quietly failed would accuse the user of something that did not happen.
    if(!storage_simply_remove(app->storage, furi_string_get_cstr(path))) {
        FURI_LOG_W("T5577", "Could not drop %s", furi_string_get_cstr(path));
    }
    furi_string_free(path);
}

void t5577_app_scene_test_result_on_enter(void* context) {
    T5577App* app = context;

    t5577_app_scene_test_result_drop_backup(app);

    char status[T5577_APP_TEST_LINE] = {0};
    char hint[T5577_APP_TEST_LINE] = {0};

    notification_message(app->notifications, t5577_app_scene_test_result_lines(app, status, hint));

    t5577_test_view_set_status(app->test_view, status);
    t5577_test_view_set_hint(app->test_view, hint);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewTest);
}

bool t5577_app_scene_test_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back goes past the test rather than into it: the test scene writes the tag on entry.
    if(event.type != SceneManagerEventTypeBack) return false;

    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);

    return true;
}

void t5577_app_scene_test_result_on_exit(void* context) {
    UNUSED(context);
}
