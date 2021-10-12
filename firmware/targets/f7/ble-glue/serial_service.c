#include "serial_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define SERIAL_SERVICE_TAG "serial service"

#define SERIAL_SVC_DATA_LEN_MAX 245

typedef struct {
    uint16_t svc_handle;
    uint16_t rx_char_handle;
    uint16_t tx_char_handle;
    RpcSession* rpc_session;
    osSemaphoreId_t rpc_sem;
} SerialSvc;

static SerialSvc* serial_svc;

static const uint8_t service_uuid[] = {0x00, 0x00, 0xfe, 0x60, 0xcc, 0x7a, 0x48, 0x2a, 0x98, 0x4a, 0x7f, 0x2e, 0xd5, 0xb3, 0xe5, 0x8f};
static const uint8_t char_rx_uuid[] = {0x00, 0x00, 0xfe, 0x61, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19};
static const uint8_t char_tx_uuid[] = {0x00, 0x00, 0xfe, 0x62, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19};

void serial_svc_rpc_send_bytes_callback(void* context, uint8_t* bytes, size_t bytes_len) {
    size_t bytes_sent = 0;
    while(bytes_sent < bytes_len) {
        size_t bytes_remain = bytes_len - bytes_sent;
        if(bytes_remain > SERIAL_SVC_DATA_LEN_MAX) {
            serial_svc_update_rx(&bytes[bytes_sent], SERIAL_SVC_DATA_LEN_MAX);
            bytes_sent += SERIAL_SVC_DATA_LEN_MAX;
        } else {
            serial_svc_update_rx(&bytes[bytes_sent], bytes_remain);
            bytes_sent += bytes_remain;
        }
        osSemaphoreAcquire(serial_svc->rpc_sem, osWaitForever);
    }
}

static SVCCTL_EvtAckStatus_t serial_svc_event_handler(void *event) {
    SVCCTL_EvtAckStatus_t ret = SVCCTL_EvtNotAck;
    hci_event_pckt* event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)event)->data);
    evt_blecore_aci* blecore_evt = (evt_blecore_aci*)event_pckt->data;
    aci_gatt_attribute_modified_event_rp0* attribute_modified;
    if(event_pckt->evt == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
        if(blecore_evt->ecode == ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE) {
            attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blecore_evt->data;
            if(attribute_modified->Attr_Handle == serial_svc->tx_char_handle + 2) {
                // Descriptor handle
                ret = SVCCTL_EvtAckFlowEnable;
                FURI_LOG_D(SERIAL_SERVICE_TAG, "TX descriptor event");
            } else if(attribute_modified->Attr_Handle == serial_svc->tx_char_handle + 1) {
                FURI_LOG_D(SERIAL_SERVICE_TAG, "Received %d bytes", attribute_modified->Attr_Data_Length);
                rpc_feed_bytes(serial_svc->rpc_session, attribute_modified->Attr_Data, attribute_modified->Attr_Data_Length, 1000);
                // serial_svc_update_rx(attribute_modified->Attr_Data, attribute_modified->Attr_Data_Length);
                ret = SVCCTL_EvtAckFlowEnable;
            }
        } else if(blecore_evt->ecode == ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE) {
            osSemaphoreRelease(serial_svc->rpc_sem);
            FURI_LOG_D(SERIAL_SERVICE_TAG, "Ack received", blecore_evt->ecode);
            ret = SVCCTL_EvtAckFlowEnable;
        }
    }
    return ret;
}

void serial_svc_start(Rpc* rpc) {
    tBleStatus status;
    serial_svc = furi_alloc(sizeof(SerialSvc));
    serial_svc->rpc_sem = osSemaphoreNew(1, 0, NULL);
    // Register event handler
    SVCCTL_RegisterSvcHandler(serial_svc_event_handler);

    // Add service
    status = aci_gatt_add_service(UUID_TYPE_128, (Service_UUID_t *)service_uuid, PRIMARY_SERVICE, 6, &serial_svc->svc_handle);
    if(status) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to add Serial service: %d", status);
    }

    // Add TX characteristics
    status = aci_gatt_add_char(serial_svc->svc_handle, UUID_TYPE_128, (const Char_UUID_t*)char_tx_uuid,
                                SERIAL_SVC_DATA_LEN_MAX,
                                CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_READ,
                                ATTR_PERMISSION_AUTHEN_READ | ATTR_PERMISSION_AUTHEN_WRITE,
                                GATT_NOTIFY_ATTRIBUTE_WRITE,
                                10,
                                CHAR_VALUE_LEN_VARIABLE,
                                &serial_svc->tx_char_handle);
    if(status) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to add TX characteristic: %d", status);
    }

    // Add RX characteristic
    status = aci_gatt_add_char(serial_svc->svc_handle, UUID_TYPE_128, (const Char_UUID_t*)char_rx_uuid,
                                SERIAL_SVC_DATA_LEN_MAX,                                  
                                CHAR_PROP_READ | CHAR_PROP_INDICATE,
                                ATTR_PERMISSION_AUTHEN_READ,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_VARIABLE,
                                &serial_svc->rx_char_handle);
    if(status) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to add RX characteristic: %d", status);
    }
}

void serial_svc_stop() {
    tBleStatus status;
    if(serial_svc) {
        // Delete characteristics
        status = aci_gatt_del_char(serial_svc->svc_handle, serial_svc->tx_char_handle);
        if(status) {
            FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to delete TX characteristic: %d", status);
        }
        status = aci_gatt_del_char(serial_svc->svc_handle, serial_svc->rx_char_handle);
        if(status) {
            FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to delete RX characteristic: %d", status);
        }
        // Delete service
        status = aci_gatt_del_service(serial_svc->svc_handle);
        if(status) {
            FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to delete Serial service: %d", status);
        }
        free(serial_svc);
        serial_svc = NULL;
    }
}

void serial_svc_set_rpc_session(RpcSession* rpc_session) {
    furi_assert(rpc_session);
    // Set session
    serial_svc->rpc_session = rpc_session;
    // Set callback
    rpc_set_send_bytes_callback(serial_svc->rpc_session, serial_svc_rpc_send_bytes_callback, NULL);
}


bool serial_svc_update_rx(uint8_t* data, uint8_t data_len) {
    if(data_len > SERIAL_SVC_DATA_LEN_MAX) {
        return false;
    }
    FURI_LOG_D(SERIAL_SERVICE_TAG, "Updating char %d len", data_len);
    tBleStatus result = aci_gatt_update_char_value(serial_svc->svc_handle,
                                        serial_svc->rx_char_handle,
                                        0,
                                        data_len,
                                        data);
    if(result) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed updating RX characteristic: %d", result);
    }
    return result != BLE_STATUS_SUCCESS;
}
