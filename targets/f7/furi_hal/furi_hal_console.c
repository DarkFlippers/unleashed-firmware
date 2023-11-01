#include <furi_hal_console.h>
#include <furi_hal_uart.h>

#include <stdbool.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_usart.h>

#include <furi.h>

#define TAG "FuriHalConsole"

#ifdef HEAP_PRINT_DEBUG
#define CONSOLE_BAUDRATE 1843200
#else
#define CONSOLE_BAUDRATE 230400
#endif

typedef struct {
    bool alive;
    FuriHalConsoleTxCallback tx_callback;
    void* tx_callback_context;
} FuriHalConsole;

FuriHalConsole furi_hal_console = {
    .alive = false,
    .tx_callback = NULL,
    .tx_callback_context = NULL,
};

void furi_hal_console_init() {
    furi_hal_uart_init(FuriHalUartIdUSART1, CONSOLE_BAUDRATE);
    furi_hal_console.alive = true;
}

void furi_hal_console_enable() {
    furi_hal_uart_set_irq_cb(FuriHalUartIdUSART1, NULL, NULL);
    while(!LL_USART_IsActiveFlag_TC(USART1))
        ;
    furi_hal_uart_set_br(FuriHalUartIdUSART1, CONSOLE_BAUDRATE);
    furi_hal_console.alive = true;
}

void furi_hal_console_disable() {
    while(!LL_USART_IsActiveFlag_TC(USART1))
        ;
    furi_hal_console.alive = false;
}

void furi_hal_console_set_tx_callback(FuriHalConsoleTxCallback callback, void* context) {
    FURI_CRITICAL_ENTER();
    furi_hal_console.tx_callback = callback;
    furi_hal_console.tx_callback_context = context;
    FURI_CRITICAL_EXIT();
}

void furi_hal_console_tx(const uint8_t* buffer, size_t buffer_size) {
    if(!furi_hal_console.alive) return;

    FURI_CRITICAL_ENTER();
    // Transmit data

    if(furi_hal_console.tx_callback) {
        furi_hal_console.tx_callback(buffer, buffer_size, furi_hal_console.tx_callback_context);
    }

    furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)buffer, buffer_size);
    // Wait for TC flag to be raised for last char
    while(!LL_USART_IsActiveFlag_TC(USART1))
        ;
    FURI_CRITICAL_EXIT();
}

void furi_hal_console_tx_with_new_line(const uint8_t* buffer, size_t buffer_size) {
    if(!furi_hal_console.alive) return;

    FURI_CRITICAL_ENTER();
    // Transmit data
    furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)buffer, buffer_size);
    // Transmit new line symbols
    furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\r\n", 2);
    // Wait for TC flag to be raised for last char
    while(!LL_USART_IsActiveFlag_TC(USART1))
        ;
    FURI_CRITICAL_EXIT();
}

void furi_hal_console_printf(const char format[], ...) {
    FuriString* string;
    va_list args;
    va_start(args, format);
    string = furi_string_alloc_vprintf(format, args);
    va_end(args);
    furi_hal_console_tx((const uint8_t*)furi_string_get_cstr(string), furi_string_size(string));
    furi_string_free(string);
}

void furi_hal_console_puts(const char* data) {
    furi_hal_console_tx((const uint8_t*)data, strlen(data));
}
