#include "wifi_deauther_app_i.h"
#include "wifi_deauther_uart.h"

#include <stream_buffer.h>

#define UART_CH (FuriHalUartIdUSART1)
#define BAUDRATE (115200)

struct WifideautherUart {
    WifideautherApp* app;
    FuriThread* rx_thread;
    StreamBufferHandle_t rx_stream;
    uint8_t rx_buf[RX_BUF_SIZE + 1];
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context);
};

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

void wifi_deauther_uart_set_handle_rx_data_cb(
    WifideautherUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context)) {
    furi_assert(uart);
    uart->handle_rx_data_cb = handle_rx_data_cb;
}

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxDone)

void wifi_deauther_uart_on_irq_cb(UartIrqEvent ev, uint8_t data, void* context) {
    WifideautherUart* uart = (WifideautherUart*)context;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(ev == UartIrqEventRXNE) {
        xStreamBufferSendFromISR(uart->rx_stream, &data, 1, &xHigherPriorityTaskWoken);
        furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtRxDone);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static int32_t uart_worker(void* context) {
    WifideautherUart* uart = (void*)context;

    uart->rx_stream = xStreamBufferCreate(RX_BUF_SIZE, 1);

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtRxDone) {
            size_t len = xStreamBufferReceive(uart->rx_stream, uart->rx_buf, RX_BUF_SIZE, 0);
            if(len > 0) {
                if(uart->handle_rx_data_cb) uart->handle_rx_data_cb(uart->rx_buf, len, uart->app);
            }
        }
    }

    vStreamBufferDelete(uart->rx_stream);

    return 0;
}

void wifi_deauther_uart_tx(uint8_t* data, size_t len) {
    furi_hal_uart_tx(UART_CH, data, len);
}

WifideautherUart* wifi_deauther_uart_init(WifideautherApp* app) {
    WifideautherUart* uart = malloc(sizeof(WifideautherUart));

    furi_hal_console_disable();
    furi_hal_uart_set_br(UART_CH, BAUDRATE);
    furi_hal_uart_set_irq_cb(UART_CH, wifi_deauther_uart_on_irq_cb, uart);

    uart->app = app;
    uart->rx_thread = furi_thread_alloc();
    furi_thread_set_name(uart->rx_thread, "WifideautherUartRxThread");
    furi_thread_set_stack_size(uart->rx_thread, 1024);
    furi_thread_set_context(uart->rx_thread, uart);
    furi_thread_set_callback(uart->rx_thread, uart_worker);

    furi_thread_start(uart->rx_thread);
    return uart;
}

void wifi_deauther_uart_free(WifideautherUart* uart) {
    furi_assert(uart);

    furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
    furi_thread_join(uart->rx_thread);
    furi_thread_free(uart->rx_thread);

    furi_hal_uart_set_irq_cb(UART_CH, NULL, NULL);
    furi_hal_console_enable();

    free(uart);
}