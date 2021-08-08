#include "furi-hal-subghz.h"

#include <furi-hal-gpio.h>
#include <furi-hal-spi.h>
#include <furi-hal-interrupt.h>
#include <furi-hal-resources.h>

#include <furi.h>
#include <cc1101.h>
#include <stdio.h>

static volatile SubGhzState furi_hal_subghz_state = SubGhzStateInit;

static const uint8_t furi_hal_subghz_preset_ook_async_regs[][2] = {
    // https://e2e.ti.com/support/wireless-connectivity/sub-1-ghz-group/sub-1-ghz/f/sub-1-ghz-forum/382066/cc1101---don-t-know-the-correct-registers-configuration

    /* GPIO GD0 */
    { CC1101_IOCFG0,    0x0D }, // GD0 as async serial data output/input

    /* FIFO and internals */
    { CC1101_FIFOTHR,   0x47 }, // The only important bit is ADC_RETENTION

    /* Packet engine */
    { CC1101_PKTCTRL0,  0x32 }, // Async, continious, no whitening

    /* Frequency Synthesizer Control */
    { CC1101_FSCTRL1,   0x06 }, // IF = (26*10^6) / (2^10) * 0x06 = 152343.75Hz

    // Modem Configuration
    { CC1101_MDMCFG0,   0x00 }, // Channel spacing is 25kHz
    { CC1101_MDMCFG1,   0x00 }, // Channel spacing is 25kHz
    { CC1101_MDMCFG2,   0x30 }, // Format ASK/OOK, No preamble/sync
    { CC1101_MDMCFG3,   0x32 }, // Data rate is 3.79372 kBaud
    { CC1101_MDMCFG4,   0x67 }, // Rx BW filter is 270.833333kHz

    /* Main Radio Control State Machine */
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle-to-rx/tx, PO_TIMEOUT is 64 cycles(149-155us)

    /* Frequency Offset Compensation Configuration */
    { CC1101_FOCCFG,    0x18 }, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

    /* Automatic Gain Control */
    { CC1101_AGCTRL1,   0x00 }, // LNA 2 gain is decreased to minimum before decreasing LNA gain
    { CC1101_AGCTRL2,   0x07 }, // MAGN_TARGET is 42 dB

    /* Wake on radio and timeouts control */
    { CC1101_WORCTRL,   0xFB }, // WOR_RES is 2^15 periods (0.91 - 0.94 s) 16.5 - 17.2 hours 

    /* Frontend configuration */
    { CC1101_FREND0,    0x11 }, // Adjusts current TX LO buffer + high is PATABLE[1]
    { CC1101_FREND1,    0xB6 }, // 

    /* Frequency Synthesizer Calibration, valid for 433.92 */
    { CC1101_FSCAL3,    0xE9 },
    { CC1101_FSCAL2,    0x2A },
    { CC1101_FSCAL1,    0x00 },
    { CC1101_FSCAL0,    0x1F }, 

    /* Magic f4ckery */
    { CC1101_TEST2,     0x81 }, // FIFOTHR ADC_RETENTION=1 matched value
    { CC1101_TEST1,     0x35 }, // FIFOTHR ADC_RETENTION=1 matched value
    { CC1101_TEST0,     0x09 }, // VCO selection calibration stage is disabled

    /* End  */
    { 0, 0 },
};

static const uint8_t furi_hal_subghz_preset_ook_async_patable[8] = {
    0x00,
    0xC0, // 10dBm 0xC0, 7dBm 0xC8, 5dBm 0x84, 0dBm 0x60, -10dBm 0x34, -15dBm 0x1D, -20dBm 0x0E, -30dBm 0x12
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

void furi_hal_subghz_init() {
    furi_assert(furi_hal_subghz_state == SubGhzStateInit);
    furi_hal_subghz_state = SubGhzStateIdle;

    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);

#ifdef FURI_HAL_SUBGHZ_TX_GPIO
    hal_gpio_init(&FURI_HAL_SUBGHZ_TX_GPIO, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
#endif

    // Reset
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_reset(device);
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);

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

    // Go to sleep
    cc1101_shutdown(device);

    furi_hal_spi_device_return(device);
    FURI_LOG_I("FuriHalSubGhz", "Init OK");
}

void furi_hal_subghz_sleep() {
    furi_assert(furi_hal_subghz_state == SubGhzStateIdle);
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);

    cc1101_switch_to_idle(device);

    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    cc1101_shutdown(device);

    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_dump_state() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    printf(
        "[furi_hal_subghz] cc1101 chip %d, version %d\r\n",
        cc1101_get_partnumber(device),
        cc1101_get_version(device)
    );
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_load_preset(FuriHalSubGhzPreset preset) {
    if(preset == FuriHalSubGhzPresetOokAsync) {
        furi_hal_subghz_load_registers(furi_hal_subghz_preset_ook_async_regs);
        furi_hal_subghz_load_patable(furi_hal_subghz_preset_ook_async_patable);
    } else {
        furi_check(0);
    }
}

uint8_t furi_hal_subghz_get_status() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    CC1101StatusRaw st;
    st.status = cc1101_get_status(device);
    furi_hal_spi_device_return(device);
    return st.status_raw;
}

