#include <furi-hal-console.h>
#include <furi-hal-uart.h>

#include <stdbool.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_usart.h>
#include <m-string.h>

#include <utilities_conf.h>

#include <furi.h>

#define TAG "FuriHalConsole"

#define CONSOLE_BAUDRATE 230400

volatile bool furi_hal_console_alive = false;

void furi_hal_console_init() {
    furi_hal_uart_init(FuriHalUartIdUSART1, CONSOLE_BAUDRATE);
    furi_hal_console_alive = true;

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_console_enable() {
    furi_hal_uart_set_irq_cb(FuriHalUartIdUSART1, NULL, NULL);
    while (!LL_USART_IsActiveFlag_TC(USART1));
    furi_hal_uart_set_br(FuriHalUartIdUSART1, CONSOLE_BAUDRATE);
    furi_hal_console_alive = true;
}

void furi_hal_console_disable() {
    while (!LL_USART_IsActiveFlag_TC(USART1));
    furi_hal_console_alive = false;
}

void furi_hal_console_tx(const uint8_t* buffer, size_t buffer_size) {
    if (!furi_hal_console_alive)
        return;

    FURI_CRITICAL_ENTER();
    // Transmit data
    furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)buffer, buffer_size);
    // Wait for TC flag to be raised for last char
    while (!LL_USART_IsActiveFlag_TC(USART1));
    FURI_CRITICAL_EXIT();
}

void furi_hal_console_tx_with_new_line(const uint8_t* buffer, size_t buffer_size) {
    if (!furi_hal_console_alive)
        return;

    FURI_CRITICAL_ENTER();
    // Transmit data
    furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)buffer, buffer_size);
    // Transmit new line symbols
    furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\r\n", 2);
    // Wait for TC flag to be raised for last char
    while (!LL_USART_IsActiveFlag_TC(USART1));
    FURI_CRITICAL_EXIT();
}

void furi_hal_console_printf(const char format[], ...) {
    string_t string;
    va_list args;
    va_start(args, format);
    string_init_vprintf(string, format, args);
    va_end(args);
    furi_hal_console_tx((const uint8_t*)string_get_cstr(string), string_size(string));
    string_clear(string);
}

void furi_hal_console_puts(const char *data) {
    furi_hal_console_tx((const uint8_t*)data, strlen(data));
}
