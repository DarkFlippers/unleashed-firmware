#pragma once

#include <stm32wbxx_ll_lptim.h>
#include <stdbool.h>

static inline void assert(bool value) {
    if (!value) asm("bkpt 1");
}

// Timer used for system ticks
#define API_HAL_TIMEBASE_TIMER_MAX  0xFFFF
#define API_HAL_TIMEBASE_TIMER_REG_LOAD_DLY 0x1
#define API_HAL_TIMEBASE_TIMER       LPTIM2
#define API_HAL_TIMEBASE_TIMER_IRQ   LPTIM2_IRQn
#define API_HAL_TIMEBASE_TIMER_CLOCK_INIT() \
{ \
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM2_CLKSOURCE_LSE); \
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPTIM2); \
} \

static inline void api_hal_timebase_timer_init() {
    API_HAL_TIMEBASE_TIMER_CLOCK_INIT();

    LL_LPTIM_Enable(API_HAL_TIMEBASE_TIMER);
    while(!LL_LPTIM_IsEnabled(API_HAL_TIMEBASE_TIMER)) {}

    LL_LPTIM_SetClockSource(API_HAL_TIMEBASE_TIMER, LL_LPTIM_CLK_SOURCE_INTERNAL);
    LL_LPTIM_SetPrescaler(API_HAL_TIMEBASE_TIMER, LL_LPTIM_PRESCALER_DIV1);
    LL_LPTIM_SetPolarity(API_HAL_TIMEBASE_TIMER, LL_LPTIM_OUTPUT_POLARITY_REGULAR);
    LL_LPTIM_SetUpdateMode(API_HAL_TIMEBASE_TIMER, LL_LPTIM_UPDATE_MODE_IMMEDIATE);
    LL_LPTIM_SetCounterMode(API_HAL_TIMEBASE_TIMER, LL_LPTIM_COUNTER_MODE_INTERNAL);
    LL_LPTIM_TrigSw(API_HAL_TIMEBASE_TIMER);
    LL_LPTIM_SetInput1Src(API_HAL_TIMEBASE_TIMER, LL_LPTIM_INPUT1_SRC_GPIO);
    LL_LPTIM_SetInput2Src(API_HAL_TIMEBASE_TIMER, LL_LPTIM_INPUT2_SRC_GPIO);

    NVIC_SetPriority(API_HAL_TIMEBASE_TIMER_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(API_HAL_TIMEBASE_TIMER_IRQ);
}

static inline uint32_t api_hal_timebase_timer_get_cnt() {
    uint32_t counter = LL_LPTIM_GetCounter(API_HAL_TIMEBASE_TIMER);
    uint32_t counter_shadow = LL_LPTIM_GetCounter(API_HAL_TIMEBASE_TIMER);
    while(counter != counter_shadow) {
        counter = counter_shadow;
        counter_shadow = LL_LPTIM_GetCounter(API_HAL_TIMEBASE_TIMER);
    }
    return counter;
}

static inline bool api_hal_timebase_timer_arr_is_ok() {
    return LL_LPTIM_IsActiveFlag_ARROK(API_HAL_TIMEBASE_TIMER);
}

static inline uint32_t api_hal_timebase_timer_get_arr() {
    return LL_LPTIM_GetAutoReload(API_HAL_TIMEBASE_TIMER);;
}

static inline void api_hal_timebase_timer_set_arr(uint32_t value) {
    value &= API_HAL_TIMEBASE_TIMER_MAX;
    if (value != api_hal_timebase_timer_get_arr()) {
        assert(api_hal_timebase_timer_arr_is_ok());
        LL_LPTIM_ClearFlag_ARROK(API_HAL_TIMEBASE_TIMER);
        LL_LPTIM_SetAutoReload(API_HAL_TIMEBASE_TIMER, value);
    }
}

static inline bool api_hal_timebase_timer_cmp_is_ok() {
    return LL_LPTIM_IsActiveFlag_CMPOK(API_HAL_TIMEBASE_TIMER);
}

static inline uint32_t api_hal_timebase_timer_get_cmp() {
    return LL_LPTIM_GetCompare(API_HAL_TIMEBASE_TIMER);;
}

static inline void api_hal_timebase_timer_set_cmp(uint32_t value) {
    value &= API_HAL_TIMEBASE_TIMER_MAX;
    if (value != api_hal_timebase_timer_get_cmp()) {
        assert(api_hal_timebase_timer_cmp_is_ok());
        LL_LPTIM_ClearFlag_CMPOK(API_HAL_TIMEBASE_TIMER);
        LL_LPTIM_SetCompare(API_HAL_TIMEBASE_TIMER, value);
    }
}

static inline bool api_hal_timebase_timer_is_safe() {
    uint16_t cmp = api_hal_timebase_timer_get_cmp();
    uint16_t cnt = api_hal_timebase_timer_get_cnt();
    uint16_t margin = (cmp > cnt) ? cmp - cnt : cnt - cmp;
    if (margin < 8) {
        return false;
    }
    if (!api_hal_timebase_timer_cmp_is_ok()) {
        return false;
    }
    return true;
}
