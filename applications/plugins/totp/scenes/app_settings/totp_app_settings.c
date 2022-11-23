#include "totp_app_settings.h"
#include "../../services/ui/ui_controls.h"
#include "../scene_director.h"
#include "../token_menu/totp_scene_token_menu.h"
#include "../../services/ui/constants.h"
#include "../../services/config/config.h"
#include "../../services/roll_value/roll_value.h"
#include "../../services/nullable/nullable.h"

#define DIGIT_TO_CHAR(digit) ((digit) + '0')

typedef enum { HoursInput, MinutesInput, ConfirmButton } Control;

typedef struct {
    int8_t tz_offset_hours;
    uint8_t tz_offset_minutes;
    TotpNullable_uint16_t current_token_index;
    Control selected_control;
} SceneState;

void totp_scene_app_settings_init(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}

void totp_scene_app_settings_activate(
    PluginState* plugin_state,
    const AppSettingsSceneContext* context) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    plugin_state->current_scene_state = scene_state;
    if(context != NULL) {
        TOTP_NULLABLE_VALUE(scene_state->current_token_index, context->current_token_index);
    } else {
        TOTP_NULLABLE_NULL(scene_state->current_token_index);
    }

    float off_int;
    float off_dec = modff(plugin_state->timezone_offset, &off_int);
    scene_state->tz_offset_hours = off_int;
    scene_state->tz_offset_minutes = 60.0f * off_dec;
}

static void two_digit_to_str(int8_t num, char* str) {
    uint8_t index = 0;
    if(num < 0) {
        str[0] = '-';
        index++;
        num = -num;
    }

    uint8_t d1 = (num / 10) % 10;
    uint8_t d2 = num % 10;
    str[index] = DIGIT_TO_CHAR(d1);
    str[index + 1] = DIGIT_TO_CHAR(d2);
    str[index + 2] = '\0';
}

void totp_scene_app_settings_render(Canvas* const canvas, PluginState* plugin_state) {
    const SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Timezone offset");
    canvas_set_font(canvas, FontSecondary);

    char tmp_str[4];
    two_digit_to_str(scene_state->tz_offset_hours, &tmp_str[0]);
    canvas_draw_str_aligned(canvas, 0, 16, AlignLeft, AlignTop, "Hours:");
    ui_control_select_render(
        canvas,
        36,
        10,
        SCREEN_WIDTH - 36,
        &tmp_str[0],
        scene_state->selected_control == HoursInput);

    two_digit_to_str(scene_state->tz_offset_minutes, &tmp_str[0]);
    canvas_draw_str_aligned(canvas, 0, 34, AlignLeft, AlignTop, "Minutes:");
    ui_control_select_render(
        canvas,
        36,
        28,
        SCREEN_WIDTH - 36,
        &tmp_str[0],
        scene_state->selected_control == MinutesInput);

    ui_control_button_render(
        canvas,
        SCREEN_WIDTH_CENTER - 24,
        50,
        48,
        13,
        "Confirm",
        scene_state->selected_control == ConfirmButton);
}

bool totp_scene_app_settings_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    if(event->input.type != InputTypePress && event->input.type != InputTypeRepeat) {
        return true;
    }

    switch(event->input.key) {
    case InputKeyUp:
        totp_roll_value_uint8_t(
            &scene_state->selected_control,
            -1,
            HoursInput,
            ConfirmButton,
            RollOverflowBehaviorStop);
        break;
    case InputKeyDown:
        totp_roll_value_uint8_t(
            &scene_state->selected_control, 1, HoursInput, ConfirmButton, RollOverflowBehaviorStop);
        break;
    case InputKeyRight:
        if(scene_state->selected_control == HoursInput) {
            totp_roll_value_int8_t(
                &scene_state->tz_offset_hours, 1, -12, 12, RollOverflowBehaviorStop);
        } else if(scene_state->selected_control == MinutesInput) {
            totp_roll_value_uint8_t(
                &scene_state->tz_offset_minutes, 15, 0, 45, RollOverflowBehaviorRoll);
        }
        break;
    case InputKeyLeft:
        if(scene_state->selected_control == HoursInput) {
            totp_roll_value_int8_t(
                &scene_state->tz_offset_hours, -1, -12, 12, RollOverflowBehaviorStop);
        } else if(scene_state->selected_control == MinutesInput) {
            totp_roll_value_uint8_t(
                &scene_state->tz_offset_minutes, -15, 0, 45, RollOverflowBehaviorRoll);
        }
        break;
    case InputKeyOk:
        if(scene_state->selected_control == ConfirmButton) {
            plugin_state->timezone_offset = (float)scene_state->tz_offset_hours +
                                            (float)scene_state->tz_offset_minutes / 60.0f;
            totp_config_file_update_timezone_offset(plugin_state->timezone_offset);

            if(!scene_state->current_token_index.is_null) {
                TokenMenuSceneContext generate_scene_context = {
                    .current_token_index = scene_state->current_token_index.value};
                totp_scene_director_activate_scene(
                    plugin_state, TotpSceneTokenMenu, &generate_scene_context);
            } else {
                totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, NULL);
            }
        }
        break;
    case InputKeyBack: {
        if(!scene_state->current_token_index.is_null) {
            TokenMenuSceneContext generate_scene_context = {
                .current_token_index = scene_state->current_token_index.value};
            totp_scene_director_activate_scene(
                plugin_state, TotpSceneTokenMenu, &generate_scene_context);
        } else {
            totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, NULL);
        }
        break;
    }
    default:
        break;
    }

    return true;
}

void totp_scene_app_settings_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_app_settings_free(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}