#pragma once

#include <stm32wbxx_ll_lptim.h>
#include <stm32wbxx_ll_bus.h>
#include <stdint.h>

// Timer used for system ticks
#define API_HAL_OS_TIMER_MAX  0xFFFF
#define API_HAL_OS_TIMER_REG_LOAD_DLY 0x1
#define API_HAL_OS_TIMER       LPTIM2
#define API_HAL_OS_TIMER_IRQ   LPTIM2_IRQn

static inline void api_hal_os_timer_init() {
    // Configure clock source
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM2_CLKSOURCE_LSE);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPTIM2);
    // Set interrupt priority and enable them
    NVIC_SetPriority(API_HAL_OS_TIMER_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(API_HAL_OS_TIMER_IRQ);
}

static inline void api_hal_os_timer_continuous(uint32_t count) {
    // Enable timer
    LL_LPTIM_Enable(API_HAL_OS_TIMER);
    while(!LL_LPTIM_IsEnabled(API_HAL_OS_TIMER));

    // Enable rutoreload match interrupt
    LL_LPTIM_EnableIT_ARRM(API_HAL_OS_TIMER);

    // Set autoreload and start counter
    LL_LPTIM_SetAutoReload(API_HAL_OS_TIMER, count);
    LL_LPTIM_StartCounter(API_HAL_OS_TIMER, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}

static inline void api_hal_os_timer_single(uint32_t count) {
    // Enable timer
    LL_LPTIM_Enable(API_HAL_OS_TIMER);
    while(!LL_LPTIM_IsEnabled(API_HAL_OS_TIMER));

    // Enable compare match interrupt
    LL_LPTIM_EnableIT_CMPM(API_HAL_OS_TIMER);

    // Set compare, autoreload and start counter
    // Include some marging to workaround ARRM behaviour
    LL_LPTIM_SetCompare(API_HAL_OS_TIMER, count-3);
    LL_LPTIM_SetAutoReload(API_HAL_OS_TIMER, count);
    LL_LPTIM_StartCounter(API_HAL_OS_TIMER, LL_LPTIM_OPERATING_MODE_ONESHOT);
}

static inline void api_hal_os_timer_reset() {
    // Hard reset timer
    // THE ONLY RELIABLEWAY to stop it according to errata
    LL_LPTIM_DeInit(API_HAL_OS_TIMER);
}

static inline uint32_t api_hal_os_timer_get_cnt() {
    uint32_t counter = LL_LPTIM_GetCounter(API_HAL_OS_TIMER);
    uint32_t counter_shadow = LL_LPTIM_GetCounter(API_HAL_OS_TIMER);
    while(counter != counter_shadow) {
        counter = counter_shadow;
        counter_shadow = LL_LPTIM_GetCounter(API_HAL_OS_TIMER);
    }
    return counter;
}
