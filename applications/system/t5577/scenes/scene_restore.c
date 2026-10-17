#include "../t5577_app.h"

typedef enum {
    RestoreStepWrite,
    RestoreStepDetect,
    RestoreStepVerify,
    RestoreStepDone,
} RestoreStep;

static uint8_t t5577_app_restore_total(const T5577App* app) {
    uint8_t total = 0;

    for(uint8_t i = 0; i < T5577_APP_RESTORE_BLOCKS; i++) {
        if(t5577_app_restore_wanted(&app->dump, i)) total++;
    }

    return total;
}

static void t5577_app_scene_restore_say(T5577App* app, T5577AppTarget target, uint8_t done) {
    char status[16];
    snprintf(status, sizeof(status), "P%u block %u", target.page, target.block);
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
    app->write_password = app->dump.password;
    memset(&app->config, 0, sizeof(app->config));

    t5577_work_view_start(app->work_view, "Writing", app->total);
    t5577_app_scene_restore_say(app, t5577_app_restore_order[0], 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWork);

    notification_message(app->notifications, &sequence_blink_start_magenta);

    t5577_app_step_later(app, 50);
}

static void t5577_app_scene_restore_write(T5577App* app, T5577AppTarget target) {
    LFRFIDT5577 write = {0};
    write.block[target.block] = app->dump.block[target.page][target.block];
    write.mask = (uint8_t)(1u << target.block);

    t5577_write_with_mask(&write, target.page, app->dump.has_password, app->write_password);

    // Writing block 7 replaces the password the chip is already asking for, so everything after
    // it has to be addressed to the new one. A Proxmark3 restore switches at the same point.
    if(target.page == 0 && target.block == 7) app->write_password = app->dump.block[0][7];
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

    while(app->block < T5577_APP_RESTORE_BLOCKS &&
          !t5577_app_restore_wanted(&app->dump, app->block)) {
        app->block++;
    }

    return app->block < T5577_APP_RESTORE_BLOCKS;
}

bool t5577_app_scene_restore_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;
    if(app->step == RestoreStepDone) return true;

    if(app->step == RestoreStepWrite) {
        if(t5577_app_restore_wanted(&app->dump, app->block)) {
            t5577_app_scene_restore_write(app, t5577_app_restore_order[app->block]);
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
        if(!t5577_app_restore_wanted(&app->dump, app->block) &&
           !t5577_app_scene_restore_advance(app)) {
            app->step = RestoreStepDone;
            scene_manager_next_scene(app->scene_manager, T5577AppSceneRestoreResult);
            return true;
        }
        t5577_app_scene_restore_say(app, t5577_app_restore_order[app->block], 0);
    } else {
        const T5577AppTarget target = t5577_app_restore_order[app->block];

        uint32_t value = 0;
        if(!t5577_block_value(&app->config, target.page, target.block, &value) ||
           value != app->dump.block[target.page][target.block]) {
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
