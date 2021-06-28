#pragma once
#include "generic-element.h"

class IconElement : public GenericElement {
public:
    IconElement();
    ~IconElement() final;
    void draw(Canvas* canvas) final;
    bool input(InputEvent* event) final;

    void set_icon(uint8_t x = 0, uint8_t y = 0, IconName name = I_Empty_1x1);

private:
    IconName name = I_Empty_1x1;
    uint8_t x = 0;
    uint8_t y = 0;
};
