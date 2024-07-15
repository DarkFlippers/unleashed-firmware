#include "cc1101_ext.h"
#include <lib/subghz/devices/cc1101_configs.h>

#include <furi_hal_region.h>
#include <furi_hal_version.h>
#include <furi_hal_rtc.h>
#include <furi_hal_spi.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi_hal_bus.h>

#include <stm32wbxx_ll_dma.h>
#include <furi_hal_cortex.h>

#include <furi.h>
#include <cc1101.h>
#include <stdio.h>

#define TAG "SubGhzDeviceCc1101Ext"

#define SUBGHZ_DEVICE_CC1101_EXT_TX_GPIO (&gpio_ext_pb2)

/* DMA Channels definition */
#define SUBGHZ_DEVICE_CC1101_EXT_DMA             (DMA2)
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_CHANNEL (LL_DMA_CHANNEL_3)
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_CHANNEL (LL_DMA_CHANNEL_4)
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_CHANNEL (LL_DMA_CHANNEL_5)
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_IRQ     (FuriHalInterruptIdDma2Ch3)
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF \
    SUBGHZ_DEVICE_CC1101_EXT_DMA, SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_CHANNEL
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF \
    SUBGHZ_DEVICE_CC1101_EXT_DMA, SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_CHANNEL
#define SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF \
    SUBGHZ_DEVICE_CC1101_EXT_DMA, SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_CHANNEL

/** Low level buffer dimensions and guard times */
#define SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_FULL (256u)
#define SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_HALF \
    (SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_FULL / 2)
#define SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_GUARD_TIME (999u >> 1)

/** SubGhz state */
typedef enum {
    SubGhzDeviceCC1101ExtStateInit, /**< Init pending */
    SubGhzDeviceCC1101ExtStateIdle, /**< Idle, energy save mode */
    SubGhzDeviceCC1101ExtStateAsyncRx, /**< Async RX started */
    SubGhzDeviceCC1101ExtStateAsyncTx, /**< Async TX started, DMA and timer is on */
} SubGhzDeviceCC1101ExtState;

/** SubGhz regulation, receive transmission on the current frequency for the
 * region */
typedef enum {
    SubGhzDeviceCC1101ExtRegulationOnlyRx, /**only Rx*/
    SubGhzDeviceCC1101ExtRegulationTxRx, /**TxRx*/
} SubGhzDeviceCC1101ExtRegulation;

typedef enum {
    SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateIdle,
    SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateReset,
    SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateRun,
} SubGhzDeviceCC1101ExtAsyncTxMiddlewareState;

typedef struct {
    SubGhzDeviceCC1101ExtAsyncTxMiddlewareState state;
    bool is_odd_level;
    uint32_t adder_duration;
} SubGhzDeviceCC1101ExtAsyncTxMiddleware;

typedef struct {
    uint32_t* buffer;
    SubGhzDeviceCC1101ExtCallback callback;
    void* callback_context;
    uint32_t gpio_tx_buff[2];
    uint32_t debug_gpio_buff[2];
    SubGhzDeviceCC1101ExtAsyncTxMiddleware middleware;
} SubGhzDeviceCC1101ExtAsyncTx;

typedef struct {
    uint32_t capture_delta_duration;
    SubGhzDeviceCC1101ExtCaptureCallback capture_callback;
    void* capture_callback_context;
} SubGhzDeviceCC1101ExtAsyncRx;

typedef struct {
    volatile SubGhzDeviceCC1101ExtState state;
    volatile SubGhzDeviceCC1101ExtRegulation regulation;
    const GpioPin* async_mirror_pin;
    FuriHalSpiBusHandle* spi_bus_handle;
    const GpioPin* g0_pin;
    SubGhzDeviceCC1101ExtAsyncTx async_tx;
    SubGhzDeviceCC1101ExtAsyncRx async_rx;
} SubGhzDeviceCC1101Ext;

