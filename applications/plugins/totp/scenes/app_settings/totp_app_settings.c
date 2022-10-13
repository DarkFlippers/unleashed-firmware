#include "totp_app_settings.h"
#include "../../services/ui/ui_controls.h"
#include "../scene_director.h"
#include "../token_menu/totp_scene_token_menu.h"
#include "../../services/ui/constants.h"
#include "../../services/config/config.h"

#define DIGIT_TO_CHAR(digit) ((digit) + '0')

typedef enum { HoursInput, MinutesInput, ConfirmButton } Control;

typedef struct {
    int8_t tz_offset_hours;
    uint8_t tz_offset_minutes;
    int16_t current_token_index;
    Control selected_control;
} SceneState;

void totp_scene_app_settings_init(PluginState* plugin_state) {
    UNUSED(plugin_state);
}

void totp_scene_app_settings_activate(
    PluginState* plugin_state,
    const AppSettingsSceneContext* context) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    plugin_state->current_scene_state = scene_state;
    if(context != NULL) {
        scene_state->current_token_index = context->current_token_index;
    } else {
        scene_state->current_token_index = -1;
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
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

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

bool totp_scene_app_settings_handle_event(PluginEvent* const event, PluginState* plugin_state) {
    if(event->type == EventTypeKey) {
        SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
        if(event->input.type == InputTypePress) {
            switch(event->input.key) {
            case InputKeyUp:
                if(scene_state->selected_control > HoursInput) {
                    scene_state->selected_control--;
                }
                break;
            case InputKeyDown:
                if(scene_state->selected_control < ConfirmButton) {
                    scene_state->selected_control++;
                }
                break;
            case InputKeyRight:
                if(scene_state->selected_control == HoursInput) {
                    if(scene_state->tz_offset_hours < 12) {
                        scene_state->tz_offset_hours++;
                    }
                } else if(scene_state->selected_control == MinutesInput) {
                    if(scene_state->tz_offset_minutes < 45) {
                        scene_state->tz_offset_minutes += 15;
                    } else {
                        scene_state->tz_offset_minutes = 0;
                    }
                }
                break;
            case InputKeyLeft:
                if(scene_state->selected_control == HoursInput) {
                    if(scene_state->tz_offset_hours > -12) {
                        scene_state->tz_offset_hours--;
                    }
                } else if(scene_state->selected_control == MinutesInput) {
                    if(scene_state->tz_offset_minutes >= 15) {
                        scene_state->tz_offset_minutes -= 15;
                    } else {
                        scene_state->tz_offset_minutes = 45;
                    }
                }
                break;
            case InputKeyOk:
                if(scene_state->selected_control == ConfirmButton) {
                    plugin_state->timezone_offset = (float)scene_state->tz_offset_hours +
                                                    (float)scene_state->tz_offset_minutes / 60.0f;
                    totp_config_file_update_timezone_offset(plugin_state->timezone_offset);

                    if(scene_state->current_token_index >= 0) {
                        TokenMenuSceneContext generate_scene_context = {
                            .current_token_index = scene_state->current_token_index};
                        totp_scene_director_activate_scene(
                            plugin_state, TotpSceneTokenMenu, &generate_scene_context);
                    } else {
                        totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, NULL);
                    }
                }
                break;
            case InputKeyBack: {
                if(scene_state->current_token_index >= 0) {
                    TokenMenuSceneContext generate_scene_context = {
                        .current_token_index = scene_state->current_token_index};
                    totp_scene_director_activate_scene(
                        plugin_state, TotpSceneTokenMenu, &generate_scene_context);
                } else {
                    totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, NULL);
                }
                break;
            }
            }
        }
    }
    return true;
}

void totp_scene_app_settings_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_app_settings_free(PluginState* plugin_state) {
    UNUSED(plugin_state);
}