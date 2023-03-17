#include "totp_scene_authenticate.h"
#include <dialogs/dialogs.h>
#include <totp_icons.h>
#include "../../../types/common.h"
#include "../../constants.h"
#include "../../../services/config/config.h"
#include "../../scene_director.h"
#include "../../totp_scenes_enum.h"
#include "../../../services/crypto/crypto.h"
#include "../../../types/user_pin_codes.h"

#define MAX_CODE_LENGTH TOTP_IV_SIZE
static const uint8_t PIN_ASTERISK_RADIUS = 3;
static const uint8_t PIN_ASTERISK_STEP = (PIN_ASTERISK_RADIUS << 1) + 2;

typedef struct {
    TotpUserPinCode code_input[MAX_CODE_LENGTH];
    uint8_t code_length;
} SceneState;

void totp_scene_authenticate_init(PluginState* plugin_state) {
    memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
}

void totp_scene_authenticate_activate(PluginState* plugin_state) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    scene_state->code_length = 0;
    memset(&scene_state->code_input[0], 0, MAX_CODE_LENGTH);
    plugin_state->current_scene_state = scene_state;
    memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
}

void totp_scene_authenticate_render(Canvas* const canvas, PluginState* plugin_state) {
    const SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    int v_shift = 0;
    if(scene_state->code_length > 0) {
        v_shift = -10;
    }

    if(plugin_state->crypto_verify_data == NULL) {
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER - 10 + v_shift,
            AlignCenter,
            AlignCenter,
            "Use arrow keys");
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER + 5 + v_shift,
            AlignCenter,
            AlignCenter,
            "to setup new PIN");
    } else {
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER + v_shift,
            AlignCenter,
            AlignCenter,
            "Use arrow keys to enter PIN");
    }

    if(scene_state->code_length > 0) {
        uint8_t left_start_x = ((scene_state->code_length - 1) * PIN_ASTERISK_STEP) >> 1;
        for(uint8_t i = 0; i < scene_state->code_length; i++) {
            canvas_draw_disc(
                canvas,
                SCREEN_WIDTH_CENTER - left_start_x + i * PIN_ASTERISK_STEP,
                SCREEN_HEIGHT_CENTER + 10,
                PIN_ASTERISK_RADIUS);
        }
    }
}

bool totp_scene_authenticate_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    if(event->input.type == InputTypeLong && event->input.key == InputKeyBack) {
        return false;
    }

    if(event->input.type != InputTypePress) {
        return true;
    }

    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    switch(event->input.key) {
    case InputKeyUp:
        if(scene_state->code_length < MAX_CODE_LENGTH) {
            scene_state->code_input[scene_state->code_length] = PinCodeArrowUp;
            scene_state->code_length++;
        }
        break;
    case InputKeyDown:
        if(scene_state->code_length < MAX_CODE_LENGTH) {
            scene_state->code_input[scene_state->code_length] = PinCodeArrowDown;
            scene_state->code_length++;
        }
        break;
    case InputKeyRight:
        if(scene_state->code_length < MAX_CODE_LENGTH) {
            scene_state->code_input[scene_state->code_length] = PinCodeArrowRight;
            scene_state->code_length++;
        }
        break;
    case InputKeyLeft:
        if(scene_state->code_length < MAX_CODE_LENGTH) {
            scene_state->code_input[scene_state->code_length] = PinCodeArrowLeft;
            scene_state->code_length++;
        }
        break;
    case InputKeyOk:
        totp_crypto_seed_iv(plugin_state, &scene_state->code_input[0], scene_state->code_length);

        if(totp_crypto_verify_key(plugin_state)) {
            FURI_LOG_D(LOGGING_TAG, "PIN is valid");
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
        } else {
            FURI_LOG_D(LOGGING_TAG, "PIN is NOT valid");
            memset(&scene_state->code_input[0], 0, MAX_CODE_LENGTH);
            memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
            scene_state->code_length = 0;

            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_buttons(message, "Try again", NULL, NULL);
            dialog_message_set_header(
                message,
                "You entered\ninvalid PIN",
                SCREEN_WIDTH_CENTER - 25,
                SCREEN_HEIGHT_CENTER - 5,
                AlignCenter,
                AlignCenter);
            dialog_message_set_icon(message, &I_DolphinCommon_56x48, 72, 17);
            dialog_message_show(plugin_state->dialogs_app, message);
            dialog_message_free(message);
        }
        break;
    case InputKeyBack:
        if(scene_state->code_length > 0) {
            scene_state->code_input[scene_state->code_length - 1] = 0;
            scene_state->code_length--;
        }
        break;
    default:
        break;
    }

    return true;
}

void totp_scene_authenticate_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;
    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_authenticate_free(const PluginState* plugin_state) {
    UNUSED(plugin_state);
}
