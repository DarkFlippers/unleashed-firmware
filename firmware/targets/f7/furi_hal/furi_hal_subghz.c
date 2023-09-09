#include <furi_hal_subghz.h>
#include <lib/subghz/devices/cc1101_configs.h>

#include <furi_hal_version.h>
#include <furi_hal_rtc.h>
#include <furi_hal_spi.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi_hal_bus.h>

#include <stm32wbxx_ll_dma.h>

#include <furi.h>
#include <cc1101.h>
#include <stdio.h>

#define TAG "FuriHalSubGhz"

static uint32_t furi_hal_subghz_debug_gpio_buff[2];

/* DMA Channels definition */
#define SUBGHZ_DMA DMA2
#define SUBGHZ_DMA_CH1_CHANNEL LL_DMA_CHANNEL_1
#define SUBGHZ_DMA_CH2_CHANNEL LL_DMA_CHANNEL_2
#define SUBGHZ_DMA_CH1_IRQ FuriHalInterruptIdDma2Ch1
#define SUBGHZ_DMA_CH1_DEF SUBGHZ_DMA, SUBGHZ_DMA_CH1_CHANNEL
#define SUBGHZ_DMA_CH2_DEF SUBGHZ_DMA, SUBGHZ_DMA_CH2_CHANNEL

/** SubGhz state */
typedef enum {
    SubGhzStateInit, /**< Init pending */

    SubGhzStateIdle, /**< Idle, energy save mode */

    SubGhzStateAsyncRx, /**< Async RX started */

    SubGhzStateAsyncTx, /**< Async TX started, DMA and timer is on */
    SubGhzStateAsyncTxLast, /**< Async TX continue, DMA completed and timer got last value to go */
    SubGhzStateAsyncTxEnd, /**< Async TX complete, cleanup needed */

} SubGhzState;

/** SubGhz regulation, receive transmission on the current frequency for the
 * region */
typedef enum {
    SubGhzRegulationOnlyRx, /**only Rx*/
    SubGhzRegulationTxRx, /**TxRx*/
} SubGhzRegulation;

typedef struct {
    volatile SubGhzState state;
    volatile SubGhzRegulation regulation;
    const GpioPin* async_mirror_pin;

    int8_t rolling_counter_mult;
    bool ext_power_amp : 1;
    bool dangerous_frequency_i : 1;
} FuriHalSubGhz;

volatile FuriHalSubGhz furi_hal_subghz = {
    .state = SubGhzStateInit,
    .regulation = SubGhzRegulationTxRx,
    .async_mirror_pin = NULL,
    .rolling_counter_mult = 1,
    .ext_power_amp = false,
    .dangerous_frequency_i = false,
};

int8_t furi_hal_subghz_get_rolling_counter_mult(void) {
    return furi_hal_subghz.rolling_counter_mult;
}

void furi_hal_subghz_set_rolling_counter_mult(int8_t mult) {
    furi_hal_subghz.rolling_counter_mult = mult;
}

void furi_hal_subghz_set_dangerous_frequency(bool state_i) {
    furi_hal_subghz.dangerous_frequency_i = state_i;
}

void furi_hal_subghz_set_ext_power_amp(bool enabled) {
    furi_hal_subghz.ext_power_amp = enabled;
}

bool furi_hal_subghz_get_ext_power_amp() {
    return furi_hal_subghz.ext_power_amp;
}

void furi_hal_subghz_set_async_mirror_pin(const GpioPin* pin) {
    furi_hal_subghz.async_mirror_pin = pin;
}

const GpioPin* furi_hal_subghz_get_data_gpio() {
    return &gpio_cc1101_g0;
}

void furi_hal_subghz_init() {
    furi_assert(furi_hal_subghz.state == SubGhzStateInit);
    furi_hal_subghz.state = SubGhzStateIdle;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);

