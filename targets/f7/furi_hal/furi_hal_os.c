#include <furi_hal_os.h>
#include <furi_hal_clock.h>
#include <furi_hal_power.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <furi_hal_idle_timer.h>

#include <stm32wbxx_ll_cortex.h>

#include <furi.h>

#include <FreeRTOS.h>
#include <task.h>

#define TAG "FuriHalOs"

#define FURI_HAL_IDLE_TIMER_CLK_HZ 32768
#define FURI_HAL_OS_TICK_HZ        configTICK_RATE_HZ

#define FURI_HAL_OS_IDLE_CNT_TO_TICKS(x) (((x) * FURI_HAL_OS_TICK_HZ) / FURI_HAL_IDLE_TIMER_CLK_HZ)
#define FURI_HAL_OS_TICKS_TO_IDLE_CNT(x) (((x) * FURI_HAL_IDLE_TIMER_CLK_HZ) / FURI_HAL_OS_TICK_HZ)

#define FURI_HAL_IDLE_TIMER_TICK_PER_EPOCH (FURI_HAL_OS_IDLE_CNT_TO_TICKS(FURI_HAL_IDLE_TIMER_MAX))
#define FURI_HAL_OS_MAX_SLEEP              (FURI_HAL_IDLE_TIMER_TICK_PER_EPOCH - 1)

#define FURI_HAL_OS_NVIC_IS_PENDING() (NVIC->ISPR[0] || NVIC->ISPR[1])
#define FURI_HAL_OS_EXTI_LINE_0_31    0
#define FURI_HAL_OS_EXTI_LINE_32_63   1

// Arbitrary (but small) number for better tick consistency
#define FURI_HAL_OS_EXTRA_CNT 3

#ifndef FURI_HAL_OS_DEBUG_AWAKE_GPIO
#define FURI_HAL_OS_DEBUG_AWAKE_GPIO (&gpio_ext_pa7)
#endif

#ifndef FURI_HAL_OS_DEBUG_TICK_GPIO
#define FURI_HAL_OS_DEBUG_TICK_GPIO (&gpio_ext_pa6)
#endif

#ifndef FURI_HAL_OS_DEBUG_SECOND_GPIO
#define FURI_HAL_OS_DEBUG_SECOND_GPIO (&gpio_ext_pa4)
#endif

#ifdef FURI_HAL_OS_DEBUG
#include <stm32wbxx_ll_gpio.h>

void furi_hal_os_timer_callback(void) {
    furi_hal_gpio_write(
        FURI_HAL_OS_DEBUG_SECOND_GPIO, !furi_hal_gpio_read(FURI_HAL_OS_DEBUG_SECOND_GPIO));
}
#endif

extern void xPortSysTickHandler(void);

static volatile uint32_t furi_hal_os_skew;

