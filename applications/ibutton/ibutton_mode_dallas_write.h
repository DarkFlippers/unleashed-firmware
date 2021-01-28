#pragma once
#include "ibutton.h"
#include "blanks_writer.h"
#include "maxim_crc.h"

class AppiButtonModeDallasWrite : public AppTemplateMode<AppiButtonState, AppiButtonEvent> {
public:
    const char* name = "dallas read";
    AppiButton* app;
    BlanksWriter* writer;

    void event(AppiButtonEvent* event, AppiButtonState* state);
    void render(Canvas* canvas, AppiButtonState* state);
    void acquire();
    void release();

    const GpioPin* one_wire_pin_record;

    AppiButtonModeDallasWrite(AppiButton* parent_app) {
        app = parent_app;

        // TODO open record
        one_wire_pin_record = &ibutton_gpio;
        writer = new BlanksWriter(one_wire_pin_record);
    };
};

void AppiButtonModeDallasWrite::event(AppiButtonEvent* event, AppiButtonState* state) {
    if(event->type == AppiButtonEvent::EventTypeTick) {
        WriterResult result =
            writer->write(KEY_DS1990, state->dallas_address[state->dallas_address_index], 8);

        if(result == WR_SAME_KEY) {
            app->blink_green();
        }

        if(result == WR_OK) {
            app->blink_red();
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

void AppiButtonModeDallasWrite::render(Canvas* canvas, AppiButtonState* state) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "< Dallas write >");
    app->render_dallas_list(canvas, state);
}

void AppiButtonModeDallasWrite::acquire() {
    writer->start();
}

void AppiButtonModeDallasWrite::release() {
    writer->stop();
}