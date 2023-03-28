#include <gui/gui.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <totp_icons.h>
#include "totp_scene_generate_token.h"
#include "../../../types/token_info.h"
#include "../../../types/common.h"
#include "../../constants.h"
#include "../../../services/totp/totp.h"
#include "../../../services/config/config.h"
#include "../../../services/crypto/crypto.h"
#include "../../../services/convert/convert.h"
#include "../../../lib/polyfills/memset_s.h"
#include "../../../lib/roll_value/roll_value.h"
#include "../../scene_director.h"
#include "../token_menu/totp_scene_token_menu.h"
#include "../../../features_config.h"
#include "../../../workers/usb_type_code/usb_type_code.h"
#ifdef TOTP_BADBT_TYPE_ENABLED
#include "../../../workers/bt_type_code/bt_type_code.h"
#endif

static const uint8_t PROGRESS_BAR_MARGIN = 3;
static const uint8_t PROGRESS_BAR_HEIGHT = 4;

typedef struct {
    uint16_t current_token_index;
    char last_code[TOTP_TOKEN_DIGITS_MAX_COUNT + 1];
    bool need_token_update;
    TokenInfo* current_token;
    uint32_t last_token_gen_time;
    TotpUsbTypeCodeWorkerContext* usb_type_code_worker_context;
    NotificationMessage const** notification_sequence_new_token;
    NotificationMessage const** notification_sequence_badusb;
    FuriMutex* last_code_update_sync;
} SceneState;

static const NotificationSequence*
    get_notification_sequence_new_token(const PluginState* plugin_state, SceneState* scene_state) {
    if(scene_state->notification_sequence_new_token == NULL) {
        uint8_t i = 0;
        uint8_t length = 4;
        if(plugin_state->notification_method & NotificationMethodVibro) {
            length += 2;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            length += 2;
        }

        scene_state->notification_sequence_new_token = malloc(sizeof(void*) * length);
        furi_check(scene_state->notification_sequence_new_token != NULL);
        scene_state->notification_sequence_new_token[i++] = &message_display_backlight_on;
        scene_state->notification_sequence_new_token[i++] = &message_green_255;
        if(plugin_state->notification_method & NotificationMethodVibro) {
            scene_state->notification_sequence_new_token[i++] = &message_vibro_on;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            scene_state->notification_sequence_new_token[i++] = &message_note_c5;
        }

        scene_state->notification_sequence_new_token[i++] = &message_delay_50;

        if(plugin_state->notification_method & NotificationMethodVibro) {
            scene_state->notification_sequence_new_token[i++] = &message_vibro_off;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            scene_state->notification_sequence_new_token[i++] = &message_sound_off;
        }

        scene_state->notification_sequence_new_token[i++] = NULL;
    }

    return (NotificationSequence*)scene_state->notification_sequence_new_token;
}

static const NotificationSequence*
    get_notification_sequence_automation(const PluginState* plugin_state, SceneState* scene_state) {
    if(scene_state->notification_sequence_badusb == NULL) {
        uint8_t i = 0;
        uint8_t length = 3;
        if(plugin_state->notification_method & NotificationMethodVibro) {
            length += 2;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            length += 6;
        }

        scene_state->notification_sequence_badusb = malloc(sizeof(void*) * length);
        furi_check(scene_state->notification_sequence_badusb != NULL);

        scene_state->notification_sequence_badusb[i++] = &message_blue_255;
        if(plugin_state->notification_method & NotificationMethodVibro) {
            scene_state->notification_sequence_badusb[i++] = &message_vibro_on;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            scene_state->notification_sequence_badusb[i++] = &message_note_d5; //-V525
            scene_state->notification_sequence_badusb[i++] = &message_delay_50;
            scene_state->notification_sequence_badusb[i++] = &message_note_e4;
            scene_state->notification_sequence_badusb[i++] = &message_delay_50;
            scene_state->notification_sequence_badusb[i++] = &message_note_f3;
        }

        scene_state->notification_sequence_badusb[i++] = &message_delay_50;

        if(plugin_state->notification_method & NotificationMethodVibro) {
            scene_state->notification_sequence_badusb[i++] = &message_vibro_off;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            scene_state->notification_sequence_badusb[i++] = &message_sound_off;
        }

        scene_state->notification_sequence_badusb[i++] = NULL;
    }

    return (NotificationSequence*)scene_state->notification_sequence_badusb;
}

