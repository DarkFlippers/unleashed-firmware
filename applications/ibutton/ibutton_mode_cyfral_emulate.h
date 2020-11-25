#pragma once
#include "ibutton.h"
#include "cyfral_emulator.h"

class AppiButtonModeCyfralEmulate : public AppTemplateMode<AppiButtonState, AppiButtonEvent> {
public:
    const char* name = "cyfral emulate";
    AppiButton* app;
    CyfralEmulator* cyfral_emulator;

    void event(AppiButtonEvent* event, AppiButtonState* state);
    void render(CanvasApi* canvas, AppiButtonState* state);
    void acquire();
    void release();

    AppiButtonModeCyfralEmulate(AppiButton* parent_app) {
        app = parent_app;

        // TODO open record
        const GpioPin* one_wire_pin_record = &ibutton_gpio;
        cyfral_emulator = new CyfralEmulator(one_wire_pin_record);
    };
};

void AppiButtonModeCyfralEmulate::event(AppiButtonEvent* event, AppiButtonState* state) {
    if(event->type == AppiButtonEvent::EventTypeTick) {
        // repeat key sending 8 times
        cyfral_emulator->send(state->cyfral_address[state->cyfral_address_index], 4, 8);
        app->blink_green();

    } else if(event->type == AppiButtonEvent::EventTypeKey) {
        if(event->value.input.state && event->value.input.input == InputUp) {
            app->decrease_cyfral_address();
        }

        if(event->value.input.state && event->value.input.input == InputDown) {
            app->increase_cyfral_address();
        }
    }
}

void AppiButtonModeCyfralEmulate::render(CanvasApi* canvas, AppiButtonState* state) {
    canvas->set_font(canvas, FontSecondary);
    canvas->draw_str(canvas, 2, 25, "< Cyfral emulate");

    app->render_cyfral_list(canvas, state);
}

void AppiButtonModeCyfralEmulate::acquire() {
    cyfral_emulator->start();
}

void AppiButtonModeCyfralEmulate::release() {
    cyfral_emulator->stop();
}