#ifdef FURI_HAL_SUBGHZ_TX_GPIO
    furi_hal_gpio_init(&FURI_HAL_SUBGHZ_TX_GPIO, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
#endif

    // Reset
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_reset(&furi_hal_spi_bus_handle_subghz);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHighImpedance);

    // Prepare GD0 for power on self test
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    // GD0 low
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHW);
    while(furi_hal_gpio_read(&gpio_cc1101_g0) != false)
        ;

    // GD0 high
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHW | CC1101_IOCFG_INV);
    while(furi_hal_gpio_read(&gpio_cc1101_g0) != true)
        ;

    // Reset GD0 to floating state
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    // RF switches
    furi_hal_gpio_init(&gpio_rf_sw_0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG2, CC1101IocfgHW);

    // Go to sleep
    cc1101_shutdown(&furi_hal_spi_bus_handle_subghz);

    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_subghz_sleep() {
    furi_assert(furi_hal_subghz.state == SubGhzStateIdle);
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);

    cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);

    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    cc1101_shutdown(&furi_hal_spi_bus_handle_subghz);

    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_dump_state() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    printf(
        "[furi_hal_subghz] cc1101 chip %d, version %d\r\n",
        cc1101_get_partnumber(&furi_hal_spi_bus_handle_subghz),
        cc1101_get_version(&furi_hal_spi_bus_handle_subghz));
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_load_custom_preset(const uint8_t* preset_data) {
    //load config
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_reset(&furi_hal_spi_bus_handle_subghz);
    uint32_t i = 0;
    uint8_t pa[8] = {0};
    while(preset_data[i]) {
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, preset_data[i], preset_data[i + 1]);
        i += 2;
    }
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

    //load pa table
    memcpy(&pa[0], &preset_data[i + 2], 8);
    furi_hal_subghz_load_patable(pa);

    //show debug
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        i = 0;
        FURI_LOG_D(TAG, "Loading custom preset");
        while(preset_data[i]) {
            FURI_LOG_D(TAG, "Reg[%lu]: %02X=%02X", i, preset_data[i], preset_data[i + 1]);
            i += 2;
        }
        for(uint8_t y = i; y < i + 10; y++) {
            FURI_LOG_D(TAG, "PA[%u]:  %02X", y, preset_data[y]);
        }
    }
}

