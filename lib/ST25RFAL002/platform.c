#include "platform.h"
#include <assert.h>
#include <main.h>
#include <furi.h>
#include <api-hal-spi.h>

static osThreadAttr_t platform_irq_thread_attr;
static volatile osThreadId_t platform_irq_thread_id = NULL;
static volatile PlatformIrqCallback platform_irq_callback = NULL;
static ApiHalSpiDevice* platform_st25r3916 = NULL;
static const GpioPin pin = {ST25R_INT_PORT, ST25R_INT_PIN};

void nfc_isr(void* _ctx) {
    if(platform_irq_callback
       && platformGpioIsHigh(ST25R_INT_PORT, ST25R_INT_PIN)) {
        osThreadFlagsSet(platform_irq_thread_id, 0x1);
    }
}

void platformIrqWorker() {
    while(1) {
        uint32_t flags = osThreadFlagsWait(0x1, osFlagsWaitAny, osWaitForever);
        if (flags & 0x1) {
            platform_irq_callback();
        }
    }
}

void platformEnableIrqCallback() {
    hal_gpio_init(&pin, GpioModeInterruptRise, GpioPullNo, GpioSpeedLow);
    hal_gpio_enable_int_callback(&pin);
}

void platformDisableIrqCallback() {
    hal_gpio_init(&pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    hal_gpio_disable_int_callback(&pin);
}

void platformSetIrqCallback(PlatformIrqCallback callback) {
    platform_irq_callback = callback;
    platform_irq_thread_attr.name = "rfal_irq_worker";
    platform_irq_thread_attr.stack_size = 1024;
    platform_irq_thread_attr.priority = osPriorityISR;
    platform_irq_thread_id = osThreadNew(platformIrqWorker, NULL, &platform_irq_thread_attr);
    hal_gpio_add_int_callback(&pin, nfc_isr, NULL);
    // Disable interrupt callback as the pin is shared between 2 apps
    // It is enabled in rfalLowPowerModeStop()
    hal_gpio_disable_int_callback(&pin);
}

HAL_StatusTypeDef platformSpiTxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len) {
    furi_assert(platform_st25r3916);
    bool ret = false;
    if (txBuf && rxBuf) {
        ret = api_hal_spi_bus_trx(platform_st25r3916->bus, (uint8_t*)txBuf, rxBuf, len, 1000);
    } else if (txBuf) {
        ret = api_hal_spi_bus_tx(platform_st25r3916->bus, (uint8_t*)txBuf, len, 1000);
    } else if (rxBuf) {
        ret = api_hal_spi_bus_rx(platform_st25r3916->bus, (uint8_t*)rxBuf, len, 1000);
    }

    if(!ret) {
        asm("bkpt 1");
        return HAL_ERROR;
    } else {
        return HAL_OK;
    }
}

void platformProtectST25RComm() {
    platform_st25r3916 = (ApiHalSpiDevice*)api_hal_spi_device_get(ApiHalSpiDeviceIdNfc);
}

void platformUnprotectST25RComm() {
    furi_assert(platform_st25r3916);
    api_hal_spi_device_return(platform_st25r3916);
}
