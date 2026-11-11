#include "../t5577_app.h"

static void
    t5577_app_scene_clear_result_button(GuiButtonType type, InputType input, void* context) {
    T5577App* app = context;

    UNUSED(type); // one button on this screen, and the widget calls back only for its own

    if(input != InputTypeShort) return;

    // One event for both labels: what the button does is on_event's to decide, from the same
    // field on_enter labelled it from.
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
}

// Whether this screen has a word worth offering to keep. `found` is Nothing unless the sweep
// opened the tag, so this is also the "it worked" test. Two readers that must not disagree: the
// label on the button, and what pressing it does.
static bool t5577_app_scene_clear_result_keepable(const T5577App* app) {
    return app->clear.found == T5577ClearFoundTyped;
}

// What there is to say where the sweep cannot name a word - because it opened the tag on an
// attempt that names none, or did not open it at all. The same two diagnoses either way: a tag
// that never answered and a tag whose password is in no list end a sweep identically and cannot
// be told apart from here, but a list that was never there and a list that stopped part way
// both can be, and are worth as much on this screen as on the failure one - the word may have
// been in the part that was never read.
static const char* t5577_app_scene_clear_result_why(const T5577App* app) {
    if(app->dict_failed) {
        return app->cleared ? "Password unknown: the\nlist stopped part way" :
                              "Password list stopped:\ncheck the SD card";
    }

    if(app->candidates == 0) {
        return app->cleared ? "Opened, but with no\npassword list to try" :
                              "No password list:\ncheck the SD card";
    }

    return app->cleared ? "Opened, but by none of\nthe words it tried" :
                          "No password matched,\nor no tag answered";
}

void t5577_app_scene_clear_result_on_enter(void* context) {
    T5577App* app = context;

    // A widget rather than a popup: a popup takes a press but cannot label one, and an offer to
    // keep the password has to be visible before it is pressed. The dolphin went with it - this
    // is Sniff Password's shape, the app's other screen that reports a recovered word, and for
    // the same reason: a 91 pixel icon leaves a column too narrow for a word, a sentence and a
    // button, and "Cleared!" already says what the icon would.
    widget_reset(app->widget);

    char word[9] = ""; // the password, where the sweep can name one
    const char* detail = "";

    if(app->cleared) {
        // What opened the tag, which is the finding the sweep otherwise throws away - this
        // screen used to show every one of these cases as "Cleared!" and nothing else.
        //
        // A switch with no default, because getting it wrong here prints a wrong password: a
        // value added to the enum is then a build failure.
        switch(app->clear.found) {
        case T5577ClearFoundWord:
        case T5577ClearFoundTyped:
            snprintf(word, sizeof(word), "%08lX", (unsigned long)app->clear.password);
            // What opened it, not what the tag was asking for - see T5577_APP_CLEAR_OPEN_TRIES
            // for the unprotected tag that produces the same evidence.
            detail = "Opened by this password";
            break;
        case T5577ClearFoundOpen:
            detail = "The tag was asking for\nno password";
            break;
        case T5577ClearFoundNothing:
            // Said rather than left blank, on a screen that otherwise reads as the named case.
            detail = t5577_app_scene_clear_result_why(app);
            break;
        }
    } else {
        detail = t5577_app_scene_clear_result_why(app);
    }

    widget_add_string_element(
        app->widget,
        0,
        2,
        AlignLeft,
        AlignTop,
        FontPrimary,
        app->cleared ? "Cleared!" : "Not cleared");

    uint8_t detail_y = 18;

    if(word[0] != '\0') {
        widget_add_string_element(app->widget, 0, 16, AlignLeft, AlignTop, FontPrimary, word);
        detail_y = 30;
    }

    widget_add_text_box_element(
        app->widget, 0, detail_y, 128, 52 - detail_y, AlignLeft, AlignTop, detail, false);

    // Always a button, because Back out of a result screen is a thing to know rather than see.
    // It offers the word only where keeping it can answer something: a word out of a list is
    // already in a list, so that Save could only say so.
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,
        t5577_app_scene_clear_result_keepable(app) ? "Save" : "OK",
        t5577_app_scene_clear_result_button,
        app);

    notification_message(app->notifications, app->cleared ? &sequence_success : &sequence_error);

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);
}

bool t5577_app_scene_clear_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back goes past the sweep rather than into it: that scene writes the tag on entry.
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventConfirm) return false;

    // The same test that put "Save" on the button rather than "OK". Dedup and the telling are
    // that call's: the word may be one the firmware ships or one the user kept earlier, and both
    // are worth being told rather than written again.
    //
    // A card that refused the write is the one answer that leaves something to do here, so the
    // screen stays up with the word still on it and the button still pressable.
    if(t5577_app_scene_clear_result_keepable(app) &&
       !t5577_app_dict_save(app, app->clear.password)) {
        return true;
    }

    // Out to the menu otherwise: the word, where there was one worth keeping, is in a list now.
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);

    return true;
}

void t5577_app_scene_clear_result_on_exit(void* context) {
    T5577App* app = context;
    widget_reset(app->widget);
}
