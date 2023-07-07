#include <gui/gui.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <totp_icons.h>
#include <roll_value.h>
#include "totp_scene_generate_token.h"
#include "../../../types/token_info.h"
#include "../../../types/common.h"
#include "../../constants.h"
#include "../../../services/config/config.h"
#include "../../scene_director.h"
#include "../token_menu/totp_scene_token_menu.h"
#include "../../../features_config.h"
#include "../../../workers/generate_totp_code/generate_totp_code.h"
#include "../../../workers/usb_type_code/usb_type_code.h"
#ifdef TOTP_BADBT_TYPE_ENABLED
#include "../../../workers/bt_type_code/bt_type_code.h"
#endif
#include "../../fonts/active_font.h"

#define PROGRESS_BAR_MARGIN (3)
#define PROGRESS_BAR_HEIGHT (4)

typedef struct {
    uint8_t progress_bar_x;
    uint8_t progress_bar_width;
    uint8_t code_total_length;
    uint8_t code_offset_x;
    uint8_t code_offset_x_inc;
    uint8_t code_offset_y;
} UiPrecalculatedDimensions;

typedef struct {
    char last_code[TOTP_TOKEN_DIGITS_MAX_COUNT + 1];
    TotpUsbTypeCodeWorkerContext* usb_type_code_worker_context;
    NotificationMessage const** notification_sequence_new_token;
    NotificationMessage const** notification_sequence_automation;
    FuriMutex* last_code_update_sync;
    TotpGenerateCodeWorkerContext* generate_code_worker_context;
    UiPrecalculatedDimensions ui_precalculated_dimensions;
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
    if(scene_state->notification_sequence_automation == NULL) {
        uint8_t i = 0;
        uint8_t length = 3;
        if(plugin_state->notification_method & NotificationMethodVibro) {
            length += 2;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            length += 6;
        }

        scene_state->notification_sequence_automation = malloc(sizeof(void*) * length);
        furi_check(scene_state->notification_sequence_automation != NULL);

        scene_state->notification_sequence_automation[i++] = &message_blue_255;
        if(plugin_state->notification_method & NotificationMethodVibro) {
            scene_state->notification_sequence_automation[i++] = &message_vibro_on;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            scene_state->notification_sequence_automation[i++] = &message_note_d5; //-V525
            scene_state->notification_sequence_automation[i++] = &message_delay_50;
            scene_state->notification_sequence_automation[i++] = &message_note_e4;
            scene_state->notification_sequence_automation[i++] = &message_delay_50;
            scene_state->notification_sequence_automation[i++] = &message_note_f3;
        }

        scene_state->notification_sequence_automation[i++] = &message_delay_50;

        if(plugin_state->notification_method & NotificationMethodVibro) {
            scene_state->notification_sequence_automation[i++] = &message_vibro_off;
        }

        if(plugin_state->notification_method & NotificationMethodSound) {
            scene_state->notification_sequence_automation[i++] = &message_sound_off;
        }

        scene_state->notification_sequence_automation[i++] = NULL;
    }

    return (NotificationSequence*)scene_state->notification_sequence_automation;
}

static void update_totp_params(PluginState* const plugin_state, size_t token_index) {
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    if(totp_token_info_iterator_go_to(iterator_context, token_index)) {
        totp_generate_code_worker_notify(
            scene_state->generate_code_worker_context, TotpGenerateCodeWorkerEventForceUpdate);
    }
}

static void draw_totp_code(Canvas* const canvas, const PluginState* const plugin_state) {
    const SceneState* scene_state = plugin_state->current_scene_state;
    const TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    uint8_t code_length = totp_token_info_iterator_get_current_token(iterator_context)->digits;
    uint8_t offset_x = scene_state->ui_precalculated_dimensions.code_offset_x;
    const FONT_INFO* current_font;
    switch(plugin_state->selected_font) {
    case 0:
        current_font = &modeNine_15ptFontInfo;
        break;
    case 1:
        current_font = &redHatMono_16ptFontInfo;
        break;
    case 2:
        current_font = &bedstead_17ptFontInfo;
        break;
    case 3:
        current_font = &zector_18ptFontInfo;
        break;
    case 4:
        current_font = &_712Serif_24ptFontInfo;
        break;
    case 5:
        current_font = &graph35pix_12ptFontInfo;
        break;
    case 6:
        current_font = &karmaFuture_14ptFontInfo;
        break;
    case 7:
        current_font = &funclimbingDemo_18ptFontInfo;
        break;
    case 8:
        current_font = &dPComic_18ptFontInfo;
        break;
    case 9:
        current_font = &pixelFlag_18ptFontInfo;
        break;
    default:
        current_font = &modeNine_15ptFontInfo;
        break;
    }
    uint8_t char_width = current_font->charInfo[0].width;
    uint8_t offset_x_inc = scene_state->ui_precalculated_dimensions.code_offset_x_inc;
    for(uint8_t i = 0; i < code_length; i++) {
        char ch = scene_state->last_code[i];
        if(ch >= current_font->startChar && ch <= current_font->endChar) {
            uint8_t char_index = ch - current_font->startChar;
            canvas_draw_xbm(
                canvas,
                offset_x,
                scene_state->ui_precalculated_dimensions.code_offset_y,
                char_width,
                current_font->height,
                &current_font->data[current_font->charInfo[char_index].offset]);
        }

        offset_x += offset_x_inc;
    }
}

