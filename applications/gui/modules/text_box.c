#include "text_box.h"
#include "gui/canvas.h"
#include <m-string.h>
#include <furi.h>
#include <gui/elements.h>
#include <stdint.h>

struct TextBox {
    View* view;
    void* context;
    TextBoxExitCallback callback;
};

typedef struct {
    const char* text;
    char* text_pos;
    string_t text_formatted;
    size_t scroll_pos;
    size_t scroll_num;
    TextBoxFont font;
    bool formatted;
} TextBoxModel;

static void text_box_process_down(TextBox* text_box) {
    with_view_model(
        text_box->view, (TextBoxModel * model) {
            if(model->scroll_pos < model->scroll_num - 1) {
                model->scroll_pos++;
                // Search next line start
                while(*model->text_pos++ != '\n')
                    ;
            }
            return true;
        });
}

static void text_box_process_up(TextBox* text_box) {
    with_view_model(
        text_box->view, (TextBoxModel * model) {
            if(model->scroll_pos > 0) {
                model->scroll_pos--;
                // Reach last symbol of previous line
                model->text_pos--;
                // Search prevous line start
                while((model->text_pos != model->text) && (*(--model->text_pos) != '\n'))
                    ;
                if(*model->text_pos == '\n') {
                    model->text_pos++;
                }
            }
            return true;
        });
}

static void text_box_process_back(TextBox* text_box) {
    if(text_box->callback) {
        text_box->callback(text_box->context);
    }
}

static void text_box_insert_endline(Canvas* canvas, TextBoxModel* model) {
    size_t i = 0;
    size_t line_width = 0;
    const char* str = model->text;
    size_t line_num = 0;

    const size_t text_width = 140;

    while(str[i] != '\0') {
        char symb = str[i++];
        if(symb != '\n') {
            line_width += canvas_glyph_width(canvas, symb) + 1;
            if(line_width > text_width) {
                line_num++;
                line_width = 0;
                string_push_back(model->text_formatted, '\n');
            }
        } else {
            line_num++;
            line_width = 0;
        }
        string_push_back(model->text_formatted, symb);
    }
    line_num++;
    model->text = string_get_cstr(model->text_formatted);
    model->text_pos = (char*)model->text;
    model->scroll_num = MAX(line_num - 4, 0);
    model->scroll_pos = 0;
}

static void text_box_view_draw_callback(Canvas* canvas, void* _model) {
    TextBoxModel* model = _model;

    if(!model->formatted) {
        text_box_insert_endline(canvas, model);
        model->formatted = true;
    }

    canvas_clear(canvas);
    elements_slightly_rounded_frame(canvas, 0, 0, 124, 64);
    if(model->font == TextBoxFontText) {
        canvas_set_font(canvas, FontSecondary);
    } else if(model->font == TextBoxFontHex) {
        canvas_set_font(canvas, FontKeyboard);
    }
    elements_multiline_text(canvas, 3, 11, model->text_pos);
    elements_scrollbar(canvas, model->scroll_pos, model->scroll_num);
}

static bool text_box_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);

    TextBox* text_box = context;
    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyDown) {
            text_box_process_down(text_box);
            consumed = true;
        } else if(event->key == InputKeyUp) {
            text_box_process_up(text_box);
            consumed = true;
        } else if(event->key == InputKeyBack) {
            text_box_process_back(text_box);
            consumed = true;
        }
    }
    return consumed;
}

TextBox* text_box_alloc() {
    TextBox* text_box = furi_alloc(sizeof(TextBox));
    text_box->view = view_alloc();
    view_set_context(text_box->view, text_box);
    view_allocate_model(text_box->view, ViewModelTypeLocking, sizeof(TextBoxModel));
    view_set_draw_callback(text_box->view, text_box_view_draw_callback);
    view_set_input_callback(text_box->view, text_box_view_input_callback);

    with_view_model(
        text_box->view, (TextBoxModel * model) {
            model->text = NULL;
            string_init_set_str(model->text_formatted, "");
            model->formatted = false;
            model->font = TextBoxFontText;
            return true;
        });

    return text_box;
}

void text_box_free(TextBox* text_box) {
    furi_assert(text_box);

    with_view_model(
        text_box->view, (TextBoxModel * model) {
            string_clear(model->text_formatted);
            return true;
        });
    view_free(text_box->view);
    free(text_box);
}

View* text_box_get_view(TextBox* text_box) {
    furi_assert(text_box);
    return text_box->view;
}

void text_box_clean(TextBox* text_box) {
    furi_assert(text_box);

    with_view_model(
        text_box->view, (TextBoxModel * model) {
            model->text = NULL;
            string_set_str(model->text_formatted, "");
            model->font = TextBoxFontText;
            return true;
        });
    text_box->context = NULL;
    text_box->callback = NULL;
}

void text_box_set_text(TextBox* text_box, const char* text) {
    furi_assert(text_box);
    furi_assert(text);

    with_view_model(
        text_box->view, (TextBoxModel * model) {
            model->text = text;
            string_reserve(model->text_formatted, strlen(text));
            model->formatted = false;
            return true;
        });
}

void text_box_set_font(TextBox* text_box, TextBoxFont font) {
    furi_assert(text_box);

    with_view_model(
        text_box->view, (TextBoxModel * model) {
            model->font = font;
            return true;
        });
}

void text_box_set_context(TextBox* text_box, void* context) {
    furi_assert(text_box);
    text_box->context = context;
}

void text_box_set_exit_callback(TextBox* text_box, TextBoxExitCallback callback) {
    furi_assert(text_box);
    text_box->callback = callback;
}
