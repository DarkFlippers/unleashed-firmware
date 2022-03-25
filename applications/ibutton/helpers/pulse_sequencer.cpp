#include "pulse_sequencer.h"

#include <furi.h>
#include <furi_hal.h>

void PulseSequencer::set_periods(
    uint32_t* _periods,
    uint16_t _periods_count,
    bool _pin_start_state) {
    periods = _periods;
    periods_count = _periods_count;
    pin_start_state = _pin_start_state;
}

void PulseSequencer::start() {
    period_index = 1;
    pin_state = pin_start_state;
    hal_gpio_write(&ibutton_gpio, pin_state);
    pin_state = !pin_state;

    hal_gpio_init(&ibutton_gpio, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_ibutton_emulate_start(
        periods[period_index], PulseSequencer::timer_elapsed_callback, this);
}

void PulseSequencer::stop() {
    furi_hal_ibutton_emulate_stop();
}

PulseSequencer::~PulseSequencer() {
    stop();
}

void PulseSequencer::timer_elapsed_callback(void* context) {
    PulseSequencer* self = static_cast<PulseSequencer*>(context);

    furi_hal_ibutton_emulate_set_next(self->periods[self->period_index]);

    if(self->period_index == 0) {
        self->pin_state = self->pin_start_state;
    } else {
        self->pin_state = !self->pin_state;
    }

    hal_gpio_write(&ibutton_gpio, self->pin_state);

    self->period_index++;

    if(self->period_index == self->periods_count) {
        self->period_index = 0;
    }
}