static void int_token_to_str(uint32_t i_token_code, char* str, TokenDigitsCount len) {
    if(i_token_code == OTP_ERROR) {
        memset(&str[0], '-', len);
    } else {
        for(int i = len - 1; i >= 0; i--) {
            str[i] = CONVERT_DIGIT_TO_CHAR(i_token_code % 10);
            i_token_code = i_token_code / 10;
        }
    }

    str[len] = '\0';
}

static TOTP_ALGO get_totp_algo_impl(TokenHashAlgo algo) {
    switch(algo) {
    case SHA1:
        return TOTP_ALGO_SHA1;
    case SHA256:
        return TOTP_ALGO_SHA256;
    case SHA512:
        return TOTP_ALGO_SHA512;
    default:
        break;
    }

    return NULL;
}

static void update_totp_params(PluginState* const plugin_state) {
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    if(scene_state->current_token_index < plugin_state->tokens_count) {
        TokenInfo* tokenInfo =
            list_element_at(plugin_state->tokens_list, scene_state->current_token_index)->data;

        scene_state->need_token_update = true;
        scene_state->current_token = tokenInfo;
    }
}

void totp_scene_generate_token_init(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}

void totp_scene_generate_token_activate(
    PluginState* plugin_state,
    const GenerateTokenSceneContext* context) {
    if(!plugin_state->token_list_loaded) {
        TokenLoadingResult token_load_result = totp_config_file_load_tokens(plugin_state);
        if(token_load_result != TokenLoadingResultSuccess) {
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_buttons(message, NULL, "Okay", NULL);
            if(token_load_result == TokenLoadingResultWarning) {
                dialog_message_set_text(
                    message,
                    "Unable to load some tokens\nPlease review conf file",
                    SCREEN_WIDTH_CENTER,
                    SCREEN_HEIGHT_CENTER,
                    AlignCenter,
                    AlignCenter);
            } else if(token_load_result == TokenLoadingResultError) {
                dialog_message_set_text(
                    message,
                    "Unable to load tokens\nPlease review conf file",
                    SCREEN_WIDTH_CENTER,
                    SCREEN_HEIGHT_CENTER,
                    AlignCenter,
                    AlignCenter);
            }

            dialog_message_show(plugin_state->dialogs_app, message);
            dialog_message_free(message);
        }
    }
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    if(context == NULL || context->current_token_index > plugin_state->tokens_count) {
        scene_state->current_token_index = 0;
    } else {
        scene_state->current_token_index = context->current_token_index;
    }
    scene_state->need_token_update = true;
    plugin_state->current_scene_state = scene_state;
    FURI_LOG_D(LOGGING_TAG, "Timezone set to: %f", (double)plugin_state->timezone_offset);
    update_totp_params(plugin_state);

    scene_state->last_code_update_sync = furi_mutex_alloc(FuriMutexTypeNormal);
    if(plugin_state->automation_method & AutomationMethodBadUsb) {
        scene_state->usb_type_code_worker_context = totp_usb_type_code_worker_start(
            &scene_state->last_code[0],
            TOTP_TOKEN_DIGITS_MAX_COUNT + 1,
            scene_state->last_code_update_sync);
    }

#ifdef TOTP_BADBT_TYPE_ENABLED

    if(plugin_state->automation_method & AutomationMethodBadBt) {
        if(plugin_state->bt_type_code_worker_context == NULL) {
            plugin_state->bt_type_code_worker_context = totp_bt_type_code_worker_init();
        }
        totp_bt_type_code_worker_start(
            plugin_state->bt_type_code_worker_context,
            &scene_state->last_code[0],
            TOTP_TOKEN_DIGITS_MAX_COUNT + 1,
            scene_state->last_code_update_sync);
    }
#endif
}

