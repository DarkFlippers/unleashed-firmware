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
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventRetry);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventPassword);
    }
}

// A read of a protected tag stops at the first block it is not allowed, so a dump of one or
// two blocks is the normal outcome rather than a broken one - but it is not the whole tag
// either, and saying so is what tells the user there is a password left to find.
static void t5577_app_scene_read_result_found(T5577App* app, bool whole) {
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
        (app->config.block0 & LFRFID_T5577_PWD) ? ", password set" : "",
        app->config.inverted ? ", inverted" : "");
    widget_add_string_element(
        app->widget, 0, 28, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(line));

    const uint8_t read = t5577_app_blocks_read(&app->dump);
    furi_string_printf(line, "%u of %u blocks", read, T5577_APP_BLOCKS);
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
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Only if it is protected", 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(
        message,
        "A password read writes\nblock 0 of a tag that\nhas no password set",
        64,
        20,
        AlignCenter,
        AlignTop);
    dialog_message_set_buttons(message, "Cancel", NULL, "Go on");
    const DialogMessageButton pressed = dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    return pressed == DialogMessageButtonRight;
}

static void t5577_app_scene_read_result_nothing(T5577App* app) {
    widget_add_string_element(
        app->widget, 64, 4, AlignCenter, AlignTop, FontPrimary, "Nothing found");

    // The two things that account for a silent tag, in the order worth trying.
    widget_add_string_multiline_element(
        app->widget,
        64,
        20,
        AlignCenter,
        AlignTop,
        FontSecondary,
        app->password_entered ? "That password did\nnot open the tag" :
                                "Hold the tag still,\nor it needs a password");

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Retry", t5577_app_scene_read_result_nothing_button, app);
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
        const bool whole = t5577_app_blocks_read(&app->dump) == T5577_APP_BLOCKS;
        t5577_app_scene_read_result_found(app, whole);
        if(first) {
            // The chime is for a tag read end to end. A partial read is still a read, and
            // still counts to the dolphin, but it is not the sound of a job finished.
            notification_message(
                app->notifications, whole ? &sequence_success : &sequence_blink_blue_100);
            dolphin_deed(DolphinDeedRfidReadSuccess);
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
