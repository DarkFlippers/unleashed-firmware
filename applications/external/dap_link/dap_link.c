#include <dap.h>
#include <furi.h>
#include <furi_hal_version.h>
#include <furi_hal_gpio.h>
#include <furi_hal_uart.h>
#include <furi_hal_console.h>
#include <furi_hal_resources.h>
#include <furi_hal_power.h>
#include <stm32wbxx_ll_usart.h>
#include <stm32wbxx_ll_lpuart.h>

#include "dap_link.h"
#include "dap_config.h"
#include "gui/dap_gui.h"
#include "usb/dap_v2_usb.h"
#include <dialogs/dialogs.h>
#include "dap_link_icons.h"

/***************************************************************************/
/****************************** DAP COMMON *********************************/
/***************************************************************************/

struct DapApp {
    FuriThread* dap_thread;
    FuriThread* cdc_thread;
    FuriThread* gui_thread;

    DapState state;
    DapConfig config;
};

void dap_app_get_state(DapApp* app, DapState* state) {
    *state = app->state;
}

#define DAP_PROCESS_THREAD_TICK 500

typedef enum {
    DapThreadEventStop = (1 << 0),
} DapThreadEvent;

void dap_thread_send_stop(FuriThread* thread) {
    furi_thread_flags_set(furi_thread_get_id(thread), DapThreadEventStop);
}

GpioPin flipper_dap_swclk_pin;
GpioPin flipper_dap_swdio_pin;
GpioPin flipper_dap_reset_pin;
GpioPin flipper_dap_tdo_pin;
GpioPin flipper_dap_tdi_pin;

/***************************************************************************/
/****************************** DAP PROCESS ********************************/
/***************************************************************************/

typedef struct {
    uint8_t data[DAP_CONFIG_PACKET_SIZE];
    uint8_t size;
} DapPacket;

typedef enum {
    DAPThreadEventStop = DapThreadEventStop,
    DAPThreadEventRxV1 = (1 << 1),
    DAPThreadEventRxV2 = (1 << 2),
    DAPThreadEventUSBConnect = (1 << 3),
    DAPThreadEventUSBDisconnect = (1 << 4),
    DAPThreadEventApplyConfig = (1 << 5),
    DAPThreadEventAll = DAPThreadEventStop | DAPThreadEventRxV1 | DAPThreadEventRxV2 |
                        DAPThreadEventUSBConnect | DAPThreadEventUSBDisconnect |
                        DAPThreadEventApplyConfig,
} DAPThreadEvent;

#define USB_SERIAL_NUMBER_LEN 16
char usb_serial_number[USB_SERIAL_NUMBER_LEN] = {0};

const char* dap_app_get_serial(DapApp* app) {
    UNUSED(app);
    return usb_serial_number;
}

static void dap_app_rx1_callback(void* context) {
    furi_assert(context);
    FuriThreadId thread_id = (FuriThreadId)context;
    furi_thread_flags_set(thread_id, DAPThreadEventRxV1);
}

static void dap_app_rx2_callback(void* context) {
    furi_assert(context);
    FuriThreadId thread_id = (FuriThreadId)context;
    furi_thread_flags_set(thread_id, DAPThreadEventRxV2);
}

static void dap_app_usb_state_callback(bool state, void* context) {
    furi_assert(context);
    FuriThreadId thread_id = (FuriThreadId)context;
    if(state) {
        furi_thread_flags_set(thread_id, DAPThreadEventUSBConnect);
    } else {
        furi_thread_flags_set(thread_id, DAPThreadEventUSBDisconnect);
    }
}

static void dap_app_process_v1() {
    DapPacket tx_packet;
    DapPacket rx_packet;
    memset(&tx_packet, 0, sizeof(DapPacket));
    rx_packet.size = dap_v1_usb_rx(rx_packet.data, DAP_CONFIG_PACKET_SIZE);
    dap_process_request(rx_packet.data, rx_packet.size, tx_packet.data, DAP_CONFIG_PACKET_SIZE);
    dap_v1_usb_tx(tx_packet.data, DAP_CONFIG_PACKET_SIZE);
}

