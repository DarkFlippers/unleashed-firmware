#include "furi_hal_nfc_i.h"
#include "furi_hal_nfc_tech_i.h"

#include <stm32wbxx_ll_tim.h>

#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi_hal_bus.h>

#define TAG "FuriHalNfcTimer"

#define FURI_HAL_NFC_TIMER_US_IN_S (1000000UL)

/**
 * To enable timer debug output on GPIO, define the FURI_HAL_NFC_TIMER_DEBUG macro
 * Example: ./fbt --extra-define=FURI_HAL_NFC_TIMER_DEBUG
 */

typedef enum {
    FuriHalNfcTimerFwt,
    FuriHalNfcTimerBlockTx,
    FuriHalNfcTimerCount,
} FuriHalNfcTimer;

typedef struct {
    TIM_TypeDef* timer;
    FuriHalBus bus;
    uint32_t prescaler;
    uint32_t freq_khz;
    FuriHalNfcEventInternalType event;
    FuriHalInterruptId irq_id;
    IRQn_Type irq_type;
#ifdef FURI_HAL_NFC_TIMER_DEBUG
    const GpioPin* pin;
#endif
} FuriHalNfcTimerConfig;

static const FuriHalNfcTimerConfig furi_hal_nfc_timers[FuriHalNfcTimerCount] = {
    [FuriHalNfcTimerFwt] =
        {
            .timer = TIM1,
            .bus = FuriHalBusTIM1,
            .event = FuriHalNfcEventInternalTypeTimerFwtExpired,
            .irq_id = FuriHalInterruptIdTim1UpTim16,
            .irq_type = TIM1_UP_TIM16_IRQn,
#ifdef FURI_HAL_NFC_TIMER_DEBUG
            .pin = &gpio_ext_pa7,
#endif
        },
    [FuriHalNfcTimerBlockTx] =
        {
            .timer = TIM17,
            .bus = FuriHalBusTIM17,
            .event = FuriHalNfcEventInternalTypeTimerBlockTxExpired,
            .irq_id = FuriHalInterruptIdTim1TrgComTim17,
            .irq_type = TIM1_TRG_COM_TIM17_IRQn,
#ifdef FURI_HAL_NFC_TIMER_DEBUG
            .pin = &gpio_ext_pa6,
#endif
        },
};

static void furi_hal_nfc_timer_irq_callback(void* context) {
    // Returning removed const-ness
    const FuriHalNfcTimerConfig* config = context;
    if(LL_TIM_IsActiveFlag_UPDATE(config->timer)) {
        LL_TIM_ClearFlag_UPDATE(config->timer);
        furi_hal_nfc_event_set(config->event);
#ifdef FURI_HAL_NFC_TIMER_DEBUG
        furi_hal_gpio_write(timer_config->pin, false);
#endif
    }
}

