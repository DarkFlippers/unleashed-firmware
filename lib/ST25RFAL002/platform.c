#include "platform.h"
#include <assert.h>
#include <main.h>
#include <furi.h>
#include <api-hal-spi.h>

static osThreadAttr_t platform_irq_thread_attr;
static volatile osThreadId_t platform_irq_thread_id = NULL;
static volatile PlatformIrqCallback platform_irq_callback = NULL;

void nfc_isr(void* _pin, void* _ctx) {
    uint32_t pin = (uint32_t)_pin;
    if(pin == NFC_IRQ_Pin
        && platform_irq_callback
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

void platformSetIrqCallback(PlatformIrqCallback callback) {
    platform_irq_callback = callback;
    platform_irq_thread_attr.name = "rfal_irq_worker";
    platform_irq_thread_attr.stack_size = 1024;
    platform_irq_thread_attr.priority = osPriorityISR;
    platform_irq_thread_id = osThreadNew(platformIrqWorker, NULL, &platform_irq_thread_attr);
    api_interrupt_add(nfc_isr, InterruptTypeExternalInterrupt, NULL);
}

HAL_StatusTypeDef platformSpiTxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len) {
    HAL_StatusTypeDef ret;
    if (txBuf && rxBuf) {
        ret = HAL_SPI_TransmitReceive(&SPI_R, (uint8_t*)txBuf, rxBuf, len, HAL_MAX_DELAY);
    } else if (txBuf) {
        ret = HAL_SPI_Transmit(&SPI_R, (uint8_t*)txBuf, len, HAL_MAX_DELAY);
    } else if (rxBuf) {
        ret = HAL_SPI_Receive(&SPI_R, (uint8_t*)rxBuf, len, HAL_MAX_DELAY);
    }
    
    if(ret != HAL_OK) {
        asm("bkpt 1");
        exit(255);
    }
    return ret;
}

void platformProtectST25RComm() {
    api_hal_spi_lock(&SPI_R);
    NFC_SPI_Reconfigure();
}

void platformUnprotectST25RComm() {
    api_hal_spi_unlock(&SPI_R);
}
