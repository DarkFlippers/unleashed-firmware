#pragma once
#include <stdint.h>

class PulseSequencer {
public:
    void set_periods(uint32_t* periods, uint16_t periods_count, bool pin_start_state);
    void start();
    void stop();

    ~PulseSequencer();

private:
    uint16_t period_index;
    uint16_t periods_count;
    uint32_t* periods;
    bool pin_start_state;
    bool pin_state;

    void init_timer(uint32_t period);

    void reset_period_index(PulseSequencer* _this);

    void (*callback_pointer)(void*, void*);

    static void timer_elapsed_callback(void* comp_ctx);
};