void furi_hal_subghz_load_registers(const uint8_t* data) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_reset(&furi_hal_spi_bus_handle_subghz);
    uint32_t i = 0;
    while(data[i]) {
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, data[i], data[i + 1]);
        i += 2;
    }
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_load_patable(const uint8_t data[8]) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_set_pa_table(&furi_hal_spi_bus_handle_subghz, data);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_write_packet(const uint8_t* data, uint8_t size) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_tx(&furi_hal_spi_bus_handle_subghz);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_FIFO, size);
    cc1101_write_fifo(&furi_hal_spi_bus_handle_subghz, data, size);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_flush_rx() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_rx(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_flush_tx() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_tx(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

bool furi_hal_subghz_rx_pipe_not_empty() {
    CC1101RxBytes status[1];
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_read_reg(
        &furi_hal_spi_bus_handle_subghz, (CC1101_STATUS_RXBYTES) | CC1101_BURST, (uint8_t*)status);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
    // TODO: Find reason why RXFIFO_OVERFLOW doesnt work correctly
    if(status->NUM_RXBYTES > 0) {
        return true;
    } else {
        return false;
    }
}

bool furi_hal_subghz_is_rx_data_crc_valid() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    uint8_t data[1];
    cc1101_read_reg(&furi_hal_spi_bus_handle_subghz, CC1101_STATUS_LQI | CC1101_BURST, data);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
    if(((data[0] >> 7) & 0x01)) {
        return true;
    } else {
        return false;
    }
}

void furi_hal_subghz_read_packet(uint8_t* data, uint8_t* size) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_read_fifo(&furi_hal_spi_bus_handle_subghz, data, size);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_shutdown() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    // Reset and shutdown
    cc1101_shutdown(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_reset() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);
    cc1101_reset(&furi_hal_spi_bus_handle_subghz);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_idle() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

void furi_hal_subghz_rx() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

bool furi_hal_subghz_tx() {
    if(furi_hal_subghz.regulation != SubGhzRegulationTxRx) return false;
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_switch_to_tx(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
    return true;
}

float furi_hal_subghz_get_rssi() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    int32_t rssi_dec = cc1101_get_rssi(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

    float rssi = rssi_dec;
    if(rssi_dec >= 128) {
        rssi = ((rssi - 256.0f) / 2.0f) - 74.0f;
    } else {
        rssi = (rssi / 2.0f) - 74.0f;
    }

    return rssi;
}

uint8_t furi_hal_subghz_get_lqi() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    uint8_t data[1];
    cc1101_read_reg(&furi_hal_spi_bus_handle_subghz, CC1101_STATUS_LQI | CC1101_BURST, data);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
    return data[0] & 0x7F;
}

/* 
 Modified by @tkerby & MX to the full YARD Stick One extended range of 281-361 MHz, 378-481 MHz, and 749-962 MHz. 
 These changes are at your own risk. The PLL may not lock and FZ devs have warned of possible damage!
 */

bool furi_hal_subghz_is_frequency_valid(uint32_t value) {
    if(!(value >= 281000000 && value <= 361000000) &&
       !(value >= 378000000 && value <= 481000000) &&
       !(value >= 749000000 && value <= 962000000)) {
        return false;
    }

    return true;
}

uint32_t furi_hal_subghz_set_frequency_and_path(uint32_t value) {
    // Set these values to the extended frequency range only. They dont define if you can transmit but do select the correct RF path
    value = furi_hal_subghz_set_frequency(value);
    if(value >= 281000000 && value <= 361000000) {
        furi_hal_subghz_set_path(FuriHalSubGhzPath315);
    } else if(value >= 378000000 && value <= 481000000) {
        furi_hal_subghz_set_path(FuriHalSubGhzPath433);
    } else if(value >= 749000000 && value <= 962000000) {
        furi_hal_subghz_set_path(FuriHalSubGhzPath868);
    } else {
        furi_crash("SubGhz: Incorrect frequency during set.");
    }
    return value;
}

bool furi_hal_subghz_is_tx_allowed(uint32_t value) {
    bool allow_extended_for_int = furi_hal_subghz.dangerous_frequency_i;

    if(!(allow_extended_for_int) &&
       !(value >= 299999755 && value <= 350000335) && // was increased from 348 to 350
       !(value >= 386999938 && value <= 467750000) && // was increased from 464 to 467.75
       !(value >= 778999847 && value <= 928000000)) {
        FURI_LOG_I(TAG, "Frequency blocked - outside default range");
        return false;
    } else if(
        (allow_extended_for_int) && //
        !furi_hal_subghz_is_frequency_valid(value)) {
        FURI_LOG_I(TAG, "Frequency blocked - outside dangerous range");
        return false;
    }

    return true;
}

uint32_t furi_hal_subghz_set_frequency(uint32_t value) {
    if(furi_hal_subghz_is_tx_allowed(value)) {
        furi_hal_subghz.regulation = SubGhzRegulationTxRx;
    } else {
        furi_hal_subghz.regulation = SubGhzRegulationOnlyRx;
    }

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    uint32_t real_frequency = cc1101_set_frequency(&furi_hal_spi_bus_handle_subghz, value);
    cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);

    while(true) {
        CC1101Status status = cc1101_get_status(&furi_hal_spi_bus_handle_subghz);
        if(status.STATE == CC1101StateIDLE) break;
    }

    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
    return real_frequency;
}