static void on_new_token_code_generated(bool time_left, void* context) {
    const PluginState* plugin_state = context;
    const TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    if(totp_token_info_iterator_get_total_count(iterator_context) == 0) {
        return;
    }

    SceneState* scene_state = plugin_state->current_scene_state;
    const TokenInfo* current_token = totp_token_info_iterator_get_current_token(iterator_context);

    const FONT_INFO* current_font;
    switch(plugin_state->selected_font) {
    case 0:
        current_font = &modeNine_15ptFontInfo;
        break;
    case 1:
        current_font = &redHatMono_16ptFontInfo;
        break;
    case 2:
        current_font = &bedstead_17ptFontInfo;
        break;
    case 3:
        current_font = &zector_18ptFontInfo;
        break;
    case 4:
        current_font = &_712Serif_24ptFontInfo;
        break;
    case 5:
        current_font = &graph35pix_12ptFontInfo;
        break;
    case 6:
        current_font = &karmaFuture_14ptFontInfo;
        break;
    case 7:
        current_font = &funclimbingDemo_18ptFontInfo;
        break;
    case 8:
        current_font = &dPComic_18ptFontInfo;
        break;
    case 9:
        current_font = &pixelFlag_18ptFontInfo;
        break;
    default:
        current_font = &modeNine_15ptFontInfo;
        break;
    }

    uint8_t char_width = current_font->charInfo[0].width;
    scene_state->ui_precalculated_dimensions.code_total_length =
        current_token->digits * (char_width + current_font->spacePixels);
    scene_state->ui_precalculated_dimensions.code_offset_x =
        (SCREEN_WIDTH - scene_state->ui_precalculated_dimensions.code_total_length) >> 1;
    scene_state->ui_precalculated_dimensions.code_offset_x_inc =
        char_width + current_font->spacePixels;
    scene_state->ui_precalculated_dimensions.code_offset_y =
        SCREEN_HEIGHT_CENTER - (current_font->height >> 1);

    if(time_left) {
        notification_message(
            plugin_state->notification_app,
            get_notification_sequence_new_token(plugin_state, plugin_state->current_scene_state));
    }
}

static void on_code_lifetime_updated_generated(float code_lifetime_percent, void* context) {
    SceneState* scene_state = context;
    scene_state->ui_precalculated_dimensions.progress_bar_width =
        (uint8_t)((float)(SCREEN_WIDTH - (PROGRESS_BAR_MARGIN << 1)) * code_lifetime_percent);
    scene_state->ui_precalculated_dimensions.progress_bar_x =
        ((SCREEN_WIDTH - (PROGRESS_BAR_MARGIN << 1) -
          scene_state->ui_precalculated_dimensions.progress_bar_width) >>
         1) +
        PROGRESS_BAR_MARGIN;
}

void totp_scene_generate_token_activate(PluginState* plugin_state) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);

    plugin_state->current_scene_state = scene_state;
    FURI_LOG_D(LOGGING_TAG, "Timezone set to: %f", (double)plugin_state->timezone_offset);

    scene_state->last_code_update_sync = furi_mutex_alloc(FuriMutexTypeNormal);
    if(plugin_state->automation_method & AutomationMethodBadUsb) {
        scene_state->usb_type_code_worker_context = totp_usb_type_code_worker_start(
            scene_state->last_code,
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
            scene_state->last_code,
            TOTP_TOKEN_DIGITS_MAX_COUNT + 1,
            scene_state->last_code_update_sync);
    }
#endif
    const TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    scene_state->generate_code_worker_context = totp_generate_code_worker_start(
        scene_state->last_code,
        totp_token_info_iterator_get_current_token(iterator_context),
        scene_state->last_code_update_sync,
        plugin_state->timezone_offset,
        plugin_state->iv);

    totp_generate_code_worker_set_code_generated_handler(
        scene_state->generate_code_worker_context, &on_new_token_code_generated, plugin_state);

    totp_generate_code_worker_set_lifetime_changed_handler(
        scene_state->generate_code_worker_context,
        &on_code_lifetime_updated_generated,
        scene_state);

    update_totp_params(
        plugin_state, totp_token_info_iterator_get_current_token_index(iterator_context));
}

