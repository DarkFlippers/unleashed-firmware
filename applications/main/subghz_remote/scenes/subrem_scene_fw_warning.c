#include "../subghz_remote_app_i.h"
#include "../helpers/subrem_custom_event.h"
#ifdef FW_ORIGIN_Official
typedef enum {
    SceneFwWarningStateAttention,
    SceneFwWarningStateAccept,
} SceneFwWarningState;

static void subrem_scene_fw_warning_widget_render(SubGhzRemoteApp* app, SceneFwWarningState state);

static void
    subrem_scene_fw_warning_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    if(type == InputTypeShort) {
        SubRemCustomEvent event = SubRemCustomEventSceneFwWarningExit;

        switch(scene_manager_get_scene_state(app->scene_manager, SubRemSceneFwWarning)) {
        case SceneFwWarningStateAttention:
            if(result == GuiButtonTypeRight) {
                event = SubRemCustomEventSceneFwWarningNext;
            }
            break;

        case SceneFwWarningStateAccept:
            if(result == GuiButtonTypeRight) {
                event = SubRemCustomEventSceneFwWarningContinue;
            }

            break;
        }

        view_dispatcher_send_custom_event(app->view_dispatcher, event);
    }
}

static void
    subrem_scene_fw_warning_widget_render(SubGhzRemoteApp* app, SceneFwWarningState state) {
    furi_assert(app);
    Widget* widget = app->widget;

    widget_reset(widget);

    switch(state) {
    case SceneFwWarningStateAttention:
        widget_add_button_element(
            widget, GuiButtonTypeLeft, "Exit", subrem_scene_fw_warning_widget_callback, app);
        widget_add_button_element(
            widget, GuiButtonTypeRight, "Continue", subrem_scene_fw_warning_widget_callback, app);
        widget_add_string_element(
            widget, 64, 12, AlignCenter, AlignBottom, FontPrimary, "Not official FW");
        widget_add_string_multiline_element(
            widget,
            64,
            32,
            AlignCenter,
            AlignCenter,
            FontSecondary,
            "You are using custom firmware\nPlease download a compatible\nversion of the application");
        break;

    case SceneFwWarningStateAccept:
        widget_add_button_element(
            widget, GuiButtonTypeLeft, "Exit", subrem_scene_fw_warning_widget_callback, app);
        widget_add_button_element(
            widget, GuiButtonTypeRight, "Accept", subrem_scene_fw_warning_widget_callback, app);
        widget_add_string_element(
            widget, 64, 12, AlignCenter, AlignBottom, FontPrimary, "Not official FW");
        widget_add_string_multiline_element(
            widget,
            64,
            32,
            AlignCenter,
            AlignCenter,
            FontSecondary,
            "Yes, I understand that\nthe application can\nbreak my subghz key file");
        break;
    }
}

void subrem_scene_fw_warning_on_enter(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;

    scene_manager_set_scene_state(
        app->scene_manager, SubRemSceneFwWarning, SceneFwWarningStateAttention);

    subrem_scene_fw_warning_widget_render(app, SceneFwWarningStateAttention);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDWidget);
}

bool subrem_scene_fw_warning_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubRemCustomEventSceneFwWarningExit) {
            scene_manager_stop(app->scene_manager);
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
        } else if(event.event == SubRemCustomEventSceneFwWarningNext) {
            scene_manager_set_scene_state(
                app->scene_manager, SubRemSceneFwWarning, SceneFwWarningStateAccept);
            subrem_scene_fw_warning_widget_render(app, SceneFwWarningStateAccept);
            view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDWidget);
            consumed = true;
        } else if(event.event == SubRemCustomEventSceneFwWarningContinue) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    }

    return consumed;
}

void subrem_scene_fw_warning_on_exit(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    widget_reset(app->widget);
}
#endif