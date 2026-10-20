#include "../t5577_app.h"

typedef enum {
    TestStepDetect,
    TestStepRead,
    TestStepBackup,
    TestStepProbe,
    TestStepProbeRead,
    TestStepRestore,
    TestStepRestoreRead,
} TestStep;

// Page 0 blocks 1 to 7, in the order that keeps the blast radius smallest for longest. Blocks
// 1 and 2 carry the data on every protocol this firmware writes, so they go late; 3 to 6 are
// padding on most of them; block 7 is the password store, and an ordinary block only because a
// tag that wants a password is turned away before any of this.
static const uint8_t t5577_app_test_order[] = {3, 4, 5, 6, 1, 2, 7};

static_assert(
    COUNT_OF(t5577_app_test_order) == T5577_APP_TEST_BLOCKS,
    "every block a test probes needs a place in the order");

// One bit flipped, which is the smallest change that is still a change. A write that is accepted
// has to read back as something different from what was there, or there is no way to tell it
// from a write the chip quietly dropped.
static uint32_t t5577_app_test_probe(uint32_t value) {
    return value ^ 1u;
}

static void t5577_app_test_say(T5577App* app, const char* doing) {
    char status[T5577_TEST_VIEW_LINE_MAX];
    snprintf(status, sizeof(status), "Block %u: %s", app->block, doing);
    t5577_test_view_set_status(app->test_view, status);
}

// No password on any frame: this version turns away a tag that asks for one, so a password frame
// here would only ever reach a chip that has none - and be read as a write of its block 0.
static void t5577_app_test_write(const T5577App* app, uint32_t value) {
    t5577_write_block(0, app->block, value, false, 0);
}

// On to the next block, or false when there is none left - which a stop also means.
static bool t5577_app_scene_test_next(T5577App* app) {
    if(app->test_stop != T5577TestStopNone) return false;

    app->order++;

    return app->order < T5577_APP_TEST_BLOCKS;
}

// A block that refused the probe was never changed, and the probe read is what proved it: the
// word it returned was the original. So putting it back needs no second look.
static bool t5577_app_scene_test_refused(const T5577App* app) {
    return app->test_stop == T5577TestStopNone && !(app->test_writeable & (1u << app->block));
}

static bool t5577_app_scene_test_finish(T5577App* app) {
    scene_manager_next_scene(app->scene_manager, T5577AppSceneTestResult);

    return true;
}

void t5577_app_scene_test_on_enter(void* context) {
    T5577App* app = context;

    app->step = TestStepDetect;
    app->order = 0;
    app->block = 1;
    app->test_stop = T5577TestStopNone;
    app->test_writeable = 0;
    app->test_backed_up = false;
    memset(&app->dump, 0, sizeof(app->dump));

    t5577_test_view_reset(app->test_view);
    t5577_test_view_set_status(app->test_view, "Finding tag");
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewTest);

    notification_message(app->notifications, &sequence_blink_start_magenta);

    // As in a read: the screen goes up before the first command takes the thread.
    t5577_app_step_later(app, 50);
}

// Nothing is written until the whole of page 0 has been read and saved: a block that cannot be
// read cannot be put back, and a probe is only safe because there is a way back from it.
static bool t5577_app_scene_test_detect(T5577App* app) {
    if(!t5577_detect(&app->config)) {
        app->test_stop = T5577TestStopNoTag;
        return false;
    }

    // Deliberately not supported rather than quietly skipped: testing a protected tag means
    // addressing every frame to its password, and getting that wrong writes block 0.
    if(t5577_config_needs_password(app->config.block0)) {
        app->test_stop = T5577TestStopPassword;
        return false;
    }

    t5577_dump_set(&app->dump, 0, 0, app->config.block0);

    return true;
}

static bool t5577_app_scene_test_read(T5577App* app) {
    uint32_t value = 0;
    if(!t5577_block_value(&app->config, 0, app->block, &value)) {
        app->test_stop = T5577TestStopUnreadable;
        return false;
    }

    t5577_dump_set(&app->dump, 0, app->block, value);
    app->block++;

    return true;
}

// The way back, on the card, before the first write. One fixed name rather than a generated
// one: the result screen has a line to name it in, the newest backup is the one a failed run
// needs, and a run a week from now should not leave a card full of them.
static bool t5577_app_scene_test_backup(T5577App* app) {
    strlcpy(app->name, T5577_APP_TEST_BACKUP, sizeof(app->name));
    t5577_app_test_backup_path(app->path);

    if(t5577_file_save(&app->dump, furi_string_get_cstr(app->path))) {
        app->test_backed_up = true;
        return true;
    }

    app->test_stop = T5577TestStopNoBackup;

    return false;
}

