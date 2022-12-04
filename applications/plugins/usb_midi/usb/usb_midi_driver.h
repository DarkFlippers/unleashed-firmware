#pragma once
#include <furi_hal_usb.h>

extern FuriHalUsbInterface midi_usb_interface;

typedef void (*MidiRxCallback)(void* context);

void midi_usb_set_context(void* context);

void midi_usb_set_rx_callback(MidiRxCallback callback);

size_t midi_usb_rx(uint8_t* buffer, size_t size);

size_t midi_usb_tx(uint8_t* buffer, uint8_t size);