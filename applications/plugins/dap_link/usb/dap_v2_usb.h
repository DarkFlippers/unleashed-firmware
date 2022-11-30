#pragma once
#include <furi_hal_usb.h>
#include <usb_cdc.h>

extern FuriHalUsbInterface dap_v2_usb_hid;

// receive callback type
typedef void (*DapRxCallback)(void* context);

typedef void (*DapStateCallback)(bool state, void* context);

/************************************ V1 ***************************************/

int32_t dap_v1_usb_tx(uint8_t* buffer, uint8_t size);

size_t dap_v1_usb_rx(uint8_t* buffer, size_t size);

void dap_v1_usb_set_rx_callback(DapRxCallback callback);

/************************************ V2 ***************************************/

int32_t dap_v2_usb_tx(uint8_t* buffer, uint8_t size);

size_t dap_v2_usb_rx(uint8_t* buffer, size_t size);

void dap_v2_usb_set_rx_callback(DapRxCallback callback);

/************************************ CDC **************************************/

typedef void (*DapCDCControlLineCallback)(uint8_t state, void* context);
typedef void (*DapCDCConfigCallback)(struct usb_cdc_line_coding* config, void* context);

int32_t dap_cdc_usb_tx(uint8_t* buffer, uint8_t size);

size_t dap_cdc_usb_rx(uint8_t* buffer, size_t size);

void dap_cdc_usb_set_rx_callback(DapRxCallback callback);

void dap_cdc_usb_set_control_line_callback(DapCDCControlLineCallback callback);

void dap_cdc_usb_set_config_callback(DapCDCConfigCallback callback);

void dap_cdc_usb_set_context(void* context);

/*********************************** Common ************************************/

void dap_common_usb_set_context(void* context);

void dap_common_usb_set_state_callback(DapStateCallback callback);

void dap_common_usb_alloc_name(const char* name);

void dap_common_usb_free_name();
