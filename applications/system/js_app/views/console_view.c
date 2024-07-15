#include "../js_app_i.h"
#include "console_font.h"

#define CONSOLE_LINES   8
#define CONSOLE_CHAR_W  5
#define LINE_BREAKS_MAX 3
#define LINE_LEN_MAX    (128 / CONSOLE_CHAR_W)

struct JsConsoleView {
    View* view;
};

typedef struct {
    FuriString* text[CONSOLE_LINES];
} JsConsoleViewModel;

static void console_view_draw_callback(Canvas* canvas, void* _model) {
    JsConsoleViewModel* model = _model;

    canvas_set_color(canvas, ColorBlack);
    canvas_set_custom_u8g2_font(canvas, u8g2_font_spleen5x8_mr);
    uint8_t line_h = canvas_current_font_height(canvas);

    for(size_t i = 0; i < CONSOLE_LINES; i++) {
        canvas_draw_str(canvas, 0, (i + 1) * line_h - 1, furi_string_get_cstr(model->text[i]));
        if(furi_string_size(model->text[i]) > LINE_LEN_MAX) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 128 - 7, (i + 1) * line_h - 1, "...");
            canvas_set_custom_u8g2_font(canvas, u8g2_font_spleen5x8_mr);
        }
    }
}

static bool console_view_input_callback(InputEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);
    return false;
}

void console_view_push_line(JsConsoleView* console_view, const char* text, bool line_trimmed) {
    with_view_model(
        console_view->view,
        JsConsoleViewModel * model,
        {
            FuriString* str_temp = model->text[0];
            for(size_t i = 0; i < CONSOLE_LINES - 1; i++) {
                model->text[i] = model->text[i + 1];
            }
            if(!line_trimmed) {
                furi_string_printf(str_temp, "%.*s", LINE_LEN_MAX, text);
            } else {
                // Leave some space for dots
                furi_string_printf(str_temp, "%.*s  ", LINE_LEN_MAX - 1, text);
            }
            model->text[CONSOLE_LINES - 1] = str_temp;
        },
        true);
}

void console_view_print(JsConsoleView* console_view, const char* text) {
    char line_buf[LINE_LEN_MAX + 1];
    uint8_t line_buf_cnt = 0;
    uint8_t utf8_bytes_left = 0;
    uint8_t line_break_cnt = 0;
    bool line_trim = false;

    for(size_t i = 0; i < strlen(text); i++) {
        if(text[i] & 0x80) { // UTF8 or another non-ascii character byte
            if(utf8_bytes_left > 0) {
                utf8_bytes_left--;
                if(utf8_bytes_left == 0) {
                    line_buf[line_buf_cnt++] = '?';
                }
            } else {
                if((text[i] & 0xE0) == 0xC0) {
                    utf8_bytes_left = 1;
                } else if((text[i] & 0xF0) == 0xE0) {
                    utf8_bytes_left = 2;
                } else if((text[i] & 0xF8) == 0xF0) {
                    utf8_bytes_left = 3;
                } else {
                    line_buf[line_buf_cnt++] = '?';
                }
            }
        } else {
            if(utf8_bytes_left > 0) {
                utf8_bytes_left = 0;
                line_buf[line_buf_cnt++] = '?';
                if(line_buf_cnt >= LINE_LEN_MAX) {
                    line_break_cnt++;
                    if(line_break_cnt >= LINE_BREAKS_MAX) {
                        line_trim = true;
                        break;
                    }
                    line_buf[line_buf_cnt] = '\0';
                    console_view_push_line(console_view, line_buf, false);
                    line_buf_cnt = 1;
                    line_buf[0] = ' ';
                }
            }

            if(text[i] == '\n') {
                line_buf[line_buf_cnt] = '\0';
                line_buf_cnt = 0;
                console_view_push_line(console_view, line_buf, false);
            } else {
                line_buf[line_buf_cnt++] = text[i];
            }

            if(line_buf_cnt >= LINE_LEN_MAX) {
                line_break_cnt++;
                if(line_break_cnt >= LINE_BREAKS_MAX) {
                    line_trim = true;
                    break;
                }
                line_buf[line_buf_cnt] = '\0';
                console_view_push_line(console_view, line_buf, false);
                line_buf_cnt = 1;
                line_buf[0] = ' ';
            }
        }
    }
    if(line_buf_cnt > 0) {
        line_buf[line_buf_cnt] = '\0';
        console_view_push_line(console_view, line_buf, line_trim);
    }
}

JsConsoleView* console_view_alloc(void) {
    JsConsoleView* console_view = malloc(sizeof(JsConsoleView));
    console_view->view = view_alloc();
    view_set_draw_callback(console_view->view, console_view_draw_callback);
    view_set_input_callback(console_view->view, console_view_input_callback);
    view_allocate_model(console_view->view, ViewModelTypeLocking, sizeof(JsConsoleViewModel));

    with_view_model(
        console_view->view,
        JsConsoleViewModel * model,
        {
            for(size_t i = 0; i < CONSOLE_LINES; i++) {
                model->text[i] = furi_string_alloc();
            }
        },
        true);
    return console_view;
}

void console_view_free(JsConsoleView* console_view) {
    with_view_model(
        console_view->view,
        JsConsoleViewModel * model,
        {
            for(size_t i = 0; i < CONSOLE_LINES; i++) {
                furi_string_free(model->text[i]);
            }
        },
        false);
    view_free(console_view->view);
    free(console_view);
}

View* console_view_get_view(JsConsoleView* console_view) {
    return console_view->view;
}
