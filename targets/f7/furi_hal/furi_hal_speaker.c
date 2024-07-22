#include <furi_hal_speaker.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <furi_hal_power.h>
#include <furi_hal_bus.h>

#include <stm32wbxx_ll_tim.h>
#include <furi_hal_cortex.h>

#define TAG "FuriHalSpeaker"

#define FURI_HAL_SPEAKER_TIMER      TIM16
#define FURI_HAL_SPEAKER_CHANNEL    LL_TIM_CHANNEL_CH1
#define FURI_HAL_SPEAKER_PRESCALER  500
#define FURI_HAL_SPEAKER_MAX_VOLUME 60

static FuriMutex* furi_hal_speaker_mutex = NULL;

// #define FURI_HAL_SPEAKER_NEW_VOLUME

void furi_hal_speaker_init(void) {
    furi_assert(furi_hal_speaker_mutex == NULL);
    furi_hal_speaker_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_speaker_deinit(void) {
    furi_check(furi_hal_speaker_mutex != NULL);
    furi_mutex_free(furi_hal_speaker_mutex);
    furi_hal_speaker_mutex = NULL;
}

bool furi_hal_speaker_acquire(uint32_t timeout) {
    furi_check(!FURI_IS_IRQ_MODE());

    if(furi_mutex_acquire(furi_hal_speaker_mutex, timeout) == FuriStatusOk) {
        furi_hal_power_insomnia_enter();
        furi_hal_bus_enable(FuriHalBusTIM16);
        furi_hal_gpio_init_ex(
            &gpio_speaker, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn14TIM16);
        return true;
    } else {
        return false;
    }
}

void furi_hal_speaker_release(void) {
    furi_check(!FURI_IS_IRQ_MODE());
    furi_check(furi_hal_speaker_is_mine());

    furi_hal_speaker_stop();
    furi_hal_gpio_init(&gpio_speaker, GpioModeAnalog, GpioPullDown, GpioSpeedLow);

    furi_hal_bus_disable(FuriHalBusTIM16);
    furi_hal_power_insomnia_exit();

    furi_check(furi_mutex_release(furi_hal_speaker_mutex) == FuriStatusOk);
}

bool furi_hal_speaker_is_mine(void) {
    return (FURI_IS_IRQ_MODE()) ||
           (furi_mutex_get_owner(furi_hal_speaker_mutex) == furi_thread_get_current_id());
}

static inline uint32_t furi_hal_speaker_calculate_autoreload(float frequency) {
    uint32_t autoreload = (SystemCoreClock / FURI_HAL_SPEAKER_PRESCALER / frequency) - 1;
    if(autoreload < 2) {
        autoreload = 2;
    } else if(autoreload > UINT16_MAX) {
        autoreload = UINT16_MAX;
    }

    return autoreload;
}

static inline uint32_t furi_hal_speaker_calculate_compare(float volume) {
    if(volume < 0) volume = 0;
    if(volume > 1) volume = 1;
    volume = volume * volume * volume;

#ifdef FURI_HAL_SPEAKER_NEW_VOLUME
    uint32_t compare_value = volume * FURI_HAL_SPEAKER_MAX_VOLUME;
    uint32_t clip_value = volume * LL_TIM_GetAutoReload(FURI_HAL_SPEAKER_TIMER) / 2;
    if(compare_value > clip_value) {
        compare_value = clip_value;
    }
#else
    uint32_t compare_value = volume * LL_TIM_GetAutoReload(FURI_HAL_SPEAKER_TIMER) / 2;
#endif

    if(compare_value == 0) {
        compare_value = 1;
    }

    return compare_value;
}

void furi_hal_speaker_start(float frequency, float volume) {
    furi_check(furi_hal_speaker_is_mine());

    if(volume <= 0) {
        furi_hal_speaker_stop();
        return;
    }

    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = FURI_HAL_SPEAKER_PRESCALER - 1;
    TIM_InitStruct.Autoreload = furi_hal_speaker_calculate_autoreload(frequency);
    LL_TIM_Init(FURI_HAL_SPEAKER_TIMER, &TIM_InitStruct);

    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
    TIM_OC_InitStruct.CompareValue = furi_hal_speaker_calculate_compare(volume);
    LL_TIM_OC_Init(FURI_HAL_SPEAKER_TIMER, FURI_HAL_SPEAKER_CHANNEL, &TIM_OC_InitStruct);

    LL_TIM_EnableAllOutputs(FURI_HAL_SPEAKER_TIMER);
    LL_TIM_EnableCounter(FURI_HAL_SPEAKER_TIMER);
}

void furi_hal_speaker_set_volume(float volume) {
    furi_check(furi_hal_speaker_is_mine());
    if(volume <= 0) {
        furi_hal_speaker_stop();
        return;
    }

#if FURI_HAL_SPEAKER_CHANNEL == LL_TIM_CHANNEL_CH1
    LL_TIM_OC_SetCompareCH1(FURI_HAL_SPEAKER_TIMER, furi_hal_speaker_calculate_compare(volume));
#else
#error Invalid channel
#endif
}

void furi_hal_speaker_stop(void) {
    furi_check(furi_hal_speaker_is_mine());
    LL_TIM_DisableAllOutputs(FURI_HAL_SPEAKER_TIMER);
    LL_TIM_DisableCounter(FURI_HAL_SPEAKER_TIMER);
}
