#include "rfid-timer-emulator.h"

extern TIM_HandleTypeDef htim1;
/*
static uint16_t times_index = 0;

constexpr uint16_t hid_237_34672_count = 528;
constexpr uint8_t hid_237_34672[hid_237_34672_count] = {
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,
    8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,
    10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10,
    10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,
    8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,
    8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,
    8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10,
    10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,
    8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,
    10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10,
    10, 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,
    8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  10, 10,
    10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10,
    10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10,
    10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10,
    8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  10,
    10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10,
    10, 10, 8,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 8,  8,  8,  8,  8,  8,
};

static void callback_hid(void* _hw, void* ctx) {
    //RfidTimerEmulator* _this = static_cast<RfidTimerEmulator*>(ctx);
    TIM_HandleTypeDef* hw = static_cast<TIM_HandleTypeDef*>(_hw);

    if(hw == &htim1) {
        hw->Instance->ARR = hid_237_34672[times_index] - 1;
        hw->Instance->CCR1 = hid_237_34672[times_index] / 2; // - 1

        times_index++;
        if(times_index >= hid_237_34672_count) {
            times_index = 0;
        }
    }
}

typedef struct {
    uint8_t arr;
    uint8_t ccr;
} TimerTick;

constexpr TimerTick indala_data[] = {
    {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 3, .ccr = 2}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 2},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 3, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1}, {.arr = 2, .ccr = 1},
    {.arr = 2, .ccr = 1},
};

constexpr uint16_t indala_size = sizeof(indala_data) / sizeof(TimerTick);

static void callback_indala(void* _hw, void* ctx) {
    //RfidTimerEmulator* _this = static_cast<RfidTimerEmulator*>(ctx);
    TIM_HandleTypeDef* hw = static_cast<TIM_HandleTypeDef*>(_hw);

    if(hw == &htim1) {
        hw->Instance->ARR = indala_data[times_index].arr - 1;
        hw->Instance->CCR1 = indala_data[times_index].ccr;

        times_index++;
        if(times_index >= indala_size) {
            times_index = 0;
        }
    }
}
*/

RfidTimerEmulator::RfidTimerEmulator() {
}

RfidTimerEmulator::~RfidTimerEmulator() {
    std::map<Type, EncoderGeneric*>::iterator it;

    for(it = encoders.begin(); it != encoders.end(); ++it) {
        delete it->second;
        encoders.erase(it);
    }
}

void RfidTimerEmulator::start(Type type) {
    if(encoders.count(type)) {
        current_encoder = encoders.find(type)->second;
        uint8_t em_data[5] = {0x53, 0x00, 0x5F, 0xB3, 0xC2};
        uint8_t hid_data[3] = {0xED, 0x87, 0x70};

        switch(type) {
        case Type::EM:
            current_encoder->init(em_data, 5);
            break;
        case Type::HID:
            current_encoder->init(hid_data, 3);
            break;
        case Type::Indala:
            current_encoder->init(nullptr, 5);
            break;
        }

        api_hal_rfid_tim_emulate(125000);
        api_hal_rfid_pins_emulate();

        api_interrupt_add(timer_update_callback, InterruptTypeTimerUpdate, this);

        for(size_t i = WWDG_IRQn; i <= DMAMUX1_OVR_IRQn; i++) {
            HAL_NVIC_SetPriority(static_cast<IRQn_Type>(i), 15, 0);
        }

        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

        api_hal_rfid_tim_emulate_start();
    } else {
        // not found
    }
}

void RfidTimerEmulator::stop() {
    api_hal_rfid_tim_emulate_stop();

    api_interrupt_remove(timer_update_callback, InterruptTypeTimerUpdate);
}

void RfidTimerEmulator::emulate() {
}

void RfidTimerEmulator::timer_update_callback(void* _hw, void* ctx) {
    RfidTimerEmulator* _this = static_cast<RfidTimerEmulator*>(ctx);
    TIM_HandleTypeDef* hw = static_cast<TIM_HandleTypeDef*>(_hw);

    if(hw == &LFRFID_TIM) {
        bool result;
        bool polarity;
        uint16_t period;
        uint16_t pulse;

        do {
            _this->current_encoder->get_next(&polarity, &period, &pulse);
            result = _this->pulse_joiner.push_pulse(polarity, period, pulse);
        } while(result == false);

        _this->pulse_joiner.pop_pulse(&period, &pulse);

        hw->Instance->ARR = period - 1;
        hw->Instance->CCR1 = pulse;
    }
}
