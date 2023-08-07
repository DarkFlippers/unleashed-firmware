#include "totp_scene_add_new_token.h"
#include "../../../types/common.h"
#include "../../constants.h"
#include "../../scene_director.h"
#include "totp_input_text.h"
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../ui_controls.h"
#include "../../common_dialogs.h"
#include <roll_value.h>
#include "../generate_token/totp_scene_generate_token.h"

char* TOKEN_ALGO_LIST[] = {"SHA1", "SHA256", "SHA512", "Steam"};
char* TOKEN_DIGITS_TEXT_LIST[] = {"5 digits", "6 digits", "8 digits"};
TokenDigitsCount TOKEN_DIGITS_VALUE_LIST[] = {
    TokenDigitsCountFive,
    TokenDigitsCountSix,
    TokenDigitsCountEight};

typedef enum {
    TokenNameTextBox,
    TokenSecretTextBox,
    TokenAlgoSelect,
    TokenLengthSelect,
    TokenDurationSelect,
    ConfirmButton,
} Control;

typedef struct {
    char* token_name;
    size_t token_name_length;
    char* token_secret;
    size_t token_secret_length;
    bool saved;
    Control selected_control;
    int16_t screen_y_offset;
    TokenHashAlgo algo;
    uint8_t digits_count_index;
    uint8_t duration;
    FuriString* duration_text;
} SceneState;

struct TotpAddContext {
    SceneState* scene_state;
    const CryptoSettings* crypto_settings;
};

enum TotpIteratorUpdateTokenResultsEx { TotpIteratorUpdateTokenResultInvalidSecret = 1 };

static void update_duration_text(SceneState* scene_state) {
    furi_string_printf(scene_state->duration_text, "%d sec.", scene_state->duration);
}

static TotpIteratorUpdateTokenResult add_token_handler(TokenInfo* tokenInfo, const void* context) {
    const struct TotpAddContext* context_t = context;
    if(!token_info_set_secret(
           tokenInfo,
           context_t->scene_state->token_secret,
           context_t->scene_state->token_secret_length,
           PlainTokenSecretEncodingBase32,
           context_t->crypto_settings)) {
        return TotpIteratorUpdateTokenResultInvalidSecret;
    }

    furi_string_set_strn(
        tokenInfo->name,
        context_t->scene_state->token_name,
        context_t->scene_state->token_name_length + 1);
    tokenInfo->algo = context_t->scene_state->algo;
    tokenInfo->digits = TOKEN_DIGITS_VALUE_LIST[context_t->scene_state->digits_count_index];
    tokenInfo->duration = context_t->scene_state->duration;

    return TotpIteratorUpdateTokenResultSuccess;
}

static void ask_user_input(
    const PluginState* plugin_state,
    const char* header,
    char** user_input,
    size_t* user_input_length) {
    InputTextResult input_result;
    if(*user_input != NULL) {
        strlcpy(input_result.user_input, *user_input, INPUT_BUFFER_SIZE);
    }

    totp_input_text(plugin_state->gui, header, &input_result);
    if(input_result.success) {
        if(*user_input != NULL) {
            free(*user_input);
        }
        *user_input = strdup(input_result.user_input);
        *user_input_length = input_result.user_input_length;
    }
}

void totp_scene_add_new_token_activate(PluginState* plugin_state) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    plugin_state->current_scene_state = scene_state;
    scene_state->token_name = "Name";
    scene_state->token_name_length = strlen(scene_state->token_name);
    scene_state->token_secret = "Secret";
    scene_state->token_secret_length = strlen(scene_state->token_secret);

    scene_state->screen_y_offset = 0;

    scene_state->digits_count_index = 1;

    scene_state->duration = TokenDurationDefault;
    scene_state->duration_text = furi_string_alloc();
    update_duration_text(scene_state);
}

void totp_scene_add_new_token_render(Canvas* const canvas, const PluginState* plugin_state) {
    const SceneState* scene_state = plugin_state->current_scene_state;

    ui_control_text_box_render(
        canvas,
        10 - scene_state->screen_y_offset,
        scene_state->token_name,
        scene_state->selected_control == TokenNameTextBox);
    ui_control_text_box_render(
        canvas,
        27 - scene_state->screen_y_offset,
        scene_state->token_secret,
        scene_state->selected_control == TokenSecretTextBox);
    ui_control_select_render(
        canvas,
        0,
        44 - scene_state->screen_y_offset,
        SCREEN_WIDTH,
        TOKEN_ALGO_LIST[scene_state->algo],
        scene_state->selected_control == TokenAlgoSelect);
    ui_control_select_render(
        canvas,
        0,
        61 - scene_state->screen_y_offset,
        SCREEN_WIDTH,
        TOKEN_DIGITS_TEXT_LIST[scene_state->digits_count_index],
        scene_state->selected_control == TokenLengthSelect);

    ui_control_select_render(
        canvas,
        0,
        78 - scene_state->screen_y_offset,
        SCREEN_WIDTH,
        furi_string_get_cstr(scene_state->duration_text),
        scene_state->selected_control == TokenDurationSelect);

    ui_control_button_render(
        canvas,
        SCREEN_WIDTH_CENTER - 24,
        101 - scene_state->screen_y_offset,
        48,
        13,
        "Confirm",
        scene_state->selected_control == ConfirmButton);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, SCREEN_WIDTH, 10);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Add new token");
    canvas_set_font(canvas, FontSecondary);
}