void furi_hal_subghz_load_registers(const uint8_t data[][2]) {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_reset(device);
    uint32_t i = 0;
    while (data[i][0]) {
        cc1101_write_reg(device, data[i][0], data[i][1]);
        i++;
    }
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_load_patable(const uint8_t data[8]) {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_set_pa_table(device, data);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_write_packet(const uint8_t* data, uint8_t size) {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_flush_tx(device);
    cc1101_write_fifo(device, data, size);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_flush_rx() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_flush_rx(device);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_read_packet(uint8_t* data, uint8_t* size) {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_read_fifo(device, data, size);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_shutdown() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    // Reset and shutdown
    cc1101_shutdown(device);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_reset() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_switch_to_idle(device);
    cc1101_reset(device);
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_idle() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_switch_to_idle(device);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_rx() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_switch_to_rx(device);
    furi_hal_spi_device_return(device);
}

void furi_hal_subghz_tx() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    cc1101_switch_to_tx(device);
    furi_hal_spi_device_return(device);
}

float furi_hal_subghz_get_rssi() {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    int32_t rssi_dec = cc1101_get_rssi(device);
    furi_hal_spi_device_return(device);

    float rssi = rssi_dec;
    if(rssi_dec >= 128) {
        rssi = ((rssi - 256.0f) / 2.0f) - 74.0f;
    } else {
        rssi = (rssi / 2.0f) - 74.0f;
    }

    return rssi;
}

bool furi_hal_subghz_is_frequency_valid(uint32_t value) {
    if(!(value >= 299999755 && value <= 348000335) &&
       !(value >= 386999938 && value <= 464000000) &&
       !(value >= 778999847 && value <= 928000000)) {
        return false;
    }
    return true;
}

uint32_t furi_hal_subghz_set_frequency_and_path(uint32_t value) {
    value = furi_hal_subghz_set_frequency(value);
    if(value >= 299999755 && value <= 348000335) {
        furi_hal_subghz_set_path(FuriHalSubGhzPath315);
    } else if(value >= 386999938 && value <= 464000000) {
        furi_hal_subghz_set_path(FuriHalSubGhzPath433);
    } else if(value >= 778999847 && value <= 928000000) {
        furi_hal_subghz_set_path(FuriHalSubGhzPath868);
    } else {
        furi_check(0);
    }
    return value;
}

uint32_t furi_hal_subghz_set_frequency(uint32_t value) {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);

    uint32_t real_frequency = cc1101_set_frequency(device, value);
    cc1101_calibrate(device);

    furi_hal_spi_device_return(device);

    return real_frequency;
}

void furi_hal_subghz_set_path(FuriHalSubGhzPath path) {
    const FuriHalSpiDevice* device = furi_hal_spi_device_get(FuriHalSpiDeviceIdSubGhz);
    if (path == FuriHalSubGhzPath433) {
        hal_gpio_write(&gpio_rf_sw_0, 0);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);
    } else if (path == FuriHalSubGhzPath315) {
        hal_gpio_write(&gpio_rf_sw_0, 1);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW);
    } else if (path == FuriHalSubGhzPath868) {
        hal_gpio_write(&gpio_rf_sw_0, 1);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);
    } else if (path == FuriHalSubGhzPathIsolate) {
        hal_gpio_write(&gpio_rf_sw_0, 0);
        cc1101_write_reg(device, CC1101_IOCFG2, CC1101IocfgHW);
    } else {
        furi_check(0);
    }
    furi_hal_spi_device_return(device);
}

volatile uint32_t furi_hal_subghz_capture_delta_duration = 0;
volatile FuriHalSubGhzCaptureCallback furi_hal_subghz_capture_callback = NULL;
volatile void* furi_hal_subghz_capture_callback_context = NULL;

static void furi_hal_subghz_capture_ISR() {
    // Channel 1
    if(LL_TIM_IsActiveFlag_CC1(TIM2)) {
        LL_TIM_ClearFlag_CC1(TIM2);
        furi_hal_subghz_capture_delta_duration = LL_TIM_IC_GetCaptureCH1(TIM2);
        if (furi_hal_subghz_capture_callback) {
            furi_hal_subghz_capture_callback(true, furi_hal_subghz_capture_delta_duration,
                (void*)furi_hal_subghz_capture_callback_context
            );
        }
    }
    // Channel 2
    if(LL_TIM_IsActiveFlag_CC2(TIM2)) {
        LL_TIM_ClearFlag_CC2(TIM2);
        if (furi_hal_subghz_capture_callback) {
            furi_hal_subghz_capture_callback(false, LL_TIM_IC_GetCaptureCH2(TIM2) - furi_hal_subghz_capture_delta_duration,
                (void*)furi_hal_subghz_capture_callback_context
            );
        }
    }
}

void furi_hal_subghz_set_async_rx_callback(FuriHalSubGhzCaptureCallback callback, void* context) {
    furi_hal_subghz_capture_callback = callback;
    furi_hal_subghz_capture_callback_context = context;
}

