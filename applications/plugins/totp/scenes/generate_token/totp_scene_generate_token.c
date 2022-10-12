#include <gui/gui.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include "totp_scene_generate_token.h"
#include "../../types/token_info.h"
#include "../../types/common.h"
#include "../../services/ui/icons.h"
#include "../../services/ui/constants.h"
#include "../../services/totp/totp.h"
#include "../../services/config/config.h"
#include "../scene_director.h"
#include "../token_menu/totp_scene_token_menu.h"

#define TOKEN_LIFETIME 30
#define DIGIT_TO_CHAR(digit) ((digit) + '0')

typedef struct {
    uint8_t current_token_index;
    char last_code[9];
    char* last_code_name;
    bool need_token_update;
    uint32_t last_token_gen_time;
} SceneState;

static const NotificationSequence sequence_short_vibro_and_sound = {
    &message_display_backlight_on,
    &message_green_255,
    &message_vibro_on,
    &message_note_c5,
    &message_delay_50,
    &message_vibro_off,
    &message_sound_off,
    NULL,
};

static void i_token_to_str(uint32_t i_token_code, char* str, TokenDigitsCount len) {
    if(len == TOTP_8_DIGITS) {
        str[8] = '\0';
    } else if(len == TOTP_6_DIGITS) {
        str[6] = '\0';
    }

    if(i_token_code == 0) {
        if(len > TOTP_6_DIGITS) {
            str[7] = '-';
            str[6] = '-';
        }

        str[5] = '-';
        str[4] = '-';
        str[3] = '-';
        str[2] = '-';
        str[1] = '-';
        str[0] = '-';
    } else {
        if(len == TOTP_8_DIGITS) {
            str[7] = DIGIT_TO_CHAR(i_token_code % 10);
            str[6] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
            str[5] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
        } else if(len == TOTP_6_DIGITS) {
            str[5] = DIGIT_TO_CHAR(i_token_code % 10);
        }

        str[4] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
        str[3] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
        str[2] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
        str[1] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
        str[0] = DIGIT_TO_CHAR((i_token_code = i_token_code / 10) % 10);
    }
}

TOTP_ALGO get_totp_algo_impl(TokenHashAlgo algo) {
    switch(algo) {
    case SHA1:
        return TOTP_ALGO_SHA1;
    case SHA256:
        return TOTP_ALGO_SHA256;
    case SHA512:
        return TOTP_ALGO_SHA512;
    }

    return NULL;
}

void update_totp_params(PluginState* const plugin_state) {
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    if(scene_state->current_token_index < plugin_state->tokens_count) {
        TokenInfo* tokenInfo =
            (TokenInfo*)(list_element_at(
                             plugin_state->tokens_list, scene_state->current_token_index)
                             ->data);

        scene_state->need_token_update = true;
        scene_state->last_code_name = tokenInfo->name;
    }
}

void totp_scene_generate_token_init(PluginState* plugin_state) {
    UNUSED(plugin_state);
}

