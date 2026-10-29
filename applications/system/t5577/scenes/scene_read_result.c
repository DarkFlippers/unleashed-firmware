#include "../t5577_app.h"
#include <t5577_icons.h>
#include <dolphin/dolphin.h>

// Two layouts, two callbacks: the same button means different things on a screen offering a save
// and on one offering a password, and deciding which from the app state afterwards is how a
// button ends up doing the other thing.
static void
    t5577_app_scene_read_result_found_button(GuiButtonType type, InputType input, void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeCenter) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventSave);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventInfo);
    } else if(type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventRetry);
    }
}

static void
    t5577_app_scene_read_result_nothing_button(GuiButtonType type, InputType input, void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventRetryPlain);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventPassword);
    }
}

// A dump short of the whole tag is a tag that stopped answering part way through, not a
// protected one: protection refuses the block 0 read the detect itself needs, so a protected tag
// reads as nothing at all. Naming the block it went quiet at is what separates the two.
static void t5577_app_scene_read_result_found(T5577App* app, uint8_t read) {
    const bool whole = read == T5577_BLOCKS;
    FuriString* line = furi_string_alloc();

    widget_add_string_element(
        app->widget, 0, 2, AlignLeft, AlignTop, FontPrimary, whole ? "T5577 read" : "Read in part");

    furi_string_printf(line, "Block 0: %08lX", app->config.block0);
    widget_add_string_element(
        app->widget, 0, 16, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(line));

    furi_string_printf(
        line,
        "RF/%u%s%s",
        app->config.clock,
        t5577_config_needs_password(app->config.block0) ? ", password set" : "",
        app->config.inverted ? ", inverted" : "");
    widget_add_string_element(
        app->widget, 0, 28, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(line));

    if(whole) {
        furi_string_printf(line, "%u of %u blocks", read, T5577_BLOCKS);
    } else {
        // Which block stopped the dump is the only thing on this screen that says where to
        // look: a tag held badly gives up part way, where one with a password gives nothing.
        furi_string_printf(
            line,
            "%u of %u, quiet at P%ub%u",
            read,
            T5577_BLOCKS,
            app->lost_at.page,
            app->lost_at.block);
    }
    widget_add_string_element(
        app->widget, 0, 40, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(line));

    widget_add_button_element(
        app->widget, GuiButtonTypeCenter, "Save", t5577_app_scene_read_result_found_button, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Info", t5577_app_scene_read_result_found_button, app);
    // Worth offering only where there is something left to gain by it.
    if(!whole) {
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Retry", t5577_app_scene_read_result_found_button, app);
    }

    furi_string_free(line);
}

// The frame a passworded read sends is the same length as a write, and the chip tells them
// apart by a bit the reader cannot see: sent to a tag that has no password, it writes the
// password into block 0 and the tag stops reading as anything. A tag that was merely held
// badly looks exactly like a tag that wants a password, so the choice is the user's.
static bool t5577_app_scene_read_result_warn_password(T5577App* app) {
    return t5577_app_ask(
        app,
        "Only if it is protected",
        "A password read writes\nblock 0 of a tag that\nhas no password set",
        "Go on");
}

static void t5577_app_scene_read_result_nothing(T5577App* app) {
    widget_add_string_element(
        app->widget, 64, 4, AlignCenter, AlignTop, FontPrimary, "Nothing found");

    // The two things that account for a silent tag, in the order worth trying. After a
    // password has been sent there is a third, and it is the one the user cannot see: the tag
    // may have had no password, in which case that frame wrote block 0 and the silence is the
    // tag's configuration gone. Saying "that password did not open it" would name the one cause
    // that is safe to retry and hide the one that is not.
    widget_add_string_multiline_element(
        app->widget,
        64,
        20,
        AlignCenter,
        AlignTop,
        FontSecondary,
        app->password_sent ?
            "Wrong password - or it\nnever had one, and that\nframe wrote block 0" :
            "Hold the tag still,\nor it needs a password");

    // Reading again without the password is the only move left that cannot make it worse.
    widget_add_button_element(
        app->widget,
        GuiButtonTypeLeft,
        app->password_entered ? "No pass" : "Retry",
        t5577_app_scene_read_result_nothing_button,
        app);
    widget_add_button_element(
        app->widget,
        GuiButtonTypeRight,
        "Password",
        t5577_app_scene_read_result_nothing_button,
        app);
}

void t5577_app_scene_read_result_on_enter(void* context) {
    T5577App* app = context;

    // Once, not every time the screen is come back to from Info.
    const bool first =
        scene_manager_get_scene_state(app->scene_manager, T5577AppSceneReadResult) == 0;
    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneReadResult, 1);

    widget_reset(app->widget);

    if(!t5577_config_found(&app->config)) {
        t5577_app_scene_read_result_nothing(app);
    } else {
        const uint8_t read = t5577_dump_blocks_read(&app->dump);
        const bool whole = read == T5577_BLOCKS;
        t5577_app_scene_read_result_found(app, read);
        if(first) {
            // The chime is for a tag read end to end. A partial read is still a read, and
            // still counts to the dolphin, but it is not the sound of a job finished.
            notification_message(
                app->notifications, whole ? &sequence_success : &sequence_blink_blue_100);
            dolphin_deed(DolphinDeedRfidReadSuccess);

            // The detect drops a password the tag turned out not to ask for, which makes this
            // the one place that knows, from the tag's own answer, that the warning before the
            // password entry has come true. The read worked, so nothing else on this screen
            // would ever mention it.
            //
            // On password_sent and not password_entered: a plain read is tried first and also
            // leaves with_pass clear, so the typed word alone would accuse the reads that never
            // sent the frame - the ones this is here to encourage.
            if(app->password_sent && !app->config.with_pass) {
                t5577_app_say(
                    app,
                    "Tag wants no password",
                    "That frame wrote block 0.\nCheck it below, and wipe\nthe tag if it reads wrong");
            }
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);
}

bool t5577_app_scene_read_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Same as after a write: Back goes past the read rather than into it.
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case T5577AppCustomEventRetryPlain:
        t5577_app_forget_password(app);
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneRead);
        return true;
    case T5577AppCustomEventRetry:
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneRead);
        return true;
    case T5577AppCustomEventPassword:
        if(t5577_app_scene_read_result_warn_password(app)) {
            scene_manager_next_scene(app->scene_manager, T5577AppScenePassword);
        }
        return true;
    case T5577AppCustomEventSave:
        app->renaming = false;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneSaveName);
        return true;
    case T5577AppCustomEventInfo:
        scene_manager_next_scene(app->scene_manager, T5577AppSceneInfo);
        return true;
    default:
        return false;
    }
}

void t5577_app_scene_read_result_on_exit(void* context) {
    T5577App* app = context;
    widget_reset(app->widget);
}