void update_screen_y_offset(SceneState* scene_state) {
    if(scene_state->selected_control > TokenLengthSelect) {
        scene_state->screen_y_offset = 51;
    } else if(scene_state->selected_control > TokenAlgoSelect) {
        scene_state->screen_y_offset = 34;
    } else if(scene_state->selected_control > TokenSecretTextBox) {
        scene_state->screen_y_offset = 17;
    } else {
        scene_state->screen_y_offset = 0;
    }
}

bool totp_scene_add_new_token_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    SceneState* scene_state = plugin_state->current_scene_state;

    if(event->input.type == InputTypePress) {
        switch(event->input.key) {
        case InputKeyUp:
            totp_roll_value_uint8_t(
                &scene_state->selected_control,
                -1,
                TokenNameTextBox,
                ConfirmButton,
                RollOverflowBehaviorStop);
            update_screen_y_offset(scene_state);
            break;
        case InputKeyDown:
            totp_roll_value_uint8_t(
                &scene_state->selected_control,
                1,
                TokenNameTextBox,
                ConfirmButton,
                RollOverflowBehaviorStop);
            update_screen_y_offset(scene_state);
            break;
        case InputKeyRight:
            if(scene_state->selected_control == TokenAlgoSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->algo,
                    1,
                    TokenHashAlgoSha1,
                    TokenHashAlgoSteam,
                    RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == TokenLengthSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->digits_count_index, 1, 0, 2, RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == TokenDurationSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->duration, 15, 15, 255, RollOverflowBehaviorStop);
                update_duration_text(scene_state);
            }
            break;
        case InputKeyLeft:
            if(scene_state->selected_control == TokenAlgoSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->algo,
                    -1,
                    TokenHashAlgoSha1,
                    TokenHashAlgoSteam,
                    RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == TokenLengthSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->digits_count_index, -1, 0, 2, RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == TokenDurationSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->duration, -15, 15, 255, RollOverflowBehaviorStop);
                update_duration_text(scene_state);
            }
            break;
        case InputKeyOk:
            break;
        case InputKeyBack:
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken);
            break;
        default:
            break;
        }
    } else if(event->input.type == InputTypeRelease && event->input.key == InputKeyOk) {
        switch(scene_state->selected_control) {
        case TokenNameTextBox:
            ask_user_input(
                plugin_state,
                "Token name",
                &scene_state->token_name,
                &scene_state->token_name_length);
            break;
        case TokenSecretTextBox:
            ask_user_input(
                plugin_state,
                "Token secret",
                &scene_state->token_secret,
                &scene_state->token_secret_length);
            break;
        case TokenAlgoSelect:
            break;
        case TokenLengthSelect:
            break;
        case TokenDurationSelect:
            break;
        case ConfirmButton: {
            struct TotpAddContext add_context = {
                .scene_state = scene_state, .crypto_settings = &plugin_state->crypto_settings};
            TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            TotpIteratorUpdateTokenResult add_result = totp_token_info_iterator_add_new_token(
                iterator_context, &add_token_handler, &add_context);

            if(add_result == TotpIteratorUpdateTokenResultSuccess) {
                totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken);
            } else if(add_result == TotpIteratorUpdateTokenResultInvalidSecret) {
                DialogMessage* message = dialog_message_alloc();
                dialog_message_set_buttons(message, "Back", NULL, NULL);
                dialog_message_set_text(
                    message,
                    "Token secret is invalid",
                    SCREEN_WIDTH_CENTER,
                    SCREEN_HEIGHT_CENTER,
                    AlignCenter,
                    AlignCenter);
                dialog_message_show(plugin_state->dialogs_app, message);
                dialog_message_free(message);
                scene_state->selected_control = TokenSecretTextBox;
                update_screen_y_offset(scene_state);
            } else if(add_result == TotpIteratorUpdateTokenResultFileUpdateFailed) {
                totp_dialogs_config_updating_error(plugin_state);
            }

            break;
        }
        default:
            break;
        }
    }

    return true;
}

void totp_scene_add_new_token_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    free(scene_state->token_name);
    free(scene_state->token_secret);

    furi_string_free(scene_state->duration_text);

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}