void furi_hal_subghz_set_path(FuriHalSubGhzPath path) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    if(path == FuriHalSubGhzPath433) {
        furi_hal_gpio_write(&gpio_rf_sw_0, 0);
        cc1101_write_reg(
            &furi_hal_spi_bus_handle_subghz, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);
    } else if(path == FuriHalSubGhzPath315) {
        furi_hal_gpio_write(&gpio_rf_sw_0, 1);
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG2, CC1101IocfgHW);
    } else if(path == FuriHalSubGhzPath868) {
        furi_hal_gpio_write(&gpio_rf_sw_0, 1);
        cc1101_write_reg(
            &furi_hal_spi_bus_handle_subghz, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);
    } else if(path == FuriHalSubGhzPathIsolate) {
        furi_hal_gpio_write(&gpio_rf_sw_0, 0);
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG2, CC1101IocfgHW);
    } else {
        furi_crash("SubGhz: Incorrect path during set.");
    }
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

static bool furi_hal_subghz_start_debug() {
    bool ret = false;
    if(furi_hal_subghz.async_mirror_pin != NULL) {
        furi_hal_gpio_init(
            furi_hal_subghz.async_mirror_pin,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh);
        ret = true;
    }
    return ret;
}

static bool furi_hal_subghz_stop_debug() {
    bool ret = false;
    if(furi_hal_subghz.async_mirror_pin != NULL) {
        furi_hal_gpio_init(
            furi_hal_subghz.async_mirror_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        ret = true;
    }
    return ret;
}

volatile uint32_t furi_hal_subghz_capture_delta_duration = 0;
volatile FuriHalSubGhzCaptureCallback furi_hal_subghz_capture_callback = NULL;
volatile void* furi_hal_subghz_capture_callback_context = NULL;

static void furi_hal_subghz_capture_ISR() {
    // Channel 1
    if(LL_TIM_IsActiveFlag_CC1(TIM2)) {
        LL_TIM_ClearFlag_CC1(TIM2);
        furi_hal_subghz_capture_delta_duration = LL_TIM_IC_GetCaptureCH1(TIM2);
        if(furi_hal_subghz_capture_callback) {
            if(furi_hal_subghz.async_mirror_pin != NULL)
                furi_hal_gpio_write(furi_hal_subghz.async_mirror_pin, false);

            furi_hal_subghz_capture_callback(
                true,
                furi_hal_subghz_capture_delta_duration,
                (void*)furi_hal_subghz_capture_callback_context);
        }
    }
    // Channel 2
    if(LL_TIM_IsActiveFlag_CC2(TIM2)) {
        LL_TIM_ClearFlag_CC2(TIM2);
        if(furi_hal_subghz_capture_callback) {
            if(furi_hal_subghz.async_mirror_pin != NULL)
                furi_hal_gpio_write(furi_hal_subghz.async_mirror_pin, true);

            furi_hal_subghz_capture_callback(
                false,
                LL_TIM_IC_GetCaptureCH2(TIM2) - furi_hal_subghz_capture_delta_duration,
                (void*)furi_hal_subghz_capture_callback_context);
        }
    }
}

void furi_hal_subghz_start_async_rx(FuriHalSubGhzCaptureCallback callback, void* context) {
    furi_assert(furi_hal_subghz.state == SubGhzStateIdle);
    furi_hal_subghz.state = SubGhzStateAsyncRx;

    furi_hal_subghz_capture_callback = callback;
    furi_hal_subghz_capture_callback_context = context;

    furi_hal_gpio_init_ex(
        &gpio_cc1101_g0, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

    furi_hal_bus_enable(FuriHalBusTIM2);

    // Timer: base
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0x7FFFFFFE;
    // Clock division for capture filter
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV4;
    LL_TIM_Init(TIM2, &TIM_InitStruct);

    // Timer: advanced
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(TIM2);
    LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_TI2FP2);
    LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_EnableMasterSlaveMode(TIM2);
    LL_TIM_DisableDMAReq_TRIG(TIM2);
    LL_TIM_DisableIT_TRIG(TIM2);

    // Timer: channel 1 indirect
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_INDIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_FALLING);

    // Timer: channel 2 direct
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_IC_SetFilter(
        TIM2,
        LL_TIM_CHANNEL_CH2,
        LL_TIM_IC_FILTER_FDIV32_N8); // Capture filter: 1/(64000000/64/4/32*8) = 16us

    // ISR setup
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, furi_hal_subghz_capture_ISR, NULL);

    // Interrupts and channels
    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    // Start timer
    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);

    // Start debug
    furi_hal_subghz_start_debug();

    // Switch to RX
    furi_hal_subghz_rx();

    // Clear the variable after the end of the session
    furi_hal_subghz_capture_delta_duration = 0;
}