static void furi_hal_nfc_timer_init(FuriHalNfcTimer timer) {
    const FuriHalNfcTimerConfig* config = &furi_hal_nfc_timers[timer];

    furi_hal_bus_enable(config->bus);

    LL_TIM_SetOnePulseMode(config->timer, LL_TIM_ONEPULSEMODE_SINGLE);
    LL_TIM_EnableUpdateEvent(config->timer);
    LL_TIM_SetCounterMode(config->timer, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockSource(config->timer, LL_TIM_CLOCKSOURCE_INTERNAL);

    furi_hal_interrupt_set_isr(
        config->irq_id,
        furi_hal_nfc_timer_irq_callback,
        // Warning: casting const-ness away
        (FuriHalNfcTimerConfig*)config);
    NVIC_SetPriority(config->irq_type, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(config->irq_type);
#ifdef FURI_HAL_NFC_TIMER_DEBUG
    furi_hal_gpio_init(config->pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(config->pin, false);
#endif
}

static void furi_hal_nfc_timer_deinit(FuriHalNfcTimer timer) {
    const FuriHalNfcTimerConfig* config = &furi_hal_nfc_timers[timer];

    LL_TIM_ClearFlag_UPDATE(config->timer);
    furi_hal_interrupt_set_isr(config->irq_id, NULL, NULL);
    NVIC_DisableIRQ(config->irq_type);

    if(furi_hal_bus_is_enabled(config->bus)) {
        furi_hal_bus_disable(config->bus);
    }
#ifdef FURI_HAL_NFC_TIMER_DEBUG
    furi_hal_gpio_init_simple(config->pin, GpioModeAnalog);
    furi_hal_gpio_write(config->pin, false);
#endif
}

static int32_t furi_hal_nfc_timer_get_compensation(FuriHalNfcTimer timer) {
    const FuriHalNfcTechBase* current_tech = furi_hal_nfc_tech[furi_hal_nfc.tech];

    if(furi_hal_nfc.mode == FuriHalNfcModePoller) {
        const FuriHalNfcPollerCompensation* comp = &current_tech->poller.compensation;
        if(timer == FuriHalNfcTimerFwt)
            return comp->fwt;
        else if(timer == FuriHalNfcTimerBlockTx)
            return comp->fdt;

    } else if(furi_hal_nfc.mode == FuriHalNfcModeListener) {
        const FuriHalNfcListenerCompensation* comp = &current_tech->listener.compensation;
        if(timer == FuriHalNfcTimerBlockTx) return comp->fdt;
    }

    return 0;
}

static inline bool furi_hal_nfc_timer_is_running(FuriHalNfcTimer timer) {
    return LL_TIM_IsEnabledCounter(furi_hal_nfc_timers[timer].timer) != 0;
}

static void furi_hal_nfc_timer_start_core_ticks(FuriHalNfcTimer timer, uint64_t core_ticks) {
    furi_check(!furi_hal_nfc_timer_is_running(timer));

    const FuriHalNfcTimerConfig* config = &furi_hal_nfc_timers[timer];
    furi_check(furi_hal_bus_is_enabled(config->bus));

    const uint32_t prescaler = (core_ticks - 1) / UINT16_MAX;
    furi_check(prescaler <= UINT16_MAX);

    const uint32_t arr_reg = core_ticks / (prescaler + 1);
    furi_check(arr_reg <= UINT16_MAX);

    LL_TIM_DisableIT_UPDATE(config->timer);

    LL_TIM_SetPrescaler(config->timer, prescaler);
    LL_TIM_SetAutoReload(config->timer, arr_reg);

    LL_TIM_GenerateEvent_UPDATE(config->timer);
    while(!LL_TIM_IsActiveFlag_UPDATE(config->timer))
        ;
    LL_TIM_ClearFlag_UPDATE(config->timer);

    LL_TIM_EnableIT_UPDATE(config->timer);
    LL_TIM_EnableCounter(config->timer);
#ifdef FURI_HAL_NFC_TIMER_DEBUG
    furi_hal_gpio_write(config->pin, true);
#endif
}

static void furi_hal_nfc_timer_start_us(FuriHalNfcTimer timer, uint32_t time_us) {
    furi_hal_nfc_timer_start_core_ticks(
        timer, SystemCoreClock / FURI_HAL_NFC_TIMER_US_IN_S * time_us);
}

static void furi_hal_nfc_timer_start_fc(FuriHalNfcTimer timer, uint32_t time_fc) {
    const int32_t comp_fc = furi_hal_nfc_timer_get_compensation(timer);
    // Not starting the timer if the compensation value is greater than the requested delay
    if(comp_fc >= (int32_t)time_fc) return;

    furi_hal_nfc_timer_start_core_ticks(
        timer, ((uint64_t)SystemCoreClock * (time_fc - comp_fc)) / FURI_HAL_NFC_CARRIER_HZ);
}

static void furi_hal_nfc_timer_stop(FuriHalNfcTimer timer) {
    const FuriHalNfcTimerConfig* config = &furi_hal_nfc_timers[timer];

    LL_TIM_DisableIT_UPDATE(config->timer);
    LL_TIM_DisableCounter(config->timer);
    LL_TIM_SetCounter(config->timer, 0);
    LL_TIM_SetAutoReload(config->timer, 0);

    if(LL_TIM_IsActiveFlag_UPDATE(config->timer)) {
        LL_TIM_ClearFlag_UPDATE(config->timer);
    }
#ifdef FURI_HAL_NFC_TIMER_DEBUG
    furi_hal_gpio_write(config->pin, false);
#endif
}

void furi_hal_nfc_timers_init(void) {
    for(size_t i = 0; i < FuriHalNfcTimerCount; i++) {
        furi_hal_nfc_timer_init(i);
    }
}

void furi_hal_nfc_timers_deinit(void) {
    for(size_t i = 0; i < FuriHalNfcTimerCount; i++) {
        furi_hal_nfc_timer_deinit(i);
    }
}

void furi_hal_nfc_timer_fwt_start(uint32_t time_fc) {
    furi_hal_nfc_timer_start_fc(FuriHalNfcTimerFwt, time_fc);
}

void furi_hal_nfc_timer_fwt_stop(void) {
    furi_hal_nfc_timer_stop(FuriHalNfcTimerFwt);
}

void furi_hal_nfc_timer_block_tx_start(uint32_t time_fc) {
    furi_hal_nfc_timer_start_fc(FuriHalNfcTimerBlockTx, time_fc);
}

void furi_hal_nfc_timer_block_tx_start_us(uint32_t time_us) {
    furi_hal_nfc_timer_start_us(FuriHalNfcTimerBlockTx, time_us);
}

void furi_hal_nfc_timer_block_tx_stop(void) {
    furi_hal_nfc_timer_stop(FuriHalNfcTimerBlockTx);
}

bool furi_hal_nfc_timer_block_tx_is_running(void) {
    return furi_hal_nfc_timer_is_running(FuriHalNfcTimerBlockTx);
}
