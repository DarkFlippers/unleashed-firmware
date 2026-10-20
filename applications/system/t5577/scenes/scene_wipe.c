#include "../t5577_app.h"

// Proxmark3 "lf t55xx wipe" default config block 0: ASK/Manchester, RF/32, 7 data blocks,
// password disabled. Leaves the tag in a known-good, openly re-writable blank state, and matching
// Proxmark3 means a tag wiped by either tool looks the same to the other.
#define T5577_APP_WIPE_BLOCK0                                          \
    (LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x000880E0

// Blocks 1..7 one per step, then the configuration, then one read back. The configuration goes
// last so that the write being verified is the one immediately before the read: that is all this
// proves, since the data blocks are never read back and a tag that drifted away during them and
// returned for the end still reads as wiped.
typedef enum {
    WipeStepData,
    WipeStepConfig,
    WipeStepVerify,
} WipeStep;

// Seven data blocks, the configuration, and the read back.
#define T5577_APP_WIPE_STEPS (LFRFID_T5577_BLOCKS_IN_PAGE_0 + 1)

void t5577_app_scene_wipe_on_enter(void* context) {
    T5577App* app = context;

    app->step = WipeStepData;
    app->block = 1;
    app->done = 0;
    app->wipe_result = T5577BlockCannotVerify;

    t5577_work_view_start(app->work_view, "Wiping", T5577_APP_WIPE_STEPS);
    t5577_work_view_set_status(app->work_view, "Keep tag still", 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWork);

    notification_message(app->notifications, &sequence_blink_start_magenta);

    // One field across the whole wipe, so the carrier is not dropped between blocks.
    t5577_field_start();

    t5577_app_step_later(app, 50);
}

// No password on any frame: a protected tag rejects the writes, and cannot slip past the read
// back either, because PWD is bit 4 of the very word being compared - a tag still enforcing one
// can never read back as this configuration.
static void t5577_app_scene_wipe_write(const T5577App* app) {
    t5577_write_block(0, app->block, app->block == 0 ? T5577_APP_WIPE_BLOCK0 : 0, false, 0);
}

static T5577BlockResult t5577_app_scene_wipe_verify(void) {
    const T5577ReadRequest request = {
        .page = 0,
        .block = 0,
        .clock = t5577_read_clock_from_config(T5577_APP_WIPE_BLOCK0),
    };

    return t5577_block_holds(&request, T5577_APP_WIPE_BLOCK0);
}

bool t5577_app_scene_wipe_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back is taken but not acted on: a wipe is nine frames that cannot be half sent, and the
    // blocks already zeroed are not put back by leaving early.
    if(event.type == SceneManagerEventTypeBack) return true;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    char status[24];

    switch((WipeStep)app->step) {
    case WipeStepData:
        t5577_app_scene_wipe_write(app);
        app->done++;
        app->block++;

        if(app->block >= LFRFID_T5577_BLOCKS_IN_PAGE_0) {
            app->step = WipeStepConfig;
            app->block = 0;
        }

        snprintf(status, sizeof(status), "Block %u", app->block);
        t5577_work_view_set_status(app->work_view, status, app->done);
        break;

    case WipeStepConfig:
        t5577_app_scene_wipe_write(app);
        app->done++;
        app->step = WipeStepVerify;

        // The read back is one unsplittable second, and the longest step by far, so it is said
        // out loud rather than looking like the bar has stalled.
        t5577_work_view_set_status(app->work_view, "Checking tag", app->done);
        break;

    case WipeStepVerify:
        app->wipe_result = t5577_app_scene_wipe_verify();
        scene_manager_next_scene(app->scene_manager, T5577AppSceneWipeResult);
        return true;
    }

    t5577_app_step_later(app, 10);

    return true;
}

void t5577_app_scene_wipe_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    t5577_field_stop();
    notification_message(app->notifications, &sequence_blink_stop);
}
