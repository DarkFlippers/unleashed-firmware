#include "remote.h"
#include "../subghz_remote_app_i.h"

#include <input/input.h>
#include <gui/elements.h>

#define SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH 12

struct SubRemViewRemote {
    View* view;
    SubRemViewRemoteCallback callback;
    void* context;
};

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

    SubRemViewRemoteState state;

    uint8_t pressed_btn;
} SubRemViewRemoteModel;

void subrem_view_remote_set_callback(
    SubRemViewRemote* subrem_view_remote,
    SubRemViewRemoteCallback callback,
    void* context) {
    furi_assert(subrem_view_remote);

    subrem_view_remote->callback = callback;
    subrem_view_remote->context = context;
}

void subrem_view_remote_add_data_to_show(SubRemViewRemote* subrem_view_remote, const char** labels) {
    furi_assert(subrem_view_remote);

    with_view_model(
        subrem_view_remote->view,
        SubRemViewRemoteModel * model,
        {
            strncpy(model->up_label, labels[0], SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->down_label, labels[1], SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->left_label, labels[2], SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->right_label, labels[3], SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);
            strncpy(model->ok_label, labels[4], SUBREM_VIEW_REMOTE_MAX_LABEL_LENGTH);

            // furi_string_set(model->up_label, up_label);
            // furi_string_set(model->down_label, down_label);
            // furi_string_set(model->left_label, left_label);
            // furi_string_set(model->right_label, right_label);
            // furi_string_set(model->ok_label, ok_label);
        },
        true);
}

void subrem_view_remote_set_state(
    SubRemViewRemote* subrem_view_remote,
    SubRemViewRemoteState state,
    uint8_t presed_btn) {
    furi_assert(subrem_view_remote);
    with_view_model(
        subrem_view_remote->view,
        SubRemViewRemoteModel * model,
        {
            model->state = state;
            model->pressed_btn = presed_btn;
        },
        true);
}

void subrem_view_remote_draw(Canvas* canvas, SubRemViewRemoteModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_clear(canvas);

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
    canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);

    if(model->state == SubRemViewRemoteStateIdle) {
        canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, "Idle");
    } else {
        switch(model->state) {
        case SubRemViewRemoteStateSending:
            canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, "Send");
            break;
        case SubRemViewRemoteStateLoading:
            canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, "Load");
            break;
        default:
#if FURI_DEBUG
            canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, "Wrong_state");
#endif
            break;
        }

        switch(model->pressed_btn) {
        case SubRemSubKeyNameUp:
            canvas_draw_icon(canvas, 116, 17, &I_Pin_arrow_up_7x9);
            break;
        case SubRemSubKeyNameDown:
            canvas_draw_icon_ex(canvas, 116, 17, &I_Pin_arrow_up_7x9, IconRotation180);
            break;
        case SubRemSubKeyNameLeft:
            canvas_draw_icon_ex(canvas, 115, 18, &I_Pin_arrow_up_7x9, IconRotation270);
            break;
        case SubRemSubKeyNameRight:
            canvas_draw_icon_ex(canvas, 115, 18, &I_Pin_arrow_up_7x9, IconRotation90);
            break;
        case SubRemSubKeyNameOk:
            canvas_draw_icon(canvas, 116, 18, &I_Pin_star_7x7);
            break;
        }
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
            },
            false);
        subrem_view_remote->callback(SubRemCustomEventViewRemoteBack, subrem_view_remote->context);
        return true;
    } else if(event->key == InputKeyBack && event->type == InputTypeShort) {
        with_view_model(
            subrem_view_remote->view,
            SubRemViewRemoteModel * model,
            { model->pressed_btn = 0; },
            true);
        subrem_view_remote->callback(
            SubRemCustomEventViewRemoteForcedStop, subrem_view_remote->context);
        return true;
    } else if(event->key == InputKeyBack) {
        return true;
    }
    // BACK button processing end

    if(event->key == InputKeyUp && event->type == InputTypePress) {
        subrem_view_remote->callback(
            SubRemCustomEventViewRemoteStartUP, subrem_view_remote->context);
        return true;
    } else if(event->key == InputKeyDown && event->type == InputTypePress) {
        subrem_view_remote->callback(
            SubRemCustomEventViewRemoteStartDOWN, subrem_view_remote->context);
        return true;
    } else if(event->key == InputKeyLeft && event->type == InputTypePress) {
        subrem_view_remote->callback(
            SubRemCustomEventViewRemoteStartLEFT, subrem_view_remote->context);
        return true;
    } else if(event->key == InputKeyRight && event->type == InputTypePress) {
        subrem_view_remote->callback(
            SubRemCustomEventViewRemoteStartRIGHT, subrem_view_remote->context);
        return true;
    } else if(event->key == InputKeyOk && event->type == InputTypePress) {
        subrem_view_remote->callback(
            SubRemCustomEventViewRemoteStartOK, subrem_view_remote->context);
        return true;
    } else if(event->type == InputTypeRelease) {
        subrem_view_remote->callback(SubRemCustomEventViewRemoteStop, subrem_view_remote->context);
        return true;
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
            model->state = SubRemViewRemoteStateIdle;

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

            // model->up_label = furi_string_alloc_set_str("N/A");
            // model->down_label = furi_string_alloc_set_str("N/A");
            // model->left_label = furi_string_alloc_set_str("N/A");
            // model->right_label = furi_string_alloc_set_str("N/A");
            // model->ok_label = furi_string_alloc_set_str("N/A");

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