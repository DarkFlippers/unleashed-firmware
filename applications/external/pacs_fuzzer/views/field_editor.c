#include "field_editor.h"
#include "../fuzzer_i.h"

#include <input/input.h>
#include <gui/elements.h>
#include <toolbox/hex.h>

#define UID_STR_LENGTH 25
#define EDITOR_STRING_Y 50

struct FuzzerViewFieldEditor {
    View* view;
    FuzzerViewFieldEditorCallback callback;
    void* context;
};

// TODO model
typedef struct {
    uint8_t* uid;
    uint8_t uid_size;
    uint8_t index;
    FuriString* uid_str;
    bool lo;
} FuzzerViewFieldEditorModel;

void fuzzer_view_field_editor_set_callback(
    FuzzerViewFieldEditor* view_edit,
    FuzzerViewFieldEditorCallback callback,
    void* context) {
    furi_assert(view_edit);

    view_edit->callback = callback;
    view_edit->context = context;
}

void fuzzer_view_field_editor_draw(Canvas* canvas, FuzzerViewFieldEditorModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignTop, "Left and right: select byte");
    canvas_draw_str_aligned(canvas, 64, 15, AlignCenter, AlignTop, "Up and down: adjust byte");

    char msg_index[18];
    canvas_set_font(canvas, FontPrimary);
    snprintf(msg_index, sizeof(msg_index), "Field index : %d", model->index);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, msg_index);

    // ####### Editor #######
    FuriString* temp_s = model->uid_str;
    canvas_set_font(canvas, FontSecondary);

    furi_string_reset(temp_s);
    for(int i = -3; i != 0; i++) {
        if(0 <= (model->index + i)) {
            furi_string_cat_printf(temp_s, "%2X ", model->uid[model->index + i]);
        }
    }
    canvas_draw_str_aligned(
        canvas, 52, EDITOR_STRING_Y, AlignRight, AlignBottom, furi_string_get_cstr(temp_s));

    furi_string_reset(temp_s);
    for(int i = 1; i != 4; i++) {
        if((model->index + i) < model->uid_size) {
            furi_string_cat_printf(temp_s, " %2X", model->uid[model->index + i]);
        }
    }
    canvas_draw_str_aligned(
        canvas, 77, EDITOR_STRING_Y, AlignLeft, AlignBottom, furi_string_get_cstr(temp_s));

    canvas_set_font(canvas, FontPrimary);

    furi_string_reset(temp_s);
    furi_string_cat_printf(temp_s, "<%02X>", model->uid[model->index]);
    canvas_draw_str_aligned(
        canvas, 64, EDITOR_STRING_Y, AlignCenter, AlignBottom, furi_string_get_cstr(temp_s));

    uint16_t w = canvas_string_width(canvas, furi_string_get_cstr(temp_s));
    w -= 11; // '<' & '>'
    w /= 2;

    if(model->lo) {
        canvas_draw_line(canvas, 64 + 1, EDITOR_STRING_Y + 2, 64 + w, EDITOR_STRING_Y + 2);
    } else {
        canvas_draw_line(canvas, 64 - w, EDITOR_STRING_Y + 2, 64 - 1, EDITOR_STRING_Y + 2);
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
                if(event->type == InputTypeShort) {
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
                if(event->type == InputTypeShort) {
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
    // TODO delete only for debug
    // FuzzerViewFieldEditor* view_edit = context;
    // uint8_t temp[8] = {
    //     0x12,
    //     0x34,
    //     0x56,
    //     0x78,
    //     0x90,
    //     0xAB,
    //     0xCD,
    //     0xEF,
    // };
    // with_view_model(
    //     view_edit->view,
    //     FuzzerViewFieldEditorModel * model,
    //     {
    //         memcpy(model->uid, &temp, 8);

    //         // memset(model->uid, 0xCC, 8);
    //         model->index = 0;
    //         model->uid_size = 8;
    //     },
    //     true);
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

            model->uid = malloc(8);
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