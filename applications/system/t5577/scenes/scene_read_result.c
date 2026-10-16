#include "../t5577_app.h"
#include <t5577_icons.h>

// Two layouts, two callbacks: the same button means different things on a screen offering a save
// and on one offering a password, and deciding which from the app state afterwards is how a
// button ends up doing the other thing.
static void t5577_app_scene_read_result_found_button(
    GuiButtonType type,
    InputType input,
    void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeCenter) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventSave);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventInfo);
    }
}

static void t5577_app_scene_read_result_nothing_button(
    GuiButtonType type,
    InputType input,
    void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventRetry);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventPassword);
    }
}

static void t5577_app_scene_read_result_found(T5577App* app) {
    FuriString* line = furi_string_alloc();

    widget_add_string_element(
        app->widget, 0, 2, AlignLeft, AlignTop, FontPrimary, "T5577 read");

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
        app->widget,
        GuiButtonTypeCenter,
        "Save",
        t5577_app_scene_read_result_found_button,
        app);
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Info", t5577_app_scene_read_result_found_button, app);

    furi_string_free(line);
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
        app->widget,
        GuiButtonTypeLeft,
        "Retry",
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

    widget_reset(app->widget);

    if(app->config.clock == T5577ReadClockUnsupported) {
        t5577_app_scene_read_result_nothing(app);
    } else {
        t5577_app_scene_read_result_found(app);
        notification_message(app->notifications, &sequence_success);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);
}

bool t5577_app_scene_read_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case T5577AppCustomEventRetry:
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneRead);
        return true;
    case T5577AppCustomEventPassword:
        scene_manager_next_scene(app->scene_manager, T5577AppScenePassword);
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
