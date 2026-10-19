#include "../t5577_app.h"

// The grid is still on screen from the run, verdicts and all, so the result is two lines on the
// same view rather than a screen of its own: which blocks took a write is the answer, and it has
// already been drawn, block by block, as it was found out.
#define T5577_APP_TEST_LINE 24

static void t5577_app_scene_test_result_stopped(T5577App* app, char* status, char* hint) {
    const uint8_t block = app->block;

    switch(app->test_stop) {
    case T5577TestStopNoTag:
        strlcpy(status, "No tag found", T5577_APP_TEST_LINE);
        strlcpy(hint, "Hold it on the back", T5577_APP_TEST_LINE);
        return;
    case T5577TestStopPassword:
        strlcpy(status, "Tag has a password", T5577_APP_TEST_LINE);
        strlcpy(hint, "Not supported yet", T5577_APP_TEST_LINE);
        return;
    case T5577TestStopUnreadable:
        snprintf(status, T5577_APP_TEST_LINE, "Block %u did not read", block);
        strlcpy(hint, "Nothing was written", T5577_APP_TEST_LINE);
        return;
    case T5577TestStopNoBackup:
        strlcpy(status, "No backup saved", T5577_APP_TEST_LINE);
        strlcpy(hint, "Nothing was written", T5577_APP_TEST_LINE);
        return;
    case T5577TestStopLost:
        snprintf(status, T5577_APP_TEST_LINE, "Lost tag at block %u", block);
        strlcpy(hint, "Check it, then retry", T5577_APP_TEST_LINE);
        return;
    case T5577TestStopOdd:
        snprintf(status, T5577_APP_TEST_LINE, "Block %u read oddly", block);
        strlcpy(hint, "Check it, then retry", T5577_APP_TEST_LINE);
        return;
    default:
        // The one outcome that leaves the tag wrong, so it is the one that names the way back.
        snprintf(status, T5577_APP_TEST_LINE, "Block %u is not back", block);
        strlcpy(hint, "Write the backup back", T5577_APP_TEST_LINE);
        return;
    }
}

void t5577_app_scene_test_result_on_enter(void* context) {
    T5577App* app = context;

    char status[T5577_APP_TEST_LINE];
    char hint[T5577_APP_TEST_LINE];

    if(app->test_stop == T5577TestStopNone) {
        uint8_t writeable = 0;
        for(uint8_t block = 1; block < LFRFID_T5577_BLOCKS_IN_PAGE_0; block++) {
            if(app->test_writeable & (1u << block)) writeable++;
        }

        if(writeable == app->done) {
            strlcpy(status, "Full T5577", sizeof(status));
        } else if(writeable == 0) {
            strlcpy(status, "Nothing writeable", sizeof(status));
        } else {
            snprintf(status, sizeof(status), "%u of %u writeable", writeable, app->done);
        }

        // Said rather than left for the user to find in the Saved list.
        strlcpy(hint, "Backup is on the card", sizeof(hint));
        notification_message(app->notifications, &sequence_success);
    } else {
        t5577_app_scene_test_result_stopped(app, status, hint);
        notification_message(app->notifications, &sequence_error);
    }

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
