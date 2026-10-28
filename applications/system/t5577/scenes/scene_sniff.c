#include "../t5577_app.h"

#define TAG "T5577Sniff"

/** How often the capture is checked, in milliseconds. */
#define T5577_SNIFF_TICK_MS 200

/** Ticks with no new gap that mean the burst is over.
 *
 * A sender's own pauses are milliseconds - the longest is a programming time - so a second of
 * nothing is well past any of them and well inside the patience of someone holding a Flipper
 * against a cloner.
 */
#define T5577_SNIFF_QUIET_TICKS 5

/** Ticks to wait before giving up with nothing. The user has to trigger the other device after
 * starting this, so the window is long enough to get to it.
 */
#define T5577_SNIFF_TIMEOUT_TICKS 150

/** Gaps in one tick that fill the trace.
 *
 * A cloner mid-frame puts a hundred or more through a tick; a Flipper on a desk averages five.
 * Forty is well above the second and well below the first, so the trace sits near its floor on
 * noise and saturates on a sender - which is the distinction somebody positioning the coil is
 * looking for.
 */
#define T5577_SNIFF_TRACE_FULL 40

void t5577_app_scene_sniff_on_enter(void* context) {
    T5577App* app = context;

    if(app->sniff == NULL) app->sniff = t5577_sniff_alloc();

    // A fresh capture deserves its own result screen, chime and all.
    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneSniffResult, 0);

    app->sniff_gaps = 0;
    app->sniff_quiet = 0;
    app->sniff_ticks = 0;

    // The listen view rather than the work view - see t5577_listen_view.h for why a bar is the
    // wrong instrument for a capture.
    t5577_listen_view_start(app->listen_view, "Listening");
    t5577_listen_view_set_status(app->listen_view, "Nothing heard yet");
    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewListen);

    notification_message(app->notifications, &sequence_blink_start_cyan);

    t5577_sniff_start(app->sniff);
    t5577_app_step_later(app, T5577_SNIFF_TICK_MS);
}

static void t5577_app_scene_sniff_conclude(T5577App* app) {
    // Stopped before it is decoded, and decoded before anything is reported: the decode walks
    // the ring in place, so a gap landing mid-walk would be framed into a sequence already
    // half read.
    t5577_sniff_stop(app->sniff);
    t5577_sniff_decode(app->sniff, &app->sniff_result);

    const T5577SniffResult* result = &app->sniff_result;

    FURI_LOG_I(
        TAG,
        "%lu stalls %lu gaps %lu bursts%s | %u frames %u dropped | %lu / %lu us | split %u",
        (unsigned long)result->progress.stalls,
        (unsigned long)result->progress.gaps,
        (unsigned long)result->progress.bursts,
        result->progress.overflowed ? " (ring wrapped)" : "",
        result->frames,
        result->dropped,
        (unsigned long)result->short_us,
        (unsigned long)result->long_us,
        result->split_us);

    // Every stall's width, in band or not, as `<low edge> us: <count>` for the bins that saw
    // anything. This is what says whether a sender this could not decode was using gaps the
    // band never admitted - the frame delimiter among them, if its start gap is wide.
    FuriString* widths = furi_string_alloc();

    for(uint8_t i = 0; i < T5577_SNIFF_WIDTH_BUCKETS; i++) {
        if(result->widths.bucket[i] == 0) continue;

        furi_string_cat_printf(
            widths,
            " %u:%u",
            (unsigned)(i * T5577_SNIFF_WIDTH_BUCKET_US),
            result->widths.bucket[i]);
    }

    FURI_LOG_I(TAG, "stall widths us:count |%s", furi_string_get_cstr(widths));
    furi_string_free(widths);

    // Debug, not info: the serial log handler busy-waits on the UART a byte at a time, so one
    // of these lines costs about 6 ms of app thread and a full capture's worth costs 200 ms,
    // against a decode that takes half a millisecond. `log level debug` brings them back.
    for(uint8_t i = 0; i < result->frames; i++) {
        const T5577SniffFrame* frame = &result->frame[i];

        FURI_LOG_D(
            TAG,
            "  [%u] kind %u why %u | %u symbols%s | page %u block %u lock %u | pwd %08lX "
            "data %08lX | pause %lu%s us",
            i,
            frame->kind,
            frame->why,
            frame->symbols,
            frame->leading_zero ? " lz" : "",
            frame->page ? 1 : 0,
            frame->block,
            frame->lock ? 1 : 0,
            (unsigned long)frame->password,
            (unsigned long)frame->data,
            (unsigned long)frame->pause_us,
            frame->pause_capped ? "+" : "");
    }

    scene_manager_next_scene(app->scene_manager, T5577AppSceneSniffResult);
}

bool t5577_app_scene_sniff_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventStep) return false;

    T5577SniffProgress progress;
    t5577_sniff_poll(app->sniff, &progress);

    // Gaps rather than bursts: a burst is only counted once the pause after it arrives, so
    // waiting on that would read a command still being sent as silence.
    const uint32_t arrived = progress.gaps - app->sniff_gaps;

    if(arrived > 0) {
        app->sniff_quiet = 0;
        app->sniff_gaps = progress.gaps;
    } else {
        // Unguarded, like sniff_ticks below: the capture ends at T5577_SNIFF_TIMEOUT_TICKS and
        // this only moves on ticks that move that too, so it cannot reach a byte's ceiling.
        app->sniff_quiet++;
    }

    // One sample per tick, whatever happened - a flat trace is a measurement, and a screen that
    // only moved on activity could not be told from one that had stopped updating. The two
    // numbers go over as measured; turning them into pixels is the view's business.
    t5577_listen_view_push(app->listen_view, arrived, T5577_SNIFF_TRACE_FULL);

    app->sniff_ticks++;

    // A whole command has been and gone, and nothing has arrived since. Not stopped on the ring
    // filling up: it keeps the newest gaps, so a sender that overruns it loses the start of its
    // sequence - but noise alone can overrun it too, and stopping there would report silence
    // because the coupling was poor.
    const bool done = (progress.bursts > 0 && app->sniff_quiet >= T5577_SNIFF_QUIET_TICKS) ||
                      app->sniff_ticks >= T5577_SNIFF_TIMEOUT_TICKS;

    if(done) {
        t5577_app_scene_sniff_conclude(app);
        return true;
    }

    // Runs, not gaps. A line that changed on the first in-band gap was tried and is a lie: a
    // desk produces those by the hundred - 826 in thirty seconds - so it said a reader had been
    // found while the Flipper sat on one. "Bursts" rather than "commands" because this counter
    // uses T5577_SNIFF_FRAME_MIN, which admits the two-symbol commands and therefore admits the
    // occasional noise pair; what reaches T5577_SNIFF_COMMAND_MIN is decided by the decode.
    char status[24];

    if(progress.bursts > 0) {
        snprintf(status, sizeof(status), "Heard %lu bursts", (unsigned long)progress.bursts);
    } else {
        strlcpy(status, "Nothing heard yet", sizeof(status));
    }

    t5577_listen_view_set_status(app->listen_view, status);

    t5577_app_step_later(app, T5577_SNIFF_TICK_MS);

    return true;
}

void t5577_app_scene_sniff_on_exit(void* context) {
    T5577App* app = context;

    t5577_app_step_stop(app);
    t5577_sniff_stop(app->sniff);
    notification_message(app->notifications, &sequence_blink_stop);
}
