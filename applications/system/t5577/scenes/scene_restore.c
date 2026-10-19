#include "../t5577_app.h"

typedef enum {
    RestoreStepWrite,
    RestoreStepDetect,
    RestoreStepVerify,
} RestoreStep;

static void t5577_app_scene_restore_say(T5577App* app, uint8_t done) {
    if(app->order >= T5577_RESTORE_BLOCKS) return;

    const T5577Target target = t5577_restore_target(app->order);

    char status[16];
    snprintf(status, sizeof(status), "P%u block %u", target.page, target.block);
    t5577_work_view_set_status(app->work_view, status, done);
}

// The first place in the write order, at or after `from`, that a write puts a block back. Both
// walks step with this, so neither has to know what the order holds, and T5577_RESTORE_BLOCKS
// coming back means there is nothing left.
static uint8_t t5577_app_scene_restore_seek(const T5577Dump* dump, uint8_t from) {
    uint8_t index = from;

    while(index < T5577_RESTORE_BLOCKS && !t5577_restore_wanted(dump, index))
        index++;

    return index;
}

// Nothing more to do with the tag. Pushed rather than switched to, so Back from the result
// goes past this scene instead of into it and writing the tag again.
static bool t5577_app_scene_restore_finish(T5577App* app) {
    scene_manager_next_scene(app->scene_manager, T5577AppSceneRestoreResult);

    return true;
}

void t5577_app_scene_restore_on_enter(void* context) {
    T5577App* app = context;

    app->step = RestoreStepWrite;
    app->order = t5577_app_scene_restore_seek(&app->dump, 0);
    app->done = 0;
    app->checked = 0;
    app->bad = 0;
    app->bad_trace = 0;
    app->lost = 0;
    app->lost_at = (T5577Target){0};
    app->bad_at = (T5577Target){0};

    t5577_work_view_start(app->work_view, "Writing", t5577_restore_total(&app->dump));
    t5577_app_scene_restore_say(app, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWork);

    notification_message(app->notifications, &sequence_blink_start_magenta);

    // As in a read: the screen goes up before the first write takes the thread.
    t5577_app_step_later(app, 50);
}

static void t5577_app_scene_restore_write(T5577App* app, uint8_t index, T5577Target target) {
    // Which password addresses this frame is the other half of the write order, so it comes from
    // the same place: writing block 7 replaces the one the chip asks for, and everything after
    // it carries the new word.
    bool with_pass = false;
    const uint32_t password = t5577_restore_password(&app->dump, index, &with_pass);

    t5577_write_block(
        target.page, target.block, app->dump.block[target.page][target.block], with_pass, password);
}

// Read the tag back as a reader would, rather than trusting the writes - t5577_write_with_mask
// cannot report anything, so this is the only evidence a restore has.
//
// A plain read first, always: it is the one that cannot damage anything, and a chip that answers
// it needs no password. Only when it says nothing is a password frame worth sending, because
// that frame writes block 0 of a chip that turns out not to want one - and then only with a
// password there is reason to believe in, which is block 7's word when that block went back and
// otherwise the one the dump was read with.
//
// What it cannot know is what the chip holds now: the test is on the configuration the restore
// wrote, so a block 0 the chip refused leaves this sending a password the chip is not asking
// for. A plain read having already failed is what keeps that from being the first thing tried.
static bool t5577_app_scene_restore_detect(T5577App* app) {
    if(t5577_detect(&app->config)) return true;

    // The whole order has gone back by now, so this asks for the password the last frame used.
    bool have = false;
    const uint32_t password = t5577_restore_password(&app->dump, T5577_RESTORE_BLOCKS, &have);
    const bool asks = t5577_dump_was_read(&app->dump, 0, 0) &&
                      t5577_config_needs_password(app->dump.block[0][0]);

    if(!asks || !have) return false;

    return t5577_detect_with_pass(&app->config, password);
}

// Compare one block against the dump. Page 0 block 0 is the one block the detect has already
// read off the tag, so reading it again would spend a second proving what is in hand.
static void t5577_app_scene_restore_check(T5577App* app, T5577Target target) {
    const bool is_config = target.page == 0 && target.block == 0;

    uint32_t value = app->config.block0;
    const bool answered = is_config ||
                          t5577_block_value(&app->config, target.page, target.block, &value);

    // A block that would not read back and a block that read back different are not the same
    // failure: the first is a tag that stopped answering, the second a write the chip refused.
    // Counting them together is how a lost tag gets reported as a locked block.
    if(!answered) {
        if(app->lost == 0) app->lost_at = target;
        app->lost++;
    } else if(value != app->dump.block[target.page][target.block]) {
        // A refused traceability block is the chip working as it was made to, so it is counted
        // apart from a write the chip refused for any other reason. What cannot be told apart
        // is a write that was simply dropped, which is why the screen says the block is
        // unchanged rather than claiming to know the chip locked it.
        if(t5577_block_is_trace(target.page, target.block)) {
            app->bad_trace++;
        } else {
            if(app->bad == 0) app->bad_at = target;
            app->bad++;
        }
    }

    app->checked++;
}

bool t5577_app_scene_restore_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    if(app->step == RestoreStepWrite) {
        if(app->order < T5577_RESTORE_BLOCKS) {
            t5577_app_scene_restore_write(app, app->order, t5577_restore_target(app->order));
            app->done++;
            app->order = t5577_app_scene_restore_seek(&app->dump, app->order + 1);
        }

        if(app->order >= T5577_RESTORE_BLOCKS) {
            app->step = RestoreStepDetect;
            // The check is its own half of the job, and gets its own bar.
            t5577_work_view_start(app->work_view, "Checking", app->done);
            t5577_work_view_set_status(app->work_view, "Finding tag", 0);
        } else {
            t5577_app_scene_restore_say(app, app->done);
        }
    } else if(app->step == RestoreStepDetect) {
        if(!t5577_app_scene_restore_detect(app)) return t5577_app_scene_restore_finish(app);

        // Check in the order the writes went in, starting at the first block that was written.
        app->step = RestoreStepVerify;
        app->order = t5577_app_scene_restore_seek(&app->dump, 0);
        if(app->order >= T5577_RESTORE_BLOCKS) return t5577_app_scene_restore_finish(app);

        t5577_app_scene_restore_say(app, 0);
    } else {
        t5577_app_scene_restore_check(app, t5577_restore_target(app->order));

        // A tag that has stopped answering will not answer the rest of the order either, and
        // every block it is asked for costs a second and a half of silence.
        if(app->lost >= T5577_APP_LOST_MAX) return t5577_app_scene_restore_finish(app);

        app->order = t5577_app_scene_restore_seek(&app->dump, app->order + 1);
        if(app->order >= T5577_RESTORE_BLOCKS) return t5577_app_scene_restore_finish(app);

        t5577_app_scene_restore_say(app, app->checked);
    }

    t5577_app_step_later(app, 10);

    return true;
}

void t5577_app_scene_restore_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    notification_message(app->notifications, &sequence_blink_stop);
}
