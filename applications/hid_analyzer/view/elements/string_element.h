#pragma once
#include "generic_element.h"

class StringElement : public GenericElement {
public:
    StringElement();
    ~StringElement() final;
    void draw(Canvas* canvas) final;
    bool input(InputEvent* event) final;

    void set_text(
        const char* text = NULL,
        uint8_t x = 0,
        uint8_t y = 0,
        uint8_t fit_width = 0,
        Align horizontal = AlignLeft,
        Align vertical = AlignTop,
        Font font = FontPrimary);

private:
    const char* text = NULL;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t fit_width = 0;
    Align horizontal = AlignLeft;
    Align vertical = AlignTop;
    Font font = FontPrimary;
};
