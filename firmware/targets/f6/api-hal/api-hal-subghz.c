#include "api-hal-subghz.h"

#include <api-hal-gpio.h>
#include <api-hal-spi.h>
#include <api-hal-interrupt.h>
#include <api-hal-resources.h>

#include <furi.h>
#include <cc1101.h>
#include <stdio.h>

static const uint8_t api_hal_subghz_preset_ook_async_regs[][2] = {
    /* Base setting */
    { CC1101_IOCFG0,    0x0D }, // GD0 as async serial data output/input
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle to TRX, ~150us OSC guard time

    /* Async OOK Specific things */
    { CC1101_MDMCFG2,   0x30 }, // ASK/OOK, No preamble/sync
    { CC1101_PKTCTRL0,  0x32 }, // Async, no CRC, Infinite
    { CC1101_FREND0,    0x01 }, // OOK/ASK PATABLE

    /* End  */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_ook_async_patable[8] = {
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t api_hal_subghz_preset_mp_regs[][2] = {
    { CC1101_IOCFG0, 0x0D },
    { CC1101_FIFOTHR, 0x07 },
    { CC1101_PKTCTRL0, 0x32 },
    //{ CC1101_FSCTRL1,  0x0E },
    { CC1101_FSCTRL1, 0x06 },
    { CC1101_FREQ2, 0x10 },
    { CC1101_FREQ1, 0xB0 },
    { CC1101_FREQ0, 0x7F },
    { CC1101_MDMCFG4, 0x17 },
    { CC1101_MDMCFG3, 0x32 },
    { CC1101_MDMCFG2, 0x30 },   //<---OOK/ASK
    { CC1101_MDMCFG1, 0x23 },
    { CC1101_MDMCFG0, 0xF8 },
    { CC1101_MCSM0, 0x18 },
    { CC1101_FOCCFG, 0x18 },
    { CC1101_AGCTRL2, 0x07 },
    { CC1101_AGCTRL1, 0x00 },
    { CC1101_AGCTRL0, 0x91 },
    { CC1101_WORCTRL, 0xFB },
    { CC1101_FREND1, 0xB6 },
    //{ CC1101_FREND0,   0x11 },
    { CC1101_FREND0, 0x01 },
    { CC1101_FSCAL3, 0xE9 },
    { CC1101_FSCAL2, 0x2A },
    { CC1101_FSCAL1, 0x00 },
    { CC1101_FSCAL0, 0x1F },
    { CC1101_TEST2, 0x88 },
    { CC1101_TEST1, 0x31 },
    { CC1101_TEST0, 0x09 },

    /* End  */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_mp_patable[8] = {
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t api_hal_subghz_preset_2fsk_packet_regs[][2] = {
    /* Base setting */
    { CC1101_IOCFG0,    0x06 }, // GD0 as async serial data output/input
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle to TRX, ~150us OSC guard time

    /* Magic */
    { CC1101_TEST2,     0x81},
    { CC1101_TEST1,     0x35},
    { CC1101_TEST0,     0x09},

    /* End */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_2fsk_packet_patable[8] = {
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void api_hal_subghz_init() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    // Reset and shutdown
    cc1101_reset(device);

    // Prepare GD0 for power on self test
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    // GD0 low
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHW);
    while(hal_gpio_read(&gpio_cc1101_g0) != false);

    // GD0 high
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHW | CC1101_IOCFG_INV);
    while(hal_gpio_read(&gpio_cc1101_g0) != true);

    // Reset GD0 to floating state
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    // RF switches
    hal_gpio_init(&gpio_rf_sw_0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW);

    // Turn off oscillator
    cc1101_shutdown(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_dump_state() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    printf(
        "[api_hal_subghz] cc1101 chip %d, version %d\r\n",
        cc1101_get_partnumber(device),
        cc1101_get_version(device)
    );
    api_hal_spi_device_return(device);
}

void api_hal_subghz_load_preset(ApiHalSubGhzPreset preset) {
    if(preset == ApiHalSubGhzPresetOokAsync) {
        api_hal_subghz_load_registers(api_hal_subghz_preset_ook_async_regs);
        api_hal_subghz_load_patable(api_hal_subghz_preset_ook_async_patable);
    } else if(preset == ApiHalSubGhzPreset2FskPacket) {
        api_hal_subghz_load_registers(api_hal_subghz_preset_2fsk_packet_regs);
        api_hal_subghz_load_patable(api_hal_subghz_preset_2fsk_packet_patable);
    } else if(preset == ApiHalSubGhzPresetMP) {
        api_hal_subghz_load_registers(api_hal_subghz_preset_mp_regs);
        api_hal_subghz_load_patable(api_hal_subghz_preset_mp_patable);
    }
}

uint8_t api_hal_subghz_get_status() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    CC1101StatusRaw st;
    st.status = cc1101_get_status(device);
    api_hal_spi_device_return(device);
    return st.status_raw;
}

void api_hal_subghz_load_registers(const uint8_t data[][2]) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_reset(device);
    uint32_t i = 0;
    while (data[i][0]) {
        cc1101_write_reg(device, data[i][0], data[i][1]);
        i++;
    }
    api_hal_spi_device_return(device);
}

void api_hal_subghz_load_patable(const uint8_t data[8]) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_set_pa_table(device, data);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_write_packet(const uint8_t* data, uint8_t size) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_flush_tx(device);
    cc1101_write_fifo(device, data, size);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_flush_rx() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_flush_rx(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_read_packet(uint8_t* data, uint8_t* size) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_read_fifo(device, data, size);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_shutdown() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    // Reset and shutdown
    cc1101_shutdown(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_reset() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_reset(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_idle() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_switch_to_idle(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_rx() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_switch_to_rx(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_tx() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_switch_to_idle(device);
    cc1101_switch_to_tx(device);
    api_hal_spi_device_return(device);
}

float api_hal_subghz_get_rssi() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    int32_t rssi_dec = cc1101_get_rssi(device);
    api_hal_spi_device_return(device);

    float rssi = rssi_dec;
    if(rssi_dec >= 128) {
        rssi = ((rssi - 256.0f) / 2.0f) - 74.0f;
    } else {
        rssi = (rssi / 2.0f) - 74.0f;
    }

    return rssi;
}

uint32_t api_hal_subghz_set_frequency_and_path(uint32_t value) {
    value = api_hal_subghz_set_frequency(value);
    if(value >= 300000000 && value <= 348000335) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(value >= 387000000 && value <= 464000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath433);
    } else if(value >= 779000000 && value <= 928000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath868);
    } else {
        furi_check(0);
    }
    return value;
}

uint32_t api_hal_subghz_set_frequency(uint32_t value) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);

    // Compensate rounding
    if (value % cc1101_get_frequency_step(device) > (cc1101_get_frequency_step(device) / 2)) {
        value += cc1101_get_frequency_step(device);
    }

    uint32_t real_frequency = cc1101_set_frequency(device, value);
    cc1101_calibrate(device);

    api_hal_spi_device_return(device);

    return real_frequency;
}

void api_hal_subghz_set_path(ApiHalSubGhzPath path) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    if (path == ApiHalSubGhzPath433) {
        hal_gpio_write(&gpio_rf_sw_0, 0);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);
    } else if (path == ApiHalSubGhzPath315) {
        hal_gpio_write(&gpio_rf_sw_0, 1);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW);
    } else if (path == ApiHalSubGhzPath868) {
        hal_gpio_write(&gpio_rf_sw_0, 1);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);
    } else if (path == ApiHalSubGhzPathIsolate) {
        hal_gpio_write(&gpio_rf_sw_0, 0);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW);
    } else {
        furi_check(0);
    }
    api_hal_spi_device_return(device);
}

