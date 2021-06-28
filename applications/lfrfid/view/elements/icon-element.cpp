#include "icon-element.h"

IconElement::IconElement() {
}

IconElement::~IconElement() {
}

void IconElement::draw(Canvas* canvas) {
    if(name != I_Empty_1x1) {
        canvas_draw_icon_name(canvas, x, y, name);
    }
}

bool IconElement::input(InputEvent* event) {
    return false;
}

void IconElement::set_icon(uint8_t _x, uint8_t _y, IconName _name) {
    lock_model();
    name = _name;
    x = _x;
    y = _y;
    unlock_model(true);
}