void totp_scene_generate_token_render(Canvas* const canvas, PluginState* plugin_state) {
    if(plugin_state->tokens_count == 0) {
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER - 10,
            AlignCenter,
            AlignCenter,
            "Token list is empty");
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER + 10,
            AlignCenter,
            AlignCenter,
            "Press OK button to add");
        return;
    }

    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    FuriHalRtcDateTime curr_dt;
    furi_hal_rtc_get_datetime(&curr_dt);
    uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

    bool is_new_token_time = curr_ts % scene_state->current_token->duration == 0;
    if(is_new_token_time && scene_state->last_token_gen_time != curr_ts) {
        scene_state->need_token_update = true;
    }

    if(scene_state->need_token_update) {
        scene_state->need_token_update = false;
        scene_state->last_token_gen_time = curr_ts;

        const TokenInfo* tokenInfo = scene_state->current_token;

        if(tokenInfo->token != NULL && tokenInfo->token_length > 0) {
            furi_mutex_acquire(scene_state->last_code_update_sync, FuriWaitForever);
            size_t key_length;
            uint8_t* key = totp_crypto_decrypt(
                tokenInfo->token, tokenInfo->token_length, &plugin_state->iv[0], &key_length);

            int_token_to_str(
                totp_at(
                    get_totp_algo_impl(tokenInfo->algo),
                    tokenInfo->digits,
                    key,
                    key_length,
                    curr_ts,
                    plugin_state->timezone_offset,
                    tokenInfo->duration),
                scene_state->last_code,
                tokenInfo->digits);
            memset_s(key, key_length, 0, key_length);
            free(key);
        } else {
            furi_mutex_acquire(scene_state->last_code_update_sync, FuriWaitForever);
            int_token_to_str(0, scene_state->last_code, tokenInfo->digits);
        }

        furi_mutex_release(scene_state->last_code_update_sync);

        if(is_new_token_time) {
            notification_message(
                plugin_state->notification_app,
                get_notification_sequence_new_token(plugin_state, scene_state));
        }
    }

    canvas_set_font(canvas, FontPrimary);
    uint16_t token_name_width = canvas_string_width(canvas, scene_state->current_token->name);
    if(SCREEN_WIDTH - token_name_width > 18) {
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER - 20,
            AlignCenter,
            AlignCenter,
            scene_state->current_token->name);
    } else {
        canvas_draw_str_aligned(
            canvas,
            9,
            SCREEN_HEIGHT_CENTER - 20,
            AlignLeft,
            AlignCenter,
            scene_state->current_token->name);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 0, SCREEN_HEIGHT_CENTER - 24, 9, 9);
        canvas_draw_box(canvas, SCREEN_WIDTH - 10, SCREEN_HEIGHT_CENTER - 24, 9, 9);
        canvas_set_color(canvas, ColorBlack);
    }

    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas,
        SCREEN_WIDTH_CENTER,
        SCREEN_HEIGHT_CENTER,
        AlignCenter,
        AlignCenter,
        scene_state->last_code);

    const uint8_t TOKEN_LIFETIME = scene_state->current_token->duration;
    float percentDone = (float)(TOKEN_LIFETIME - curr_ts % TOKEN_LIFETIME) / (float)TOKEN_LIFETIME;
    uint8_t barWidth = (uint8_t)((float)(SCREEN_WIDTH - (PROGRESS_BAR_MARGIN << 1)) * percentDone);
    uint8_t barX =
        ((SCREEN_WIDTH - (PROGRESS_BAR_MARGIN << 1) - barWidth) >> 1) + PROGRESS_BAR_MARGIN;

    canvas_draw_box(
        canvas,
        barX,
        SCREEN_HEIGHT - PROGRESS_BAR_MARGIN - PROGRESS_BAR_HEIGHT,
        barWidth,
        PROGRESS_BAR_HEIGHT);

    if(plugin_state->tokens_count > 1) {
        canvas_draw_icon(canvas, 0, SCREEN_HEIGHT_CENTER - 24, &I_totp_arrow_left_8x9);
        canvas_draw_icon(
            canvas, SCREEN_WIDTH - 9, SCREEN_HEIGHT_CENTER - 24, &I_totp_arrow_right_8x9);
    }

