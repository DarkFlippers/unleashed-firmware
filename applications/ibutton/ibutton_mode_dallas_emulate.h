#pragma once
#include "ibutton.h"
#include "one_wire_slave_gpio.h"
#include "one_wire_device_ds_1990.h"

class AppiButtonModeDallasEmulate : public AppTemplateMode<AppiButtonState, AppiButtonEvent> {
public:
    const char* name = "dallas emulate";
    AppiButton* app;
    OneWireGpioSlave* onewire_slave;
    DS1990 key;

    void event(AppiButtonEvent* event, AppiButtonState* state);
    void render(CanvasApi* canvas, AppiButtonState* state);
    void acquire();
    void release();

    AppiButtonModeDallasEmulate(AppiButton* parent_app)
        : key(1, 2, 3, 4, 5, 6, 7) {
        app = parent_app;

        // TODO open record
        const GpioPin* one_wire_pin_record = &ibutton_gpio;
        onewire_slave = new OneWireGpioSlave(one_wire_pin_record);
        onewire_slave->attach(key);
    };
};

void AppiButtonModeDallasEmulate::event(AppiButtonEvent* event, AppiButtonState* state) {
    if(event->type == AppiButtonEvent::EventTypeTick) {
        onewire_slave->detach(key);
        memcpy(key.id_storage, state->dallas_address[state->dallas_address_index], 8);
        onewire_slave->attach(key);

        if(onewire_slave->emulate()) {
            app->blink_green();
        }
    } else if(event->type == AppiButtonEvent::EventTypeKey) {
        if(event->value.input.state && event->value.input.input == InputUp) {
            app->decrease_dallas_address();
        }

        if(event->value.input.state && event->value.input.input == InputDown) {
            app->increase_dallas_address();
        }
    }
}

void AppiButtonModeDallasEmulate::render(CanvasApi* canvas, AppiButtonState* state) {
    canvas->set_font(canvas, FontSecondary);
    canvas->draw_str(canvas, 2, 25, "< Dallas emulate >");

    app->render_dallas_list(canvas, state);
}

void AppiButtonModeDallasEmulate::acquire() {
    onewire_slave->start();
}

void AppiButtonModeDallasEmulate::release() {
    onewire_slave->stop();
}