#include "serial_service.h"
#include "app_common.h"
#include <ble/ble.h>

#include <furi.h>

#define TAG "BtSerialSvc"

typedef struct {
    uint16_t svc_handle;
    uint16_t rx_char_handle;
    uint16_t tx_char_handle;
    uint16_t flow_ctrl_char_handle;
    FuriMutex* buff_size_mtx;
    uint32_t buff_size;
    uint16_t bytes_ready_to_receive;
    SerialServiceEventCallback callback;
    void* context;
} SerialSvc;

static SerialSvc* serial_svc = NULL;

static const uint8_t service_uuid[] =
    {0x00, 0x00, 0xfe, 0x60, 0xcc, 0x7a, 0x48, 0x2a, 0x98, 0x4a, 0x7f, 0x2e, 0xd5, 0xb3, 0xe5, 0x8f};
static const uint8_t char_tx_uuid[] =
    {0x00, 0x00, 0xfe, 0x61, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19};
static const uint8_t char_rx_uuid[] =
    {0x00, 0x00, 0xfe, 0x62, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19};
static const uint8_t flow_ctrl_uuid[] =
    {0x00, 0x00, 0xfe, 0x63, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19};

static SVCCTL_EvtAckStatus_t serial_svc_event_handler(void* event) {
    SVCCTL_EvtAckStatus_t ret = SVCCTL_EvtNotAck;
    hci_event_pckt* event_pckt = (hci_event_pckt*)(((hci_uart_pckt*)event)->data);
    evt_blecore_aci* blecore_evt = (evt_blecore_aci*)event_pckt->data;
    aci_gatt_attribute_modified_event_rp0* attribute_modified;
    if(event_pckt->evt == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
        if(blecore_evt->ecode == ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE) {
            attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blecore_evt->data;
            if(attribute_modified->Attr_Handle == serial_svc->rx_char_handle + 2) {
                // Descriptor handle
                ret = SVCCTL_EvtAckFlowEnable;
                FURI_LOG_D(TAG, "RX descriptor event");
            } else if(attribute_modified->Attr_Handle == serial_svc->rx_char_handle + 1) {
                FURI_LOG_D(TAG, "Received %d bytes", attribute_modified->Attr_Data_Length);
                if(serial_svc->callback) {
                    furi_check(
                        furi_mutex_acquire(serial_svc->buff_size_mtx, FuriWaitForever) ==
                        FuriStatusOk);
                    if(attribute_modified->Attr_Data_Length > serial_svc->bytes_ready_to_receive) {
                        FURI_LOG_W(
                            TAG,
                            "Received %d, while was ready to receive %d bytes. Can lead to buffer overflow!",
                            attribute_modified->Attr_Data_Length,
                            serial_svc->bytes_ready_to_receive);
                    }
                    serial_svc->bytes_ready_to_receive -= MIN(
                        serial_svc->bytes_ready_to_receive, attribute_modified->Attr_Data_Length);
                    SerialServiceEvent event = {
                        .event = SerialServiceEventTypeDataReceived,
                        .data = {
                            .buffer = attribute_modified->Attr_Data,
                            .size = attribute_modified->Attr_Data_Length,
                        }};
                    uint32_t buff_free_size = serial_svc->callback(event, serial_svc->context);
                    FURI_LOG_D(TAG, "Available buff size: %d", buff_free_size);
                    furi_check(furi_mutex_release(serial_svc->buff_size_mtx) == FuriStatusOk);
                }
                ret = SVCCTL_EvtAckFlowEnable;
            }
        } else if(blecore_evt->ecode == ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE) {
            FURI_LOG_T(TAG, "Ack received", blecore_evt->ecode);
            if(serial_svc->callback) {
                SerialServiceEvent event = {
                    .event = SerialServiceEventTypeDataSent,
                };
                serial_svc->callback(event, serial_svc->context);
            }
            ret = SVCCTL_EvtAckFlowEnable;
        }
    }
    return ret;
}

