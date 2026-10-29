#include "../t5577_app.h"

#define TAG "T5577Restore"

// A failed plain detect is three reads of block 0 and then no more, so a badly coupled tag can
// lose all three - and a false negative here escalates to the one frame that can set a lock
// bit, which is worth asking again to avoid.
#define T5577_APP_RESTORE_OPEN_TRIES 3

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
    app->open_try = 0;
    app->done = 0;
    app->checked = 0;
    app->bad = 0;
    app->bad_trace = 0;
    app->lost = 0;
    app->read_back = T5577RestoreReadBackSilent;
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

// The last resort of the read back, reached only once every plain detect has come back silent.
// Reading the tag at all is the only evidence a restore has, since t5577_write_with_mask cannot
// report anything - but the plain reads run first and this does not, because the frame it sends
// writes block 0 of a chip that turns out not to want a password, and only a word there is
// reason to believe in is worth that: block 7's when that block went back, otherwise the one the
// dump was read with.
//
// What it cannot know is what the chip holds now: the test is on the configuration the restore
// wrote, so a block 0 the chip refused leaves this sending a password the chip is not asking
// for.
static bool t5577_app_scene_restore_password(T5577App* app) {
    // The whole order has gone back, so the word is the last frame's - but whether to send it
    // is not, and t5577_restore_password() answers that from the block 0 just written.
    bool have = false;
    const uint32_t password = t5577_restore_password(&app->dump, T5577_RESTORE_BLOCKS, &have);
    const bool asks = t5577_dump_was_read(&app->dump, 0, 0) &&
                      t5577_config_needs_password(app->dump.block[0][0]);

    if(!asks || !have) {
        // Both are properties of the dump, not of the tag - nothing has been heard from the tag
        // at all by here. `asks` false is a written configuration that wants no password, so
        // there is nothing to send; `have` false is the actionable one, a configuration that
        // wants one with no word in the dump to check it with.
        if(asks) app->read_back = T5577RestoreReadBackNoWord;

        FURI_LOG_W(
            TAG,
            "restore: no read back after %u plain reads - written config %s, password %s",
            app->open_try,
            asks ? "wants a password" : "wants none",
            have ? "available" : "unknown");
        return false;
    }

    if(!t5577_detect_with_pass(&app->config, password)) {
        // The frame that can set a lock bit has gone out and nothing came back, so which tag it
        // reached is now unknown.
        app->read_back = T5577RestoreReadBackSent;

        FURI_LOG_W(TAG, "restore: password read back failed, sent %08lX", (unsigned long)password);
        return false;
    }

    FURI_LOG_I(TAG, "restore: found tag, block0 %08lX", (unsigned long)app->config.block0);

    // with_pass clear means t5577_detect dropped it: the chip answered and its own block 0 asks
    // for nothing, so the frame above went out at a tag that was not asking. It answered
    // afterwards, so it is not locked - but this is the near miss the retries exist to avoid.
    if(!app->config.with_pass) {
        FURI_LOG_E(
            TAG,
            "restore: sent %08lX to a chip that asks for nothing - block 0 may hold %08lX now",
            (unsigned long)password,
            (unsigned long)(password << 1));
    }

    return true;
}

// Compare one block against the dump. Page 0 block 0 is the one block the detect has already
// read off the tag, so reading it again would spend a second proving what is in hand.
static void t5577_app_scene_restore_check(T5577App* app, T5577Target target) {
    const bool is_config = target.page == 0 && target.block == 0;
    const uint32_t expected = app->dump.block[target.page][target.block];

    // "Does it hold this word" before "what word does it hold": t5577_block_holds() matches at
    // every alignment and both polarities, where t5577_block_value() has to resolve an unknown
    // word at the single offset the detect measured. Block 1 of an EM4100 key is nine 1s and
    // then long runs of one bit - the fewest transitions of any block to lock onto, and a
    // capture filled at the maximum rate - so a key that was written perfectly is exactly where
    // recovering the word loses, and the restore then calls that a tag that stopped answering.
    if(!is_config) {
        const T5577ReadRequest request = {
            .page = target.page,
            .block = target.block,
            .clock = app->config.clock,
            .with_pass = app->config.with_pass,
            .password = app->config.password,
        };

        if(t5577_block_holds(&request, expected) == T5577BlockHeld) {
            app->checked++;
            return;
        }
    }

    // It did not hold the word. Recovering what it does hold is what tells a refused write from
    // a tag that went quiet, and it is only worth the second read on this path.
    uint32_t value = app->config.block0;
    const bool answered = is_config ||
                          t5577_block_value(&app->config, target.page, target.block, &value);

    // A block that would not read back and a block that read back different are not the same
    // failure: the first is a tag that stopped answering, the second a write the chip refused.
    // Counting them together is how a lost tag gets reported as a locked block.
    if(!answered) {
        FURI_LOG_W(TAG, "restore: P%u block %u did not answer", target.page, target.block);
        if(app->lost == 0) app->lost_at = target;
        app->lost++;
    } else if(value != expected) {
        FURI_LOG_W(
            TAG,
            "restore: P%u block %u is %08lX, wrote %08lX",
            target.page,
            target.block,
            (unsigned long)value,
            (unsigned long)expected);
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
        // A tick per attempt. An attempt is most of two seconds whether or not a tag answers -
        // three reads of block 0, one per rate - so the three of them and the password read
        // back after were the better part of six seconds inside one handler, long enough that
        // Back went unlooked at until it ended and the presses made meanwhile arrived in a heap.
        if(app->open_try < T5577_APP_RESTORE_OPEN_TRIES) {
            app->open_try++;
            if(!t5577_detect(&app->config)) {
                // The last silent attempt is the one that commits to the password frame, so
                // the screen says so on this tick rather than the one that spends two seconds
                // sending it - otherwise the words arrive after the risk does.
                if(app->open_try == T5577_APP_RESTORE_OPEN_TRIES) {
                    t5577_work_view_set_status(app->work_view, "Trying password", 0);
                }

                t5577_app_step_later(app, 10);
                return true;
            }
        } else if(!t5577_app_scene_restore_password(app)) {
            return t5577_app_scene_restore_finish(app);
        }

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
