#include "field_editor.h"
#include "../fuzzer_i.h"

#include <input/input.h>
#include <gui/elements.h>
#include <toolbox/hex.h>

#define FIELD_EDITOR_V2

#define GUI_DISPLAY_WIDTH 128
#define GUI_DISPLAY_HEIGHT 64

#define GUI_DISPLAY_HORIZONTAL_CENTER 64
#define GUI_DISPLAY_VERTICAL_CENTER 32

#define UID_STR_LENGTH 25

#ifdef FIELD_EDITOR_V2
#define EDITOR_STRING_Y 38
#else
#define EDITOR_STRING_Y 50
#endif

struct FuzzerViewFieldEditor {
    View* view;
    FuzzerViewFieldEditorCallback callback;
    void* context;
};

typedef struct {
    uint8_t* uid;
    uint8_t uid_size;

    FuriString* uid_str;

    uint8_t index;
    bool lo;
    bool allow_edit;
} FuzzerViewFieldEditorModel;

void fuzzer_view_field_editor_set_callback(
    FuzzerViewFieldEditor* view_edit,
    FuzzerViewFieldEditorCallback callback,
    void* context) {
    furi_assert(view_edit);

    view_edit->callback = callback;
    view_edit->context = context;
}

void fuzzer_view_field_editor_reset_data(
    FuzzerViewFieldEditor* view_edit,
    const FuzzerPayload* new_uid,
    bool allow_edit) {
    furi_assert(view_edit);
    furi_assert(new_uid->data);

    with_view_model(
        view_edit->view,
        FuzzerViewFieldEditorModel * model,
        {
            memcpy(model->uid, new_uid->data, new_uid->data_size);
            model->index = 0;
            model->lo = false;
            model->uid_size = new_uid->data_size;
            model->allow_edit = allow_edit;
        },
        true);
}

void fuzzer_view_field_editor_get_uid(FuzzerViewFieldEditor* view_edit, FuzzerPayload* output_uid) {
    furi_assert(view_edit);
    furi_assert(output_uid);
    with_view_model(
        view_edit->view,
        FuzzerViewFieldEditorModel * model,
        {
            output_uid->data_size = model->uid_size;
            memcpy(output_uid->data, model->uid, model->uid_size);
        },
        true);
}

uint8_t fuzzer_view_field_editor_get_index(FuzzerViewFieldEditor* view_edit) {
    furi_assert(view_edit);
    uint8_t index;
    with_view_model(
        view_edit->view, FuzzerViewFieldEditorModel * model, { index = model->index; }, true);
    return index;
}

void fuzzer_view_field_editor_draw(Canvas* canvas, FuzzerViewFieldEditorModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

#ifdef FIELD_EDITOR_V2

    canvas_set_font(canvas, FontSecondary);
    if(model->allow_edit) {
        canvas_draw_icon(canvas, 2, 4, &I_ButtonLeft_4x7);
        canvas_draw_icon(canvas, 8, 4, &I_ButtonRight_4x7);

        canvas_draw_icon_ex(canvas, 62, 3, &I_Pin_arrow_up_7x9, IconRotation180);
        canvas_draw_icon(canvas, 69, 3, &I_Pin_arrow_up_7x9);

        canvas_draw_str(canvas, 14, 10, "select byte");
        canvas_draw_str(canvas, 79, 10, "adjust byte");
    } else {
        canvas_draw_icon(canvas, 35, 4, &I_ButtonLeft_4x7);
        canvas_draw_icon(canvas, 41, 4, &I_ButtonRight_4x7);
        canvas_draw_str(canvas, 49, 10, "select byte");
    }

    char msg_index[18];
    canvas_set_font(canvas, FontPrimary);
    snprintf(msg_index, sizeof(msg_index), "Field index : %d", model->index);

    canvas_draw_str_aligned(
        canvas, GUI_DISPLAY_HORIZONTAL_CENTER, 24, AlignCenter, AlignBottom, msg_index);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_icon(canvas, 4, 52, &I_Pin_back_arrow_10x8);
    canvas_draw_icon(canvas, 85, 52, &I_Ok_btn_9x9);

    canvas_draw_str(canvas, 16, 60, "Back");
    canvas_draw_str(canvas, 96, 60, "Attack");
#else
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas,
        GUI_DISPLAY_HORIZONTAL_CENTER,
        5,
        AlignCenter,
        AlignTop,
        "Left and right: select byte");
    canvas_draw_str_aligned(
        canvas,
        GUI_DISPLAY_HORIZONTAL_CENTER,
        15,
        AlignCenter,
        AlignTop,
        "Up and down: adjust byte");

    char msg_index[18];
    canvas_set_font(canvas, FontPrimary);
    snprintf(msg_index, sizeof(msg_index), "Field index : %d", model->index);
    canvas_draw_str_aligned(
        canvas, GUI_DISPLAY_HORIZONTAL_CENTER, 28, AlignCenter, AlignTop, msg_index);