static SubGhzDeviceCC1101Ext* subghz_device_cc1101_ext = NULL;

static bool subghz_device_cc1101_ext_check_init(void) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateInit);
    subghz_device_cc1101_ext->state = SubGhzDeviceCC1101ExtStateIdle;

    bool ret = false;
    CC1101Status cc1101_status = {0};

    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(100 * 1000);
    do {
        // Reset
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->spi_bus_handle->miso,
            GpioModeInput,
            GpioPullUp,
            GpioSpeedLow);

        cc1101_status = cc1101_reset(subghz_device_cc1101_ext->spi_bus_handle);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }
        cc1101_status = cc1101_write_reg(
            subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG0, CC1101IocfgHighImpedance);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }
        // Prepare GD0 for power on self test
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->g0_pin, GpioModeInput, GpioPullUp, GpioSpeedLow);

        // GD0 low
        cc1101_status = cc1101_write_reg(
            subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG0, CC1101IocfgHW);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }
        while(furi_hal_gpio_read(subghz_device_cc1101_ext->g0_pin) != false) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                //timeout
                break;
            }
        }
        if(furi_hal_cortex_timer_is_expired(timer)) {
            //timeout
            break;
        }

        // GD0 high
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->g0_pin, GpioModeInput, GpioPullDown, GpioSpeedLow);
        cc1101_status = cc1101_write_reg(
            subghz_device_cc1101_ext->spi_bus_handle,
            CC1101_IOCFG0,
            CC1101IocfgHW | CC1101_IOCFG_INV);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }
        while(furi_hal_gpio_read(subghz_device_cc1101_ext->g0_pin) != true) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                //timeout
                break;
            }
        }
        if(furi_hal_cortex_timer_is_expired(timer)) {
            //timeout
            break;
        }

        // Reset GD0 to floating state
        cc1101_status = cc1101_write_reg(
            subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG0, CC1101IocfgHighImpedance);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

        // Reset GDO2 (!TX/RX) to floating state
        cc1101_status = cc1101_write_reg(
            subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG2, CC1101IocfgHighImpedance);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }

        // Go to sleep
        cc1101_status = cc1101_shutdown(subghz_device_cc1101_ext->spi_bus_handle);
        if(cc1101_status.CHIP_RDYn != 0) {
            //timeout or error
            break;
        }
        ret = true;
    } while(false);

    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);

    if(ret) {
        FURI_LOG_I(TAG, "Init OK");
    } else {
        FURI_LOG_E(TAG, "Init failed");
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    }
    return ret;
}

bool subghz_device_cc1101_ext_alloc(void) {
    furi_assert(subghz_device_cc1101_ext == NULL);
    subghz_device_cc1101_ext = malloc(sizeof(SubGhzDeviceCC1101Ext));
    subghz_device_cc1101_ext->state = SubGhzDeviceCC1101ExtStateInit;
    subghz_device_cc1101_ext->regulation = SubGhzDeviceCC1101ExtRegulationTxRx;
    subghz_device_cc1101_ext->async_mirror_pin = NULL;
    subghz_device_cc1101_ext->spi_bus_handle = &furi_hal_spi_bus_handle_external;
    subghz_device_cc1101_ext->g0_pin = SUBGHZ_DEVICE_CC1101_EXT_TX_GPIO;

    subghz_device_cc1101_ext->async_rx.capture_delta_duration = 0;

    furi_hal_spi_bus_handle_init(subghz_device_cc1101_ext->spi_bus_handle);
    return subghz_device_cc1101_ext_check_init();
}

void subghz_device_cc1101_ext_free(void) {
    furi_assert(subghz_device_cc1101_ext != NULL);
    furi_hal_spi_bus_handle_deinit(subghz_device_cc1101_ext->spi_bus_handle);
    free(subghz_device_cc1101_ext);
    subghz_device_cc1101_ext = NULL;
}

