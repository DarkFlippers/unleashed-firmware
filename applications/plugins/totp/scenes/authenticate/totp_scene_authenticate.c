#include "totp_scene_authenticate.h"
#include <dialogs/dialogs.h>
#include "../../types/common.h"
#include "../../services/ui/icons.h"
#include "../../services/ui/constants.h"
#include "../../services/config/config.h"
#include "../scene_director.h"
#include "../totp_scenes_enum.h"

#define MAX_CODE_LENGTH TOTP_IV_SIZE
#define CRYPTO_VERIFY_KEY "FFF_Crypto_pass"
#define CRYPTO_VERIFY_KEY_LENGTH 16

typedef struct {
    uint8_t code_input[MAX_CODE_LENGTH];
    uint8_t code_length;
} SceneState;

void totp_scene_authenticate_init(PluginState* plugin_state) {
    memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
}

void totp_scene_authenticate_activate(PluginState* plugin_state) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    scene_state->code_length = 0;
    memset(&scene_state->code_input[0], 0, MAX_CODE_LENGTH);
    plugin_state->current_scene_state = scene_state;
}

void totp_scene_authenticate_render(Canvas* const canvas, PluginState* plugin_state) {
    SceneState* scene_state = (SceneState *)plugin_state->current_scene_state;

    int v_shift = 0;
    if (scene_state->code_length > 0) {
        v_shift = -10;
    }

    if (plugin_state->crypto_verify_data == NULL) {
        canvas_draw_str_aligned(canvas, SCREEN_WIDTH_CENTER, SCREEN_HEIGHT_CENTER - 10 + v_shift, AlignCenter, AlignCenter, "Use arrow keys");
        canvas_draw_str_aligned(canvas, SCREEN_WIDTH_CENTER, SCREEN_HEIGHT_CENTER + 5 + v_shift, AlignCenter, AlignCenter, "to setup new PIN");
    } else {
        canvas_draw_str_aligned(canvas, SCREEN_WIDTH_CENTER, SCREEN_HEIGHT_CENTER + v_shift, AlignCenter, AlignCenter, "Use arrow keys to enter PIN");
    }
    const uint8_t PIN_ASTERISK_RADIUS = 3;
    const uint8_t PIN_ASTERISK_STEP = (PIN_ASTERISK_RADIUS << 1) + 2;
    if (scene_state->code_length > 0) {
        uint8_t left_start_x = (scene_state->code_length - 1) * PIN_ASTERISK_STEP >> 1;
        for (uint8_t i = 0; i < scene_state->code_length; i++) {
            canvas_draw_disc(
                canvas, 
                SCREEN_WIDTH_CENTER - left_start_x + i * PIN_ASTERISK_STEP, 
                SCREEN_HEIGHT_CENTER + 10, 
                PIN_ASTERISK_RADIUS);
        }
    }
}

