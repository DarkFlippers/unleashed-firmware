#include <furi_hal_adc.h>
#include <furi_hal_bus.h>
#include <furi_hal_cortex.h>
#include <furi_hal_power.h>

#include <furi.h>

#include <stm32wbxx_ll_adc.h>
#include <stm32wbxx_ll_system.h>

struct FuriHalAdcHandle {
    ADC_TypeDef* adc;
    FuriMutex* mutex;
    uint32_t full_scale;
};

static const uint32_t furi_hal_adc_clock[] = {
    [FuriHalAdcClockSync16] = LL_ADC_CLOCK_SYNC_PCLK_DIV4,
    [FuriHalAdcClockSync32] = LL_ADC_CLOCK_SYNC_PCLK_DIV2,
    [FuriHalAdcClockSync64] = LL_ADC_CLOCK_SYNC_PCLK_DIV1,
};

static const uint8_t furi_hal_adc_clock_div[] = {
    [FuriHalAdcClockSync16] = 4,
    [FuriHalAdcClockSync32] = 2,
    [FuriHalAdcClockSync64] = 1,
};

static const uint32_t furi_hal_adc_oversample_ratio[] = {
    [FuriHalAdcOversample2] = LL_ADC_OVS_RATIO_2,
    [FuriHalAdcOversample4] = LL_ADC_OVS_RATIO_4,
    [FuriHalAdcOversample8] = LL_ADC_OVS_RATIO_8,
    [FuriHalAdcOversample16] = LL_ADC_OVS_RATIO_16,
    [FuriHalAdcOversample32] = LL_ADC_OVS_RATIO_32,
    [FuriHalAdcOversample64] = LL_ADC_OVS_RATIO_64,
    [FuriHalAdcOversample128] = LL_ADC_OVS_RATIO_128,
    [FuriHalAdcOversample256] = LL_ADC_OVS_RATIO_256,
};

static const uint32_t furi_hal_adc_oversample_shift[] = {
    [FuriHalAdcOversample2] = LL_ADC_OVS_SHIFT_RIGHT_1,
    [FuriHalAdcOversample4] = LL_ADC_OVS_SHIFT_RIGHT_2,
    [FuriHalAdcOversample8] = LL_ADC_OVS_SHIFT_RIGHT_3,
    [FuriHalAdcOversample16] = LL_ADC_OVS_SHIFT_RIGHT_4,
    [FuriHalAdcOversample32] = LL_ADC_OVS_SHIFT_RIGHT_5,
    [FuriHalAdcOversample64] = LL_ADC_OVS_SHIFT_RIGHT_6,
    [FuriHalAdcOversample128] = LL_ADC_OVS_SHIFT_RIGHT_7,
    [FuriHalAdcOversample256] = LL_ADC_OVS_SHIFT_RIGHT_8,
};

static const uint32_t furi_hal_adc_sampling_time[] = {
    [FuriHalAdcSamplingtime2_5] = LL_ADC_SAMPLINGTIME_2CYCLES_5,
    [FuriHalAdcSamplingtime6_5] = LL_ADC_SAMPLINGTIME_6CYCLES_5,
    [FuriHalAdcSamplingtime12_5] = LL_ADC_SAMPLINGTIME_12CYCLES_5,
    [FuriHalAdcSamplingtime24_5] = LL_ADC_SAMPLINGTIME_24CYCLES_5,
    [FuriHalAdcSamplingtime47_5] = LL_ADC_SAMPLINGTIME_47CYCLES_5,
    [FuriHalAdcSamplingtime92_5] = LL_ADC_SAMPLINGTIME_92CYCLES_5,
    [FuriHalAdcSamplingtime247_5] = LL_ADC_SAMPLINGTIME_247CYCLES_5,
    [FuriHalAdcSamplingtime640_5] = LL_ADC_SAMPLINGTIME_640CYCLES_5,
};