void subghz_device_cc1101_ext_set_async_mirror_pin(const GpioPin* pin) {
    subghz_device_cc1101_ext->async_mirror_pin = pin;
}

const GpioPin* subghz_device_cc1101_ext_get_data_gpio(void) {
    return subghz_device_cc1101_ext->g0_pin;
}

bool subghz_device_cc1101_ext_is_connect(void) {
    bool ret = false;

    if(subghz_device_cc1101_ext == NULL) { // not initialized
        ret = subghz_device_cc1101_ext_alloc();
        subghz_device_cc1101_ext_free();
    } else { // initialized
        furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
        uint8_t partnumber = cc1101_get_partnumber(subghz_device_cc1101_ext->spi_bus_handle);
        furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
        ret = (partnumber != 0) && (partnumber != 0xFF);
    }

    return ret;
}

void subghz_device_cc1101_ext_sleep(void) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateIdle);
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);

    cc1101_switch_to_idle(subghz_device_cc1101_ext->spi_bus_handle);

    cc1101_write_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    furi_hal_gpio_init(subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    cc1101_shutdown(subghz_device_cc1101_ext->spi_bus_handle);

    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_dump_state(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    printf(
        "[subghz_device_cc1101_ext] cc1101 chip %d, version %d\r\n",
        cc1101_get_partnumber(subghz_device_cc1101_ext->spi_bus_handle),
        cc1101_get_version(subghz_device_cc1101_ext->spi_bus_handle));
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_load_custom_preset(const uint8_t* preset_data) {
    //load config
    subghz_device_cc1101_ext_reset();
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    uint32_t i = 0;
    uint8_t pa[8] = {0};
    while(preset_data[i]) {
        cc1101_write_reg(
            subghz_device_cc1101_ext->spi_bus_handle, preset_data[i], preset_data[i + 1]);
        i += 2;
    }
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);

    //load pa table
    memcpy(&pa[0], &preset_data[i + 2], 8);
    subghz_device_cc1101_ext_load_patable(pa);

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

void subghz_device_cc1101_ext_load_registers(const uint8_t* data) {
    subghz_device_cc1101_ext_reset();
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    uint32_t i = 0;
    while(data[i]) {
        cc1101_write_reg(subghz_device_cc1101_ext->spi_bus_handle, data[i], data[i + 1]);
        i += 2;
    }
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_load_patable(const uint8_t data[8]) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_set_pa_table(subghz_device_cc1101_ext->spi_bus_handle, data);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_write_packet(const uint8_t* data, uint8_t size) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_flush_tx(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_write_reg(subghz_device_cc1101_ext->spi_bus_handle, CC1101_FIFO, size);
    cc1101_write_fifo(subghz_device_cc1101_ext->spi_bus_handle, data, size);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_flush_rx(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_flush_rx(subghz_device_cc1101_ext->spi_bus_handle);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_flush_tx(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_flush_tx(subghz_device_cc1101_ext->spi_bus_handle);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

bool subghz_device_cc1101_ext_rx_pipe_not_empty(void) {
    CC1101RxBytes status[1];
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_read_reg(
        subghz_device_cc1101_ext->spi_bus_handle,
        (CC1101_STATUS_RXBYTES) | CC1101_BURST,
        (uint8_t*)status);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
    if(status->NUM_RXBYTES > 0) {
        return true;
    } else {
        return false;
    }
}

bool subghz_device_cc1101_ext_is_rx_data_crc_valid(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    uint8_t data[1];
    cc1101_read_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_STATUS_LQI | CC1101_BURST, data);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
    if((data[0] >> 7) & 0x01) {
        return true;
    } else {
        return false;
    }
}

void subghz_device_cc1101_ext_read_packet(uint8_t* data, uint8_t* size) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_read_fifo(subghz_device_cc1101_ext->spi_bus_handle, data, size);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_shutdown(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    // Reset and shutdown
    cc1101_shutdown(subghz_device_cc1101_ext->spi_bus_handle);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_reset(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    furi_hal_gpio_init(subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_switch_to_idle(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_reset(subghz_device_cc1101_ext->spi_bus_handle);
    // Warning: push pull cc1101 clock output on GD0
    cc1101_write_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    // Reset GDO2 (!TX/RX) to floating state
    cc1101_write_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG2, CC1101IocfgHighImpedance);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_idle(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_switch_to_idle(subghz_device_cc1101_ext->spi_bus_handle);
    //waiting for the chip to switch to IDLE mode
    furi_check(cc1101_wait_status_state(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101StateIDLE, 10000));
    // Reset GDO2 (!TX/RX) to floating state
    cc1101_write_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG2, CC1101IocfgHighImpedance);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

void subghz_device_cc1101_ext_rx(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_switch_to_rx(subghz_device_cc1101_ext->spi_bus_handle);
    //waiting for the chip to switch to Rx mode
    furi_check(
        cc1101_wait_status_state(subghz_device_cc1101_ext->spi_bus_handle, CC1101StateRX, 10000));
    // Go GDO2 (!TX/RX) to high (RX state)
    cc1101_write_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG2, CC1101IocfgHW | CC1101_IOCFG_INV);

    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
}

bool subghz_device_cc1101_ext_tx(void) {
    if(subghz_device_cc1101_ext->regulation != SubGhzDeviceCC1101ExtRegulationTxRx) return false;
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    cc1101_switch_to_tx(subghz_device_cc1101_ext->spi_bus_handle);
    //waiting for the chip to switch to Tx mode
    furi_check(
        cc1101_wait_status_state(subghz_device_cc1101_ext->spi_bus_handle, CC1101StateTX, 10000));
    // Go GDO2 (!TX/RX) to low (TX state)
    cc1101_write_reg(subghz_device_cc1101_ext->spi_bus_handle, CC1101_IOCFG2, CC1101IocfgHW);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
    return true;
}

float subghz_device_cc1101_ext_get_rssi(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    int32_t rssi_dec = cc1101_get_rssi(subghz_device_cc1101_ext->spi_bus_handle);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);

    float rssi = rssi_dec;
    if(rssi_dec >= 128) {
        rssi = ((rssi - 256.0f) / 2.0f) - 74.0f;
    } else {
        rssi = (rssi / 2.0f) - 74.0f;
    }

    return rssi;
}

uint8_t subghz_device_cc1101_ext_get_lqi(void) {
    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    uint8_t data[1];
    cc1101_read_reg(
        subghz_device_cc1101_ext->spi_bus_handle, CC1101_STATUS_LQI | CC1101_BURST, data);
    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
    return data[0] & 0x7F;
}

bool subghz_device_cc1101_ext_is_frequency_valid(uint32_t value) {
    if(!(value >= 299999755 && value <= 348000335) &&
       !(value >= 386999938 && value <= 464000000) &&
       !(value >= 778999847 && value <= 928000000)) {
        return false;
    }

    return true;
}

uint32_t subghz_device_cc1101_ext_set_frequency(uint32_t value) {
    if(furi_hal_region_is_frequency_allowed(value)) {
        subghz_device_cc1101_ext->regulation = SubGhzDeviceCC1101ExtRegulationTxRx;
    } else {
        subghz_device_cc1101_ext->regulation = SubGhzDeviceCC1101ExtRegulationOnlyRx;
    }

    furi_hal_spi_acquire(subghz_device_cc1101_ext->spi_bus_handle);
    uint32_t real_frequency =
        cc1101_set_frequency(subghz_device_cc1101_ext->spi_bus_handle, value);
    cc1101_calibrate(subghz_device_cc1101_ext->spi_bus_handle);

    while(true) {
        CC1101Status status = cc1101_get_status(subghz_device_cc1101_ext->spi_bus_handle);
        if(status.STATE == CC1101StateIDLE) break;
    }

    furi_hal_spi_release(subghz_device_cc1101_ext->spi_bus_handle);
    return real_frequency;
}

static bool subghz_device_cc1101_ext_start_debug(void) {
    bool ret = false;
    if(subghz_device_cc1101_ext->async_mirror_pin != NULL) {
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->async_mirror_pin,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh);
        ret = true;
    }
    return ret;
}

static bool subghz_device_cc1101_ext_stop_debug(void) {
    bool ret = false;
    if(subghz_device_cc1101_ext->async_mirror_pin != NULL) {
        furi_hal_gpio_init(
            subghz_device_cc1101_ext->async_mirror_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        ret = true;
    }
    return ret;
}

static void subghz_device_cc1101_ext_capture_ISR(void* context) {
    UNUSED(context);
    if(!furi_hal_gpio_read(subghz_device_cc1101_ext->g0_pin)) {
        if(subghz_device_cc1101_ext->async_rx.capture_callback) {
            if(subghz_device_cc1101_ext->async_mirror_pin != NULL)
                furi_hal_gpio_write(subghz_device_cc1101_ext->async_mirror_pin, false);

            subghz_device_cc1101_ext->async_rx.capture_callback(
                true,
                LL_TIM_GetCounter(TIM17) << 1,
                (void*)subghz_device_cc1101_ext->async_rx.capture_callback_context);
        }
    } else {
        if(subghz_device_cc1101_ext->async_rx.capture_callback) {
            if(subghz_device_cc1101_ext->async_mirror_pin != NULL)
                furi_hal_gpio_write(subghz_device_cc1101_ext->async_mirror_pin, true);

            subghz_device_cc1101_ext->async_rx.capture_callback(
                false,
                LL_TIM_GetCounter(TIM17) << 1,
                (void*)subghz_device_cc1101_ext->async_rx.capture_callback_context);
        }
    }
    LL_TIM_SetCounter(TIM17, 4); //8>>1
}

void subghz_device_cc1101_ext_start_async_rx(
    SubGhzDeviceCC1101ExtCaptureCallback callback,
    void* context) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateIdle);
    subghz_device_cc1101_ext->state = SubGhzDeviceCC1101ExtStateAsyncRx;

    subghz_device_cc1101_ext->async_rx.capture_callback = callback;
    subghz_device_cc1101_ext->async_rx.capture_callback_context = context;

    furi_hal_bus_enable(FuriHalBusTIM17);

    // Configure TIM
    //Set the timer resolution to 2 us
    LL_TIM_SetPrescaler(TIM17, (64 << 1) - 1);
    LL_TIM_SetCounterMode(TIM17, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetAutoReload(TIM17, 0xFFFF);
    LL_TIM_SetClockDivision(TIM17, LL_TIM_CLOCKDIVISION_DIV1);

    // Timer: advanced
    LL_TIM_SetClockSource(TIM17, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(TIM17);
    LL_TIM_DisableDMAReq_TRIG(TIM17);
    LL_TIM_DisableIT_TRIG(TIM17);

    furi_hal_gpio_init(
        subghz_device_cc1101_ext->g0_pin, GpioModeInterruptRiseFall, GpioPullUp, GpioSpeedVeryHigh);
    furi_hal_gpio_remove_int_callback(subghz_device_cc1101_ext->g0_pin);
    furi_hal_gpio_add_int_callback(
        subghz_device_cc1101_ext->g0_pin,
        subghz_device_cc1101_ext_capture_ISR,
        subghz_device_cc1101_ext->async_rx.capture_callback);

    // Start timer
    LL_TIM_SetCounter(TIM17, 0);
    LL_TIM_EnableCounter(TIM17);

    // Start debug
    subghz_device_cc1101_ext_start_debug();

    // Switch to RX
    subghz_device_cc1101_ext_rx();

    //Clear the variable after the end of the session
    subghz_device_cc1101_ext->async_rx.capture_delta_duration = 0;
}

void subghz_device_cc1101_ext_stop_async_rx(void) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateAsyncRx);
    subghz_device_cc1101_ext->state = SubGhzDeviceCC1101ExtStateIdle;

    // Shutdown radio
    subghz_device_cc1101_ext_idle();

    FURI_CRITICAL_ENTER();
    furi_hal_bus_disable(FuriHalBusTIM17);

    // Stop debug
    subghz_device_cc1101_ext_stop_debug();

    FURI_CRITICAL_EXIT();
    furi_hal_gpio_remove_int_callback(subghz_device_cc1101_ext->g0_pin);
    furi_hal_gpio_init(subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void subghz_device_cc1101_ext_async_tx_middleware_idle(
    SubGhzDeviceCC1101ExtAsyncTxMiddleware* middleware) {
    middleware->state = SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateIdle;
    middleware->is_odd_level = false;
    middleware->adder_duration = SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_GUARD_TIME;
}

static inline uint32_t subghz_device_cc1101_ext_async_tx_middleware_get_duration(
    SubGhzDeviceCC1101ExtAsyncTxMiddleware* middleware,
    SubGhzDeviceCC1101ExtCallback callback) {
    uint32_t ret = 0;
    bool is_level = false;

    if(middleware->state == SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateReset) return 0;

    while(1) {
        LevelDuration ld = callback(subghz_device_cc1101_ext->async_tx.callback_context);
        if(level_duration_is_reset(ld)) {
            middleware->state = SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateReset;
            if(!middleware->is_odd_level) {
                return 0;
            } else {
                return middleware->adder_duration;
            }
        } else if(level_duration_is_wait(ld)) {
            middleware->is_odd_level = !middleware->is_odd_level;
            ret = middleware->adder_duration + SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_GUARD_TIME;
            middleware->adder_duration = 0;
            return ret;
        }

        is_level = level_duration_get_level(ld);

        if(middleware->state == SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateIdle) {
            if(is_level != middleware->is_odd_level) {
                middleware->state = SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateRun;
                middleware->is_odd_level = is_level;
                middleware->adder_duration = level_duration_get_duration(ld);
                return SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_GUARD_TIME;
            } else {
                continue;
            }
        }

        if(middleware->state == SubGhzDeviceCC1101ExtAsyncTxMiddlewareStateRun) {
            if(is_level == middleware->is_odd_level) {
                middleware->adder_duration += level_duration_get_duration(ld);
                continue;
            } else {
                middleware->is_odd_level = is_level;
                ret = middleware->adder_duration;
                middleware->adder_duration = level_duration_get_duration(ld);
                return ret;
            }
        }
    }
}

static void subghz_device_cc1101_ext_async_tx_refill(uint32_t* buffer, size_t samples) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateAsyncTx);

    while(samples > 0) {
        volatile uint32_t duration = subghz_device_cc1101_ext_async_tx_middleware_get_duration(
            &subghz_device_cc1101_ext->async_tx.middleware,
            subghz_device_cc1101_ext->async_tx.callback);
        if(duration == 0) {
            *buffer = 0;
            buffer++;
            samples--;
            LL_DMA_DisableIT_HT(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF);
            LL_DMA_DisableIT_TC(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF);
            if(LL_DMA_IsActiveFlag_HT3(SUBGHZ_DEVICE_CC1101_EXT_DMA)) {
                LL_DMA_ClearFlag_HT3(SUBGHZ_DEVICE_CC1101_EXT_DMA);
            }
            if(LL_DMA_IsActiveFlag_TC3(SUBGHZ_DEVICE_CC1101_EXT_DMA)) {
                LL_DMA_ClearFlag_TC3(SUBGHZ_DEVICE_CC1101_EXT_DMA);
            }
            break;
        } else {
            // Lowest possible value is 4us
            if(duration < 4) duration = 4;
            // Divide by 2 since timer resolution is 2us
            // Subtract 1 since we counting from 0
            *buffer = (duration >> 1) - 1;
            buffer++;
            samples--;
        }
    }
}

static void subghz_device_cc1101_ext_async_tx_dma_isr(void* context) {
    UNUSED(context);
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateAsyncTx);

#if SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_CHANNEL == LL_DMA_CHANNEL_3
    if(LL_DMA_IsActiveFlag_HT3(SUBGHZ_DEVICE_CC1101_EXT_DMA)) {
        LL_DMA_ClearFlag_HT3(SUBGHZ_DEVICE_CC1101_EXT_DMA);
        subghz_device_cc1101_ext_async_tx_refill(
            subghz_device_cc1101_ext->async_tx.buffer,
            SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_HALF);
    }
    if(LL_DMA_IsActiveFlag_TC3(SUBGHZ_DEVICE_CC1101_EXT_DMA)) {
        LL_DMA_ClearFlag_TC3(SUBGHZ_DEVICE_CC1101_EXT_DMA);
        subghz_device_cc1101_ext_async_tx_refill(
            subghz_device_cc1101_ext->async_tx.buffer +
                SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_HALF,
            SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_HALF);
    }
#else
#error Update this code. Would you kindly?
#endif
}

bool subghz_device_cc1101_ext_start_async_tx(SubGhzDeviceCC1101ExtCallback callback, void* context) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateIdle);
    furi_assert(callback);

    //If transmission is prohibited by regional settings
    if(subghz_device_cc1101_ext->regulation != SubGhzDeviceCC1101ExtRegulationTxRx) return false;

    subghz_device_cc1101_ext->async_tx.callback = callback;
    subghz_device_cc1101_ext->async_tx.callback_context = context;

    subghz_device_cc1101_ext->state = SubGhzDeviceCC1101ExtStateAsyncTx;

    subghz_device_cc1101_ext->async_tx.buffer =
        malloc(SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_FULL * sizeof(uint32_t));

    //Signal generation with mem-to-mem DMA
    furi_hal_gpio_write(subghz_device_cc1101_ext->g0_pin, false);
    furi_hal_gpio_init(
        subghz_device_cc1101_ext->g0_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    // Configure DMA  update timer
    LL_DMA_SetMemoryAddress(
        SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF, (uint32_t)subghz_device_cc1101_ext->async_tx.buffer);
    LL_DMA_SetPeriphAddress(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF, (uint32_t) & (TIM17->ARR));
    LL_DMA_ConfigTransfer(
        SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF,
        LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_CIRCULAR | LL_DMA_PERIPH_NOINCREMENT |
            LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_WORD |
            LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetDataLength(
        SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF, SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_FULL);
    LL_DMA_SetPeriphRequest(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF, LL_DMAMUX_REQ_TIM17_UP);

    LL_DMA_EnableIT_TC(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF);
    LL_DMA_EnableIT_HT(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF);
    LL_DMA_EnableChannel(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF);

    furi_hal_interrupt_set_isr(
        SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_IRQ, subghz_device_cc1101_ext_async_tx_dma_isr, NULL);

    furi_hal_bus_enable(FuriHalBusTIM17);

    // Configure TIM
    // Set the timer resolution to 2 us
    LL_TIM_SetCounterMode(TIM17, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM17, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetAutoReload(TIM17, 500);
    LL_TIM_SetPrescaler(TIM17, (64 << 1) - 1);
    LL_TIM_SetClockSource(TIM17, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(TIM17);

    subghz_device_cc1101_ext_async_tx_middleware_idle(
        &subghz_device_cc1101_ext->async_tx.middleware);
    subghz_device_cc1101_ext_async_tx_refill(
        subghz_device_cc1101_ext->async_tx.buffer, SUBGHZ_DEVICE_CC1101_EXT_ASYNC_TX_BUFFER_FULL);

    // Configure tx gpio dma
    const GpioPin* gpio = subghz_device_cc1101_ext->g0_pin;

    subghz_device_cc1101_ext->async_tx.gpio_tx_buff[0] = (uint32_t)gpio->pin << GPIO_NUMBER;
    subghz_device_cc1101_ext->async_tx.gpio_tx_buff[1] = gpio->pin;

    LL_DMA_SetMemoryAddress(
        SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF,
        (uint32_t)subghz_device_cc1101_ext->async_tx.gpio_tx_buff);
    LL_DMA_SetPeriphAddress(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF, (uint32_t) & (gpio->port->BSRR));
    LL_DMA_ConfigTransfer(
        SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF,
        LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_CIRCULAR | LL_DMA_PERIPH_NOINCREMENT |
            LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_WORD |
            LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetDataLength(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF, 2);
    LL_DMA_SetPeriphRequest(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF, LL_DMAMUX_REQ_TIM17_UP);
    LL_DMA_EnableChannel(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF);

    // Start debug
    if(subghz_device_cc1101_ext_start_debug()) {
        gpio = subghz_device_cc1101_ext->async_mirror_pin;
        subghz_device_cc1101_ext->async_tx.debug_gpio_buff[0] = (uint32_t)gpio->pin << GPIO_NUMBER;
        subghz_device_cc1101_ext->async_tx.debug_gpio_buff[1] = gpio->pin;

        LL_DMA_SetMemoryAddress(
            SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF,
            (uint32_t)subghz_device_cc1101_ext->async_tx.debug_gpio_buff);
        LL_DMA_SetPeriphAddress(
            SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF, (uint32_t) & (gpio->port->BSRR));
        LL_DMA_ConfigTransfer(
            SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF,
            LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_CIRCULAR | LL_DMA_PERIPH_NOINCREMENT |
                LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_WORD |
                LL_DMA_PRIORITY_LOW);
        LL_DMA_SetDataLength(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF, 2);
        LL_DMA_SetPeriphRequest(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF, LL_DMAMUX_REQ_TIM17_UP);
        LL_DMA_EnableChannel(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF);
    }

    // Start counter
    LL_TIM_EnableDMAReq_UPDATE(TIM17);

    subghz_device_cc1101_ext_tx();

    LL_TIM_SetCounter(TIM17, 0);
    LL_TIM_EnableCounter(TIM17);

    return true;
}

bool subghz_device_cc1101_ext_is_async_tx_complete(void) {
    return (subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateAsyncTx) &&
           (LL_TIM_GetAutoReload(TIM17) == 0);
}

void subghz_device_cc1101_ext_stop_async_tx(void) {
    furi_assert(subghz_device_cc1101_ext->state == SubGhzDeviceCC1101ExtStateAsyncTx);

    // Shutdown radio
    subghz_device_cc1101_ext_idle();

    // Deinitialize GPIO
    furi_hal_gpio_write(subghz_device_cc1101_ext->g0_pin, false);
    furi_hal_gpio_init(subghz_device_cc1101_ext->g0_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    // Deinitialize Timer
    furi_hal_bus_disable(FuriHalBusTIM17);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTim1TrgComTim17, NULL, NULL);

    // Deinitialize DMA
    LL_DMA_DeInit(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_DEF);
    LL_DMA_DisableChannel(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH4_DEF);
    furi_hal_interrupt_set_isr(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH3_IRQ, NULL, NULL);

    // Stop debug
    if(subghz_device_cc1101_ext_stop_debug()) {
        LL_DMA_DisableChannel(SUBGHZ_DEVICE_CC1101_EXT_DMA_CH5_DEF);
    }

    free(subghz_device_cc1101_ext->async_tx.buffer);

    subghz_device_cc1101_ext->state = SubGhzDeviceCC1101ExtStateIdle;
}