bool totp_scene_authenticate_handle_event(PluginEvent* const event, PluginState* plugin_state) {
    if(event->type == EventTypeKey) {
        if (event->input.type == InputTypeLong && event->input.key == InputKeyBack) {
            return false;
        } else if(event->input.type == InputTypePress) {
            SceneState* scene_state = (SceneState *)plugin_state->current_scene_state;

            const uint8_t ARROW_UP_CODE = 2;
            const uint8_t ARROW_RIGHT_CODE = 8;
            const uint8_t ARROW_DOWN_CODE = 11;
            const uint8_t ARROW_LEFT_CODE = 5;

            switch(event->input.key) {
                case InputKeyUp:
                    if (scene_state->code_length < MAX_CODE_LENGTH) {
                        scene_state->code_input[scene_state->code_length] = ARROW_UP_CODE;
                        scene_state->code_length++;
                    }
                    break;
                case InputKeyDown:
                    if (scene_state->code_length < MAX_CODE_LENGTH) {
                        scene_state->code_input[scene_state->code_length] = ARROW_DOWN_CODE;
                        scene_state->code_length++;
                    }
                    break;
                case InputKeyRight:
                    if (scene_state->code_length < MAX_CODE_LENGTH) {
                        scene_state->code_input[scene_state->code_length] = ARROW_RIGHT_CODE;
                        scene_state->code_length++;
                    }
                    break;
                case InputKeyLeft:
                    if (scene_state->code_length < MAX_CODE_LENGTH) {
                        scene_state->code_input[scene_state->code_length] = ARROW_LEFT_CODE;
                        scene_state->code_length++;
                    }
                    break;
                case InputKeyOk:
                    if (plugin_state->crypto_verify_data == NULL) {
                        FURI_LOG_D(LOGGING_TAG, "Generating new IV");
                        furi_hal_random_fill_buf(&plugin_state->base_iv[0], TOTP_IV_SIZE);
                    }

                    memcpy(&plugin_state->iv[0], &plugin_state->base_iv[0], TOTP_IV_SIZE);
                    for (uint8_t i = 0; i < scene_state->code_length; i++) {
                        plugin_state->iv[i] = plugin_state->iv[i] ^ (uint8_t)(scene_state->code_input[i] * (i + 1));
                    }

                    if (plugin_state->crypto_verify_data == NULL) {
                        FURI_LOG_D(LOGGING_TAG, "Generating crypto verify data");
                        plugin_state->crypto_verify_data = malloc(CRYPTO_VERIFY_KEY_LENGTH);
                        plugin_state->crypto_verify_data_length = CRYPTO_VERIFY_KEY_LENGTH;
                        Storage* storage = totp_open_storage();
                        FlipperFormat* config_file = totp_open_config_file(storage);
                        furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, &plugin_state->iv[0]);
                        furi_hal_crypto_encrypt((uint8_t* )CRYPTO_VERIFY_KEY, plugin_state->crypto_verify_data, CRYPTO_VERIFY_KEY_LENGTH);
                        furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);
                        flipper_format_insert_or_update_hex(config_file, TOTP_CONFIG_KEY_BASE_IV, plugin_state->base_iv, TOTP_IV_SIZE);
                        flipper_format_insert_or_update_hex(config_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, plugin_state->crypto_verify_data, CRYPTO_VERIFY_KEY_LENGTH);
                        totp_close_config_file(config_file);
                        totp_close_storage();
                    }

                    uint8_t decrypted_key[CRYPTO_VERIFY_KEY_LENGTH];
                    furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, &plugin_state->iv[0]);
                    furi_hal_crypto_decrypt(plugin_state->crypto_verify_data, &decrypted_key[0], CRYPTO_VERIFY_KEY_LENGTH);
                    furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);

                    bool key_valid = true;
                    for (uint8_t i = 0; i < CRYPTO_VERIFY_KEY_LENGTH && key_valid; i++) {
                        if (decrypted_key[i] != CRYPTO_VERIFY_KEY[i]) key_valid = false;
                    }

                    if (key_valid) {
                        FURI_LOG_D(LOGGING_TAG, "PIN is valid");
                        totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
                    } else {
                        FURI_LOG_D(LOGGING_TAG, "PIN is NOT valid");
                        memset(&scene_state->code_input[0], 0, MAX_CODE_LENGTH);
                        memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
                        scene_state->code_length = 0;

                        DialogMessage* message = dialog_message_alloc();
                        dialog_message_set_buttons(message, "Try again", NULL, NULL);
                        dialog_message_set_header(message, "You entered\ninvalid PIN", SCREEN_WIDTH_CENTER - 25, SCREEN_HEIGHT_CENTER - 5, AlignCenter, AlignCenter);
                        dialog_message_set_icon(message, &I_DolphinCommon_56x48, 72, 17);
                        dialog_message_show(plugin_state->dialogs, message);
                        dialog_message_free(message);
                    }
                    break;
                case InputKeyBack:
                    if (scene_state->code_length > 0) {
                        scene_state->code_input[scene_state->code_length - 1] = 0;
                        scene_state->code_length--;
                    }
                    break;
            }
        }
    }

    return true;
}

void totp_scene_authenticate_deactivate(PluginState* plugin_state) {
    if (plugin_state->current_scene_state == NULL) return;
    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_authenticate_free(PluginState* plugin_state) {
    UNUSED(plugin_state);
}
