#include <api-hal-os.h>
#include <api-hal-os-timer.h>
#include <api-hal-power.h>
#include <stm32wbxx_ll_cortex.h>

#include <FreeRTOS.h>
#include <cmsis_os.h>

#define API_HAL_OS_CLK_FREQUENCY 32768
#define API_HAL_OS_TICK_PER_SECOND 1024
#define API_HAL_OS_CLK_PER_TICK (API_HAL_OS_CLK_FREQUENCY / API_HAL_OS_TICK_PER_SECOND)
#define API_HAL_OS_TICK_PER_EPOCH (API_HAL_OS_TIMER_MAX / API_HAL_OS_CLK_PER_TICK)
#define API_HAL_OS_MAX_SLEEP (API_HAL_OS_TICK_PER_EPOCH - 1)

#ifdef API_HAL_OS_DEBUG
#include <stm32wbxx_ll_gpio.h>

#define LED_SLEEP_PORT GPIOA
#define LED_SLEEP_PIN LL_GPIO_PIN_7
#define LED_TICK_PORT GPIOA
#define LED_TICK_PIN LL_GPIO_PIN_6
#define LED_SECOND_PORT GPIOA
#define LED_SECOND_PIN LL_GPIO_PIN_4

void api_hal_os_timer_callback() {
    LL_GPIO_TogglePin(LED_SECOND_PORT, LED_SECOND_PIN);
}
#endif

volatile uint32_t api_hal_os_skew = 0;

void api_hal_os_init() {
    LL_DBGMCU_APB1_GRP2_FreezePeriph(LL_DBGMCU_APB1_GRP2_LPTIM2_STOP);

    api_hal_os_timer_init();
    api_hal_os_timer_continuous(API_HAL_OS_CLK_PER_TICK);

#ifdef API_HAL_OS_DEBUG
    LL_GPIO_SetPinMode(LED_SLEEP_PORT, LED_SLEEP_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LED_TICK_PORT, LED_TICK_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LED_SECOND_PORT, LED_SECOND_PIN, LL_GPIO_MODE_OUTPUT);
    osTimerId_t second_timer = osTimerNew(api_hal_os_timer_callback, osTimerPeriodic, NULL, NULL);
    osTimerStart(second_timer, 1024);
#endif
}

void LPTIM2_IRQHandler(void) {
    // Autoreload
    if(LL_LPTIM_IsActiveFlag_ARRM(API_HAL_OS_TIMER)) {
        LL_LPTIM_ClearFLAG_ARRM(API_HAL_OS_TIMER);
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            #ifdef API_HAL_OS_DEBUG
            LL_GPIO_TogglePin(LED_TICK_PORT, LED_TICK_PIN);
            #endif
            xPortSysTickHandler();
        }
    }
    if(LL_LPTIM_IsActiveFlag_CMPM(API_HAL_OS_TIMER)) {
        LL_LPTIM_ClearFLAG_CMPM(API_HAL_OS_TIMER);
    }
}

static inline uint32_t api_hal_os_sleep(TickType_t expected_idle_ticks) {
    // Stop ticks
    api_hal_os_timer_reset();
    LL_SYSTICK_DisableIT();

    // Start wakeup timer
    api_hal_os_timer_single(expected_idle_ticks * API_HAL_OS_CLK_PER_TICK);

#ifdef API_HAL_OS_DEBUG
    LL_GPIO_ResetOutputPin(LED_SLEEP_PORT, LED_SLEEP_PIN);
#endif

    // Go to sleep mode
    api_hal_power_sleep();

#ifdef API_HAL_OS_DEBUG
    LL_GPIO_SetOutputPin(LED_SLEEP_PORT, LED_SLEEP_PIN);
#endif

    // Calculate how much time we spent in the sleep
    uint32_t after_cnt = api_hal_os_timer_get_cnt() + api_hal_os_skew;
    uint32_t after_tick = after_cnt / API_HAL_OS_CLK_PER_TICK;
    api_hal_os_skew = after_cnt % API_HAL_OS_CLK_PER_TICK;

    bool cmpm = LL_LPTIM_IsActiveFlag_CMPM(API_HAL_OS_TIMER);
    bool arrm = LL_LPTIM_IsActiveFlag_ARRM(API_HAL_OS_TIMER);
    if (cmpm && arrm) after_tick += expected_idle_ticks;

    // Prepare tick timer for new round
    api_hal_os_timer_reset();

    // Resume ticks
    LL_SYSTICK_EnableIT();
    api_hal_os_timer_continuous(API_HAL_OS_CLK_PER_TICK);

    return after_tick;
}

void vPortSuppressTicksAndSleep(TickType_t expected_idle_ticks) {
    // Limit mount of ticks to maximum that timer can count
    if (expected_idle_ticks > API_HAL_OS_MAX_SLEEP) {
        expected_idle_ticks = API_HAL_OS_MAX_SLEEP;
    }

    // Stop IRQ handling, no one should disturb us till we finish 
    __disable_irq();

    // Confirm OS that sleep is still possible
    if (eTaskConfirmSleepModeStatus() == eAbortSleep) {
        __enable_irq();
        return;
    }

    // Sleep and track how much ticks we spent sleeping
    uint32_t completed_ticks = api_hal_os_sleep(expected_idle_ticks);

    // Reenable IRQ
    __enable_irq();

    // Notify system about time spent in sleep
    if (completed_ticks > 0) {
        if (completed_ticks > expected_idle_ticks) {
            vTaskStepTick(expected_idle_ticks);
        } else {
            vTaskStepTick(completed_ticks);
        }
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
    asm("bkpt 1");
    while(1) {};
}
