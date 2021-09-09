#include "serial_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define SERIAL_SERVICE_TAG "serial service"

typedef struct {
    uint16_t svc_handle;
    uint16_t rx_char_handle;
    uint16_t tx_char_handle;
} SerialSvc;

static SerialSvc serial_svc;

static SVCCTL_EvtAckStatus_t serial_svc_event_handler(void *event) {
    SVCCTL_EvtAckStatus_t ret = SVCCTL_EvtNotAck;
    hci_event_pckt* event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)event)->data);
    evt_blecore_aci* blecore_evt = (evt_blecore_aci*)event_pckt->data;
    aci_gatt_attribute_modified_event_rp0* attribute_modified;
    if(event_pckt->evt == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
        if(blecore_evt->ecode == ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE) {
            attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blecore_evt->data;
            if(attribute_modified->Attr_Handle == serial_svc.tx_char_handle + 2) {
                // Descriptor handle
                ret = SVCCTL_EvtAckFlowEnable;
                FURI_LOG_D(SERIAL_SERVICE_TAG, "TX descriptor event");
            } else if(attribute_modified->Attr_Handle == serial_svc.tx_char_handle + 1) {
                FURI_LOG_I(SERIAL_SERVICE_TAG, "Data len: %d", attribute_modified->Attr_Data_Length);
                for(uint8_t i = 0; i < attribute_modified->Attr_Data_Length; i++) {
                    printf("%02X ", attribute_modified->Attr_Data[i]);
                }
                printf("\r\n");
                serial_svc_update_rx(attribute_modified->Attr_Data, attribute_modified->Attr_Data_Length);
                ret = SVCCTL_EvtAckFlowEnable;
            }
        } else if(blecore_evt->ecode == ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE) {
            FURI_LOG_I(SERIAL_SERVICE_TAG, "Ack received", blecore_evt->ecode);
            ret = SVCCTL_EvtAckFlowEnable;
        }
    }
    return ret;
}

bool serial_svc_init() {
    tBleStatus status;
    const uint8_t service_uuid[] = {SERIAL_SVC_UUID_128};
    const uint8_t char_rx_uuid[] = {SERIAL_CHAR_RX_UUID_128};
    const uint8_t char_tx_uuid[] = {SERIAL_CHAR_TX_UUID_128};

    // Register event handler
    SVCCTL_RegisterSvcHandler(serial_svc_event_handler);

    // Add service
    status = aci_gatt_add_service(UUID_TYPE_128, (Service_UUID_t *)service_uuid, PRIMARY_SERVICE, 6, &serial_svc.svc_handle);
    if(status) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to add Serial service: %d", status);
    }

    // Add TX characteristics
    status = aci_gatt_add_char(serial_svc.svc_handle, UUID_TYPE_128, (const Char_UUID_t*)char_tx_uuid ,
                                SERIAL_SVC_DATA_LEN_MAX,
                                CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_NOTIFY_ATTRIBUTE_WRITE,
                                10,
                                CHAR_VALUE_LEN_VARIABLE,
                                &serial_svc.tx_char_handle);
    if(status) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to add TX characteristic: %d", status);
    }

    // Add RX characteristic
    status = aci_gatt_add_char(serial_svc.svc_handle, UUID_TYPE_128, (const Char_UUID_t*)char_rx_uuid ,
                                SERIAL_SVC_DATA_LEN_MAX,                                  
                                CHAR_PROP_READ | CHAR_PROP_INDICATE,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_VARIABLE,
                                &serial_svc.rx_char_handle);
    if(status) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed to add RX characteristic: %d", status);
    }

    return status != BLE_STATUS_SUCCESS;
}

bool serial_svc_update_rx(uint8_t* data, uint8_t data_len) {
    furi_assert(data_len < SERIAL_SVC_DATA_LEN_MAX);

    tBleStatus result = aci_gatt_update_char_value(serial_svc.svc_handle,
                                          serial_svc.rx_char_handle,
                                          0,
                                          data_len,
                                          data);
    if(result) {
        FURI_LOG_E(SERIAL_SERVICE_TAG, "Failed updating RX characteristic: %d", result);
    }
    return result != BLE_STATUS_SUCCESS;
}
