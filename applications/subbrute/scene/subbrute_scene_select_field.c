#include "subbrute_scene_select_field.h"

void center_displayed_key(SubBruteState* context, uint8_t index) {
    const char* key_cstr = string_get_cstr(context->key);
    uint8_t str_index = (index * 3);

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

    string_reset(context->notification_msg);
    string_set_str(context->notification_msg, display_menu);
}

void subbrute_scene_select_field_on_enter(SubBruteState* context) {
    string_clear(context->notification_msg);
}

void subbrute_scene_select_field_on_exit(SubBruteState* context) {
    UNUSED(context);
}

void subbrute_scene_select_field_on_tick(SubBruteState* context) {
    UNUSED(context);
}

void subbrute_scene_select_field_on_event(SubBruteEvent event, SubBruteState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            //const char* key_cstr = string_get_cstr(context->key);

            // don't look, it's ugly but I'm a python dev so...
            /*uint8_t nb_bytes = 0;
            for(uint8_t i = 0; i < strlen(key_cstr); i++) {
                if(' ' == key_cstr[i]) {
                    nb_bytes++;
                }
            }*/

            switch(event.key) {
            case InputKeyDown:
            case InputKeyUp:
                break;
            case InputKeyLeft:
                if(context->key_index > 0) {
                    context->key_index--;
                }
                break;
            case InputKeyRight:
                if(context->key_index < 7) {
                    context->key_index++;
                }
                break;
            case InputKeyOk:
                string_reset(context->notification_msg);
                context->current_scene = SceneAttack;
                break;
            case InputKeyBack:
                string_reset(context->notification_msg);
                context->current_scene = SceneSelectFile;
                break;
            }
            //FURI_LOG_D(TAG, "Position: %d/%d", context->key_index, nb_bytes);
        }
    }
}

void subbrute_scene_select_field_on_draw(Canvas* canvas, SubBruteState* context) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    //canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "use < > to select field");

    char msg_index[18];
    snprintf(msg_index, sizeof(msg_index), "Field index : %d", context->key_index);
    canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignTop, msg_index);

    center_displayed_key(context, context->key_index);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 64, 40, AlignCenter, AlignTop, string_get_cstr(context->notification_msg));
}
