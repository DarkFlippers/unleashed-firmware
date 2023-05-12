#include "transmitter.h"
#include "../subghz_remote_app_i.h"

#include <input/input.h>
#include <gui/elements.h>

#define SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH 16
struct SubRemViewRemote {
    View* view;
    SubRemViewRemoteCallback callback;
    void* context;
};
// FIXME: drop
// static char* char_to_str(char* str, int i) {
//     char* converted = malloc(sizeof(char) * i + 1);
//     memcpy(converted, str, i);

//     converted[i] = '\0';

//     return converted;
// }

// TODO: model

typedef struct {
    // FuriString* up_label;
    // FuriString* down_label;
    // FuriString* left_label;
    // FuriString* right_label;
    // FuriString* ok_label;

    char* up_label;
    char* down_label;
    char* left_label;
    char* right_label;
    char* ok_label;

    uint8_t pressed_btn;
    // bool show_button;
    // FuriString* temp_button_id;
    // bool draw_temp_button;
} SubRemViewRemoteModel;

void subrem_view_remote_set_callback(
    SubRemViewRemote* subrem_view_remote,
    SubRemViewRemoteCallback callback,
    void* context) {
    furi_assert(subrem_view_remote);

    subrem_view_remote->callback = callback;
    subrem_view_remote->context = context;
}

void subrem_view_remote_add_data_to_show(
    SubRemViewRemote* subrem_view_remote,
    const char* up_label,
    const char* down_label,
    const char* left_label,
    const char* right_label,
    const char* ok_label) {
    furi_assert(subrem_view_remote);

    with_view_model(
        subrem_view_remote->view,
        SubRemViewRemoteModel * model,
        {
            strncpy(model->up_label, up_label, SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->down_label, down_label, SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->left_label, left_label, SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->right_label, right_label, SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->ok_label, ok_label, SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);

            // model->up_label = char_to_str((char*)up_label, 16);
            // model->down_label = char_to_str((char*)down_label, 16);
            // model->left_label = char_to_str((char*)left_label, 16);
            // model->right_label = char_to_str((char*)right_label, 16);
            // model->ok_label = char_to_str((char*)ok_label, 16);

            // furi_string_set(model->up_label, up_label);
            // furi_string_set(model->down_label, down_label);
            // furi_string_set(model->left_label, left_label);
            // furi_string_set(model->right_label, right_label);
            // furi_string_set(model->ok_label, ok_label);
        },
        true);
}

void subrem_view_remote_draw(Canvas* canvas, SubRemViewRemoteModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    //map found, draw all the things
    canvas_clear(canvas);

    //canvas_set_font(canvas, FontPrimary);
    //canvas_draw_str(canvas, 0, 10, "U: ");
    //canvas_draw_str(canvas, 0, 20, "L: ");
    //canvas_draw_str(canvas, 0, 30, "R: ");
    //canvas_draw_str(canvas, 0, 40, "D: ");
    //canvas_draw_str(canvas, 0, 50, "Ok: ");

    //PNGs are located in assets/icons/SubGHzRemote before compilation

    //Icons for Labels
    //canvas_draw_icon(canvas, 0, 0, &I_SubGHzRemote_LeftAlignedButtons_9x64);
    canvas_draw_icon(canvas, 1, 5, &I_ButtonUp_7x4);
    canvas_draw_icon(canvas, 1, 15, &I_ButtonDown_7x4);
    canvas_draw_icon(canvas, 2, 23, &I_ButtonLeft_4x7);
    canvas_draw_icon(canvas, 2, 33, &I_ButtonRight_4x7);
    canvas_draw_icon(canvas, 0, 42, &I_Ok_btn_9x9);
    canvas_draw_icon(canvas, 0, 53, &I_back_10px);

    //Labels
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 10, model->up_label);
    canvas_draw_str(canvas, 10, 20, model->down_label);
    canvas_draw_str(canvas, 10, 30, model->left_label);
    canvas_draw_str(canvas, 10, 40, model->right_label);
    canvas_draw_str(canvas, 10, 50, model->ok_label);
    // canvas_draw_str(canvas, 10, 10, furi_string_get_cstr(model->up_label));
    // canvas_draw_str(canvas, 10, 10, furi_string_get_cstr(model->up_label));
    // canvas_draw_str(canvas, 10, 10, furi_string_get_cstr(model->up_label));
    // canvas_draw_str(canvas, 10, 10, furi_string_get_cstr(model->up_label));
    // canvas_draw_str(canvas, 10, 10, furi_string_get_cstr(model->up_label));

    canvas_draw_str_aligned(canvas, 11, 62, AlignLeft, AlignBottom, "Hold=Exit.");

    //Status text and indicator
    // canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, app->send_status);

    canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
    switch(model->pressed_btn) {
    case 0:
        break;
    case 1:
        canvas_draw_icon(canvas, 116, 17, &I_Pin_arrow_up_7x9);
        break;
    case 2:
        canvas_draw_icon_ex(canvas, 116, 17, &I_Pin_arrow_up_7x9, IconRotation180);
        break;
    case 3:
        canvas_draw_icon_ex(canvas, 115, 18, &I_Pin_arrow_up_7x9, IconRotation90);
        break;
    case 4:
        canvas_draw_icon_ex(canvas, 115, 18, &I_Pin_arrow_up_7x9, IconRotation270);
        break;
    case 5:
        canvas_draw_icon(canvas, 116, 18, &I_Pin_star_7x7);
        break;
    }

    //Repeat indicator
    //canvas_draw_str_aligned(canvas, 125, 40, AlignRight, AlignBottom, "Repeat:");
    //canvas_draw_icon(canvas, 115, 39, &I_SubGHzRemote_Repeat_12x14);
    //canvas_draw_str_aligned(canvas, 125, 62, AlignRight, AlignBottom, int_to_char(app->repeat));
}

