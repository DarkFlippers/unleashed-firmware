#include "../example_date_time_input.h"

static void
    example_date_time_input_scene_confirm_dialog_callback(DialogExResult result, void* context) {
    ExampleDateTimeInput* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

static void example_date_time_input_scene_update_view(void* context) {
    ExampleDateTimeInput* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_header(dialog_ex, "The date and time are", 64, 0, AlignCenter, AlignTop);

    uint8_t hour = app->date_time.hour;
    char label_hour[4] = "";
    if(furi_hal_rtc_get_locale_timeformat() == FuriHalRtcLocaleTimeFormat12h) {
        if(hour < 12) {
            snprintf(label_hour, sizeof(label_hour), " AM");
        } else {
            snprintf(label_hour, sizeof(label_hour), " PM");
        }
        hour %= 12;
        if(hour == 0) hour = 12;
    }

    char buffer[29] = {};
    snprintf(
        buffer,
        sizeof(buffer),
        "%04d-%02d-%02d\n%02d:%02d:%02d%s",
        app->date_time.year,
        app->date_time.month,
        app->date_time.day,
        hour,
        app->date_time.minute,
        app->date_time.second,
        label_hour);
    dialog_ex_set_text(dialog_ex, buffer, 64, 29, AlignCenter, AlignCenter);

    dialog_ex_set_left_button_text(dialog_ex, "Date");
    dialog_ex_set_right_button_text(dialog_ex, "Time");
    dialog_ex_set_center_button_text(dialog_ex, "Both");

    dialog_ex_set_result_callback(
        dialog_ex, example_date_time_input_scene_confirm_dialog_callback);
    dialog_ex_set_context(dialog_ex, app);
}

void example_date_time_input_scene_show_date_time_on_enter(void* context) {
    furi_assert(context);
    ExampleDateTimeInput* app = context;

    example_date_time_input_scene_update_view(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleDateTimeInputViewIdShowDateTime);
}

bool example_date_time_input_scene_show_date_time_on_event(void* context, SceneManagerEvent event) {
    ExampleDateTimeInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultCenter:
            app->edit_date = true;
            app->edit_time = true;
            scene_manager_next_scene(app->scene_manager, ExampleDateTimeInputSceneInputDateTime);
            consumed = true;
            break;
        case DialogExResultLeft:
            app->edit_date = true;
            app->edit_time = false;
            scene_manager_next_scene(app->scene_manager, ExampleDateTimeInputSceneInputDateTime);
            consumed = true;
            break;
        case DialogExResultRight:
            app->edit_date = false;
            app->edit_time = true;
            scene_manager_next_scene(app->scene_manager, ExampleDateTimeInputSceneInputDateTime);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

void example_date_time_input_scene_show_date_time_on_exit(void* context) {
    UNUSED(context);
}