void furi_hal_subghz_stop_async_rx() {
    furi_assert(furi_hal_subghz.state == SubGhzStateAsyncRx);
    furi_hal_subghz.state = SubGhzStateIdle;

    // Shutdown radio
    furi_hal_subghz_idle();

    FURI_CRITICAL_ENTER();
    furi_hal_bus_disable(FuriHalBusTIM2);

    // Stop debug
    furi_hal_subghz_stop_debug();

    FURI_CRITICAL_EXIT();
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);

    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

typedef struct {
    uint32_t* buffer;
    LevelDuration carry_ld;
    FuriHalSubGhzAsyncTxCallback callback;
    void* callback_context;
    uint64_t duty_high;
    uint64_t duty_low;
} FuriHalSubGhzAsyncTx;

static FuriHalSubGhzAsyncTx furi_hal_subghz_async_tx = {0};

static void furi_hal_subghz_async_tx_refill(uint32_t* buffer, size_t samples) {
    furi_assert(furi_hal_subghz.state == SubGhzStateAsyncTx);
    while(samples > 0) {
        bool is_odd = samples % 2;
        LevelDuration ld;
        if(level_duration_is_reset(furi_hal_subghz_async_tx.carry_ld)) {
            ld = furi_hal_subghz_async_tx.callback(furi_hal_subghz_async_tx.callback_context);
        } else {
            ld = furi_hal_subghz_async_tx.carry_ld;
            furi_hal_subghz_async_tx.carry_ld = level_duration_reset();
        }

        if(level_duration_is_wait(ld)) {
            *buffer = API_HAL_SUBGHZ_ASYNC_TX_GUARD_TIME;
            buffer++;
            samples--;
        } else if(level_duration_is_reset(ld)) {
            *buffer = 0;
            buffer++;
            samples--;
            LL_DMA_DisableIT_HT(SUBGHZ_DMA_CH1_DEF);
            LL_DMA_DisableIT_TC(SUBGHZ_DMA_CH1_DEF);
            LL_TIM_EnableIT_UPDATE(TIM2);
            break;
        } else {
            bool level = level_duration_get_level(ld);

            // Inject guard time if level is incorrect
            if(is_odd != level) {
                *buffer = API_HAL_SUBGHZ_ASYNC_TX_GUARD_TIME;
                buffer++;
                samples--;
                if(is_odd) {
                    furi_hal_subghz_async_tx.duty_high += API_HAL_SUBGHZ_ASYNC_TX_GUARD_TIME;
                } else {
                    furi_hal_subghz_async_tx.duty_low += API_HAL_SUBGHZ_ASYNC_TX_GUARD_TIME;
                }

                // Special case: prevent buffer overflow if sample is last
                if(samples == 0) {
                    furi_hal_subghz_async_tx.carry_ld = ld;
                    break;
                }
            }

            uint32_t duration = level_duration_get_duration(ld);
            furi_assert(duration > 0);
            *buffer = duration;
            buffer++;
            samples--;

            if(is_odd) {
                furi_hal_subghz_async_tx.duty_high += duration;
            } else {
                furi_hal_subghz_async_tx.duty_low += duration;
            }
        }
    }
}

