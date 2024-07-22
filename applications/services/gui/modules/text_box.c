#include "text_box.h"
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include <stdint.h>

#define TEXT_BOX_TEXT_WIDTH           (120)
#define TEXT_BOX_TEXT_HEIGHT          (56)
#define TEXT_BOX_MAX_LINES_PER_SCREEN (10)

#define TEXT_BOX_LINES_SCROLL_SPEED_MEDIUM     (3)
#define TEXT_BOX_LINES_SCROLL_SPEED_FAST       (5)
#define TEXT_BOX_LINES_SCROLL_SPEED_SATURATION (9)

struct TextBox {
    View* view;

    uint16_t button_held_for_ticks;
};

typedef struct {
    TextBoxFont font;
    TextBoxFocus focus;
    const char* text;

    int32_t scroll_pos;
    int32_t scroll_num;
    int32_t lines_on_screen;

    int32_t line_offset;
    int32_t text_offset;
    FuriString* text_on_screen;
    FuriString* text_line;

    bool formatted;
} TextBoxModel;

static void text_box_process_down(TextBox* text_box, uint8_t lines) {
    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            if(model->scroll_pos + lines < model->scroll_num) {
                model->scroll_pos += lines;
            } else {
                if(model->scroll_num > 0) {
                    model->scroll_pos = model->scroll_num - 1;
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
            if(model->scroll_pos - lines > 0) {
                model->scroll_pos -= lines;
            } else {
                model->scroll_pos = 0;
            }
        },
        true);
}