void serial_svc_start() {
    tBleStatus status;
    serial_svc = malloc(sizeof(SerialSvc));
    // Register event handler
    SVCCTL_RegisterSvcHandler(serial_svc_event_handler);

    // Add service
    status = aci_gatt_add_service(
        UUID_TYPE_128, (Service_UUID_t*)service_uuid, PRIMARY_SERVICE, 10, &serial_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Serial service: %d", status);
    }

    // Add RX characteristics
    status = aci_gatt_add_char(
        serial_svc->svc_handle,
        UUID_TYPE_128,
        (const Char_UUID_t*)char_rx_uuid,
        SERIAL_SVC_DATA_LEN_MAX,
        CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_READ,
        ATTR_PERMISSION_AUTHEN_READ | ATTR_PERMISSION_AUTHEN_WRITE,
        GATT_NOTIFY_ATTRIBUTE_WRITE,
        10,
        CHAR_VALUE_LEN_VARIABLE,
        &serial_svc->rx_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add RX characteristic: %d", status);
    }

    // Add TX characteristic
    status = aci_gatt_add_char(
        serial_svc->svc_handle,
        UUID_TYPE_128,
        (const Char_UUID_t*)char_tx_uuid,
        SERIAL_SVC_DATA_LEN_MAX,
        CHAR_PROP_READ | CHAR_PROP_INDICATE,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_VARIABLE,
        &serial_svc->tx_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add TX characteristic: %d", status);
    }
    // Add Flow Control characteristic
    status = aci_gatt_add_char(
        serial_svc->svc_handle,
        UUID_TYPE_128,
        (const Char_UUID_t*)flow_ctrl_uuid,
        sizeof(uint32_t),
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &serial_svc->flow_ctrl_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Flow Control characteristic: %d", status);
    }
    // Allocate buffer size mutex
    serial_svc->buff_size_mtx = furi_mutex_alloc(FuriMutexTypeNormal);
}

void serial_svc_set_callbacks(
    uint16_t buff_size,
    SerialServiceEventCallback callback,
    void* context) {
    furi_assert(serial_svc);
    serial_svc->callback = callback;
    serial_svc->context = context;
    serial_svc->buff_size = buff_size;
    serial_svc->bytes_ready_to_receive = buff_size;
    uint32_t buff_size_reversed = REVERSE_BYTES_U32(serial_svc->buff_size);
    aci_gatt_update_char_value(
        serial_svc->svc_handle,
        serial_svc->flow_ctrl_char_handle,
        0,
        sizeof(uint32_t),
        (uint8_t*)&buff_size_reversed);
}

void serial_svc_notify_buffer_is_empty() {
    furi_assert(serial_svc);
    furi_assert(serial_svc->buff_size_mtx);

    furi_check(furi_mutex_acquire(serial_svc->buff_size_mtx, FuriWaitForever) == FuriStatusOk);
    if(serial_svc->bytes_ready_to_receive == 0) {
        FURI_LOG_D(TAG, "Buffer is empty. Notifying client");
        serial_svc->bytes_ready_to_receive = serial_svc->buff_size;
        uint32_t buff_size_reversed = REVERSE_BYTES_U32(serial_svc->buff_size);
        aci_gatt_update_char_value(
            serial_svc->svc_handle,
            serial_svc->flow_ctrl_char_handle,
            0,
            sizeof(uint32_t),
            (uint8_t*)&buff_size_reversed);
    }
    furi_check(furi_mutex_release(serial_svc->buff_size_mtx) == FuriStatusOk);
}

void serial_svc_stop() {
    tBleStatus status;
    if(serial_svc) {
        // Delete characteristics
        status = aci_gatt_del_char(serial_svc->svc_handle, serial_svc->tx_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete TX characteristic: %d", status);
        }
        status = aci_gatt_del_char(serial_svc->svc_handle, serial_svc->rx_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete RX characteristic: %d", status);
        }
        status = aci_gatt_del_char(serial_svc->svc_handle, serial_svc->flow_ctrl_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Flow Control characteristic: %d", status);
        }
        // Delete service
        status = aci_gatt_del_service(serial_svc->svc_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Serial service: %d", status);
        }
        // Delete buffer size mutex
        furi_mutex_free(serial_svc->buff_size_mtx);
        free(serial_svc);
        serial_svc = NULL;
    }
}

bool serial_svc_is_started() {
    return serial_svc != NULL;
}

bool serial_svc_update_tx(uint8_t* data, uint16_t data_len) {
    if(data_len > SERIAL_SVC_DATA_LEN_MAX) {
        return false;
    }

    for(uint16_t remained = data_len; remained > 0;) {
        uint8_t value_len = MIN(SERIAL_SVC_CHAR_VALUE_LEN_MAX, remained);
        uint16_t value_offset = data_len - remained;
        remained -= value_len;

        tBleStatus result = aci_gatt_update_char_value_ext(
            0,
            serial_svc->svc_handle,
            serial_svc->tx_char_handle,
            remained ? 0x00 : 0x02,
            data_len,
            value_offset,
            value_len,
            data + value_offset);

        if(result) {
            FURI_LOG_E(TAG, "Failed updating TX characteristic: %d", result);
            return false;
        }
    }

    return true;
}
