#include "flipfrid_scene_select_field.h"

void flipfrid_center_displayed_key(FlipFridState* context, uint8_t index) {
    char key_cstr[18];
    uint8_t key_len = 18;
    uint8_t str_index = (index * 3);
    int data_len = sizeof(context->data) / sizeof(context->data[0]);
    int key_index = 0;

    if(context->proto == EM4100) {
        key_len = 16;
    }
    if(context->proto == PAC) {
        key_len = 13;
    }
    if(context->proto == H10301) {
        key_len = 10;
    }

    for(uint8_t i = 0; i < data_len; i++) {
        if(context->data[i] < 9) {
            key_index +=
                snprintf(&key_cstr[key_index], key_len - key_index, "0%X ", context->data[i]);
        } else {
            key_index +=
                snprintf(&key_cstr[key_index], key_len - key_index, "%X ", context->data[i]);
        }
    }

    char display_menu[17] = {
        'X', 'X', ' ', 'X', 'X', ' ', '<', 'X', 'X', '>', ' ', 'X', 'X', ' ', 'X', 'X', '\0'};

    if(index > 1) {
        display_menu[0] = key_cstr[str_index - 6];
        display_menu[1] = key_cstr[str_index - 5];
    } else {
        display_menu[0] = ' ';
        display_menu[1] = ' ';
    }

    if(index > 0) {
        display_menu[3] = key_cstr[str_index - 3];
        display_menu[4] = key_cstr[str_index - 2];
    } else {
        display_menu[3] = ' ';
        display_menu[4] = ' ';
    }

    display_menu[7] = key_cstr[str_index];
    display_menu[8] = key_cstr[str_index + 1];

    if((str_index + 4) <= (uint8_t)strlen(key_cstr)) {
        display_menu[11] = key_cstr[str_index + 3];
        display_menu[12] = key_cstr[str_index + 4];
    } else {
        display_menu[11] = ' ';
        display_menu[12] = ' ';
    }

    if((str_index + 8) <= (uint8_t)strlen(key_cstr)) {
        display_menu[14] = key_cstr[str_index + 6];
        display_menu[15] = key_cstr[str_index + 7];
    } else {
        display_menu[14] = ' ';
        display_menu[15] = ' ';
    }

    furi_string_reset(context->notification_msg);
    furi_string_set(context->notification_msg, display_menu);
}

void flipfrid_scene_select_field_on_enter(FlipFridState* context) {
    furi_string_reset(context->notification_msg);
}

void flipfrid_scene_select_field_on_exit(FlipFridState* context) {
    UNUSED(context);
}

void flipfrid_scene_select_field_on_tick(FlipFridState* context) {
    UNUSED(context);
}

void flipfrid_scene_select_field_on_event(FlipFridEvent event, FlipFridState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            const char* key_cstr = furi_string_get_cstr(context->data_str);
            int data_len = sizeof(context->data) / sizeof(context->data[0]);

            // don't look, it's ugly but I'm a python dev so...
            uint8_t nb_bytes = 0;
            for(uint8_t i = 0; i < strlen(key_cstr); i++) {
                if(' ' == key_cstr[i]) {
                    nb_bytes++;
                }
            }

            switch(event.key) {
            case InputKeyDown:
                for(uint8_t i = 0; i < data_len; i++) {
                    if(context->key_index == i) {
                        context->data[i] = (context->data[i] - 1);
                    }
                }
                break;
            case InputKeyUp:
                for(uint8_t i = 0; i < data_len; i++) {
                    if(context->key_index == i) {
                        context->data[i] = (context->data[i] + 1);
                    }
                }
                break;
            case InputKeyLeft:
                if(context->key_index > 0) {
                    context->key_index = context->key_index - 1;
                }
                break;
            case InputKeyRight:
                if(context->key_index < nb_bytes) {
                    context->key_index = context->key_index + 1;
                }
                break;
            case InputKeyOk:
                furi_string_reset(context->notification_msg);
                context->current_scene = SceneAttack;
                break;
            case InputKeyBack:
                context->key_index = 0;
                furi_string_reset(context->notification_msg);
                context->current_scene = SceneSelectFile;
                break;
            default:
                break;
            }
            FURI_LOG_D(TAG, "Position: %d/%d", context->key_index, nb_bytes);
        }
    }
}

void flipfrid_scene_select_field_on_draw(Canvas* canvas, FlipFridState* context) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    //canvas_draw_frame(canvas, 0, 0, 128, 64);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 12, 5, AlignLeft, AlignTop, "Left and right: select byte");
    canvas_draw_str_aligned(canvas, 12, 15, AlignLeft, AlignTop, "Up and down: adjust byte");

    char msg_index[18];
    canvas_set_font(canvas, FontPrimary);
    snprintf(msg_index, sizeof(msg_index), "Field index : %d", context->key_index);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, msg_index);

    flipfrid_center_displayed_key(context, context->key_index);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 64, 45, AlignCenter, AlignTop, furi_string_get_cstr(context->notification_msg));
}