#endif
    // ####### Editor #######
    FuriString* temp_s = model->uid_str;
    canvas_set_font(canvas, FontSecondary);

    furi_string_reset(temp_s);
    for(int i = -3; i != 0; i++) {
        if(0 <= (model->index + i)) {
            furi_string_cat_printf(temp_s, "%02X ", model->uid[model->index + i]);
        }
    }
    canvas_draw_str_aligned(
        canvas, 52, EDITOR_STRING_Y, AlignRight, AlignBottom, furi_string_get_cstr(temp_s));

    furi_string_reset(temp_s);
    for(int i = 1; i != 4; i++) {
        if((model->index + i) < model->uid_size) {
            furi_string_cat_printf(temp_s, " %02X", model->uid[model->index + i]);
        }
    }
    canvas_draw_str_aligned(
        canvas, 77, EDITOR_STRING_Y, AlignLeft, AlignBottom, furi_string_get_cstr(temp_s));

    canvas_set_font(canvas, FontPrimary);

    furi_string_reset(temp_s);
    furi_string_cat_printf(temp_s, "<%02X>", model->uid[model->index]);
    canvas_draw_str_aligned(
        canvas,
        GUI_DISPLAY_HORIZONTAL_CENTER,
        EDITOR_STRING_Y,
        AlignCenter,
        AlignBottom,
        furi_string_get_cstr(temp_s));

    uint16_t w = canvas_string_width(canvas, furi_string_get_cstr(temp_s));
    w -= 11; // '<' & '>'
    w /= 2;

    if(model->allow_edit) {
        if(model->lo) {
            canvas_draw_line(
                canvas,
                GUI_DISPLAY_HORIZONTAL_CENTER + 1,
                EDITOR_STRING_Y + 2,
                GUI_DISPLAY_HORIZONTAL_CENTER + w,
                EDITOR_STRING_Y + 2);
        } else {
            canvas_draw_line(
                canvas,
                GUI_DISPLAY_HORIZONTAL_CENTER - w,
                EDITOR_STRING_Y + 2,
                GUI_DISPLAY_HORIZONTAL_CENTER - 1,
                EDITOR_STRING_Y + 2);
        }
    } else {
        // canvas_draw_line(
        //     canvas,
        //     GUI_DISPLAY_HORIZONTAL_CENTER - w,
        //     EDITOR_STRING_Y + 2,
        //     GUI_DISPLAY_HORIZONTAL_CENTER + w,
        //     EDITOR_STRING_Y + 2);
    }
    // ####### Editor #######
}

