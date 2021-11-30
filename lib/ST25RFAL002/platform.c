#include "platform.h"
#include <assert.h>
#include <main.h>
#include <furi.h>
#include <furi-hal-spi.h>

static osThreadAttr_t platform_irq_thread_attr;
static volatile osThreadId_t platform_irq_thread_id = NULL;
static volatile PlatformIrqCallback platform_irq_callback = NULL;
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
    hal_gpio_init(&pin, GpioModeInterruptRise, GpioPullDown, GpioSpeedLow);
    hal_gpio_enable_int_callback(&pin);
}

void platformDisableIrqCallback() {
    hal_gpio_init(&pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    hal_gpio_disable_int_callback(&pin);
}

void platformSetIrqCallback(PlatformIrqCallback callback) {
    platform_irq_callback = callback;
    platform_irq_thread_attr.name = "RfalIrqWorker";
    platform_irq_thread_attr.stack_size = 1024;
    platform_irq_thread_attr.priority = osPriorityISR;
    platform_irq_thread_id = osThreadNew(platformIrqWorker, NULL, &platform_irq_thread_attr);
    hal_gpio_add_int_callback(&pin, nfc_isr, NULL);
    // Disable interrupt callback as the pin is shared between 2 apps
    // It is enabled in rfalLowPowerModeStop()
    hal_gpio_disable_int_callback(&pin);
}

HAL_StatusTypeDef platformSpiTxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len) {
    bool ret = false;
    if (txBuf && rxBuf) {
        ret = furi_hal_spi_bus_trx(&furi_hal_spi_bus_handle_nfc, (uint8_t*)txBuf, rxBuf, len, 1000);
    } else if (txBuf) {
        ret = furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_nfc, (uint8_t*)txBuf, len, 1000);
    } else if (rxBuf) {
        ret = furi_hal_spi_bus_rx(&furi_hal_spi_bus_handle_nfc, (uint8_t*)rxBuf, len, 1000);
    }

    if(!ret) {
        asm("bkpt 1");
        return HAL_ERROR;
    } else {
        return HAL_OK;
    }
}

void platformProtectST25RComm() {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_nfc);
}

void platformUnprotectST25RComm() {
    furi_hal_spi_release(&furi_hal_spi_bus_handle_nfc);
}