void totp_scene_generate_token_activate(
    PluginState* plugin_state,
    const GenerateTokenSceneContext* context) {
    if(!plugin_state->token_list_loaded) {
        totp_config_file_load_tokens(plugin_state);
    }
    SceneState* scene_state = malloc(sizeof(SceneState));
    if(context == NULL) {
        scene_state->current_token_index = 0;
    } else {
        scene_state->current_token_index = context->current_token_index;
    }
    scene_state->need_token_update = true;
    plugin_state->current_scene_state = scene_state;
    FURI_LOG_D(LOGGING_TAG, "Timezone set to: %f", (double)plugin_state->timezone_offset);
    update_totp_params(plugin_state);
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

    bool is_new_token_time = curr_ts % TOKEN_LIFETIME == 0;
    if(is_new_token_time && scene_state->last_token_gen_time != curr_ts) {
        scene_state->need_token_update = true;
    }

    if(scene_state->need_token_update) {
        scene_state->need_token_update = false;
        scene_state->last_token_gen_time = curr_ts;

        TokenInfo* tokenInfo =
            (TokenInfo*)(list_element_at(
                             plugin_state->tokens_list, scene_state->current_token_index)
                             ->data);

        uint8_t* key = malloc(tokenInfo->token_length);

        furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, &plugin_state->iv[0]);
        furi_hal_crypto_decrypt(tokenInfo->token, key, tokenInfo->token_length);
        furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);

        i_token_to_str(
            totp_at(
                get_totp_algo_impl(tokenInfo->algo),
                token_info_get_digits_count(tokenInfo),
                key,
                tokenInfo->token_length,
                curr_ts,
                plugin_state->timezone_offset,
                TOKEN_LIFETIME),
            scene_state->last_code,
            tokenInfo->digits);
        memset(key, 0, tokenInfo->token_length);
        free(key);

        if(is_new_token_time) {
            notification_message(plugin_state->notification, &sequence_short_vibro_and_sound);
        }
    }

    canvas_set_font(canvas, FontPrimary);
    uint16_t token_name_width = canvas_string_width(canvas, scene_state->last_code_name);
    if(SCREEN_WIDTH - token_name_width > 18) {
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER - 20,
            AlignCenter,
            AlignCenter,
            scene_state->last_code_name);
    } else {
        canvas_draw_str_aligned(
            canvas,
            9,
            SCREEN_HEIGHT_CENTER - 20,
            AlignLeft,
            AlignCenter,
            scene_state->last_code_name);
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

    const uint8_t BAR_MARGIN = 3;
    const uint8_t BAR_HEIGHT = 4;
    float percentDone = (float)(TOKEN_LIFETIME - curr_ts % TOKEN_LIFETIME) / (float)TOKEN_LIFETIME;
    uint8_t barWidth = (uint8_t)((float)(SCREEN_WIDTH - (BAR_MARGIN << 1)) * percentDone);
    uint8_t barX = ((SCREEN_WIDTH - (BAR_MARGIN << 1) - barWidth) >> 1) + BAR_MARGIN;

    canvas_draw_box(canvas, barX, SCREEN_HEIGHT - BAR_MARGIN - BAR_HEIGHT, barWidth, BAR_HEIGHT);

    if(plugin_state->tokens_count > 1) {
        canvas_draw_xbm(
            canvas,
            0,
            SCREEN_HEIGHT_CENTER - 24,
            ICON_ARROW_LEFT_8x9_WIDTH,
            ICON_ARROW_LEFT_8x9_HEIGHT,
            &ICON_ARROW_LEFT_8x9[0]);
        canvas_draw_xbm(
            canvas,
            SCREEN_WIDTH - 9,
            SCREEN_HEIGHT_CENTER - 24,
            ICON_ARROW_RIGHT_8x9_WIDTH,
            ICON_ARROW_RIGHT_8x9_HEIGHT,
            &ICON_ARROW_RIGHT_8x9[0]);
    }
}

bool totp_scene_generate_token_handle_event(PluginEvent* const event, PluginState* plugin_state) {
    if(event->type == EventTypeKey) {
        if(event->input.type == InputTypeLong && event->input.key == InputKeyBack) {
            return false;
        } else if(event->input.type == InputTypePress) {
            SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
            switch(event->input.key) {
            case InputKeyUp:
                break;
            case InputKeyDown:
                break;
            case InputKeyRight:
                if(scene_state->current_token_index < plugin_state->tokens_count - 1) {
                    scene_state->current_token_index++;
                } else {
                    scene_state->current_token_index = 0;
                }
                update_totp_params(plugin_state);
                break;
            case InputKeyLeft:
                if(scene_state->current_token_index > 0) {
                    scene_state->current_token_index--;
                } else {
                    scene_state->current_token_index = plugin_state->tokens_count - 1;
                }
                update_totp_params(plugin_state);
                break;
            case InputKeyOk:
                if(plugin_state->tokens_count == 0) {
                    totp_scene_director_activate_scene(plugin_state, TotpSceneAddNewToken, NULL);
                } else {
                    TokenMenuSceneContext ctx = {
                        .current_token_index = scene_state->current_token_index};
                    totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu, &ctx);
                }
                break;
            case InputKeyBack:
                break;
            }
        }
    }

    return true;
}

void totp_scene_generate_token_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    free(scene_state->last_code);
    free(scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_generate_token_free(PluginState* plugin_state) {
    UNUSED(plugin_state);
}