bool subrem_view_remote_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubRemViewRemote* subrem_view_remote = context;

    if(event->key == InputKeyBack && event->type == InputTypeLong) {
        with_view_model(
            subrem_view_remote->view,
            SubRemViewRemoteModel * model,
            {
                strcpy(model->up_label, "N/A");
                strcpy(model->down_label, "N/A");
                strcpy(model->left_label, "N/A");
                strcpy(model->right_label, "N/A");
                strcpy(model->ok_label, "N/A");

                // furi_string_reset(model->up_label);
                // furi_string_reset(model->down_label);
                // furi_string_reset(model->left_label);
                // furi_string_reset(model->right_label);
                // furi_string_reset(model->ok_label);
            },
            false);
        return false;
    } else if(event->key == InputKeyUp) {
        if(event->type == InputTypePress) {
            with_view_model(
                subrem_view_remote->view,
                SubRemViewRemoteModel * model,
                { model->pressed_btn = 1; },
                true);
            return true;
        } else if(event->type == InputTypeRelease) {
            with_view_model(
                subrem_view_remote->view,
                SubRemViewRemoteModel * model,
                { model->pressed_btn = 0; },
                true);
            return true;
        }
    }
    return true;
}

void subrem_view_remote_enter(void* context) {
    furi_assert(context);
}

void subrem_view_remote_exit(void* context) {
    furi_assert(context);
}

SubRemViewRemote* subrem_view_remote_alloc() {
    SubRemViewRemote* subrem_view_remote = malloc(sizeof(SubRemViewRemote));

    // View allocation and configuration
    subrem_view_remote->view = view_alloc();
    view_allocate_model(
        subrem_view_remote->view, ViewModelTypeLocking, sizeof(SubRemViewRemoteModel));
    view_set_context(subrem_view_remote->view, subrem_view_remote);
    view_set_draw_callback(subrem_view_remote->view, (ViewDrawCallback)subrem_view_remote_draw);
    view_set_input_callback(subrem_view_remote->view, subrem_view_remote_input);
    view_set_enter_callback(subrem_view_remote->view, subrem_view_remote_enter);
    view_set_exit_callback(subrem_view_remote->view, subrem_view_remote_exit);

    with_view_model(
        subrem_view_remote->view,
        SubRemViewRemoteModel * model,
        {
            model->up_label = malloc(sizeof(char) * SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH + 1);
            model->down_label = malloc(sizeof(char) * SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH + 1);
            model->left_label = malloc(sizeof(char) * SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH + 1);
            model->right_label = malloc(sizeof(char) * SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH + 1);
            model->ok_label = malloc(sizeof(char) * SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH + 1);

            strcpy(model->up_label, "N/A");
            strcpy(model->down_label, "N/A");
            strcpy(model->left_label, "N/A");
            strcpy(model->right_label, "N/A");
            strcpy(model->ok_label, "N/A");

            // model->up_label = furi_string_alloc();
            // model->down_label = furi_string_alloc();
            // model->left_label = furi_string_alloc();
            // model->right_label = furi_string_alloc();
            // model->ok_label = furi_string_alloc();

            model->pressed_btn = 0;
        },
        true);
    return subrem_view_remote;
}

void subrem_view_remote_free(SubRemViewRemote* subghz_remote) {
    furi_assert(subghz_remote);

    with_view_model(
        subghz_remote->view,
        SubRemViewRemoteModel * model,
        {
            free(model->up_label);
            free(model->down_label);
            free(model->left_label);
            free(model->right_label);
            free(model->ok_label);

            // furi_string_free(model->up_label);
            // furi_string_free(model->down_label);
            // furi_string_free(model->left_label);
            // furi_string_free(model->right_label);
            // furi_string_free(model->ok_label);
        },
        true);
    view_free(subghz_remote->view);
    free(subghz_remote);
}

View* subrem_view_remote_get_view(SubRemViewRemote* subrem_view_remote) {
    furi_assert(subrem_view_remote);
    return subrem_view_remote->view;
}