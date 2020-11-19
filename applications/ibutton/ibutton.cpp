#include "ibutton.h"
#include "ibutton_mode_dallas_read.h"
#include "ibutton_mode_dallas_emulate.h"

// start app
void AppiButton::run() {
    mode[0] = new AppiButtonModeDallasRead(this);
    mode[1] = new AppiButtonModeDallasEmulate(this);

    switch_to_mode(0);

    // TODO open record
    red_led_record = &led_gpio[0];
    green_led_record = &led_gpio[1];

    // configure pin
    gpio_init(red_led_record, GpioModeOutputOpenDrain);
    gpio_init(green_led_record, GpioModeOutputOpenDrain);

    app_ready();

    AppiButtonEvent event;
    while(1) {
        if(get_event(&event, 100)) {
            if(event.type == AppiButtonEvent::EventTypeKey) {
                // press events
                if(event.value.input.state && event.value.input.input == InputBack) {
                    printf("[ibutton] bye!\n");
                    // TODO remove all widgets create by app
                    widget_enabled_set(widget, false);
                    furiac_exit(NULL);
                }

                if(event.value.input.state && event.value.input.input == InputLeft) {
                    decrease_mode();
                }

                if(event.value.input.state && event.value.input.input == InputRight) {
                    increase_mode();
                }
            }
        } else {
            event.type = AppiButtonEvent::EventTypeTick;
        }

        acquire_state();
        mode[state.mode_index]->event(&event, &state);
        release_state();

        widget_update(widget);
    };
}

// render app
void AppiButton::render(CanvasApi* canvas) {
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);
    canvas->draw_str(canvas, 2, 12, "iButton");

    mode[state.mode_index]->render(canvas, &state);
}

void AppiButton::blink_red() {
    gpio_write(red_led_record, 0);
    delay(10);
    gpio_write(red_led_record, 1);
}

void AppiButton::blink_green() {
    gpio_write(green_led_record, 0);
    delay(10);
    gpio_write(green_led_record, 1);
}

void AppiButton::increase_mode() {
    acquire_state();
    if(state.mode_index < (modes_count - 1)) {
        mode[state.mode_index]->release();
        state.mode_index++;
        mode[state.mode_index]->acquire();
    }
    release_state();
}

void AppiButton::decrease_mode() {
    acquire_state();
    if(state.mode_index > 0) {
        mode[state.mode_index]->release();
        state.mode_index--;
        mode[state.mode_index]->acquire();
    }
    release_state();
}

void AppiButton::switch_to_mode(uint8_t mode_index) {
    acquire_state();
    mode[state.mode_index]->release();
    state.mode_index = mode_index;
    mode[state.mode_index]->acquire();
    release_state();
}

// app enter function
extern "C" void app_ibutton(void* p) {
    AppiButton* app = new AppiButton();
    app->run();
}