bool fuzzer_view_field_editor_input(InputEvent* event, void* context) {
    furi_assert(context);
    FuzzerViewFieldEditor* view_edit = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        view_edit->callback(FuzzerCustomEventViewFieldEditorBack, view_edit->context);
        return true;
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        view_edit->callback(FuzzerCustomEventViewFieldEditorOk, view_edit->context);
        return true;
    } else if(event->key == InputKeyLeft) {
        with_view_model(
            view_edit->view,
            FuzzerViewFieldEditorModel * model,
            {
                if(event->type == InputTypeShort) {
                    if(!model->allow_edit) {
                        model->lo = false;
                    }
                    if(model->index > 0 || model->lo) {
                        if(!model->lo) {
                            model->index--;
                        }
                        model->lo = !model->lo;
                    }
                } else if(event->type == InputTypeLong) {
                    model->index = 0;
                    model->lo = false;
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyRight) {
        with_view_model(
            view_edit->view,
            FuzzerViewFieldEditorModel * model,
            {
                if(event->type == InputTypeShort) {
                    if(!model->allow_edit) {
                        model->lo = true;
                    }
                    if(model->index < (model->uid_size - 1) || !model->lo) {
                        if(model->lo) {
                            model->index++;
                        }
                        model->lo = !model->lo;
                    }
                } else if(event->type == InputTypeLong) {
                    model->index = model->uid_size - 1;
                    model->lo = true;
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyUp) {
        with_view_model(
            view_edit->view,
            FuzzerViewFieldEditorModel * model,
            {
                if(event->type == InputTypeShort && model->allow_edit) {
                    if(model->lo) {
                        model->uid[model->index] = (model->uid[model->index] & 0xF0) |
                                                   ((model->uid[model->index] + 1) & 0x0F);
                    } else {
                        model->uid[model->index] = ((model->uid[model->index] + 0x10) & 0xF0) |
                                                   (model->uid[model->index] & 0x0F);
                    }
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyDown) {
        with_view_model(
            view_edit->view,
            FuzzerViewFieldEditorModel * model,
            {
                if(event->type == InputTypeShort && model->allow_edit) {
                    if(model->lo) {
                        model->uid[model->index] = (model->uid[model->index] & 0xF0) |
                                                   ((model->uid[model->index] - 1) & 0x0F);
                    } else {
                        model->uid[model->index] = ((model->uid[model->index] - 0x10) & 0xF0) |
                                                   (model->uid[model->index] & 0x0F);
                    }
                }
            },
            true);
        return true;
    }

    return true;
}

void fuzzer_view_field_editor_enter(void* context) {
    furi_assert(context);
}

void fuzzer_view_field_editor_exit(void* context) {
    furi_assert(context);
}

FuzzerViewFieldEditor* fuzzer_view_field_editor_alloc() {
    FuzzerViewFieldEditor* view_edit = malloc(sizeof(FuzzerViewFieldEditor));

    // View allocation and configuration
    view_edit->view = view_alloc();
    view_allocate_model(view_edit->view, ViewModelTypeLocking, sizeof(FuzzerViewFieldEditorModel));
    view_set_context(view_edit->view, view_edit);
    view_set_draw_callback(view_edit->view, (ViewDrawCallback)fuzzer_view_field_editor_draw);
    view_set_input_callback(view_edit->view, fuzzer_view_field_editor_input);
    view_set_enter_callback(view_edit->view, fuzzer_view_field_editor_enter);
    view_set_exit_callback(view_edit->view, fuzzer_view_field_editor_exit);

    with_view_model(
        view_edit->view,
        FuzzerViewFieldEditorModel * model,
        {
            model->uid_str = furi_string_alloc();
            model->uid = malloc(fuzzer_proto_get_max_data_size());
        },
        true);

    return view_edit;
}

void fuzzer_view_field_editor_free(FuzzerViewFieldEditor* view_edit) {
    furi_assert(view_edit);

    with_view_model(
        view_edit->view,
        FuzzerViewFieldEditorModel * model,
        {
            furi_string_free(model->uid_str);
            free(model->uid);
        },
        true);
    view_free(view_edit->view);
    free(view_edit);
}

View* fuzzer_view_field_editor_get_view(FuzzerViewFieldEditor* view_edit) {
    furi_assert(view_edit);
    return view_edit->view;
}