static void furi_hal_subghz_async_tx_dma_isr() {
    furi_assert(furi_hal_subghz.state == SubGhzStateAsyncTx);

#if SUBGHZ_DMA_CH1_CHANNEL == LL_DMA_CHANNEL_1
    if(LL_DMA_IsActiveFlag_HT1(SUBGHZ_DMA)) {
        LL_DMA_ClearFlag_HT1(SUBGHZ_DMA);
        furi_hal_subghz_async_tx_refill(
            furi_hal_subghz_async_tx.buffer, API_HAL_SUBGHZ_ASYNC_TX_BUFFER_HALF);
    }
    if(LL_DMA_IsActiveFlag_TC1(SUBGHZ_DMA)) {
        LL_DMA_ClearFlag_TC1(SUBGHZ_DMA);
        furi_hal_subghz_async_tx_refill(
            furi_hal_subghz_async_tx.buffer + API_HAL_SUBGHZ_ASYNC_TX_BUFFER_HALF,
            API_HAL_SUBGHZ_ASYNC_TX_BUFFER_HALF);
    }
#else
#error Update this code. Would you kindly?
#endif
}

static void furi_hal_subghz_async_tx_timer_isr() {
    if(LL_TIM_IsActiveFlag_UPDATE(TIM2)) {
        LL_TIM_ClearFlag_UPDATE(TIM2);
        if(LL_TIM_GetAutoReload(TIM2) == 0) {
            if(furi_hal_subghz.state == SubGhzStateAsyncTx) {
                furi_hal_subghz.state = SubGhzStateAsyncTxLast;
                LL_DMA_DisableChannel(SUBGHZ_DMA_CH1_DEF);
            } else if(furi_hal_subghz.state == SubGhzStateAsyncTxLast) {
                furi_hal_subghz.state = SubGhzStateAsyncTxEnd;
                //forcibly pulls the pin to the ground so that there is no carrier
                furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullDown, GpioSpeedLow);
                LL_TIM_DisableCounter(TIM2);
            } else {
                furi_crash(NULL);
            }
        }
    }
}

bool furi_hal_subghz_start_async_tx(FuriHalSubGhzAsyncTxCallback callback, void* context) {
    furi_assert(furi_hal_subghz.state == SubGhzStateIdle);
    furi_assert(callback);

    //If transmission is prohibited by regional settings
    if(furi_hal_subghz.regulation != SubGhzRegulationTxRx) return false;

    furi_hal_subghz_async_tx.callback = callback;
    furi_hal_subghz_async_tx.callback_context = context;

    furi_hal_subghz.state = SubGhzStateAsyncTx;

    furi_hal_subghz_async_tx.duty_low = 0;
    furi_hal_subghz_async_tx.duty_high = 0;

    furi_hal_subghz_async_tx.buffer =
        malloc(API_HAL_SUBGHZ_ASYNC_TX_BUFFER_FULL * sizeof(uint32_t));

    // Connect CC1101_GD0 to TIM2 as output
    furi_hal_gpio_init_ex(
        &gpio_cc1101_g0, GpioModeAltFunctionPushPull, GpioPullDown, GpioSpeedLow, GpioAltFn1TIM2);

    // Configure DMA
    LL_DMA_InitTypeDef dma_config = {0};
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (TIM2->ARR);
    dma_config.MemoryOrM2MDstAddress = (uint32_t)furi_hal_subghz_async_tx.buffer;
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_CIRCULAR;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dma_config.NbData = API_HAL_SUBGHZ_ASYNC_TX_BUFFER_FULL;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    dma_config.Priority = LL_DMA_MODE_NORMAL;
    LL_DMA_Init(SUBGHZ_DMA_CH1_DEF, &dma_config);
    furi_hal_interrupt_set_isr(SUBGHZ_DMA_CH1_IRQ, furi_hal_subghz_async_tx_dma_isr, NULL);
    LL_DMA_EnableIT_TC(SUBGHZ_DMA_CH1_DEF);
    LL_DMA_EnableIT_HT(SUBGHZ_DMA_CH1_DEF);
    LL_DMA_EnableChannel(SUBGHZ_DMA_CH1_DEF);

    furi_hal_bus_enable(FuriHalBusTIM2);

    // Configure TIM2
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 1000;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_EnableARRPreload(TIM2);

    // Configure TIM2 CH2
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_TOGGLE;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = 0;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_LOW;
    LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableMasterSlaveMode(TIM2);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, furi_hal_subghz_async_tx_timer_isr, NULL);

    furi_hal_subghz_async_tx_refill(
        furi_hal_subghz_async_tx.buffer, API_HAL_SUBGHZ_ASYNC_TX_BUFFER_FULL);

    LL_TIM_EnableDMAReq_UPDATE(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    // Start counter
    LL_TIM_GenerateEvent_UPDATE(TIM2);
#ifdef FURI_HAL_SUBGHZ_TX_GPIO
    furi_hal_gpio_write(&FURI_HAL_SUBGHZ_TX_GPIO, true);
#endif
    furi_hal_subghz_tx();

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);

    // Start debug
    if(furi_hal_subghz_start_debug()) {
        const GpioPin* gpio = furi_hal_subghz.async_mirror_pin;
        // //Preparing bit mask
        // //Debug pin is may be only PORTB! (PB0, PB1, .., PB15)
        // furi_hal_subghz_debug_gpio_buff[0] = 0;
        // furi_hal_subghz_debug_gpio_buff[1] = 0;

        furi_hal_subghz_debug_gpio_buff[0] = (uint32_t)gpio->pin << GPIO_NUMBER;
        furi_hal_subghz_debug_gpio_buff[1] = gpio->pin;

        dma_config.MemoryOrM2MDstAddress = (uint32_t)furi_hal_subghz_debug_gpio_buff;
        dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (gpio->port->BSRR);
        dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
        dma_config.Mode = LL_DMA_MODE_CIRCULAR;
        dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
        dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
        dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
        dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
        dma_config.NbData = 2;
        dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
        dma_config.Priority = LL_DMA_PRIORITY_VERYHIGH;
        LL_DMA_Init(SUBGHZ_DMA_CH2_DEF, &dma_config);
        LL_DMA_SetDataLength(SUBGHZ_DMA_CH2_DEF, 2);
        LL_DMA_EnableChannel(SUBGHZ_DMA_CH2_DEF);
    }

    return true;
}

