#include "../t5577_app.h"

typedef enum {
    ReadStepDetect,
    ReadStepBlocks,
    ReadStepDone,
} ReadStep;

void t5577_app_scene_read_on_enter(void* context) {
    T5577App* app = context;

    app->step = ReadStepDetect;
    // A fresh read deserves its own result screen, sound and all.
    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneReadResult, 0);
    app->page = 0;
    app->block = 0;
    app->done = 0;
    app->lost = 0;
    memset(&app->dump, 0, sizeof(app->dump));

    t5577_work_view_start(app->work_view, "Reading", T5577_APP_BLOCKS);
    t5577_work_view_set_status(app->work_view, "Finding tag", 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWork);

    notification_message(app->notifications, &sequence_blink_start_cyan);

    // Long enough for the screen to be up before the first read takes the thread for a second.
    t5577_app_step_later(app, 50);
}

// Work out how a tag reads, which is the only part of a read that can fail outright: without the
// configuration there is no rate to read at and no way to know where a word begins.
static bool t5577_app_scene_read_detect(T5577App* app) {
    const bool found = app->password_entered ?
                           t5577_detect_with_pass(&app->config, app->password) :
                           t5577_detect(&app->config);
    if(!found) return false;

    // Block 0 came back twice over in the detect, so it needs no read of its own.
    t5577_dump_set(&app->dump, 0, 0, app->config.block0);
    app->done = 1;
    app->page = 0;
    app->block = 1;

    // A password the user typed is the tag's only if the tag asked for one. Recording it
    // either way puts it in the dump, and a write from that dump then addresses a password
    // frame to a chip that is not protected, which reads it as something else entirely.
    if(app->password_entered && (app->config.block0 & LFRFID_T5577_PWD)) {
        t5577_dump_set_password(&app->dump, app->password);
    }

    return true;
}

// Returns false once there is nothing left to read.
static bool t5577_app_scene_read_block(T5577App* app) {
    uint32_t value = 0;
    if(t5577_block_value(&app->config, app->page, app->block, &value)) {
        t5577_dump_set(&app->dump, app->page, app->block, value);
    } else {
        // Which block went missing first is the useful half of how many did: a protected
        // chip goes quiet from one block on, where a tag that moved loses them anywhere.
        if(app->lost == 0) app->failed = (T5577AppTarget){app->page, app->block};
        app->lost++;
    }

    app->done++;
    app->block++;

    if(app->block >= t5577_page_blocks(app->page)) {
        app->block = 0;
        app->page++;
    }

    return app->page <= 1;
}

bool t5577_app_scene_read_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    // A step that arrives after the scene is finished with has nothing left to do, and acting on
    // it would read a tag the user has already been told about.
    if(app->step == ReadStepDone) return true;

    bool more = true;
    if(app->step == ReadStepDetect) {
        more = t5577_app_scene_read_detect(app);
        app->step = ReadStepBlocks;
    } else {
        more = t5577_app_scene_read_block(app);
    }

    if(!more) {
        app->step = ReadStepDone;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneReadResult);
        return true;
    }

    char status[16];
    snprintf(status, sizeof(status), "P%u block %u", app->page, app->block);
    t5577_work_view_set_status(app->work_view, status, app->done);

    t5577_app_step_later(app, 10);

    return true;
}

void t5577_app_scene_read_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    notification_message(app->notifications, &sequence_blink_stop);
}