static const uint32_t furi_hal_adc_channel_map[] = {
    [FuriHalAdcChannel0] = LL_ADC_CHANNEL_0,
    [FuriHalAdcChannel1] = LL_ADC_CHANNEL_1,
    [FuriHalAdcChannel2] = LL_ADC_CHANNEL_2,
    [FuriHalAdcChannel3] = LL_ADC_CHANNEL_3,
    [FuriHalAdcChannel4] = LL_ADC_CHANNEL_4,
    [FuriHalAdcChannel5] = LL_ADC_CHANNEL_5,
    [FuriHalAdcChannel6] = LL_ADC_CHANNEL_6,
    [FuriHalAdcChannel7] = LL_ADC_CHANNEL_7,
    [FuriHalAdcChannel8] = LL_ADC_CHANNEL_8,
    [FuriHalAdcChannel9] = LL_ADC_CHANNEL_9,
    [FuriHalAdcChannel10] = LL_ADC_CHANNEL_10,
    [FuriHalAdcChannel11] = LL_ADC_CHANNEL_11,
    [FuriHalAdcChannel12] = LL_ADC_CHANNEL_12,
    [FuriHalAdcChannel13] = LL_ADC_CHANNEL_13,
    [FuriHalAdcChannel14] = LL_ADC_CHANNEL_14,
    [FuriHalAdcChannel15] = LL_ADC_CHANNEL_15,
    [FuriHalAdcChannel16] = LL_ADC_CHANNEL_16,
    [FuriHalAdcChannel17] = LL_ADC_CHANNEL_17,
    [FuriHalAdcChannel18] = LL_ADC_CHANNEL_18,
    [FuriHalAdcChannelVREFINT] = LL_ADC_CHANNEL_VREFINT,
    [FuriHalAdcChannelTEMPSENSOR] = LL_ADC_CHANNEL_TEMPSENSOR,
    [FuriHalAdcChannelVBAT] = LL_ADC_CHANNEL_VBAT,
};

static FuriHalAdcHandle* furi_hal_adc_handle = NULL;

void furi_hal_adc_init(void) {
    furi_hal_adc_handle = malloc(sizeof(FuriHalAdcHandle));
    furi_hal_adc_handle->adc = ADC1;
    furi_hal_adc_handle->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
}

FuriHalAdcHandle* furi_hal_adc_acquire(void) {
    furi_check(furi_mutex_acquire(furi_hal_adc_handle->mutex, FuriWaitForever) == FuriStatusOk);

    furi_hal_power_insomnia_enter();

    furi_hal_bus_enable(FuriHalBusADC);

    return furi_hal_adc_handle;
}

void furi_hal_adc_release(FuriHalAdcHandle* handle) {
    furi_check(handle);

    if(furi_hal_bus_is_enabled(FuriHalBusADC)) furi_hal_bus_disable(FuriHalBusADC);

    LL_VREFBUF_Disable();
    LL_VREFBUF_EnableHIZ();

    furi_hal_power_insomnia_exit();

    furi_check(furi_mutex_release(furi_hal_adc_handle->mutex) == FuriStatusOk);
}

void furi_hal_adc_configure(FuriHalAdcHandle* handle) {
    furi_hal_adc_configure_ex(
        handle,
        FuriHalAdcScale2048,
        FuriHalAdcClockSync64,
        FuriHalAdcOversample64,
        FuriHalAdcSamplingtime247_5);
}

