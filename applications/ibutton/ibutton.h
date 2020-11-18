#pragma once
#include "app-template.h"
#include "ibutton_mode_template.h"

// event enumeration type
typedef uint8_t event_t;

class AppiButtonState {
public:
    // state data
    // test key = {0x01, 0xFD, 0x0E, 0x84, 0x01, 0x00, 0x00, 0xDB};
    uint8_t dallas_address[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t mode_index;

    // state initializer
    AppiButtonState() {
        mode_index = 0;
    }
};

// events class
class AppiButtonEvent {
public:
    // events enum
    static const event_t EventTypeTick = 0;
    static const event_t EventTypeKey = 1;

    // payload
    union {
        InputEvent input;
    } value;

    // event type
    event_t type;
};

// our app derived from base AppTemplate class
// with template variables <state, events>
class AppiButton : public AppTemplate<AppiButtonState, AppiButtonEvent> {
public:
    GpioPin* red_led_record;
    GpioPin* green_led_record;

    static const uint8_t modes_count = 2;
    AppTemplateMode<AppiButtonState, AppiButtonEvent>* mode[modes_count] = {NULL, NULL};

    void run();
    void render(CanvasApi* canvas);

    void blink_red();
    void blink_green();

    void increase_mode();
    void decrease_mode();
    void switch_to_mode(uint8_t mode_index);
};