// True when the tag may be holding the probe, which is what decides whether putting the block
// back has to be checked.
static bool t5577_app_scene_test_probe_read(T5577App* app) {
    const uint8_t block = app->block;
    const uint32_t was = app->dump.block[0][block];

    uint32_t value = 0;
    if(!t5577_block_value(&app->config, 0, block, &value)) {
        // The tag stopped answering, so whether the probe landed is unknown - which is why the
        // original goes back anyway before this run gives up. The cell goes back to empty: a
        // filled one is this view's "being worked on", and no verdict was reached.
        t5577_test_view_set_cell(app->test_view, block, T5577TestCellUnknown);
        app->test_stop = T5577TestStopLost;
        return true;
    }

    if(value == t5577_app_test_probe(was)) {
        app->test_writeable |= (uint8_t)(1u << block);
        t5577_test_view_set_cell(app->test_view, block, T5577TestCellWriteable);
        return true;
    }

    if(value == was) {
        // Locked, read-only, or not implemented at all. Indistinguishable from here, and from a
        // Proxmark3 too: nothing on the air says why a write was dropped.
        t5577_test_view_set_cell(app->test_view, block, T5577TestCellRefused);
        return false;
    }

    t5577_test_view_set_cell(app->test_view, block, T5577TestCellUnknown);
    app->test_stop = T5577TestStopOdd;

    return true;
}

// A block that may be holding the probe is read back after the original goes on, because a test
// that leaves a block holding something the tag never had is worse than no test at all.
static bool t5577_app_scene_test_restore_read(T5577App* app) {
    const uint32_t was = app->dump.block[0][app->block];

    uint32_t value = 0;
    bool back = t5577_block_value(&app->config, 0, app->block, &value) && value == was;

    // Read twice before calling a block wrong: a word of all zeroes or all ones is the one case
    // the chip cannot distinguish on the air without the alignment agreeing, so one disagreement
    // is as likely to be this reader's as the tag's, and the verdict it leads to tells the user
    // their tag is broken. Not worth paying on a tag already known to have gone, where the
    // second read is another second and a half of silence.
    if(!back && app->test_stop == T5577TestStopNone) {
        back = t5577_block_value(&app->config, 0, app->block, &value) && value == was;
    }

    if(!back) {
        // Overwrites a pending Lost or Odd on purpose: of the two, "your tag is wrong and here
        // is the way back" is the one the user has to act on.
        app->test_stop = T5577TestStopNotRestored;
        return false;
    }

    return t5577_app_scene_test_next(app);
}

bool t5577_app_scene_test_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back is not a way out of a half-written block. Before the first write there is nothing
    // to put back, so it leaves at once; after it, the press is taken now and acted on when the
    // block in flight has had its original written back and read.
    if(event.type == SceneManagerEventTypeBack) {
        if(app->step < TestStepProbe) return false;

        if(app->test_stop == T5577TestStopNone) {
            app->test_stop = T5577TestStopStopped;
            t5577_test_view_set_status(app->test_view, "Stopping...");
        }

        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    switch((TestStep)app->step) {
    case TestStepDetect:
        if(!t5577_app_scene_test_detect(app)) return t5577_app_scene_test_finish(app);
        app->step = TestStepRead;
        t5577_test_view_set_status(app->test_view, "Reading the tag");
        break;

    case TestStepRead:
        if(!t5577_app_scene_test_read(app)) return t5577_app_scene_test_finish(app);
        if(app->block >= LFRFID_T5577_BLOCKS_IN_PAGE_0) {
            app->step = TestStepBackup;
            t5577_test_view_set_status(app->test_view, "Saving a backup");
        }
        break;

    case TestStepBackup:
        if(!t5577_app_scene_test_backup(app)) return t5577_app_scene_test_finish(app);
        app->step = TestStepProbe;
        break;

    case TestStepProbe:
        // Nothing is in flight at this point, so a stop asked for during the last pair is acted
        // on here rather than by writing another probe.
        if(app->test_stop != T5577TestStopNone) return t5577_app_scene_test_finish(app);

        // The cursor for everything downstream, including the result screen, which names the
        // block every verdict and every failure happened to.
        app->block = t5577_app_test_order[app->order];

        t5577_test_view_set_cell(app->test_view, app->block, T5577TestCellTesting);
        t5577_app_test_say(app, "writing");
        t5577_app_test_write(app, t5577_app_test_probe(app->dump.block[0][app->block]));
        app->step = TestStepProbeRead;
        break;

    case TestStepProbeRead:
        t5577_app_scene_test_probe_read(app);
        app->step = TestStepRestore;
        t5577_app_test_say(app, "putting back");
        break;

    case TestStepRestore:
        t5577_app_test_write(app, app->dump.block[0][app->block]);

        if(!t5577_app_scene_test_refused(app)) {
            app->step = TestStepRestoreRead;
            break;
        }

        if(!t5577_app_scene_test_next(app)) return t5577_app_scene_test_finish(app);
        app->step = TestStepProbe;
        break;

    case TestStepRestoreRead:
        if(!t5577_app_scene_test_restore_read(app)) return t5577_app_scene_test_finish(app);
        app->step = TestStepProbe;
        break;
    }

    t5577_app_step_later(app, 10);

    return true;
}

void t5577_app_scene_test_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    notification_message(app->notifications, &sequence_blink_stop);
}