static void dap_app_process_v2() {
    DapPacket tx_packet;
    DapPacket rx_packet;
    memset(&tx_packet, 0, sizeof(DapPacket));
    rx_packet.size = dap_v2_usb_rx(rx_packet.data, DAP_CONFIG_PACKET_SIZE);
    size_t len = dap_process_request(
        rx_packet.data, rx_packet.size, tx_packet.data, DAP_CONFIG_PACKET_SIZE);
    dap_v2_usb_tx(tx_packet.data, len);
}

void dap_app_vendor_cmd(uint8_t cmd) {
    // openocd -c "cmsis-dap cmd 81"
    if(cmd == 0x01) {
        furi_hal_power_reset();
    }
}

void dap_app_target_reset() {
    FURI_LOG_I("DAP", "Target reset");
}

static void dap_init_gpio(DapSwdPins swd_pins) {
    switch(swd_pins) {
    case DapSwdPinsPA7PA6:
        flipper_dap_swclk_pin = gpio_ext_pa7;
        flipper_dap_swdio_pin = gpio_ext_pa6;
        break;
    case DapSwdPinsPA14PA13:
        flipper_dap_swclk_pin = (GpioPin){.port = GPIOA, .pin = LL_GPIO_PIN_14};
        flipper_dap_swdio_pin = (GpioPin){.port = GPIOA, .pin = LL_GPIO_PIN_13};
        break;
    }

    flipper_dap_reset_pin = gpio_ext_pa4;
    flipper_dap_tdo_pin = gpio_ext_pb3;
    flipper_dap_tdi_pin = gpio_ext_pb2;
}