#if defined(TOTP_BADBT_TYPE_ENABLED) && defined(TOTP_BADBT_TYPE_ICON_ENABLED)
    if(plugin_state->automation_method & AutomationMethodBadBt &&
       plugin_state->bt_type_code_worker_context != NULL &&
       plugin_state->bt_type_code_worker_context->is_advertising) {
        canvas_draw_icon(
            canvas, SCREEN_WIDTH_CENTER - 5, SCREEN_HEIGHT_CENTER + 13, &I_hid_ble_10x7);
    }
#endif
}

bool totp_scene_generate_token_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    if(event->input.type == InputTypeLong && event->input.key == InputKeyBack) {
        return false;
    }

    SceneState* scene_state;
    if(event->input.type == InputTypeLong) {
        if(event->input.key == InputKeyDown &&
           plugin_state->automation_method & AutomationMethodBadUsb) {
            scene_state = (SceneState*)plugin_state->current_scene_state;
            totp_usb_type_code_worker_notify(
                scene_state->usb_type_code_worker_context, TotpUsbTypeCodeWorkerEventType);
            notification_message(
                plugin_state->notification_app,
                get_notification_sequence_automation(plugin_state, scene_state));
            return true;
        }
#ifdef TOTP_BADBT_TYPE_ENABLED
        else if(
            event->input.key == InputKeyUp &&
            plugin_state->automation_method & AutomationMethodBadBt) {
            scene_state = (SceneState*)plugin_state->current_scene_state;
            totp_bt_type_code_worker_notify(
                plugin_state->bt_type_code_worker_context, TotpBtTypeCodeWorkerEventType);
            notification_message(
                plugin_state->notification_app,
                get_notification_sequence_automation(plugin_state, scene_state));
            return true;
        }
#endif
    }

    if(event->input.type != InputTypePress && event->input.type != InputTypeRepeat) {
        return true;
    }

    scene_state = (SceneState*)plugin_state->current_scene_state;
    switch(event->input.key) {
    case InputKeyUp:
        break;
    case InputKeyDown:
        break;
    case InputKeyRight:
        totp_roll_value_uint16_t(
            &scene_state->current_token_index,
            1,
            0,
            plugin_state->tokens_count - 1,
            RollOverflowBehaviorRoll);
        update_totp_params(plugin_state);
        break;
    case InputKeyLeft:
        totp_roll_value_uint16_t(
            &scene_state->current_token_index,
            -1,
            0,
            plugin_state->tokens_count - 1,
            RollOverflowBehaviorRoll);
        update_totp_params(plugin_state);
        break;
    case InputKeyOk:
        if(plugin_state->tokens_count == 0) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, NULL);
        } else {
            TokenMenuSceneContext ctx = {.current_token_index = scene_state->current_token_index};
            totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, &ctx);
        }
        break;
    case InputKeyBack:
        break;
    default:
        break;
    }

    return true;
}

void totp_scene_generate_token_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    if(plugin_state->automation_method & AutomationMethodBadUsb) {
        totp_usb_type_code_worker_stop(scene_state->usb_type_code_worker_context);
    }
#ifdef TOTP_BADBT_TYPE_ENABLED
    if(plugin_state->automation_method & AutomationMethodBadBt) {
        totp_bt_type_code_worker_stop(plugin_state->bt_type_code_worker_context);
    }
#endif

    if(scene_state->notification_sequence_new_token != NULL) {
        free(scene_state->notification_sequence_new_token);
    }

    if(scene_state->notification_sequence_badusb != NULL) {
        free(scene_state->notification_sequence_badusb);
    }

    furi_mutex_free(scene_state->last_code_update_sync);

    free(scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_generate_token_free(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}
