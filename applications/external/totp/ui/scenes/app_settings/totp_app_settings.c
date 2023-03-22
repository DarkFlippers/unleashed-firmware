#include "totp_app_settings.h"
#include <math.h>
#include <totp_icons.h>
#include "../../ui_controls.h"
#include "../../common_dialogs.h"
#include "../../scene_director.h"
#include "../token_menu/totp_scene_token_menu.h"
#include "../../constants.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../../lib/roll_value/roll_value.h"
#include "../../../types/nullable.h"
#include "../../../features_config.h"
#ifdef TOTP_BADBT_TYPE_ENABLED
#include "../../../workers/bt_type_code/bt_type_code.h"
#endif

char* YES_NO_LIST[] = {"NO", "YES"};
char* ON_OFF_LIST[] = {"OFF", "ON"};

typedef enum {
    HoursInput,
    MinutesInput,
    Sound,
    Vibro,
    BadUsb,
#ifdef TOTP_BADBT_TYPE_ENABLED
    BadBt,
#endif
    ConfirmButton
} Control;

typedef struct {
    int8_t tz_offset_hours;
    uint8_t tz_offset_minutes;
    bool notification_sound;
    bool notification_vibro;
    bool badusb_enabled;
#ifdef TOTP_BADBT_TYPE_ENABLED
    bool badbt_enabled;
#endif
    uint8_t y_offset;
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
    scene_state->notification_sound = plugin_state->notification_method & NotificationMethodSound;
    scene_state->notification_vibro = plugin_state->notification_method & NotificationMethodVibro;
    scene_state->badusb_enabled = plugin_state->automation_method & AutomationMethodBadUsb;
#ifdef TOTP_BADBT_TYPE_ENABLED
    scene_state->badbt_enabled = plugin_state->automation_method & AutomationMethodBadBt;
#endif
}

static void two_digit_to_str(int8_t num, char* str) {
    uint8_t index = 0;
    if(num < 0) {
        str[index++] = '-';
        num = -num;
    }

    uint8_t d1 = (num / 10) % 10;
    uint8_t d2 = num % 10;
    str[index++] = CONVERT_DIGIT_TO_CHAR(d1);
    str[index++] = CONVERT_DIGIT_TO_CHAR(d2);
    str[index++] = '\0';
}

void totp_scene_app_settings_render(Canvas* const canvas, const PluginState* plugin_state) {
    const SceneState* scene_state = plugin_state->current_scene_state;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 0, 0 - scene_state->y_offset, AlignLeft, AlignTop, "Timezone offset");
    canvas_set_font(canvas, FontSecondary);

    char tmp_str[4];
    two_digit_to_str(scene_state->tz_offset_hours, &tmp_str[0]);
    canvas_draw_str_aligned(canvas, 0, 17 - scene_state->y_offset, AlignLeft, AlignTop, "Hours:");
    ui_control_select_render(
        canvas,
        36,
        10 - scene_state->y_offset,
        SCREEN_WIDTH - 36,
        &tmp_str[0],
        scene_state->selected_control == HoursInput);

    two_digit_to_str(scene_state->tz_offset_minutes, &tmp_str[0]);
    canvas_draw_str_aligned(
        canvas, 0, 35 - scene_state->y_offset, AlignLeft, AlignTop, "Minutes:");
    ui_control_select_render(
        canvas,
        36,
        28 - scene_state->y_offset,
        SCREEN_WIDTH - 36,
        &tmp_str[0],
        scene_state->selected_control == MinutesInput);

    canvas_draw_icon(
        canvas,
        SCREEN_WIDTH_CENTER - 5,
        SCREEN_HEIGHT - 5 - scene_state->y_offset,
        &I_totp_arrow_bottom_10x5);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 0, 64 - scene_state->y_offset, AlignLeft, AlignTop, "Notifications");
    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str_aligned(canvas, 0, 81 - scene_state->y_offset, AlignLeft, AlignTop, "Sound:");
    ui_control_select_render(
        canvas,
        36,
        74 - scene_state->y_offset,
        SCREEN_WIDTH - 36,
        YES_NO_LIST[scene_state->notification_sound],
        scene_state->selected_control == Sound);

    canvas_draw_str_aligned(canvas, 0, 99 - scene_state->y_offset, AlignLeft, AlignTop, "Vibro:");
    ui_control_select_render(
        canvas,
        36,
        92 - scene_state->y_offset,
        SCREEN_WIDTH - 36,
        YES_NO_LIST[scene_state->notification_vibro],
        scene_state->selected_control == Vibro);

    canvas_draw_icon(
        canvas, SCREEN_WIDTH_CENTER - 5, 123 - scene_state->y_offset, &I_totp_arrow_bottom_10x5);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 0, 128 - scene_state->y_offset, AlignLeft, AlignTop, "Automation");
    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str_aligned(
        canvas, 0, 145 - scene_state->y_offset, AlignLeft, AlignTop, "BadUSB:");
    ui_control_select_render(
        canvas,
        36,
        138 - scene_state->y_offset,
        SCREEN_WIDTH - 36,
        ON_OFF_LIST[scene_state->badusb_enabled],
        scene_state->selected_control == BadUsb);