bool furi_hal_subghz_is_async_tx_complete() {
    return furi_hal_subghz.state == SubGhzStateAsyncTxEnd;
}

void furi_hal_subghz_stop_async_tx() {
    furi_assert(
        furi_hal_subghz.state == SubGhzStateAsyncTx ||
        furi_hal_subghz.state == SubGhzStateAsyncTxLast ||
        furi_hal_subghz.state == SubGhzStateAsyncTxEnd);

    // Shutdown radio
    furi_hal_subghz_idle();
#ifdef FURI_HAL_SUBGHZ_TX_GPIO
    furi_hal_gpio_write(&FURI_HAL_SUBGHZ_TX_GPIO, false);
#endif

    // Deinitialize Timer
    FURI_CRITICAL_ENTER();
    furi_hal_bus_disable(FuriHalBusTIM2);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);

    // Deinitialize DMA
    LL_DMA_DeInit(SUBGHZ_DMA_CH1_DEF);

    furi_hal_interrupt_set_isr(SUBGHZ_DMA_CH1_IRQ, NULL, NULL);

    // Deinitialize GPIO
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    // Stop debug
    if(furi_hal_subghz_stop_debug()) {
        LL_DMA_DisableChannel(SUBGHZ_DMA_CH2_DEF);
    }

    FURI_CRITICAL_EXIT();

    free(furi_hal_subghz_async_tx.buffer);

    float duty_cycle =
        100.0f * (float)furi_hal_subghz_async_tx.duty_high /
        ((float)furi_hal_subghz_async_tx.duty_low + (float)furi_hal_subghz_async_tx.duty_high);
    FURI_LOG_D(
        TAG,
        "Async TX Radio stats: on %0.0fus, off %0.0fus, DutyCycle: %0.0f%%",
        (double)furi_hal_subghz_async_tx.duty_high,
        (double)furi_hal_subghz_async_tx.duty_low,
        (double)duty_cycle);

    furi_hal_subghz.state = SubGhzStateIdle;
}
