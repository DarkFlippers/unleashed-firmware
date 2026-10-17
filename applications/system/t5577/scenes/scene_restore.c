#include "../t5577_app.h"

// Data blocks first and block 0 last, because block 0 decides how the chip talks: write it first
// and the rest of the sequence is verified against a configuration the chip is no longer using.
// Block 7 is the password store, and it has to be in place before block 0 turns protection on.
static const uint8_t t5577_app_restore_order[] = {1, 2, 3, 4, 5, 6, 7, 0};

typedef enum {
    RestoreStepWrite,
    RestoreStepDetect,
    RestoreStepVerify,
    RestoreStepDone,
} RestoreStep;

// Page 1 is writable, and is left alone anyway: it holds the chip's traceability data, which is
// not what anyone is restoring and cannot be put back once it is gone.
static bool t5577_app_restore_wanted(const T5577App* app, uint8_t index) {
    return t5577_dump_was_read(&app->dump, 0, t5577_app_restore_order[index]);
}

static uint8_t t5577_app_restore_total(const T5577App* app) {
    uint8_t total = 0;

    for(uint8_t i = 0; i < COUNT_OF(t5577_app_restore_order); i++) {
        if(t5577_app_restore_wanted(app, i)) total++;
    }

    return total;
}

static void t5577_app_scene_restore_say(T5577App* app, uint8_t block, uint8_t done) {
    char status[16];
    snprintf(status, sizeof(status), "Block %u", block);
    t5577_work_view_set_status(app->work_view, status, done);
}

void t5577_app_scene_restore_on_enter(void* context) {
    T5577App* app = context;

    app->step = RestoreStepWrite;
    app->block = 0;
    app->done = 0;
    app->checked = 0;
    app->bad = 0;
    app->total = t5577_app_restore_total(app);
    memset(&app->config, 0, sizeof(app->config));

    t5577_work_view_start(app->work_view, "Writing", app->total);
    t5577_app_scene_restore_say(app, t5577_app_restore_order[0], 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWork);

    notification_message(app->notifications, &sequence_blink_start_magenta);

    t5577_app_step_later(app, 50);
}

static void t5577_app_scene_restore_write(T5577App* app, uint8_t block) {
    LFRFIDT5577 write = {0};
    write.block[block] = app->dump.block[0][block];
    write.mask = (uint8_t)(1u << block);

    t5577_write_with_mask(&write, 0, app->dump.has_password, app->dump.password);
}

// Read the tag back as a reader would, rather than trusting the writes. The password to use is
// the one the restored configuration asks for, which is the dump's block 7 - not the password the
// dump was read with, which belonged to whatever the tag held before.
static bool t5577_app_scene_restore_detect(T5577App* app) {
    const bool needs_pass = (app->dump.block[0][0] & LFRFID_T5577_PWD) != 0;

    return needs_pass ? t5577_detect_with_pass(&app->config, app->dump.block[0][7]) :
                        t5577_detect(&app->config);
}

// Walk the same order the writes went in, and advance to the next block that was written.
static bool t5577_app_scene_restore_advance(T5577App* app) {
    app->block++;

    while(app->block < COUNT_OF(t5577_app_restore_order) &&
          !t5577_app_restore_wanted(app, app->block)) {
        app->block++;
    }

    return app->block < COUNT_OF(t5577_app_restore_order);
}

bool t5577_app_scene_restore_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;
    if(app->step == RestoreStepDone) return true;

    if(app->step == RestoreStepWrite) {
        const uint8_t block = t5577_app_restore_order[app->block];

        if(t5577_app_restore_wanted(app, app->block)) {
            t5577_app_scene_restore_write(app, block);
            app->done++;
        }

        if(!t5577_app_scene_restore_advance(app)) {
            app->step = RestoreStepDetect;
            // The check is its own half of the job, and gets its own bar.
            t5577_work_view_start(app->work_view, "Checking", app->total);
            t5577_work_view_set_status(app->work_view, "Finding tag", 0);
        } else {
            t5577_app_scene_restore_say(app, t5577_app_restore_order[app->block], app->done);
        }
    } else if(app->step == RestoreStepDetect) {
        if(!t5577_app_scene_restore_detect(app)) {
            app->step = RestoreStepDone;
            scene_manager_next_scene(app->scene_manager, T5577AppSceneRestoreResult);
            return true;
        }

        // Start the check at the first block that was written.
        app->step = RestoreStepVerify;
        app->block = 0;
        if(!t5577_app_restore_wanted(app, app->block) && !t5577_app_scene_restore_advance(app)) {
            app->step = RestoreStepDone;
            scene_manager_next_scene(app->scene_manager, T5577AppSceneRestoreResult);
            return true;
        }
        t5577_app_scene_restore_say(app, t5577_app_restore_order[app->block], 0);
    } else {
        const uint8_t block = t5577_app_restore_order[app->block];

        uint32_t value = 0;
        if(!t5577_block_value(&app->config, 0, block, &value) ||
           value != app->dump.block[0][block]) {
            app->bad++;
        }
        app->checked++;

        if(!t5577_app_scene_restore_advance(app)) {
            app->step = RestoreStepDone;
            scene_manager_next_scene(app->scene_manager, T5577AppSceneRestoreResult);
            return true;
        }

        t5577_app_scene_restore_say(app, t5577_app_restore_order[app->block], app->checked);
    }

    t5577_app_step_later(app, 10);

    return true;
}

void t5577_app_scene_restore_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    notification_message(app->notifications, &sequence_blink_stop);
}
