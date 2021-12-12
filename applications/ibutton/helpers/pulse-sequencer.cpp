#include "pulse-sequencer.h"
#include <furi.h>
#include <callback-connector.h>
#include <furi-hal-resources.h>

void PulseSequencer::set_periods(
    uint32_t* _periods,
    uint16_t _periods_count,
    bool _pin_start_state) {
    periods = _periods;
    periods_count = _periods_count;
    pin_start_state = _pin_start_state;
}

void PulseSequencer::start() {
    callback_pointer = cbc::obtain_connector(this, &PulseSequencer::timer_elapsed_callback);
    api_interrupt_add(callback_pointer, InterruptTypeTimerUpdate, this);

    period_index = 1;
    init_timer(periods[period_index]);
    pin_state = pin_start_state;
    hal_gpio_write(&ibutton_gpio, pin_state);
    pin_state = !pin_state;

    HAL_TIM_Base_Start_IT(&htim1);
}

void PulseSequencer::stop() {
    HAL_TIM_Base_Stop_IT(&htim1);

    api_interrupt_remove(callback_pointer, InterruptTypeTimerUpdate);
    deinit_timer();
}

PulseSequencer::~PulseSequencer() {
    stop();
}

void PulseSequencer::init_timer(uint32_t period) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = period;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if(HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if(HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

    hal_gpio_init(&ibutton_gpio, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
}

void PulseSequencer::deinit_timer() {
}

void PulseSequencer::timer_elapsed_callback(void* hw, void* context) {
    PulseSequencer* _this = static_cast<PulseSequencer*>(context);
    TIM_HandleTypeDef* htim = static_cast<TIM_HandleTypeDef*>(hw);

    if(htim->Instance == TIM1) {
        htim->Instance->ARR = _this->periods[_this->period_index];

        if(_this->period_index == 0) {
            _this->pin_state = _this->pin_start_state;
        } else {
            _this->pin_state = !_this->pin_state;
        }

        hal_gpio_write(&ibutton_gpio, _this->pin_state);

        _this->period_index++;

        if(_this->period_index == _this->periods_count) {
            _this->period_index = 0;
        }
    }
}
