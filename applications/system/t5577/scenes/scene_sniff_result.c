#include "../t5577_app.h"

// What the capture came to, in the order a finding is worth having. A password the sender already
// holds opens the tag now; one being installed opens it afterwards; an unresolved frame is worth
// a word only to say which screen the two readings are on.
typedef enum {
    SniffFindingNothing,
    SniffFindingNoPassword,
    SniffFindingUnresolved,
    SniffFindingInstalled,
    SniffFindingHeld,
} SniffFinding;

typedef struct {
    SniffFinding finding;
    uint32_t word;
    // How the frame it came from was settled, so a headline drawn from a deduction about a
    // NEIGHBOURING frame does not read like one drawn from the frame itself.
    T5577SniffWhy why;
} SniffVerdict;

static SniffVerdict t5577_app_scene_sniff_result_verdict(const T5577SniffResult* result) {
    SniffVerdict best = {SniffFindingNothing, 0, 0};

    for(uint8_t i = 0; i < result->frames; i++) {
        const T5577SniffFrame* frame = &result->frame[i];
        SniffVerdict here = {SniffFindingNothing, 0, frame->why};

        if(t5577_sniff_frame_tells_password(frame)) {
            here.finding = SniffFindingHeld;
            here.word = frame->password;
        } else if(
            frame->kind == T5577SniffFramePlainWrite &&
            frame->block == LFRFID_T5577_PASSWORD_BLOCK && !frame->page) {
            here.finding = SniffFindingInstalled;
            here.word = frame->data;
        } else if(frame->kind == T5577SniffFrameAmbiguous) {
            // The password reading of the two, because that is the one worth trying; the write
            // reading is on the frame's own screen beside it.
            here.finding = SniffFindingUnresolved;
            here.word = frame->password;
        } else if(frame->kind != T5577SniffFrameUnknown && frame->symbols >= T5577_SNIFF_COMMAND_MIN) {
            // Long enough to have been addressed at a tag. A two-symbol burst that decoded as a
            // reset is not, on its own, a reader - see T5577_SNIFF_COMMAND_MIN.
            here.finding = SniffFindingNoPassword;
        }

        if(here.finding > best.finding) best = here;
    }

    return best;
}

static void
    t5577_app_scene_sniff_result_button(GuiButtonType type, InputType input, void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventRetry);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventFrames);
    }
}

