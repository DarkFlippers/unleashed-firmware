#include <furi_hal_os.h>
#include <furi_hal_clock.h>
#include <furi_hal_power.h>
#include <furi_hal_delay.h>
#include <furi_hal_idle_timer.h>
#include <stm32wbxx_ll_cortex.h>

#include <furi.h>

#define TAG "FuriHalOs"

#define FURI_HAL_IDLE_TIMER_CLK_HZ 32768
#define FURI_HAL_OS_TICK_HZ configTICK_RATE_HZ

#define FURI_HAL_OS_IDLE_CNT_TO_TICKS(x) ((x * FURI_HAL_OS_TICK_HZ) / FURI_HAL_IDLE_TIMER_CLK_HZ)
#define FURI_HAL_OS_TICKS_TO_IDLE_CNT(x) ((x * FURI_HAL_IDLE_TIMER_CLK_HZ) / FURI_HAL_OS_TICK_HZ)

#define FURI_HAL_IDLE_TIMER_TICK_PER_EPOCH (FURI_HAL_OS_IDLE_CNT_TO_TICKS(FURI_HAL_IDLE_TIMER_MAX))
#define FURI_HAL_OS_MAX_SLEEP (FURI_HAL_IDLE_TIMER_TICK_PER_EPOCH - 1)

#ifdef FURI_HAL_OS_DEBUG
#include <stm32wbxx_ll_gpio.h>

#define LED_SLEEP_PORT GPIOA
#define LED_SLEEP_PIN LL_GPIO_PIN_7
#define LED_TICK_PORT GPIOA
#define LED_TICK_PIN LL_GPIO_PIN_6
#define LED_SECOND_PORT GPIOA
#define LED_SECOND_PIN LL_GPIO_PIN_4

void furi_hal_os_timer_callback() {
    LL_GPIO_TogglePin(LED_SECOND_PORT, LED_SECOND_PIN);
}
#endif

extern void xPortSysTickHandler();

static volatile uint32_t furi_hal_os_skew = 0;

void furi_hal_os_init() {
    furi_hal_idle_timer_init();

#ifdef FURI_HAL_OS_DEBUG
    LL_GPIO_SetPinMode(LED_SLEEP_PORT, LED_SLEEP_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LED_TICK_PORT, LED_TICK_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LED_SECOND_PORT, LED_SECOND_PIN, LL_GPIO_MODE_OUTPUT);
    osTimerId_t second_timer = osTimerNew(furi_hal_os_timer_callback, osTimerPeriodic, NULL, NULL);
    osTimerStart(second_timer, FURI_HAL_OS_TICK_HZ);
#endif

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_os_tick() {
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#ifdef FURI_HAL_OS_DEBUG
        LL_GPIO_TogglePin(LED_TICK_PORT, LED_TICK_PIN);
#endif
        xPortSysTickHandler();
    }
}

static inline uint32_t furi_hal_os_sleep(TickType_t expected_idle_ticks) {
    // Stop ticks
    furi_hal_clock_suspend_tick();

    // Start wakeup timer
    furi_hal_idle_timer_start(FURI_HAL_OS_TICKS_TO_IDLE_CNT(expected_idle_ticks));

#ifdef FURI_HAL_OS_DEBUG
    LL_GPIO_ResetOutputPin(LED_SLEEP_PORT, LED_SLEEP_PIN);
#endif

    // Go to sleep mode
    furi_hal_power_sleep();

#ifdef FURI_HAL_OS_DEBUG
    LL_GPIO_SetOutputPin(LED_SLEEP_PORT, LED_SLEEP_PIN);
#endif

    // Calculate how much time we spent in the sleep
    uint32_t after_cnt = furi_hal_idle_timer_get_cnt() + furi_hal_os_skew;
    uint32_t after_tick = FURI_HAL_OS_IDLE_CNT_TO_TICKS(after_cnt);
    furi_hal_os_skew = after_cnt - (after_cnt / after_tick);

    bool cmpm = LL_LPTIM_IsActiveFlag_CMPM(FURI_HAL_IDLE_TIMER);
    bool arrm = LL_LPTIM_IsActiveFlag_ARRM(FURI_HAL_IDLE_TIMER);
    if(cmpm && arrm) after_tick += expected_idle_ticks;

    // Prepare tick timer for new round
    furi_hal_idle_timer_reset();

    // Resume ticks
    furi_hal_clock_resume_tick();
    return after_tick;
}

void vPortSuppressTicksAndSleep(TickType_t expected_idle_ticks) {
    if(!furi_hal_power_sleep_available()) {
        __WFI();
        return;
    }

    // Limit amount of ticks to maximum that timer can count
    if(expected_idle_ticks > FURI_HAL_OS_MAX_SLEEP) {
        expected_idle_ticks = FURI_HAL_OS_MAX_SLEEP;
    }

    // Stop IRQ handling, no one should disturb us till we finish
    __disable_irq();

    // Confirm OS that sleep is still possible
    if(eTaskConfirmSleepModeStatus() == eAbortSleep) {
        __enable_irq();
        return;
    }

    // Sleep and track how much ticks we spent sleeping
    uint32_t completed_ticks = furi_hal_os_sleep(expected_idle_ticks);
    // Notify system about time spent in sleep
    if(completed_ticks > 0) {
        vTaskStepTick(MIN(completed_ticks, expected_idle_ticks));
    }

    // Reenable IRQ
    __enable_irq();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    furi_crash("StackOverflow");
}