void furi_hal_os_init(void) {
    furi_hal_idle_timer_init();

#ifdef FURI_HAL_OS_DEBUG
    furi_hal_gpio_init_simple(FURI_HAL_OS_DEBUG_AWAKE_GPIO, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(FURI_HAL_OS_DEBUG_TICK_GPIO, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(FURI_HAL_OS_DEBUG_SECOND_GPIO, GpioModeOutputPushPull);
    furi_hal_gpio_write(FURI_HAL_OS_DEBUG_AWAKE_GPIO, 1);

    FuriTimer* second_timer =
        furi_timer_alloc(furi_hal_os_timer_callback, FuriTimerTypePeriodic, NULL);
    furi_timer_start(second_timer, FURI_HAL_OS_TICK_HZ);
#endif

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_os_tick(void) {
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#ifdef FURI_HAL_OS_DEBUG
        furi_hal_gpio_write(
            FURI_HAL_OS_DEBUG_TICK_GPIO, !furi_hal_gpio_read(FURI_HAL_OS_DEBUG_TICK_GPIO));
#endif
        xPortSysTickHandler();
    }
}

#ifdef FURI_HAL_OS_DEBUG
// Find out the IRQ number while debugging
static void furi_hal_os_nvic_dbg_trap(void) {
    for(int32_t i = WWDG_IRQn; i <= DMAMUX1_OVR_IRQn; i++) {
        if(NVIC_GetPendingIRQ(i)) {
            (void)i;
            // Break here
            __NOP();
        }
    }
}

// Find out the EXTI line number while debugging
static void furi_hal_os_exti_dbg_trap(uint32_t exti, uint32_t val) {
    for(uint32_t i = 0; val; val >>= 1U, ++i) {
        if(val & 1U) {
            (void)exti;
            (void)i;
            // Break here
            __NOP();
        }
    }
}
#endif

static inline bool furi_hal_os_is_pending_irq(void) {
    if(FURI_HAL_OS_NVIC_IS_PENDING()) {
#ifdef FURI_HAL_OS_DEBUG
        furi_hal_os_nvic_dbg_trap();
#endif
        return true;
    }

    uint32_t exti_lines_active;
    if((exti_lines_active = LL_EXTI_ReadFlag_0_31(LL_EXTI_LINE_ALL_0_31))) {
#ifdef FURI_HAL_OS_DEBUG
        furi_hal_os_exti_dbg_trap(FURI_HAL_OS_EXTI_LINE_0_31, exti_lines_active);
#endif
        return true;
    } else if((exti_lines_active = LL_EXTI_ReadFlag_32_63(LL_EXTI_LINE_ALL_32_63))) {
#ifdef FURI_HAL_OS_DEBUG
        furi_hal_os_exti_dbg_trap(FURI_HAL_OS_EXTI_LINE_32_63, exti_lines_active);
#endif
        return true;
    }

    return false;
}

static inline uint32_t furi_hal_os_sleep(TickType_t expected_idle_ticks) {
    // Stop ticks
    furi_hal_clock_suspend_tick();

    // Start wakeup timer
    furi_hal_idle_timer_start(FURI_HAL_OS_TICKS_TO_IDLE_CNT(expected_idle_ticks));

#ifdef FURI_HAL_OS_DEBUG
    furi_hal_gpio_write(FURI_HAL_OS_DEBUG_AWAKE_GPIO, 0);
#endif

    // Go to sleep mode
    furi_hal_power_sleep();

#ifdef FURI_HAL_OS_DEBUG
    furi_hal_gpio_write(FURI_HAL_OS_DEBUG_AWAKE_GPIO, 1);
#endif

    // Calculate how much time we spent in the sleep
    uint32_t after_cnt = furi_hal_idle_timer_get_cnt() + furi_hal_os_skew + FURI_HAL_OS_EXTRA_CNT;
    uint32_t after_tick = FURI_HAL_OS_IDLE_CNT_TO_TICKS(after_cnt);
    furi_hal_os_skew = after_cnt - FURI_HAL_OS_TICKS_TO_IDLE_CNT(after_tick);

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

    // Core2 shenanigans takes extra time, so we want to compensate tick skew by reducing sleep duration by 1 tick
    TickType_t unexpected_idle_ticks = expected_idle_ticks - 1;

    // Limit amount of ticks to maximum that timer can count
    if(unexpected_idle_ticks > FURI_HAL_OS_MAX_SLEEP) {
        unexpected_idle_ticks = FURI_HAL_OS_MAX_SLEEP;
    }

    // Stop IRQ handling, no one should disturb us till we finish
    __disable_irq();
    do {
        // Confirm OS that sleep is still possible
        if(eTaskConfirmSleepModeStatus() == eAbortSleep || furi_hal_os_is_pending_irq()) {
            break;
        }

        // Sleep and track how much ticks we spent sleeping
        uint32_t completed_ticks = furi_hal_os_sleep(unexpected_idle_ticks);
        // Notify system about time spent in sleep
        if(completed_ticks > 0) {
            if(completed_ticks > expected_idle_ticks) {
#ifdef FURI_HAL_OS_DEBUG
                furi_log_print_raw_format(
                    FuriLogLevelDebug, ">%lu\r\n", completed_ticks - expected_idle_ticks);
#endif
                completed_ticks = expected_idle_ticks;
            }
            vTaskStepTick(completed_ticks);
        }
    } while(0);
    // Reenable IRQ
    __enable_irq();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    UNUSED(xTask);
    furi_log_puts("\r\n\r\n stack overflow in ");
    furi_log_puts(pcTaskName);
    furi_log_puts("\r\n\r\n");
    furi_crash("StackOverflow");
}
