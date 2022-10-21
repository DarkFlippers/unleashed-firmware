#include "totp_scene_add_new_token.h"
#include "../../types/common.h"
#include "../../services/ui/constants.h"
#include "../scene_director.h"
#include "totp_input_text.h"
#include "../../types/token_info.h"
#include "../../services/list/list.h"
#include "../../services/base32/base32.h"
#include "../../services/config/config.h"
#include "../../services/ui/ui_controls.h"
#include "../generate_token/totp_scene_generate_token.h"

#define TOKEN_ALGO_LIST_LENGTH 3
char* TOKEN_ALGO_LIST[] = {"SHA1", "SHA256", "SHA512"};
#define TOKEN_DIGITS_LIST_LENGTH 2
char* TOKEN_DIGITS_LIST[] = {"6 digits", "8 digits"};

typedef enum {
    TokenNameTextBox,
    TokenSecretTextBox,
    TokenAlgoSelect,
    TokenLengthSelect,
    ConfirmButton,
} Control;

typedef struct {
    char* token_name;
    uint8_t token_name_length;
    char* token_secret;
    uint8_t token_secret_length;
    bool saved;
    Control selected_control;
    InputTextSceneContext* token_name_input_context;
    InputTextSceneContext* token_secret_input_context;
    InputTextSceneState* input_state;
    uint32_t input_started_at;
    int16_t current_token_index;
    int32_t screen_y_offset;
    TokenHashAlgo algo;
    TokenDigitsCount digits_count;
} SceneState;

void totp_scene_add_new_token_init(PluginState* plugin_state) {
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

void totp_scene_add_new_token_activate(
    PluginState* plugin_state,
    const TokenAddEditSceneContext* context) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    plugin_state->current_scene_state = scene_state;
    scene_state->token_name = "Name";
    scene_state->token_name_length = strlen(scene_state->token_name);
    scene_state->token_secret = "Secret";
    scene_state->token_secret_length = strlen(scene_state->token_secret);

    scene_state->token_name_input_context = malloc(sizeof(InputTextSceneContext));
    scene_state->token_name_input_context->header_text = "Enter token name";
    scene_state->token_name_input_context->callback_data = scene_state;
    scene_state->token_name_input_context->callback = on_token_name_user_comitted;

    scene_state->token_secret_input_context = malloc(sizeof(InputTextSceneContext));
    scene_state->token_secret_input_context->header_text = "Enter token secret";
    scene_state->token_secret_input_context->callback_data = scene_state;
    scene_state->token_secret_input_context->callback = on_token_secret_user_comitted;

    scene_state->screen_y_offset = 0;

    scene_state->input_state = NULL;

    if(context == NULL) {
        scene_state->current_token_index = -1;
    } else {
        scene_state->current_token_index = context->current_token_index;
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
        63 - scene_state->screen_y_offset,
        SCREEN_WIDTH,
        TOKEN_DIGITS_LIST[scene_state->digits_count],
        scene_state->selected_control == TokenLengthSelect);
    ui_control_button_render(
        canvas,
        SCREEN_WIDTH_CENTER - 24,
        85 - scene_state->screen_y_offset,
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
    if(scene_state->selected_control > TokenAlgoSelect) {
        scene_state->screen_y_offset = 35;
    } else {
        scene_state->screen_y_offset = 0;
    }
}

bool totp_scene_add_new_token_handle_event(PluginEvent* const event, PluginState* plugin_state) {
    if(event->type == EventTypeKey) {
        SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
        if(scene_state->input_started_at > 0 &&
           furi_get_tick() - scene_state->input_started_at > 300) {
            return totp_input_text_handle_event(event, scene_state->input_state);
        }

        if(event->input.type == InputTypeLong && event->input.key == InputKeyBack) {
            return false;
        } else if(event->input.type == InputTypePress) {
            switch(event->input.key) {
            case InputKeyUp:
                if(scene_state->selected_control > TokenNameTextBox) {
                    scene_state->selected_control--;
                    update_screen_y_offset(scene_state);
                }
                break;
            case InputKeyDown:
                if(scene_state->selected_control < ConfirmButton) {
                    scene_state->selected_control++;
                    update_screen_y_offset(scene_state);
                }
                break;
            case InputKeyRight:
                if(scene_state->selected_control == TokenAlgoSelect) {
                    if(scene_state->algo < SHA512) {
                        scene_state->algo++;
                    } else {
                        scene_state->algo = SHA1;
                    }
                } else if(scene_state->selected_control == TokenLengthSelect) {
                    if(scene_state->digits_count < TOTP_8_DIGITS) {
                        scene_state->digits_count++;
                    } else {
                        scene_state->digits_count = TOTP_6_DIGITS;
                    }
                }
                break;
            case InputKeyLeft:
                if(scene_state->selected_control == TokenAlgoSelect) {
                    if(scene_state->algo > SHA1) {
                        scene_state->algo--;
                    } else {
                        scene_state->algo = SHA512;
                    }
                } else if(scene_state->selected_control == TokenLengthSelect) {
                    if(scene_state->digits_count > TOTP_6_DIGITS) {
                        scene_state->digits_count--;
                    } else {
                        scene_state->digits_count = TOTP_8_DIGITS;
                    }
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
                case ConfirmButton: {
                    TokenInfo* tokenInfo = token_info_alloc();
                    bool token_secret_set = token_info_set_secret(
                        tokenInfo,
                        scene_state->token_secret,
                        scene_state->token_secret_length,
                        &plugin_state->iv[0]);

                    if(token_secret_set) {
                        tokenInfo->name = malloc(scene_state->token_name_length + 1);
                        strcpy(tokenInfo->name, scene_state->token_name);
                        tokenInfo->algo = scene_state->algo;
                        tokenInfo->digits = scene_state->digits_count;

                        if(plugin_state->tokens_list == NULL) {
                            plugin_state->tokens_list = list_init_head(tokenInfo);
                        } else {
                            list_add(plugin_state->tokens_list, tokenInfo);
                        }
                        plugin_state->tokens_count++;

                        totp_config_file_save_new_token(tokenInfo);

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
                        dialog_message_show(plugin_state->dialogs, message);
                        dialog_message_free(message);
                        scene_state->selected_control = TokenSecretTextBox;
                        update_screen_y_offset(scene_state);
                    }
                    break;
                }
                }
                break;
            case InputKeyBack:
                if(scene_state->current_token_index >= 0) {
                    GenerateTokenSceneContext generate_scene_context = {
                        .current_token_index = scene_state->current_token_index};
                    totp_scene_director_activate_scene(
                        plugin_state, TotpSceneGenerateToken, &generate_scene_context);
                } else {
                    totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
                }
                break;
            }
        }
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

    if(scene_state->input_state != NULL) {
        totp_input_text_free(scene_state->input_state);
    }

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_add_new_token_free(PluginState* plugin_state) {
    UNUSED(plugin_state);
}