void totp_scene_generate_token_render(Canvas* const canvas, PluginState* plugin_state) {
    const TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    if(totp_token_info_iterator_get_total_count(iterator_context) == 0) {
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

    const SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    canvas_set_font(canvas, FontPrimary);
    const char* token_name_cstr =
        furi_string_get_cstr(totp_token_info_iterator_get_current_token(iterator_context)->name);
    uint16_t token_name_width = canvas_string_width(canvas, token_name_cstr);
    if(SCREEN_WIDTH - token_name_width > 18) {
        canvas_draw_str_aligned(
            canvas,
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER - 20,
            AlignCenter,
            AlignCenter,
            token_name_cstr);
    } else {
        canvas_draw_str_aligned(
            canvas, 9, SCREEN_HEIGHT_CENTER - 20, AlignLeft, AlignCenter, token_name_cstr);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 0, SCREEN_HEIGHT_CENTER - 24, 9, 9);
        canvas_draw_box(canvas, SCREEN_WIDTH - 10, SCREEN_HEIGHT_CENTER - 24, 9, 9);
        canvas_set_color(canvas, ColorBlack);
    }

    draw_totp_code(canvas, plugin_state);

    canvas_draw_box(
        canvas,
        scene_state->ui_precalculated_dimensions.progress_bar_x,
        SCREEN_HEIGHT - PROGRESS_BAR_MARGIN - PROGRESS_BAR_HEIGHT,
        scene_state->ui_precalculated_dimensions.progress_bar_width,
        PROGRESS_BAR_HEIGHT);
    if(totp_token_info_iterator_get_total_count(iterator_context) > 1) {
        canvas_draw_icon(canvas, 0, SCREEN_HEIGHT_CENTER - 24, &I_totp_arrow_left_8x9);
        canvas_draw_icon(
            canvas, SCREEN_WIDTH - 8, SCREEN_HEIGHT_CENTER - 24, &I_totp_arrow_right_8x9);
    }

#ifdef TOTP_AUTOMATION_ICONS_ENABLED
    if(plugin_state->automation_method & AutomationMethodBadUsb) {
        canvas_draw_icon(
            canvas,
#ifdef TOTP_BADBT_TYPE_ENABLED
            SCREEN_WIDTH_CENTER -
                (plugin_state->automation_method & AutomationMethodBadBt ? 33 : 15),
#else
            SCREEN_WIDTH_CENTER - 15,
#endif

            SCREEN_HEIGHT_CENTER + 12,
            &I_hid_usb_31x9);
    }

#ifdef TOTP_BADBT_TYPE_ENABLED
    if(plugin_state->automation_method & AutomationMethodBadBt &&
       plugin_state->bt_type_code_worker_context != NULL &&
       totp_bt_type_code_worker_is_advertising(plugin_state->bt_type_code_worker_context)) {
        canvas_draw_icon(
            canvas,
            SCREEN_WIDTH_CENTER +
                (plugin_state->automation_method & AutomationMethodBadUsb ? 2 : -15),
            SCREEN_HEIGHT_CENTER + 12,
            &I_hid_ble_31x9);
    }
#endif
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
            const TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            totp_usb_type_code_worker_notify(
                scene_state->usb_type_code_worker_context,
                TotpUsbTypeCodeWorkerEventType,
                totp_token_info_iterator_get_current_token(iterator_context)->automation_features);
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
            const TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            totp_bt_type_code_worker_notify(
                plugin_state->bt_type_code_worker_context,
                TotpBtTypeCodeWorkerEventType,
                totp_token_info_iterator_get_current_token(iterator_context)->automation_features);
            notification_message(
                plugin_state->notification_app,
                get_notification_sequence_automation(plugin_state, scene_state));
            return true;
        }
#endif
    } else if(event->input.type == InputTypePress || event->input.type == InputTypeRepeat) {
        switch(event->input.key) {
        case InputKeyUp:
            break;
        case InputKeyDown:
            break;
        case InputKeyRight: {
            const TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            size_t current_token_index =
                totp_token_info_iterator_get_current_token_index(iterator_context);
            totp_roll_value_size_t(
                &current_token_index,
                1,
                0,
                totp_token_info_iterator_get_total_count(iterator_context) - 1,
                RollOverflowBehaviorRoll);

            update_totp_params(plugin_state, current_token_index);
            break;
        }
        case InputKeyLeft: {
            const TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            size_t current_token_index =
                totp_token_info_iterator_get_current_token_index(iterator_context);
            totp_roll_value_size_t(
                &current_token_index,
                -1,
                0,
                totp_token_info_iterator_get_total_count(iterator_context) - 1,
                RollOverflowBehaviorRoll);

            update_totp_params(plugin_state, current_token_index);
            break;
        }
        case InputKeyOk:
            break;
        case InputKeyBack:
            break;
        default:
            break;
        }
    } else if(event->input.type == InputTypeRelease && event->input.key == InputKeyOk) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu);
    }

    return true;
}

void totp_scene_generate_token_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;

    totp_generate_code_worker_stop(scene_state->generate_code_worker_context);

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

    if(scene_state->notification_sequence_automation != NULL) {
        free(scene_state->notification_sequence_automation);
    }

    furi_mutex_free(scene_state->last_code_update_sync);

    free(scene_state);
    plugin_state->current_scene_state = NULL;
}