static bool text_box_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);

    TextBox* text_box = context;
    bool consumed = false;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        int32_t scroll_speed = 1;
        if(text_box->button_held_for_ticks > TEXT_BOX_LINES_SCROLL_SPEED_FAST) {
            if(text_box->button_held_for_ticks % 2) {
                scroll_speed = 0;
            } else {
                scroll_speed =
                    (text_box->button_held_for_ticks > TEXT_BOX_LINES_SCROLL_SPEED_SATURATION) ?
                        TEXT_BOX_LINES_SCROLL_SPEED_FAST :
                        TEXT_BOX_LINES_SCROLL_SPEED_MEDIUM;
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

static bool text_box_end_of_text_reached(TextBoxModel* model) {
    return model->text[model->text_offset] == '\0';
}

static bool text_box_start_of_text_reached(TextBoxModel* model) {
    return model->text_offset == 0;
}

static void text_box_seek_next_line(Canvas* canvas, TextBoxModel* model) {
    size_t line_width = 0;

    while(!text_box_end_of_text_reached(model)) {
        char symb = model->text[model->text_offset];
        if(symb == '\n') {
            model->text_offset++;
            break;
        } else {
            size_t glyph_width = canvas_glyph_width(canvas, symb);
            if(line_width + glyph_width > TEXT_BOX_TEXT_WIDTH) {
                break;
            }
            line_width += glyph_width;
            model->text_offset++;
        }
    }
}

static void text_box_seek_end_of_prev_line(TextBoxModel* model) {
    do {
        if(text_box_start_of_text_reached(model)) break;
        model->text_offset--;
        if(text_box_start_of_text_reached(model)) break;
        if(model->text[model->text_offset] == '\n') {
            model->text_offset--;
        }
    } while(false);
}

static void text_box_seek_prev_paragraph(TextBoxModel* model) {
    while(!text_box_start_of_text_reached(model)) {
        if(model->text[model->text_offset] == '\n') {
            model->text_offset++;
            break;
        }
        model->text_offset--;
    }
}

static void text_box_seek_prev_line(Canvas* canvas, TextBoxModel* model) {
    int32_t start_text_offset = model->text_offset;

    text_box_seek_end_of_prev_line(model);
    text_box_seek_prev_paragraph(model);

    int32_t current_text_offset = model->text_offset;
    while(true) {
        text_box_seek_next_line(canvas, model);
        if(model->text_offset == start_text_offset) {
            break;
        }
        current_text_offset = model->text_offset;
    }
    model->text_offset = current_text_offset;
}

static void text_box_move_line_offset(Canvas* canvas, TextBoxModel* model, int32_t line_offset) {
    if(line_offset >= 0) {
        for(int32_t i = 0; i < line_offset; i++) {
            text_box_seek_next_line(canvas, model);
        }
    } else {
        for(int32_t i = 0; i < (-line_offset); i++) {
            text_box_seek_prev_line(canvas, model);
        }
    }
}

static void text_box_update_screen_text(Canvas* canvas, TextBoxModel* model) {
    furi_string_reset(model->text_on_screen);
    furi_string_reset(model->text_line);

    int32_t start_text_offset = model->text_offset;

    for(int32_t i = 0; i < model->lines_on_screen; i++) {
        int32_t current_line_text_offset = model->text_offset;
        text_box_seek_next_line(canvas, model);
        int32_t next_line_text_offset = model->text_offset;
        furi_string_set_strn(
            model->text_line,
            &model->text[current_line_text_offset],
            next_line_text_offset - current_line_text_offset);
        size_t str_len = furi_string_size(model->text_line);
        if(furi_string_get_char(model->text_line, str_len - 1) != '\n') {
            furi_string_push_back(model->text_line, '\n');
        }
        furi_string_cat(model->text_on_screen, model->text_line);

        if(text_box_end_of_text_reached(model)) break;
        current_line_text_offset = next_line_text_offset;
    }

    model->text_offset = start_text_offset;
}

static void text_box_update_text_on_screen(Canvas* canvas, TextBoxModel* model) {
    int32_t line_offset = model->scroll_pos - model->line_offset;
    text_box_move_line_offset(canvas, model, line_offset);
    text_box_update_screen_text(canvas, model);
    model->line_offset = model->scroll_pos;
}

static void text_box_prepare_model(Canvas* canvas, TextBoxModel* model) {
    int32_t lines_num = 0;
    model->text_offset = 0;
    model->scroll_num = 0;
    model->scroll_pos = 0;
    model->line_offset = 0;
    model->lines_on_screen = TEXT_BOX_TEXT_HEIGHT / canvas_current_font_height(canvas);

    // Cache text offset to quick final text offset update if TextBoxFocusEnd is set
    int32_t window_offset[TEXT_BOX_MAX_LINES_PER_SCREEN] = {};
    do {
        window_offset[lines_num % model->lines_on_screen] = model->text_offset;
        text_box_seek_next_line(canvas, model);
        lines_num++;
    } while(!text_box_end_of_text_reached(model));
    model->text_offset = 0;
    lines_num++;

    if(model->focus == TextBoxFocusEnd) {
        if(lines_num > model->lines_on_screen) {
            model->text_offset = window_offset[(lines_num - 1) % model->lines_on_screen];
        }
    }

    if(lines_num > model->lines_on_screen) {
        model->scroll_num = lines_num - model->lines_on_screen;
        model->scroll_pos = (model->focus == TextBoxFocusEnd) ? model->scroll_num - 1 : 0;
    }

    text_box_update_screen_text(canvas, model);
    model->line_offset = model->scroll_pos;
}

static void text_box_view_draw_callback(Canvas* canvas, void* _model) {
    TextBoxModel* model = _model;

    if(!model->text) {
        return;
    }

    canvas_clear(canvas);
    if(model->font == TextBoxFontText) {
        canvas_set_font(canvas, FontSecondary);
    } else if(model->font == TextBoxFontHex) {
        canvas_set_font(canvas, FontKeyboard);
    }

    if(!model->formatted) {
        text_box_prepare_model(canvas, model);
        model->formatted = true;
    }

    elements_slightly_rounded_frame(canvas, 0, 0, 124, 64);
    elements_scrollbar(canvas, model->scroll_pos, model->scroll_num);

    if(model->line_offset != model->scroll_pos) {
        text_box_update_text_on_screen(canvas, model);
    }
    elements_multiline_text(canvas, 3, 11, furi_string_get_cstr(model->text_on_screen));
}

TextBox* text_box_alloc(void) {
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
            model->text_on_screen = furi_string_alloc();
            model->text_line = furi_string_alloc();
            model->formatted = false;
            model->font = TextBoxFontText;
        },
        true);

    return text_box;
}

void text_box_free(TextBox* text_box) {
    furi_check(text_box);

    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            furi_string_free(model->text_on_screen);
            furi_string_free(model->text_line);
        },
        true);
    view_free(text_box->view);
    free(text_box);
}

View* text_box_get_view(TextBox* text_box) {
    furi_check(text_box);
    return text_box->view;
}

void text_box_reset(TextBox* text_box) {
    furi_check(text_box);

    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            model->text = NULL;
            model->font = TextBoxFontText;
            model->focus = TextBoxFocusStart;
            furi_string_reset(model->text_line);
            furi_string_reset(model->text_on_screen);
            model->line_offset = 0;
            model->text_offset = 0;
            model->lines_on_screen = 0;
            model->scroll_num = 0;
            model->scroll_pos = 0;
            model->formatted = false;
        },
        true);
}

void text_box_set_text(TextBox* text_box, const char* text) {
    furi_check(text_box);
    furi_check(text);

    with_view_model(
        text_box->view,
        TextBoxModel * model,
        {
            model->text = text;
            model->formatted = false;
        },
        true);
}

void text_box_set_font(TextBox* text_box, TextBoxFont font) {
    furi_check(text_box);

    with_view_model(text_box->view, TextBoxModel * model, { model->font = font; }, true);
}

void text_box_set_focus(TextBox* text_box, TextBoxFocus focus) {
    furi_check(text_box);

    with_view_model(text_box->view, TextBoxModel * model, { model->focus = focus; }, true);
}
