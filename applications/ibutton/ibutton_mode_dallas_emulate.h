#pragma once
#include "ibutton.h"
#include "one_wire_slave.h"
#include "one_wire_device_ds_1990.h"
#include "callback-connector.h"
#include <atomic>

class AppiButtonModeDallasEmulate : public AppTemplateMode<AppiButtonState, AppiButtonEvent> {
private:
    void result_callback(bool success, void* ctx);

public:
    const char* name = "dallas emulate";
    AppiButton* app;
    DS1990 key;
    OneWireSlave* onewire_slave;

    void event(AppiButtonEvent* event, AppiButtonState* state);
    void render(Canvas* canvas, AppiButtonState* state);
    void acquire();
    void release();

    std::atomic<bool> emulated_result{false};

    AppiButtonModeDallasEmulate(AppiButton* parent_app)
        : key(1, 2, 3, 4, 5, 6, 7) {
        app = parent_app;

        // TODO open record
        const GpioPin* one_wire_pin_record = &ibutton_gpio;
        onewire_slave = new OneWireSlave(one_wire_pin_record);
        onewire_slave->attach(&key);

        auto cb = cbc::obtain_connector(this, &AppiButtonModeDallasEmulate::result_callback);
        onewire_slave->set_result_callback(cb, this);
    };
};

void AppiButtonModeDallasEmulate::result_callback(bool success, void* ctx) {
    AppiButtonModeDallasEmulate* _this = static_cast<AppiButtonModeDallasEmulate*>(ctx);
    _this->emulated_result = success;
}

void AppiButtonModeDallasEmulate::event(AppiButtonEvent* event, AppiButtonState* state) {
    if(event->type == AppiButtonEvent::EventTypeTick) {
        if(emulated_result) {
            emulated_result = false;
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

    onewire_slave->deattach();
    memcpy(key.id_storage, state->dallas_address[state->dallas_address_index], 8);
    onewire_slave->attach(&key);
}

void AppiButtonModeDallasEmulate::render(Canvas* canvas, AppiButtonState* state) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "< Dallas emulate >");

    app->render_dallas_list(canvas, state);
}

void AppiButtonModeDallasEmulate::acquire() {
    onewire_slave->start();
}

void AppiButtonModeDallasEmulate::release() {
    onewire_slave->stop();
}