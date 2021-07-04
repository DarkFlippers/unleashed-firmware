#include "string-element.h"
#include <gui/elements.h>

StringElement::StringElement() {
}

StringElement::~StringElement() {
}

void StringElement::draw(Canvas* canvas) {
    if(text) {
        canvas_set_font(canvas, font);
        elements_multiline_text_aligned(canvas, x, y, horizontal, vertical, text);
    }
}

bool StringElement::input(InputEvent* event) {
    return false;
}

void StringElement::set_text(
    const char* _text,
    uint8_t _x,
    uint8_t _y,
    Align _horizontal,
    Align _vertical,
    Font _font) {
    lock_model();
    text = _text;
    x = _x;
    y = _y;
    horizontal = _horizontal;
    vertical = _vertical;
    font = _font;
    unlock_model(true);
}