#ifdef TOTP_BADBT_TYPE_ENABLED
    canvas_draw_str_aligned(canvas, 0, 163 - scene_state->y_offset, AlignLeft, AlignTop, "BadBT:");
    ui_control_select_render(
        canvas,
        36,
        156 - scene_state->y_offset,
        SCREEN_WIDTH - 36,
        ON_OFF_LIST[scene_state->badbt_enabled],
        scene_state->selected_control == BadBt);
#endif

    ui_control_button_render(
        canvas,
        SCREEN_WIDTH_CENTER - 24,
#ifdef TOTP_BADBT_TYPE_ENABLED
        178 - scene_state->y_offset,
#else
        165 - scene_state->y_offset,
#endif
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
        if(scene_state->selected_control > Vibro) {
            scene_state->y_offset = 128;
        } else if(scene_state->selected_control > MinutesInput) {
            scene_state->y_offset = 64;
        } else {
            scene_state->y_offset = 0;
        }
        break;
    case InputKeyDown:
        totp_roll_value_uint8_t(
            &scene_state->selected_control, 1, HoursInput, ConfirmButton, RollOverflowBehaviorStop);
        if(scene_state->selected_control > Vibro) {
            scene_state->y_offset = 128;
        } else if(scene_state->selected_control > MinutesInput) {
            scene_state->y_offset = 64;
        } else {
            scene_state->y_offset = 0;
        }
        break;
    case InputKeyRight:
        if(scene_state->selected_control == HoursInput) {
            totp_roll_value_int8_t(
                &scene_state->tz_offset_hours, 1, -12, 12, RollOverflowBehaviorStop);
        } else if(scene_state->selected_control == MinutesInput) {
            totp_roll_value_uint8_t(
                &scene_state->tz_offset_minutes, 15, 0, 45, RollOverflowBehaviorRoll);
        } else if(scene_state->selected_control == Sound) {
            scene_state->notification_sound = !scene_state->notification_sound;
        } else if(scene_state->selected_control == Vibro) {
            scene_state->notification_vibro = !scene_state->notification_vibro;
        } else if(scene_state->selected_control == BadUsb) {
            scene_state->badusb_enabled = !scene_state->badusb_enabled;
        }
#ifdef TOTP_BADBT_TYPE_ENABLED
        else if(scene_state->selected_control == BadBt) {
            scene_state->badbt_enabled = !scene_state->badbt_enabled;
        }
#endif
        break;
    case InputKeyLeft:
        if(scene_state->selected_control == HoursInput) {
            totp_roll_value_int8_t(
                &scene_state->tz_offset_hours, -1, -12, 12, RollOverflowBehaviorStop);
        } else if(scene_state->selected_control == MinutesInput) {
            totp_roll_value_uint8_t(
                &scene_state->tz_offset_minutes, -15, 0, 45, RollOverflowBehaviorRoll);
        } else if(scene_state->selected_control == Sound) {
            scene_state->notification_sound = !scene_state->notification_sound;
        } else if(scene_state->selected_control == Vibro) {
            scene_state->notification_vibro = !scene_state->notification_vibro;
        } else if(scene_state->selected_control == BadUsb) {
            scene_state->badusb_enabled = !scene_state->badusb_enabled;
        }
#ifdef TOTP_BADBT_TYPE_ENABLED
        else if(scene_state->selected_control == BadBt) {
            scene_state->badbt_enabled = !scene_state->badbt_enabled;
        }
#endif
        break;
    case InputKeyOk:
        if(scene_state->selected_control == ConfirmButton) {
            plugin_state->timezone_offset = (float)scene_state->tz_offset_hours +
                                            (float)scene_state->tz_offset_minutes / 60.0f;

            plugin_state->notification_method =
                (scene_state->notification_sound ? NotificationMethodSound :
                                                   NotificationMethodNone) |
                (scene_state->notification_vibro ? NotificationMethodVibro :
                                                   NotificationMethodNone);

            plugin_state->automation_method =
                scene_state->badusb_enabled ? AutomationMethodBadUsb : AutomationMethodNone;
#ifdef TOTP_BADBT_TYPE_ENABLED
            plugin_state->automation_method |= scene_state->badbt_enabled ? AutomationMethodBadBt :
                                                                            AutomationMethodNone;
#endif

            if(totp_config_file_update_user_settings(plugin_state) !=
               TotpConfigFileUpdateSuccess) {
                totp_dialogs_config_updating_error(plugin_state);
                return false;
            }

#ifdef TOTP_BADBT_TYPE_ENABLED
            if(!scene_state->badbt_enabled && plugin_state->bt_type_code_worker_context != NULL) {
                totp_bt_type_code_worker_free(plugin_state->bt_type_code_worker_context);
                plugin_state->bt_type_code_worker_context = NULL;
            }
#endif

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