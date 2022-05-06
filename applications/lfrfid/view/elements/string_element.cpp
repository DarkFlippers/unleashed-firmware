#include "string_element.h"
#include <gui/elements.h>

StringElement::StringElement() {
}

StringElement::~StringElement() {
}

void StringElement::draw(Canvas* canvas) {
    if(text) {
        string_t line;
        string_init(line);
        string_set_str(line, text);

        canvas_set_font(canvas, font);
        if(fit_width != 0) {
            elements_string_fit_width(canvas, line, fit_width);
        }
        elements_multiline_text_aligned(canvas, x, y, horizontal, vertical, string_get_cstr(line));

        string_clear(line);
    }
}

bool StringElement::input(InputEvent* /* event */) {
    return false;
}

void StringElement::set_text(
    const char* _text,
    uint8_t _x,
    uint8_t _y,
    uint8_t _fit_w,
    Align _horizontal,
    Align _vertical,
    Font _font) {
    lock_model();
    text = _text;
    x = _x;
    y = _y;
    fit_width = _fit_w;
    horizontal = _horizontal;
    vertical = _vertical;
    font = _font;
    unlock_model(true);
}
