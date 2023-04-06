#include "totp_scene_add_new_token.h"
#include "../../../types/common.h"
#include "../../constants.h"
#include "../../scene_director.h"
#include "totp_input_text.h"
#include "../../../types/token_info.h"
#include <linked_list.h>
#include "../../../services/config/config.h"
#include "../../ui_controls.h"
#include "../../common_dialogs.h"
#include <roll_value.h>
#include "../../../types/nullable.h"
#include "../generate_token/totp_scene_generate_token.h"

char* TOKEN_ALGO_LIST[] = {"SHA1", "SHA256", "SHA512", "Steam"};
char* TOKEN_DIGITS_TEXT_LIST[] = {"5 digits", "6 digits", "8 digits"};
TokenDigitsCount TOKEN_DIGITS_VALUE_LIST[] = {TOTP_5_DIGITS, TOTP_6_DIGITS, TOTP_8_DIGITS};

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
    InputTextSceneContext* token_name_input_context;
    InputTextSceneContext* token_secret_input_context;
    InputTextSceneState* input_state;
    uint32_t input_started_at;
    TotpNullable_uint16_t current_token_index;
    int16_t screen_y_offset;
    TokenHashAlgo algo;
    uint8_t digits_count_index;
    uint8_t duration;
    FuriString* duration_text;
} SceneState;

void totp_scene_add_new_token_init(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}

static void on_token_name_user_comitted(InputTextSceneCallbackResult* result) {
    SceneState* scene_state = result->callback_data;
    free(scene_state->token_name);
    scene_state->token_name = result->user_input;
    scene_state->token_name_length = result->user_input_length;
    scene_state->input_started_at = 0;
    free(result);
}

static void on_token_secret_user_comitted(InputTextSceneCallbackResult* result) {
    SceneState* scene_state = result->callback_data;
    free(scene_state->token_secret);
    scene_state->token_secret = result->user_input;
    scene_state->token_secret_length = result->user_input_length;
    scene_state->input_started_at = 0;
    free(result);
}

static void update_duration_text(SceneState* scene_state) {
    furi_string_printf(scene_state->duration_text, "%d sec.", scene_state->duration);
}

void totp_scene_add_new_token_activate(
    PluginState* plugin_state,
    const TokenAddEditSceneContext* context) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    plugin_state->current_scene_state = scene_state;
    scene_state->token_name = "Name";
    scene_state->token_name_length = strlen(scene_state->token_name);
    scene_state->token_secret = "Secret";
    scene_state->token_secret_length = strlen(scene_state->token_secret);

    scene_state->token_name_input_context = malloc(sizeof(InputTextSceneContext));
    furi_check(scene_state->token_name_input_context != NULL);
    scene_state->token_name_input_context->header_text = "Enter token name";
    scene_state->token_name_input_context->callback_data = scene_state;
    scene_state->token_name_input_context->callback = on_token_name_user_comitted;

    scene_state->token_secret_input_context = malloc(sizeof(InputTextSceneContext));
    furi_check(scene_state->token_secret_input_context != NULL);
    scene_state->token_secret_input_context->header_text = "Enter token secret";
    scene_state->token_secret_input_context->callback_data = scene_state;
    scene_state->token_secret_input_context->callback = on_token_secret_user_comitted;

    scene_state->screen_y_offset = 0;

    scene_state->digits_count_index = 1;

    scene_state->input_state = NULL;
    scene_state->duration = TOTP_TOKEN_DURATION_DEFAULT;
    scene_state->duration_text = furi_string_alloc();
    update_duration_text(scene_state);

    if(context == NULL) {
        TOTP_NULLABLE_NULL(scene_state->current_token_index);
    } else {
        TOTP_NULLABLE_VALUE(scene_state->current_token_index, context->current_token_index);
    }
}

