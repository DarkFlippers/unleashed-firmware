#pragma once
#include <stdint.h>

typedef enum {
    DapModeDisconnected,
    DapModeSWD,
    DapModeJTAG,
} DapMode;

typedef enum {
    DapVersionUnknown,
    DapVersionV1,
    DapVersionV2,
} DapVersion;

typedef struct {
    bool usb_connected;
    DapMode dap_mode;
    DapVersion dap_version;
    uint32_t dap_counter;
    uint32_t cdc_baudrate;
    uint32_t cdc_tx_counter;
    uint32_t cdc_rx_counter;
} DapState;

typedef enum {
    DapSwdPinsPA7PA6, // Pins 2, 3
    DapSwdPinsPA14PA13, // Pins 10, 12
} DapSwdPins;

typedef enum {
    DapUartTypeUSART1, // Pins 13, 14
    DapUartTypeLPUART1, // Pins 15, 16
} DapUartType;

typedef enum {
    DapUartTXRXNormal,
    DapUartTXRXSwap,
} DapUartTXRX;

typedef struct {
    DapSwdPins swd_pins;
    DapUartType uart_pins;
    DapUartTXRX uart_swap;
} DapConfig;

typedef struct DapApp DapApp;

void dap_app_get_state(DapApp* app, DapState* state);

const char* dap_app_get_serial(DapApp* app);

void dap_app_set_config(DapApp* app, DapConfig* config);

DapConfig* dap_app_get_config(DapApp* app);