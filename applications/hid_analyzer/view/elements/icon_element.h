#pragma once
#include "generic_element.h"

class IconElement : public GenericElement {
public:
    IconElement();
    ~IconElement() final;
    void draw(Canvas* canvas) final;
    bool input(InputEvent* event) final;

    void set_icon(uint8_t x = 0, uint8_t y = 0, const Icon* icon = NULL);

private:
    const Icon* icon = NULL;
    uint8_t x = 0;
    uint8_t y = 0;
};
