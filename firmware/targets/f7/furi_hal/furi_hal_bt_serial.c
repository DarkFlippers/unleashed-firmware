#include <furi_hal_bt_serial.h>
#include "dev_info_service.h"
#include "battery_service.h"
#include "serial_service.h"

#include <furi.h>

void furi_hal_bt_serial_start() {
    // Start device info
    if(!dev_info_svc_is_started()) {
        dev_info_svc_start();
    }
    // Start battery service
    if(!battery_svc_is_started()) {
        battery_svc_start();
    }
    // Start Serial service
    if(!serial_svc_is_started()) {
        serial_svc_start();
    }
}

void furi_hal_bt_serial_set_event_callback(
    uint16_t buff_size,
    FuriHalBtSerialCallback callback,
    void* context) {
    serial_svc_set_callbacks(buff_size, callback, context);
}

void furi_hal_bt_serial_notify_buffer_is_empty() {
    serial_svc_notify_buffer_is_empty();
}

void furi_hal_bt_serial_set_rpc_status(FuriHalBtSerialRpcStatus status) {
    SerialServiceRpcStatus st;
    if(status == FuriHalBtSerialRpcStatusActive) {
        st = SerialServiceRpcStatusActive;
    } else {
        st = SerialServiceRpcStatusNotActive;
    }
    serial_svc_set_rpc_status(st);
}

bool furi_hal_bt_serial_tx(uint8_t* data, uint16_t size) {
    if(size > FURI_HAL_BT_SERIAL_PACKET_SIZE_MAX) {
        return false;
    }
    return serial_svc_update_tx(data, size);
}

void furi_hal_bt_serial_stop() {
    // Stop all services
    if(dev_info_svc_is_started()) {
        dev_info_svc_stop();
    }
    // Start battery service
    if(battery_svc_is_started()) {
        battery_svc_stop();
    }
    // Start Serial service
    if(serial_svc_is_started()) {
        serial_svc_stop();
    }
}
