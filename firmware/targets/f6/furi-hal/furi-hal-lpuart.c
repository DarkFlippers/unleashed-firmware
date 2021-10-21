#include <furi-hal-lpuart.h>
#include <stdbool.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_lpuart.h>

#include <furi.h>

static void (*irq_cb)(uint8_t ev, uint8_t data);

void furi_hal_lpuart_init() {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = PC0_Pin|PC1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

    LL_LPUART_InitTypeDef LPUART_InitStruct = {0};
    LPUART_InitStruct.PrescalerValue = LL_LPUART_PRESCALER_DIV1;
    LPUART_InitStruct.BaudRate = 115200;
    LPUART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_8B;
    LPUART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
    LPUART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
    LPUART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
    LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
    LL_LPUART_Init(LPUART1, &LPUART_InitStruct);
    LL_LPUART_SetTXFIFOThreshold(LPUART1, LL_LPUART_FIFOTHRESHOLD_1_8);
    LL_LPUART_SetRXFIFOThreshold(LPUART1, LL_LPUART_FIFOTHRESHOLD_1_8);
    LL_LPUART_EnableFIFO(LPUART1);

    LL_LPUART_Enable(LPUART1);

    while((!(LL_LPUART_IsActiveFlag_TEACK(LPUART1))) || (!(LL_LPUART_IsActiveFlag_REACK(LPUART1))));

    LL_LPUART_EnableIT_RXNE_RXFNE(LPUART1);
    LL_LPUART_EnableIT_IDLE(LPUART1);
    HAL_NVIC_SetPriority(LPUART1_IRQn, 5, 0);

    FURI_LOG_I("FuriHalLpUart", "Init OK");
}

void furi_hal_lpuart_set_br(uint32_t baud) {
    if (LL_LPUART_IsEnabled(LPUART1)) {
        // Wait for transfer complete flag
        while (!LL_LPUART_IsActiveFlag_TC(LPUART1));
        LL_LPUART_Disable(LPUART1);
        uint32_t uartclk = LL_RCC_GetLPUARTClockFreq(LL_RCC_GetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1));
        if (uartclk/baud > 4095) {
            LL_LPUART_SetPrescaler(LPUART1, LL_LPUART_PRESCALER_DIV32);
            LL_LPUART_SetBaudRate(LPUART1, uartclk, LL_LPUART_PRESCALER_DIV32, baud);
        } else {
            LL_LPUART_SetPrescaler(LPUART1, LL_LPUART_PRESCALER_DIV1);
            LL_LPUART_SetBaudRate(LPUART1, uartclk, LL_LPUART_PRESCALER_DIV1, baud);
        }
        
        LL_LPUART_Enable(LPUART1);
    }
}

void furi_hal_lpuart_deinit() {
    furi_hal_lpuart_set_irq_cb(NULL);
    LL_GPIO_SetPinMode(GPIOC, PC0_Pin, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOC, PC1_Pin, LL_GPIO_MODE_ANALOG);
    LL_LPUART_Disable(LPUART1);
    LL_APB1_GRP2_DisableClock(LL_APB1_GRP2_PERIPH_LPUART1);
}

void furi_hal_lpuart_tx(const uint8_t* buffer, size_t buffer_size) {
    if (LL_LPUART_IsEnabled(LPUART1) == 0)
        return;

    while(buffer_size > 0) {
        while (!LL_LPUART_IsActiveFlag_TXE(LPUART1));

        LL_LPUART_TransmitData8(LPUART1, *buffer);
  
        buffer++;
        buffer_size--;
    }
}

void furi_hal_lpuart_set_irq_cb(void (*cb)(UartIrqEvent ev, uint8_t data)) {
    irq_cb = cb;
    if (irq_cb == NULL)
        NVIC_DisableIRQ(LPUART1_IRQn);
    else
        NVIC_EnableIRQ(LPUART1_IRQn);
}

void LPUART1_IRQHandler(void) {
    if (LL_LPUART_IsActiveFlag_RXNE_RXFNE(LPUART1)) {
        uint8_t data = LL_LPUART_ReceiveData8(LPUART1);
        irq_cb(UartIrqEventRXNE, data);
    } else if (LL_LPUART_IsActiveFlag_IDLE(LPUART1)) {
        irq_cb(UartIrqEventIDLE, 0);
        LL_LPUART_ClearFlag_IDLE(LPUART1);
    }

    //TODO: more events
}