void t5577_app_scene_sniff_result_on_enter(void* context) {
    T5577App* app = context;

    const T5577SniffResult* result = &app->sniff_result;
    const SniffVerdict verdict = t5577_app_scene_sniff_result_verdict(result);

    const char* title = "Nothing heard";
    // The counts, not advice. "Hold it on the coil" is wrong in the one case it most matters -
    // when the coil IS on a reader whose gaps this cannot measure - and a verdict naming that
    // case was tried and withdrawn: it rested on `stalls > 0`, and the trigger pin is never
    // quiet. Open air alone logs thousands of stalls with nothing in front of it, so that
    // verdict reported an unreadable reader from an empty desk. What can honestly be said is
    // what was counted.
    char counts[48];

    snprintf(
        counts,
        sizeof(counts),
        "No gap this can read.\nCarrier cut %lu times",
        (unsigned long)result->progress.stalls);

    const char* detail = counts;
    // Exactly the findings that carry one, which is what the rank already says.
    const bool word = verdict.finding >= SniffFindingUnresolved;

    // A length no other command has is the frame speaking for itself. The other two are
    // deductions from the frames around it, and a headline that does not say so reads as
    // certainty the capture did not supply.
    const bool deduced = verdict.why != T5577SniffWhyLength;

    switch(verdict.finding) {
    case SniffFindingHeld:
        title = "Password";
        detail = deduced ? "If that was a read.\nSee Frames" : "The reader sent it";
        break;
    case SniffFindingInstalled:
        title = "New password";
        detail = deduced ? "If that was a write.\nSee Frames" : "Written to block 7";
        break;
    case SniffFindingUnresolved:
        // 38 bits either way, and nothing in the capture chose. Saying which it is would be a
        // guess about somebody's tag, so the screen says there are two readings and where.
        title = "Maybe a password";
        detail = "Or a write. Both\nreadings are in Frames";
        break;
    case SniffFindingNoPassword:
        // Reports what was heard; does not conclude what was not. "No password" as a headline
        // is read as "this reader is unprotected", which is a claim about somebody's door drawn
        // from what happened to parse - and every truncation below lands here.
        title = "Commands heard";
        detail = "None of them carried\na password";
        break;
    case SniffFindingNothing:
        // Something long enough to be a command arrived and did not decode. Not gated on
        // frames: open air makes short bursts by itself, and a screen gated on those reports a
        // reader that is not there - one burst of two symbols in thirty seconds on a desk.
        if(t5577_sniff_commanded(result)) {
            title = "Can't decode";
            detail = "Something cut the\ncarrier. See Frames";
        } else if(result->frames > 0) {
            // Short bursts only - which a quiet coil also produces, so this is not a finding
            // about the reader. Said rather than titled "Nothing heard" beside a Frames button
            // that opens a populated list.
            title = "Nothing decoded";
            detail = "Only bursts too short\nto be a command";
        }
        break;
    }

    widget_reset(app->widget);
    widget_add_string_element(app->widget, 0, 2, AlignLeft, AlignTop, FontPrimary, title);

    FuriString* line = furi_string_alloc();
    uint8_t detail_y = 18;

    if(word) {
        furi_string_printf(line, "%08lX", (unsigned long)verdict.word);
        widget_add_string_element(
            app->widget, 0, 16, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(line));
        detail_y = 30;
    }

    // A capture that lost frames cannot support a statement about what was not in it, so the
    // caveat goes on this screen rather than only in the log. It replaces the detail text: at
    // two lines there is no room for both, and which of the two matters more is not close.
    // Safe to reuse `line` - the widget copied the word above rather than keeping the pointer.
    if(result->dropped > 0 || result->progress.overflowed) {
        furi_string_printf(
            line,
            "Capture was cut short:\n%s",
            result->progress.overflowed ? "earliest gaps lost" : "too many commands");
        detail = furi_string_get_cstr(line);
    }

    widget_add_text_box_element(
        app->widget, 0, detail_y, 128, 52 - detail_y, AlignLeft, AlignTop, detail, false);

    furi_string_free(line);

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Again", t5577_app_scene_sniff_result_button, app);

    // Only where there is a list to open. Everything a capture with no frames would show is
    // already on this screen.
    if(result->frames > 0) {
        widget_add_button_element(
            app->widget, GuiButtonTypeRight, "Frames", t5577_app_scene_sniff_result_button, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);

    // Once per capture, not once per visit: backing out of the frame list re-enters this scene,
    // and a success chime on the way back reads as a second find.
    if(scene_manager_get_scene_state(app->scene_manager, T5577AppSceneSniffResult) == 0) {
        scene_manager_set_scene_state(app->scene_manager, T5577AppSceneSniffResult, 1);

        notification_message(
            app->notifications,
            verdict.finding >= SniffFindingInstalled ? &sequence_success :
                                                       &sequence_blink_blue_100);
    }
}

bool t5577_app_scene_sniff_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Past the listening scene, not into it: it starts a capture on entry, so the plain back key
    // would otherwise be indistinguishable from Again.
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == T5577AppCustomEventRetry) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }

    if(event.event == T5577AppCustomEventFrames) {
        // A fresh list starts at the top rather than wherever the last capture was left.
        scene_manager_set_scene_state(app->scene_manager, T5577AppSceneSniffFrames, 0);
        scene_manager_next_scene(app->scene_manager, T5577AppSceneSniffFrames);
        return true;
    }

    return false;
}

void t5577_app_scene_sniff_result_on_exit(void* context) {
    T5577App* app = context;
    widget_reset(app->widget);
}
