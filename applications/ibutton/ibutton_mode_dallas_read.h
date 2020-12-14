#pragma once
#include "ibutton.h"
#include "one_wire_gpio.h"
#include "maxim_crc.h"

class AppiButtonModeDallasRead : public AppTemplateMode<AppiButtonState, AppiButtonEvent> {
public:
    const char* name = "dallas read";
    AppiButton* app;
    OneWireGpio* onewire;

    void event(AppiButtonEvent* event, AppiButtonState* state);
    void render(Canvas* canvas, AppiButtonState* state);
    void acquire();
    void release();

    AppiButtonModeDallasRead(AppiButton* parent_app) {
        app = parent_app;

        // TODO open record
        const GpioPin* one_wire_pin_record = &ibutton_gpio;
        onewire = new OneWireGpio(one_wire_pin_record);
    };
};

void AppiButtonModeDallasRead::event(AppiButtonEvent* event, AppiButtonState* state) {
    if(event->type == AppiButtonEvent::EventTypeTick) {
        bool result = 0;
        uint8_t address[8];

        osKernelLock();
        result = onewire->reset();
        osKernelUnlock();

        if(result) {
            printf("device on line\n");

            delay(50);
            osKernelLock();
            onewire->write(0x33);
            onewire->read_bytes(address, 8);
            osKernelUnlock();

            printf("address: %x", address[0]);
            for(uint8_t i = 1; i < 8; i++) {
                printf(":%x", address[i]);
            }
            printf("\n");

            printf("crc8: %x\n", maxim_crc8(address, 7));

            if(maxim_crc8(address, 8) == 0) {
                printf("CRC valid\n");
                memcpy(app->state.dallas_address[app->state.dallas_address_index], address, 8);
                app->blink_green();
            } else {
                printf("CRC invalid\n");
            }
        } else {
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

void AppiButtonModeDallasRead::render(Canvas* canvas, AppiButtonState* state) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "Dallas read >");
    app->render_dallas_list(canvas, state);
}

void AppiButtonModeDallasRead::acquire() {
    onewire->start();
}

void AppiButtonModeDallasRead::release() {
    onewire->stop();
}