void totp_scene_add_new_token_render(Canvas* const canvas, PluginState* plugin_state) {
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    if(scene_state->input_started_at > 0) {
        totp_input_text_render(canvas, scene_state->input_state);
        return;
    }

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

bool totp_scene_add_new_token_handle_event(PluginEvent* const event, PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    if(scene_state->input_started_at > 0 &&
       furi_get_tick() - scene_state->input_started_at > 300) {
        return totp_input_text_handle_event(event, scene_state->input_state);
    }

    if(event->input.type == InputTypeLong && event->input.key == InputKeyBack) {
        return false;
    }

    if(event->input.type != InputTypePress) {
        return true;
    }

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
            totp_roll_value_uint8_t(&scene_state->algo, 1, SHA1, STEAM, RollOverflowBehaviorRoll);
        } else if(scene_state->selected_control == TokenLengthSelect) {
            totp_roll_value_uint8_t(
                &scene_state->digits_count_index, 1, 0, 2, RollOverflowBehaviorRoll);
        } else if(scene_state->selected_control == TokenDurationSelect) {
            totp_roll_value_uint8_t(&scene_state->duration, 15, 15, 255, RollOverflowBehaviorStop);
            update_duration_text(scene_state);
        }
        break;
    case InputKeyLeft:
        if(scene_state->selected_control == TokenAlgoSelect) {
            totp_roll_value_uint8_t(&scene_state->algo, -1, SHA1, STEAM, RollOverflowBehaviorRoll);
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
        switch(scene_state->selected_control) {
        case TokenNameTextBox:
            if(scene_state->input_state != NULL) {
                totp_input_text_free(scene_state->input_state);
            }
            scene_state->input_state =
                totp_input_text_activate(scene_state->token_name_input_context);
            scene_state->input_started_at = furi_get_tick();
            break;
        case TokenSecretTextBox:
            if(scene_state->input_state != NULL) {
                totp_input_text_free(scene_state->input_state);
            }
            scene_state->input_state =
                totp_input_text_activate(scene_state->token_secret_input_context);
            scene_state->input_started_at = furi_get_tick();
            break;
        case TokenAlgoSelect:
            break;
        case TokenLengthSelect:
            break;
        case TokenDurationSelect:
            break;
        case ConfirmButton: {
            TokenInfo* tokenInfo = token_info_alloc();
            bool token_secret_set = token_info_set_secret(
                tokenInfo,
                scene_state->token_secret,
                scene_state->token_secret_length,
                PLAIN_TOKEN_ENCODING_BASE32,
                &plugin_state->iv[0]);

            if(token_secret_set) {
                tokenInfo->name = malloc(scene_state->token_name_length + 1);
                furi_check(tokenInfo->name != NULL);
                strlcpy(
                    tokenInfo->name, scene_state->token_name, scene_state->token_name_length + 1);
                tokenInfo->algo = scene_state->algo;
                tokenInfo->digits = TOKEN_DIGITS_VALUE_LIST[scene_state->digits_count_index];
                tokenInfo->duration = scene_state->duration;

                TOTP_LIST_INIT_OR_ADD(plugin_state->tokens_list, tokenInfo, furi_check);
                plugin_state->tokens_count++;

                if(totp_config_file_save_new_token(tokenInfo) != TotpConfigFileUpdateSuccess) {
                    token_info_free(tokenInfo);
                    totp_dialogs_config_updating_error(plugin_state);
                    return false;
                }

                GenerateTokenSceneContext generate_scene_context = {
                    .current_token_index = plugin_state->tokens_count - 1};
                totp_scene_director_activate_scene(
                    plugin_state, TotpSceneGenerateToken, &generate_scene_context);
            } else {
                token_info_free(tokenInfo);
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
            }
            break;
        }
        default:
            break;
        }
        break;
    case InputKeyBack:
        if(!scene_state->current_token_index.is_null) {
            GenerateTokenSceneContext generate_scene_context = {
                .current_token_index = scene_state->current_token_index.value};
            totp_scene_director_activate_scene(
                plugin_state, TotpSceneGenerateToken, &generate_scene_context);
        } else {
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
        }
        break;
    default:
        break;
    }

    return true;
}

void totp_scene_add_new_token_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    free(scene_state->token_name);
    free(scene_state->token_secret);

    free(scene_state->token_name_input_context->header_text);
    free(scene_state->token_name_input_context);

    free(scene_state->token_secret_input_context->header_text);
    free(scene_state->token_secret_input_context);

    furi_string_free(scene_state->duration_text);

    if(scene_state->input_state != NULL) {
        totp_input_text_free(scene_state->input_state);
    }

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_add_new_token_free(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}