void furi_hal_subghz_start_async_rx() {
    furi_assert(furi_hal_subghz_state == SubGhzStateIdle);
    furi_hal_subghz_state = SubGhzStateAsyncRx;

    hal_gpio_init_ex(&gpio_cc1101_g0, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

    // Timer: base
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64-1; 
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0x7FFFFFFE;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
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
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);

    // Timer: channel 2 direct
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);

    // ISR setup
    furi_hal_interrupt_set_timer_isr(TIM2, furi_hal_subghz_capture_ISR);
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);

    // Interrupts and channels
    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    // Enable NVIC
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);

    // Start timer
    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);

    // Switch to RX
    furi_hal_subghz_rx();
}

void furi_hal_subghz_stop_async_rx() {
    furi_assert(furi_hal_subghz_state == SubGhzStateAsyncRx);
    furi_hal_subghz_state = SubGhzStateIdle;

    // Shutdown radio
    furi_hal_subghz_idle();

    LL_TIM_DeInit(TIM2);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
    furi_hal_interrupt_set_timer_isr(TIM2, NULL);

    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

volatile size_t furi_hal_subghz_tx_repeat = 0;

static void furi_hal_subghz_tx_dma_isr() {
    if (LL_DMA_IsActiveFlag_TC1(DMA1)) {
        LL_DMA_ClearFlag_TC1(DMA1);
        furi_assert(furi_hal_subghz_state == SubGhzStateAsyncTx);
        if (--furi_hal_subghz_tx_repeat == 0) {
            furi_hal_subghz_state = SubGhzStateAsyncTxLast;
            LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
        }
    }
}

static void furi_hal_subghz_tx_timer_isr() {
    if(LL_TIM_IsActiveFlag_UPDATE(TIM2)) {
        LL_TIM_ClearFlag_UPDATE(TIM2);
        if (furi_hal_subghz_state == SubGhzStateAsyncTxLast) {
            LL_TIM_DisableCounter(TIM2);
            furi_hal_subghz_state = SubGhzStateAsyncTxEnd;
        }
    }
}

void furi_hal_subghz_start_async_tx(uint32_t* buffer, size_t buffer_size, size_t repeat) {
    furi_assert(furi_hal_subghz_state == SubGhzStateIdle);
    furi_hal_subghz_state = SubGhzStateAsyncTx;
    furi_hal_subghz_tx_repeat = repeat;

    // Connect CC1101_GD0 to TIM2 as output
    hal_gpio_init_ex(&gpio_cc1101_g0, GpioModeAltFunctionPushPull, GpioPullDown, GpioSpeedLow, GpioAltFn1TIM2);

    // Configure DMA
    LL_DMA_InitTypeDef dma_config = {0};
    dma_config.PeriphOrM2MSrcAddress = (uint32_t)&(TIM2->ARR);
    dma_config.MemoryOrM2MDstAddress = (uint32_t)buffer;
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_CIRCULAR;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dma_config.NbData = buffer_size / sizeof(uint32_t);
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    dma_config.Priority = LL_DMA_MODE_NORMAL;
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &dma_config);
    furi_hal_interrupt_set_dma_channel_isr(DMA1, LL_DMA_CHANNEL_1, furi_hal_subghz_tx_dma_isr);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

    // Configure TIM2
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64-1;
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
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableMasterSlaveMode(TIM2);

    furi_hal_interrupt_set_timer_isr(TIM2, furi_hal_subghz_tx_timer_isr);
    LL_TIM_EnableIT_UPDATE(TIM2);
    LL_TIM_EnableDMAReq_UPDATE(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    // Start counter
    LL_TIM_GenerateEvent_UPDATE(TIM2);
#ifdef FURI_HAL_SUBGHZ_TX_GPIO
    hal_gpio_write(&FURI_HAL_SUBGHZ_TX_GPIO, true);
#endif
    furi_hal_subghz_tx();

    // Enable NVIC
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);
}

size_t furi_hal_subghz_get_async_tx_repeat_left() {
    return furi_hal_subghz_tx_repeat;
}

void furi_hal_subghz_wait_async_tx() {
    while(furi_hal_subghz_state != SubGhzStateAsyncTxEnd) osDelay(1);
}

void furi_hal_subghz_stop_async_tx() {
    furi_assert(
        furi_hal_subghz_state == SubGhzStateAsyncTx
        || furi_hal_subghz_state == SubGhzStateAsyncTxLast
        || furi_hal_subghz_state == SubGhzStateAsyncTxEnd
    );

    // Shutdown radio
    furi_hal_subghz_idle();
#ifdef FURI_HAL_SUBGHZ_TX_GPIO
    hal_gpio_write(&FURI_HAL_SUBGHZ_TX_GPIO, false);
#endif

    // Deinitialize Timer
    LL_TIM_DeInit(TIM2);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
    furi_hal_interrupt_set_timer_isr(TIM2, NULL);

    // Deinitialize DMA
    LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_1);
    furi_hal_interrupt_set_dma_channel_isr(DMA1, LL_DMA_CHANNEL_1, NULL);

    // Deinitialize GPIO
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    furi_hal_subghz_state = SubGhzStateIdle;
}
