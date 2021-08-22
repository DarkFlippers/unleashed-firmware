#pragma once

#include <furi-hal-vcp.h>

void furi_hal_vcp_on_usb_resume();

void furi_hal_vcp_on_usb_suspend();

void furi_hal_vcp_on_cdc_control_line(uint8_t state);

void furi_hal_vcp_on_cdc_rx(const uint8_t* buffer, size_t size);

void furi_hal_vcp_on_cdc_tx_complete(size_t size);
