#pragma once
#include "ibutton.h"
#include "one_wire_gpio.h"

class AppiButtonModeDallasRead : public AppTemplateMode<AppiButtonState, AppiButtonEvent> {
public:
    const char* name = "dallas read";
    AppiButton* app;
    OneWireGpio* onewire;

    void event(AppiButtonEvent* event, AppiButtonState* state);
    void render(CanvasApi* canvas, AppiButtonState* state);
    void acquire();
    void release();

    AppiButtonModeDallasRead(AppiButton* parent_app) {
        app = parent_app;

        // TODO open record
        const GpioPin* one_wire_pin_record = &ibutton_gpio;
        onewire = new OneWireGpio(one_wire_pin_record);
    };

    uint8_t crc_8(uint8_t* buffer, uint8_t count);
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

            printf("crc8: %x\n", crc_8(address, 7));

            if(crc_8(address, 8) == 0) {
                printf("CRC valid\n");
                memcpy(app->state.dallas_address, address, 8);
                app->blink_green();
            } else {
                printf("CRC invalid\n");
            }
        } else {
        }
    }
}

void AppiButtonModeDallasRead::render(CanvasApi* canvas, AppiButtonState* state) {
    canvas->set_font(canvas, FontSecondary);
    canvas->draw_str(canvas, 2, 25, "dallas read >");
    canvas->draw_str(canvas, 2, 37, "touch me, iButton");
    {
        const uint8_t buffer_size = 32;
        char buf[buffer_size];
        snprintf(
            buf,
            buffer_size,
            "%x:%x:%x:%x:%x:%x:%x:%x",
            state->dallas_address[0],
            state->dallas_address[1],
            state->dallas_address[2],
            state->dallas_address[3],
            state->dallas_address[4],
            state->dallas_address[5],
            state->dallas_address[6],
            state->dallas_address[7]);
        canvas->draw_str(canvas, 2, 50, buf);
    }
}

uint8_t AppiButtonModeDallasRead::crc_8(uint8_t* buffer, uint8_t count) {
    const uint8_t maxim_crc8_table[256] = {
        0,   94,  188, 226, 97,  63,  221, 131, 194, 156, 126, 32,  163, 253, 31,  65,  157, 195,
        33,  127, 252, 162, 64,  30,  95,  1,   227, 189, 62,  96,  130, 220, 35,  125, 159, 193,
        66,  28,  254, 160, 225, 191, 93,  3,   128, 222, 60,  98,  190, 224, 2,   92,  223, 129,
        99,  61,  124, 34,  192, 158, 29,  67,  161, 255, 70,  24,  250, 164, 39,  121, 155, 197,
        132, 218, 56,  102, 229, 187, 89,  7,   219, 133, 103, 57,  186, 228, 6,   88,  25,  71,
        165, 251, 120, 38,  196, 154, 101, 59,  217, 135, 4,   90,  184, 230, 167, 249, 27,  69,
        198, 152, 122, 36,  248, 166, 68,  26,  153, 199, 37,  123, 58,  100, 134, 216, 91,  5,
        231, 185, 140, 210, 48,  110, 237, 179, 81,  15,  78,  16,  242, 172, 47,  113, 147, 205,
        17,  79,  173, 243, 112, 46,  204, 146, 211, 141, 111, 49,  178, 236, 14,  80,  175, 241,
        19,  77,  206, 144, 114, 44,  109, 51,  209, 143, 12,  82,  176, 238, 50,  108, 142, 208,
        83,  13,  239, 177, 240, 174, 76,  18,  145, 207, 45,  115, 202, 148, 118, 40,  171, 245,
        23,  73,  8,   86,  180, 234, 105, 55,  213, 139, 87,  9,   235, 181, 54,  104, 138, 212,
        149, 203, 41,  119, 244, 170, 72,  22,  233, 183, 85,  11,  136, 214, 52,  106, 43,  117,
        151, 201, 74,  20,  246, 168, 116, 42,  200, 150, 21,  75,  169, 247, 182, 232, 10,  84,
        215, 137, 107, 53};

    uint8_t crc = 0;

    while(count--) {
        crc = maxim_crc8_table[(crc ^ *buffer++)];
    }
    return crc;
}

void AppiButtonModeDallasRead::acquire() {
    onewire->start();
}

void AppiButtonModeDallasRead::release() {
    onewire->stop();
}