volatile uint32_t api_hal_subghz_capture_delta_duration = 0;
volatile ApiHalSubGhzCaptureCallback api_hal_subghz_capture_callback = NULL;
volatile void* api_hal_subghz_capture_callback_context = NULL;

void api_hal_subghz_set_capture_callback(ApiHalSubGhzCaptureCallback callback, void* context) {
    api_hal_subghz_capture_callback = callback;
    api_hal_subghz_capture_callback_context = context;
}

static void api_hal_subghz_capture_ISR() {
    // Channel 1
    if(LL_TIM_IsActiveFlag_CC1(TIM2)) {
        LL_TIM_ClearFlag_CC1(TIM2);
        api_hal_subghz_capture_delta_duration = LL_TIM_IC_GetCaptureCH1(TIM2);
        if (api_hal_subghz_capture_callback) {
            api_hal_subghz_capture_callback(
                ApiHalSubGhzCaptureLevelHigh,
                api_hal_subghz_capture_delta_duration,
                (void*)api_hal_subghz_capture_callback_context
            );
        }
    }
    // Channel 2
    if(LL_TIM_IsActiveFlag_CC2(TIM2)) {
        LL_TIM_ClearFlag_CC2(TIM2);
        if (api_hal_subghz_capture_callback) {
            api_hal_subghz_capture_callback(
                ApiHalSubGhzCaptureLevelLow,
                LL_TIM_IC_GetCaptureCH2(TIM2) - api_hal_subghz_capture_delta_duration,
                (void*)api_hal_subghz_capture_callback_context
            );
        }
    }
}

void api_hal_subghz_enable_capture() {
    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    hal_gpio_init_ex(&gpio_cc1101_g0, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

    // Timer: base
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64-1; 
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0xFFFFFFFF;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);

    // Timer: advanced and channel
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(TIM2);
    LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_TI2FP2);
    LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_DisableIT_TRIG(TIM2);
    LL_TIM_DisableDMAReq_TRIG(TIM2);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_EnableMasterSlaveMode(TIM2);
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_INDIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_FALLING);
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);

    // ISR setup
    api_hal_interrupt_set_timer_isr(TIM2, api_hal_subghz_capture_ISR);
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);

    // Interrupts and channels
    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    // Start timer
    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);
}

void api_hal_subghz_disable_capture() {
    LL_TIM_DeInit(TIM2);
    api_hal_interrupt_set_timer_isr(TIM2, NULL);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}
