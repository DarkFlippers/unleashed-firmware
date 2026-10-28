#include "../t5577_app.h"

static const char* t5577_app_scene_sniff_frame_kind(const T5577SniffFrame* frame) {
    switch(frame->kind) {
    case T5577SniffFrameReset:
        return "Reset";
    case T5577SniffFrameRegularRead:
        return "Back to read mode";
    case T5577SniffFramePlainRead:
        return "Read, no password";
    case T5577SniffFramePlainWrite:
        return "Write, no password";
    case T5577SniffFramePasswordedRead:
        return "Read with password";
    case T5577SniffFramePasswordedWrite:
        return "Write with password";
    case T5577SniffFrameAmbiguous:
        return "Read or write";
    case T5577SniffFrameUnknown:
        break;
    }

    return "Not decoded";
}

// Why the frame reads the way it does. The point of saying it is that two of these are
// deductions from the frames AROUND this one, not from this one - so the reasoning is on screen
// where it can be disbelieved.
static const char* t5577_app_scene_sniff_frame_why(const T5577SniffFrame* frame) {
    switch(frame->why) {
    case T5577SniffWhyLength:
        return "Only this length fits";
    case T5577SniffWhyMarker:
        // Not "only a write is followed by one" - that was the premise this app shipped on and
        // it is false, since a read ends with the same command. The wait is what separates them.
        return "A read-mode command\nfollowed it, after a wait\nonly a write explains";
    case T5577SniffWhyPause:
        return "The carrier came back\ntoo soon for a write to\nhave programmed";
    case T5577SniffWhyNone:
        break;
    }

    return NULL;
}

void t5577_app_scene_sniff_frame_on_enter(void* context) {
    T5577App* app = context;

    const T5577SniffFrame* frame = &app->sniff_result.frame[app->sniff_pick];
    FuriString* text = furi_string_alloc();

    furi_string_printf(
        text,
        "%s\n%u%s symbols%s\n",
        t5577_app_scene_sniff_frame_kind(frame),
        frame->symbols,
        frame->symbols >= T5577_SNIFF_SYMBOLS_MAX ? "+" : "",
        frame->leading_zero ? ", leading zero" : "");

    // The threshold the symbols were classed against. Shown because it is no longer a constant:
    // the capture's own two class means are used where they separate, which is the only way a
    // leading-zero sender decodes at all - both its classes sit below the fixed fallback.
    furi_string_cat_printf(text, "Split %u us\n", app->sniff_result.split_us);

    // A reset carries no page bit, so the two zeroes it is made of are not one.
    if(frame->kind != T5577SniffFrameUnknown && frame->kind != T5577SniffFrameReset) {
        furi_string_cat_printf(text, "Page %u\n", frame->page ? 1 : 0);
    }

    if(t5577_sniff_frame_addressed(frame)) {
        // Block only for an ambiguous frame: the two readings agree on the address but put
        // their lock bits in different places, so a single flat "lock" there would be true of
        // one reading and silently false of the other.
        if(frame->kind == T5577SniffFrameAmbiguous) {
            furi_string_cat_printf(text, "Block %u\n", frame->block);
        } else {
            furi_string_cat_printf(text, "Block %u, lock %u\n", frame->block, frame->lock ? 1 : 0);
        }
    }

    if(frame->kind == T5577SniffFrameAmbiguous) {
        // Both words, labelled with the reading each belongs to and neither claimed. A plain
        // write and a passworded read are the same 38 bits with the fields in different places,
        // so exactly one of these is real and nothing in the capture said which.
        furi_string_cat_printf(
            text,
            "\nIf a read: password\n%08lX, lock %u\nIf a write: %08lX\ninto the block, lock %u\n",
            (unsigned long)frame->password,
            frame->lock_alt ? 1 : 0,
            (unsigned long)frame->data,
            frame->lock ? 1 : 0);
    } else {
        if(t5577_sniff_frame_tells_password(frame)) {
            furi_string_cat_printf(text, "Password %08lX\n", (unsigned long)frame->password);
        }
        if(t5577_sniff_frame_has_data(frame)) {
            furi_string_cat_printf(text, "Data %08lX\n", (unsigned long)frame->data);
        }
    }

    const char* why = t5577_app_scene_sniff_frame_why(frame);

    if(why != NULL) furi_string_cat_printf(text, "\n%s\n", why);

    // Field-on time before whatever came next, which is the evidence the resolution above rests
    // on. Zero where nothing followed, and said as such rather than as a measurement of nothing.
    if(frame->pause_capped) {
        // A gap period is held in a uint16_t, so this is where the measurement ran out - not a
        // reading. Said plainly, because a reader waiting on a tag leaves hundreds of
        // milliseconds here and printing the ceiling as a figure is what once let that pass for
        // a write's programming time.
        furi_string_cat_str(text, "\nCarrier up past 65 ms\nafterwards\n");
    } else if(frame->pause_us > 0) {
        furi_string_cat_printf(
            text,
            "\nCarrier up %lu.%lu ms\nafterwards\n",
            (unsigned long)(frame->pause_us / 1000),
            (unsigned long)((frame->pause_us % 1000) / 100));
    } else {
        // Which is this capture ending, not the sender falling silent.
        furi_string_cat_str(text, "\nThe capture ended here\n");
    }

    widget_reset(app->widget);
    widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, furi_string_get_cstr(text));

    furi_string_free(text);

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);
}

bool t5577_app_scene_sniff_frame_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void t5577_app_scene_sniff_frame_on_exit(void* context) {
    T5577App* app = context;
    widget_reset(app->widget);
}