static void dap_deinit_gpio(DapSwdPins swd_pins) {
    // setup gpio pins to default state
    furi_hal_gpio_init(&flipper_dap_reset_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&flipper_dap_tdo_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&flipper_dap_tdi_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    if(DapSwdPinsPA14PA13 == swd_pins) {
        // PA14 and PA13 are used by SWD
        furi_hal_gpio_init_ex(
            &flipper_dap_swclk_pin,
            GpioModeAltFunctionPushPull,
            GpioPullDown,
            GpioSpeedLow,
            GpioAltFn0JTCK_SWCLK);
        furi_hal_gpio_init_ex(
            &flipper_dap_swdio_pin,
            GpioModeAltFunctionPushPull,
            GpioPullUp,
            GpioSpeedVeryHigh,
            GpioAltFn0JTMS_SWDIO);
    } else {
        furi_hal_gpio_init(&flipper_dap_swclk_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(&flipper_dap_swdio_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    }
}

static int32_t dap_process(void* p) {
    DapApp* app = p;
    DapState* dap_state = &(app->state);

    // allocate resources
    FuriHalUsbInterface* usb_config_prev;
    app->config.swd_pins = DapSwdPinsPA7PA6;
    DapSwdPins swd_pins_prev = app->config.swd_pins;

    // init pins
    dap_init_gpio(swd_pins_prev);

    // init dap
    dap_init();

    // get name
    const char* name = furi_hal_version_get_name_ptr();
    if(!name) {
        name = "Flipper";
    }
    snprintf(usb_serial_number, USB_SERIAL_NUMBER_LEN, "DAP_%s", name);

    // init usb
    usb_config_prev = furi_hal_usb_get_config();
    dap_common_usb_alloc_name(usb_serial_number);
    dap_common_usb_set_context(furi_thread_get_id(furi_thread_get_current()));
    dap_v1_usb_set_rx_callback(dap_app_rx1_callback);
    dap_v2_usb_set_rx_callback(dap_app_rx2_callback);
    dap_common_usb_set_state_callback(dap_app_usb_state_callback);
    furi_hal_usb_set_config(&dap_v2_usb_hid, NULL);

    // work
    uint32_t events;
    while(1) {
        events = furi_thread_flags_wait(DAPThreadEventAll, FuriFlagWaitAny, FuriWaitForever);

        if(!(events & FuriFlagError)) {
            if(events & DAPThreadEventRxV1) {
                dap_app_process_v1();
                dap_state->dap_counter++;
                dap_state->dap_version = DapVersionV1;
            }

            if(events & DAPThreadEventRxV2) {
                dap_app_process_v2();
                dap_state->dap_counter++;
                dap_state->dap_version = DapVersionV2;
            }

            if(events & DAPThreadEventUSBConnect) {
                dap_state->usb_connected = true;
            }

            if(events & DAPThreadEventUSBDisconnect) {
                dap_state->usb_connected = false;
                dap_state->dap_version = DapVersionUnknown;
            }

            if(events & DAPThreadEventApplyConfig) {
                if(swd_pins_prev != app->config.swd_pins) {
                    dap_deinit_gpio(swd_pins_prev);
                    swd_pins_prev = app->config.swd_pins;
                    dap_init_gpio(swd_pins_prev);
                }
            }

            if(events & DAPThreadEventStop) {
                break;
            }
        }
    }

    // deinit usb
    furi_hal_usb_set_config(usb_config_prev, NULL);
    dap_common_usb_free_name();
    dap_deinit_gpio(swd_pins_prev);
    return 0;
}

/***************************************************************************/
/****************************** CDC PROCESS ********************************/
/***************************************************************************/

typedef enum {
    CDCThreadEventStop = DapThreadEventStop,
    CDCThreadEventUARTRx = (1 << 1),
    CDCThreadEventCDCRx = (1 << 2),
    CDCThreadEventCDCConfig = (1 << 3),
    CDCThreadEventApplyConfig = (1 << 4),
    CDCThreadEventAll = CDCThreadEventStop | CDCThreadEventUARTRx | CDCThreadEventCDCRx |
                        CDCThreadEventCDCConfig | CDCThreadEventApplyConfig,
} CDCThreadEvent;

typedef struct {
    FuriStreamBuffer* rx_stream;
    FuriThreadId thread_id;
    FuriHalUartId uart_id;
    struct usb_cdc_line_coding line_coding;
} CDCProcess;

static void cdc_uart_irq_cb(UartIrqEvent ev, uint8_t data, void* ctx) {
    CDCProcess* app = ctx;

    if(ev == UartIrqEventRXNE) {
        furi_stream_buffer_send(app->rx_stream, &data, 1, 0);
        furi_thread_flags_set(app->thread_id, CDCThreadEventUARTRx);
    }
}

static void cdc_usb_rx_callback(void* context) {
    CDCProcess* app = context;
    furi_thread_flags_set(app->thread_id, CDCThreadEventCDCRx);
}

static void cdc_usb_control_line_callback(uint8_t state, void* context) {
    UNUSED(context);
    UNUSED(state);
}

static void cdc_usb_config_callback(struct usb_cdc_line_coding* config, void* context) {
    CDCProcess* app = context;
    app->line_coding = *config;
    furi_thread_flags_set(app->thread_id, CDCThreadEventCDCConfig);
}

static FuriHalUartId cdc_init_uart(
    DapUartType type,
    DapUartTXRX swap,
    uint32_t baudrate,
    void (*cb)(UartIrqEvent ev, uint8_t data, void* ctx),
    void* ctx) {
    FuriHalUartId uart_id = FuriHalUartIdUSART1;
    if(baudrate == 0) baudrate = 115200;

    switch(type) {
    case DapUartTypeUSART1:
        uart_id = FuriHalUartIdUSART1;
        furi_hal_console_disable();
        furi_hal_uart_deinit(uart_id);
        if(swap == DapUartTXRXSwap) {
            LL_USART_SetTXRXSwap(USART1, LL_USART_TXRX_SWAPPED);
        } else {
            LL_USART_SetTXRXSwap(USART1, LL_USART_TXRX_STANDARD);
        }
        furi_hal_uart_init(uart_id, baudrate);
        furi_hal_uart_set_irq_cb(uart_id, cb, ctx);
        break;
    case DapUartTypeLPUART1:
        uart_id = FuriHalUartIdLPUART1;
        furi_hal_uart_deinit(uart_id);
        if(swap == DapUartTXRXSwap) {
            LL_LPUART_SetTXRXSwap(LPUART1, LL_LPUART_TXRX_SWAPPED);
        } else {
            LL_LPUART_SetTXRXSwap(LPUART1, LL_LPUART_TXRX_STANDARD);
        }
        furi_hal_uart_init(uart_id, baudrate);
        furi_hal_uart_set_irq_cb(uart_id, cb, ctx);
        break;
    }

    return uart_id;
}

static void cdc_deinit_uart(DapUartType type) {
    switch(type) {
    case DapUartTypeUSART1:
        furi_hal_uart_deinit(FuriHalUartIdUSART1);
        LL_USART_SetTXRXSwap(USART1, LL_USART_TXRX_STANDARD);
        furi_hal_console_init();
        break;
    case DapUartTypeLPUART1:
        furi_hal_uart_deinit(FuriHalUartIdLPUART1);
        LL_LPUART_SetTXRXSwap(LPUART1, LL_LPUART_TXRX_STANDARD);
        break;
    }
}

static int32_t cdc_process(void* p) {
    DapApp* dap_app = p;
    DapState* dap_state = &(dap_app->state);

    dap_app->config.uart_pins = DapUartTypeLPUART1;
    dap_app->config.uart_swap = DapUartTXRXNormal;

    DapUartType uart_pins_prev = dap_app->config.uart_pins;
    DapUartTXRX uart_swap_prev = dap_app->config.uart_swap;

    CDCProcess* app = malloc(sizeof(CDCProcess));
    app->thread_id = furi_thread_get_id(furi_thread_get_current());
    app->rx_stream = furi_stream_buffer_alloc(512, 1);

    const uint8_t rx_buffer_size = 64;
    uint8_t* rx_buffer = malloc(rx_buffer_size);

    app->uart_id = cdc_init_uart(
        uart_pins_prev, uart_swap_prev, dap_state->cdc_baudrate, cdc_uart_irq_cb, app);

    dap_cdc_usb_set_context(app);
    dap_cdc_usb_set_rx_callback(cdc_usb_rx_callback);
    dap_cdc_usb_set_control_line_callback(cdc_usb_control_line_callback);
    dap_cdc_usb_set_config_callback(cdc_usb_config_callback);

    uint32_t events;
    while(1) {
        events = furi_thread_flags_wait(CDCThreadEventAll, FuriFlagWaitAny, FuriWaitForever);

        if(!(events & FuriFlagError)) {
            if(events & CDCThreadEventCDCConfig) {
                if(dap_state->cdc_baudrate != app->line_coding.dwDTERate) {
                    dap_state->cdc_baudrate = app->line_coding.dwDTERate;
                    if(dap_state->cdc_baudrate > 0) {
                        furi_hal_uart_set_br(app->uart_id, dap_state->cdc_baudrate);
                    }
                }
            }

            if(events & CDCThreadEventUARTRx) {
                size_t len =
                    furi_stream_buffer_receive(app->rx_stream, rx_buffer, rx_buffer_size, 0);

                if(len > 0) {
                    dap_cdc_usb_tx(rx_buffer, len);
                }
                dap_state->cdc_rx_counter += len;
            }

            if(events & CDCThreadEventCDCRx) {
                size_t len = dap_cdc_usb_rx(rx_buffer, rx_buffer_size);
                if(len > 0) {
                    furi_hal_uart_tx(app->uart_id, rx_buffer, len);
                }
                dap_state->cdc_tx_counter += len;
            }

            if(events & CDCThreadEventApplyConfig) {
                if(uart_pins_prev != dap_app->config.uart_pins ||
                   uart_swap_prev != dap_app->config.uart_swap) {
                    cdc_deinit_uart(uart_pins_prev);
                    uart_pins_prev = dap_app->config.uart_pins;
                    uart_swap_prev = dap_app->config.uart_swap;
                    app->uart_id = cdc_init_uart(
                        uart_pins_prev,
                        uart_swap_prev,
                        dap_state->cdc_baudrate,
                        cdc_uart_irq_cb,
                        app);
                }
            }

            if(events & CDCThreadEventStop) {
                break;
            }
        }
    }

    cdc_deinit_uart(uart_pins_prev);
    free(rx_buffer);
    furi_stream_buffer_free(app->rx_stream);
    free(app);

    return 0;
}

/***************************************************************************/
/******************************* MAIN APP **********************************/
/***************************************************************************/

static DapApp* dap_app_alloc() {
    DapApp* dap_app = malloc(sizeof(DapApp));
    dap_app->dap_thread = furi_thread_alloc_ex("DAP Process", 1024, dap_process, dap_app);
    dap_app->cdc_thread = furi_thread_alloc_ex("DAP CDC", 1024, cdc_process, dap_app);
    dap_app->gui_thread = furi_thread_alloc_ex("DAP GUI", 1024, dap_gui_thread, dap_app);
    return dap_app;
}

static void dap_app_free(DapApp* dap_app) {
    furi_assert(dap_app);
    furi_thread_free(dap_app->dap_thread);
    furi_thread_free(dap_app->cdc_thread);
    furi_thread_free(dap_app->gui_thread);
    free(dap_app);
}

static DapApp* app_handle = NULL;

void dap_app_disconnect() {
    app_handle->state.dap_mode = DapModeDisconnected;
}

void dap_app_connect_swd() {
    app_handle->state.dap_mode = DapModeSWD;
}

void dap_app_connect_jtag() {
    app_handle->state.dap_mode = DapModeJTAG;
}

void dap_app_set_config(DapApp* app, DapConfig* config) {
    app->config = *config;
    furi_thread_flags_set(furi_thread_get_id(app->dap_thread), DAPThreadEventApplyConfig);
    furi_thread_flags_set(furi_thread_get_id(app->cdc_thread), CDCThreadEventApplyConfig);
}

DapConfig* dap_app_get_config(DapApp* app) {
    return &app->config;
}

int32_t dap_link_app(void* p) {
    UNUSED(p);

    if(furi_hal_usb_is_locked()) {
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_header(message, "Connection\nis active!", 3, 2, AlignLeft, AlignTop);
        dialog_message_set_text(
            message,
            "Disconnect from\nPC or phone to\nuse this function.",
            3,
            30,
            AlignLeft,
            AlignTop);
        dialog_message_set_icon(message, &I_ActiveConnection_50x64, 78, 0);
        dialog_message_show(dialogs, message);
        dialog_message_free(message);
        furi_record_close(RECORD_DIALOGS);
        return -1;
    }

    // alloc app
    DapApp* app = dap_app_alloc();
    app_handle = app;

    furi_thread_start(app->dap_thread);
    furi_thread_start(app->cdc_thread);
    furi_thread_start(app->gui_thread);

    // wait until gui thread is finished
    furi_thread_join(app->gui_thread);

    // send stop event to threads
    dap_thread_send_stop(app->dap_thread);
    dap_thread_send_stop(app->cdc_thread);

    // wait for threads to stop
    furi_thread_join(app->dap_thread);
    furi_thread_join(app->cdc_thread);

    // free app
    dap_app_free(app);

    return 0;
}