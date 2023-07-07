#include "text_box.h"
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include <stdint.h>

struct TextBox {
    View* view;

    uint16_t button_held_for_ticks;
};

typedef struct {
    const char* text;
    char* text_pos;
    FuriString* text_formatted;
    int32_t scroll_pos;
    int32_t scroll_num;
    TextBoxFont font;
    TextBoxFocus focus;
    bool formatted;
} TextBoxModel;

static void text_box_process_down(TextBox* text_box, uint8_t lines) {
    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            if(model->scroll_pos < model->scroll_num - lines) {
                model->scroll_pos += lines;
                for(uint8_t i = 0; i < lines; i++) {
                    // Search next line start
                    while(*model->text_pos++ != '\n')
                        ;
                }
            } else if(lines > 1) {
                lines = model->scroll_num - model->scroll_pos - 1;
                model->scroll_pos = model->scroll_num - 1;
                for(uint8_t i = 0; i < lines; i++) {
                    // Search next line start
                    while(*model->text_pos++ != '\n')
                        ;
                }
            }
        },
        true);
}

static void text_box_process_up(TextBox* text_box, uint8_t lines) {
    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            if(model->scroll_pos > lines - 1) {
                model->scroll_pos -= lines;
                for(uint8_t i = 0; i < lines; i++) {
                    // Reach last symbol of previous line
                    model->text_pos--;
                    // Search previous line start
                    while((model->text_pos != model->text) && (*(--model->text_pos) != '\n'))
                        ;
                    if(*model->text_pos == '\n') {
                        model->text_pos++;
                    }
                }
            } else if(lines > 1) {
                lines = model->scroll_pos;
                model->scroll_pos = 0;
                model->text_pos = (char*)model->text;
            }
        },
        true);
}

static void text_box_insert_endline(Canvas* canvas, TextBoxModel* model) {
    size_t i = 0;
    size_t line_width = 0;
    const char* str = model->text;
    size_t line_num = 0;

    const size_t text_width = 120;

    while(str[i] != '\0') {
        char symb = str[i++];
        if(symb != '\n') {
            size_t glyph_width = canvas_glyph_width(canvas, symb);
            if(line_width + glyph_width > text_width) {
                line_num++;
                line_width = 0;
                furi_string_push_back(model->text_formatted, '\n');
            }
            line_width += glyph_width;
        } else {
            line_num++;
            line_width = 0;
        }
        furi_string_push_back(model->text_formatted, symb);
    }
    line_num++;
    model->text = furi_string_get_cstr(model->text_formatted);
    model->text_pos = (char*)model->text;
    if(model->focus == TextBoxFocusEnd && line_num > 5) {
        // Set text position to 5th line from the end
        for(uint8_t i = 0; i < line_num - 5; i++) {
            while(*model->text_pos++ != '\n') {
            };
        }
        model->scroll_num = line_num - 4;
        model->scroll_pos = line_num - 5;
    } else {
        model->scroll_num = MAX(line_num - 4, 0u);
        model->scroll_pos = 0;
    }
}

static void text_box_view_draw_callback(Canvas* canvas, void* _model) {
    TextBoxModel* model = _model;

    canvas_clear(canvas);
    if(model->font == TextBoxFontText) {
        canvas_set_font(canvas, FontSecondary);
    } else if(model->font == TextBoxFontHex) {
        canvas_set_font(canvas, FontKeyboard);
    }

    if(!model->formatted) {
        text_box_insert_endline(canvas, model);
        model->formatted = true;
    }

    elements_slightly_rounded_frame(canvas, 0, 0, 124, 64);
    elements_multiline_text(canvas, 3, 11, model->text_pos);
    elements_scrollbar(canvas, model->scroll_pos, model->scroll_num);
}

static bool text_box_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);

    TextBox* text_box = context;
    bool consumed = false;
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        int32_t scroll_speed = 1;
        if(text_box->button_held_for_ticks > 5) {
            if(text_box->button_held_for_ticks % 2) {
                scroll_speed = 0;
            } else {
                scroll_speed = text_box->button_held_for_ticks > 9 ? 5 : 3;
            }
        }

        if(event->key == InputKeyDown) {
            text_box_process_down(text_box, scroll_speed);
            consumed = true;
        } else if(event->key == InputKeyUp) {
            text_box_process_up(text_box, scroll_speed);
            consumed = true;
        }

        text_box->button_held_for_ticks++;
    } else if(event->type == InputTypeRelease) {
        text_box->button_held_for_ticks = 0;
        consumed = true;
    }
    return consumed;
}

TextBox* text_box_alloc() {
    TextBox* text_box = malloc(sizeof(TextBox));
    text_box->view = view_alloc();
    view_set_context(text_box->view, text_box);
    view_allocate_model(text_box->view, ViewModelTypeLocking, sizeof(TextBoxModel));
    view_set_draw_callback(text_box->view, text_box_view_draw_callback);
    view_set_input_callback(text_box->view, text_box_view_input_callback);

    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            model->text = NULL;
            model->text_formatted = furi_string_alloc_set("");
            model->formatted = false;
            model->font = TextBoxFontText;
        },
        true);

    return text_box;
}

void text_box_free(TextBox* text_box) {
    furi_assert(text_box);

    with_view_model(
        text_box->view, TextBoxModel * model, { furi_string_free(model->text_formatted); }, true);
    view_free(text_box->view);
    free(text_box);
}

View* text_box_get_view(TextBox* text_box) {
    furi_assert(text_box);
    return text_box->view;
}

void text_box_reset(TextBox* text_box) {
    furi_assert(text_box);

    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            model->text = NULL;
            furi_string_set(model->text_formatted, "");
            model->font = TextBoxFontText;
            model->focus = TextBoxFocusStart;
        },
        true);
}

void text_box_set_text(TextBox* text_box, const char* text) {
    furi_assert(text_box);
    furi_assert(text);

    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            model->text = text;
            furi_string_reset(model->text_formatted);
            furi_string_reserve(model->text_formatted, strlen(text));
            model->formatted = false;
        },
        true);
}

void text_box_set_font(TextBox* text_box, TextBoxFont font) {
    furi_assert(text_box);

    with_view_model(
        text_box->view, TextBoxModel * model, { model->font = font; }, true);
}

void text_box_set_focus(TextBox* text_box, TextBoxFocus focus) {
    furi_assert(text_box);

    with_view_model(
        text_box->view, TextBoxModel * model, { model->focus = focus; }, true);
}