void furi_hal_adc_configure_ex(
    FuriHalAdcHandle* handle,
    FuriHalAdcScale scale,
    FuriHalAdcClock clock,
    FuriHalAdcOversample oversample,
    FuriHalAdcSamplingTime sampling_time) {
    furi_check(handle);
    furi_check(scale == FuriHalAdcScale2048 || scale == FuriHalAdcScale2500);
    furi_check(clock <= FuriHalAdcClockSync64);
    furi_check(oversample <= FuriHalAdcOversampleNone);
    furi_check(sampling_time <= FuriHalAdcSamplingtime640_5);

    FuriHalCortexTimer timer;

    if(furi_hal_bus_is_enabled(FuriHalBusADC)) furi_hal_bus_disable(FuriHalBusADC);

    uint32_t trim_value = 0;
    switch(scale) {
    case FuriHalAdcScale2048:
        LL_VREFBUF_SetVoltageScaling(LL_VREFBUF_VOLTAGE_SCALE0);
        trim_value = LL_VREFBUF_SC0_GetCalibration() & 0x3FU;
        handle->full_scale = 2048;
        break;
    case FuriHalAdcScale2500:
        LL_VREFBUF_SetVoltageScaling(LL_VREFBUF_VOLTAGE_SCALE1);
        trim_value = LL_VREFBUF_SC1_GetCalibration() & 0x3FU;
        handle->full_scale = 2500;
        break;
    default:
        furi_crash();
    }
    LL_VREFBUF_SetTrimming(trim_value);
    LL_VREFBUF_Enable();
    LL_VREFBUF_DisableHIZ();

    timer = furi_hal_cortex_timer_get(500000); // 500ms to stabilize VREF
    while(!LL_VREFBUF_IsVREFReady()) {
        furi_check(!furi_hal_cortex_timer_is_expired(timer), "VREF fail");
    };

    furi_hal_bus_enable(FuriHalBusADC);

    // ADC Common config
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};
    ADC_CommonInitStruct.CommonClock = furi_hal_adc_clock[clock];
    furi_check(
        LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(handle->adc), &ADC_CommonInitStruct) ==
        SUCCESS);
    LL_ADC_SetCommonPathInternalCh(
        __LL_ADC_COMMON_INSTANCE(handle->adc),
        LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_TEMPSENSOR |
            LL_ADC_PATH_INTERNAL_VBAT);

    // ADC config part 1
    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B; //-V1048
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
    furi_check(LL_ADC_Init(handle->adc, &ADC_InitStruct) == SUCCESS);

    // ADC config part 2: groups parameters
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE; //-V1048
    ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
    furi_check(LL_ADC_REG_Init(handle->adc, &ADC_REG_InitStruct) == SUCCESS);

    // ADC config part 3: sequencer and channels
    if(oversample == FuriHalAdcOversampleNone) {
        LL_ADC_SetOverSamplingScope(handle->adc, LL_ADC_OVS_DISABLE);
    } else {
        LL_ADC_SetOverSamplingScope(handle->adc, LL_ADC_OVS_GRP_REGULAR_CONTINUED);
        LL_ADC_ConfigOverSamplingRatioShift(
            handle->adc,
            furi_hal_adc_oversample_ratio[oversample],
            furi_hal_adc_oversample_shift[oversample]);
    }

    for(FuriHalAdcChannel channel = FuriHalAdcChannel0; channel < FuriHalAdcChannelNone;
        channel++) {
        // 47.5 cycles on 64MHz is first meaningful value for internal sources sampling
        LL_ADC_SetChannelSamplingTime(
            handle->adc,
            furi_hal_adc_channel_map[channel],
            furi_hal_adc_sampling_time[sampling_time]);
        LL_ADC_SetChannelSingleDiff(
            handle->adc, furi_hal_adc_channel_map[channel], LL_ADC_SINGLE_ENDED);
    }

    // Disable ADC deep power down (enabled by default after reset state)
    LL_ADC_DisableDeepPowerDown(handle->adc);

    // Enable ADC internal voltage regulator
    LL_ADC_EnableInternalRegulator(handle->adc);
    // Delay for ADC internal voltage regulator stabilization.
    timer = furi_hal_cortex_timer_get(LL_ADC_DELAY_INTERNAL_REGUL_STAB_US);
    while(!furi_hal_cortex_timer_is_expired(timer))
        ;

    // Run ADC self calibration
    LL_ADC_StartCalibration(handle->adc, LL_ADC_SINGLE_ENDED);
    // Poll for ADC effectively calibrated
    while(LL_ADC_IsCalibrationOnGoing(handle->adc) != 0)
        ;
    // Delay between ADC end of calibration and ADC enable
    size_t end =
        DWT->CYCCNT + (LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * furi_hal_adc_clock_div[clock]);
    while(DWT->CYCCNT < end)
        ;

    // Enable ADC
    LL_ADC_ClearFlag_ADRDY(handle->adc);
    LL_ADC_Enable(handle->adc);
    while(LL_ADC_IsActiveFlag_ADRDY(handle->adc) == 0)
        ;
}

uint16_t furi_hal_adc_read(FuriHalAdcHandle* handle, FuriHalAdcChannel channel) {
    furi_check(handle);
    furi_check(channel <= FuriHalAdcChannelVBAT);
    furi_check(LL_ADC_IsEnabled(handle->adc) == 1);
    furi_check(LL_ADC_IsDisableOngoing(handle->adc) == 0);
    furi_check(LL_ADC_REG_IsConversionOngoing(handle->adc) == 0);

    LL_ADC_REG_SetSequencerRanks(
        handle->adc, LL_ADC_REG_RANK_1, furi_hal_adc_channel_map[channel]);

    LL_ADC_REG_StartConversion(handle->adc);

    while(LL_ADC_IsActiveFlag_EOC(handle->adc) == 0)
        ;
    uint16_t value = LL_ADC_REG_ReadConversionData12(handle->adc);

    return value;
}

float furi_hal_adc_convert_to_voltage(FuriHalAdcHandle* handle, uint16_t value) {
    return (float)__LL_ADC_CALC_DATA_TO_VOLTAGE(handle->full_scale, value, LL_ADC_RESOLUTION_12B);
}

float furi_hal_adc_convert_vref(FuriHalAdcHandle* handle, uint16_t value) {
    UNUSED(handle);
    return (float)__LL_ADC_CALC_VREFANALOG_VOLTAGE(value, LL_ADC_RESOLUTION_12B);
}

float furi_hal_adc_convert_temp(FuriHalAdcHandle* handle, uint16_t value) {
    return (float)__LL_ADC_CALC_TEMPERATURE(handle->full_scale, value, LL_ADC_RESOLUTION_12B);
}

float furi_hal_adc_convert_vbat(FuriHalAdcHandle* handle, uint16_t value) {
    return furi_hal_adc_convert_to_voltage(handle, value) * 3;
}
