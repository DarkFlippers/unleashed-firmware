#include "button_element.h"
#include <gui/elements.h>

ButtonElement::ButtonElement() {
}

ButtonElement::~ButtonElement() {
}

void ButtonElement::draw(Canvas* canvas) {
    if(text != nullptr) {
        canvas_set_font(canvas, FontSecondary);
        switch(type) {
        case Type::Left:
            elements_button_left(canvas, text);
            break;
        case Type::Center:
            elements_button_center(canvas, text);
            break;
        case Type::Right:
            elements_button_right(canvas, text);
            break;
        }
    }
}

bool ButtonElement::input(InputEvent* event) {
    bool consumed = false;
    if(event->type == InputTypeShort && callback != nullptr) {
        switch(type) {
        case Type::Left:
            if(event->key == InputKeyLeft) {
                callback(context);
                consumed = true;
            }
            break;
        case Type::Center:
            if(event->key == InputKeyOk) {
                callback(context);
                consumed = true;
            }
            break;
        case Type::Right:
            if(event->key == InputKeyRight) {
                callback(context);
                consumed = true;
            }
            break;
        }
    }

    return consumed;
}

void ButtonElement::set_type(Type _type, const char* _text) {
    lock_model();
    type = _type;
    text = _text;
    unlock_model(true);
}

void ButtonElement::set_callback(void* _context, ButtonElementCallback _callback) {
    context = _context;
    callback = _callback;
}
