#include "icon_element.h"

IconElement::IconElement() {
}

IconElement::~IconElement() {
}

void IconElement::draw(Canvas* canvas) {
    if(icon != NULL) {
        canvas_draw_icon(canvas, x, y, icon);
    }
}

bool IconElement::input(InputEvent* /* event */) {
    return false;
}

void IconElement::set_icon(uint8_t _x, uint8_t _y, const Icon* _icon) {
    lock_model();
    icon = _icon;
    x = _x;
    y = _y;
    unlock_model(true);
}
