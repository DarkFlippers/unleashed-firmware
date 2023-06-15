#include "serial_service.h"
#include "app_common.h"
#include <ble/ble.h>
#include "gatt_char.h"

#include <furi.h>

#include "serial_service_uuid.inc"

#define TAG "BtSerialSvc"

typedef enum {
    SerialSvcGattCharacteristicRx = 0,
    SerialSvcGattCharacteristicTx,
    SerialSvcGattCharacteristicFlowCtrl,
    SerialSvcGattCharacteristicStatus,
    SerialSvcGattCharacteristicCount,
} SerialSvcGattCharacteristicId;

static const FlipperGattCharacteristicParams serial_svc_chars[SerialSvcGattCharacteristicCount] = {
    [SerialSvcGattCharacteristicRx] =
        {.name = "RX",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = SERIAL_SVC_DATA_LEN_MAX,
         .uuid.Char_UUID_128 = SERIAL_SVC_RX_CHAR_UUID,
         .uuid_type = UUID_TYPE_128,
         .char_properties = CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_READ,
         .security_permissions = ATTR_PERMISSION_AUTHEN_READ | ATTR_PERMISSION_AUTHEN_WRITE,
         .gatt_evt_mask = GATT_NOTIFY_ATTRIBUTE_WRITE,
         .is_variable = CHAR_VALUE_LEN_VARIABLE},
    [SerialSvcGattCharacteristicTx] =
        {.name = "TX",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = SERIAL_SVC_DATA_LEN_MAX,
         .uuid.Char_UUID_128 = SERIAL_SVC_TX_CHAR_UUID,
         .uuid_type = UUID_TYPE_128,
         .char_properties = CHAR_PROP_READ | CHAR_PROP_INDICATE,
         .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
         .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
         .is_variable = CHAR_VALUE_LEN_VARIABLE},
    [SerialSvcGattCharacteristicFlowCtrl] =
        {.name = "Flow control",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = sizeof(uint32_t),
         .uuid.Char_UUID_128 = SERIAL_SVC_FLOW_CONTROL_UUID,
         .uuid_type = UUID_TYPE_128,
         .char_properties = CHAR_PROP_READ | CHAR_PROP_NOTIFY,
         .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
         .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
         .is_variable = CHAR_VALUE_LEN_CONSTANT},
    [SerialSvcGattCharacteristicStatus] = {
        .name = "RPC status",
        .data_prop_type = FlipperGattCharacteristicDataFixed,
        .data.fixed.length = sizeof(SerialServiceRpcStatus),
        .uuid.Char_UUID_128 = SERIAL_SVC_RPC_STATUS_UUID,
        .uuid_type = UUID_TYPE_128,
        .char_properties = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY,
        .security_permissions = ATTR_PERMISSION_AUTHEN_READ | ATTR_PERMISSION_AUTHEN_WRITE,
        .gatt_evt_mask = GATT_NOTIFY_ATTRIBUTE_WRITE,
        .is_variable = CHAR_VALUE_LEN_CONSTANT}};

typedef struct {
    uint16_t svc_handle;
    FlipperGattCharacteristicInstance chars[SerialSvcGattCharacteristicCount];
    FuriMutex* buff_size_mtx;
    uint32_t buff_size;
    uint16_t bytes_ready_to_receive;
    SerialServiceEventCallback callback;
    void* context;
} SerialSvc;

static SerialSvc* serial_svc = NULL;

static SVCCTL_EvtAckStatus_t serial_svc_event_handler(void* event) {
    SVCCTL_EvtAckStatus_t ret = SVCCTL_EvtNotAck;
    hci_event_pckt* event_pckt = (hci_event_pckt*)(((hci_uart_pckt*)event)->data);
    evt_blecore_aci* blecore_evt = (evt_blecore_aci*)event_pckt->data;
    aci_gatt_attribute_modified_event_rp0* attribute_modified;
    if(event_pckt->evt == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
        if(blecore_evt->ecode == ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE) {
            attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blecore_evt->data;
            if(attribute_modified->Attr_Handle ==
               serial_svc->chars[SerialSvcGattCharacteristicRx].handle + 2) {
                // Descriptor handle
                ret = SVCCTL_EvtAckFlowEnable;
                FURI_LOG_D(TAG, "RX descriptor event");
            } else if(
                attribute_modified->Attr_Handle ==
                serial_svc->chars[SerialSvcGattCharacteristicRx].handle + 1) {
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
                    FURI_LOG_D(TAG, "Available buff size: %ld", buff_free_size);
                    furi_check(furi_mutex_release(serial_svc->buff_size_mtx) == FuriStatusOk);
                }
                ret = SVCCTL_EvtAckFlowEnable;
            } else if(
                attribute_modified->Attr_Handle ==
                serial_svc->chars[SerialSvcGattCharacteristicStatus].handle + 1) {
                SerialServiceRpcStatus* rpc_status =
                    (SerialServiceRpcStatus*)attribute_modified->Attr_Data;
                if(*rpc_status == SerialServiceRpcStatusNotActive) {
                    if(serial_svc->callback) {
                        SerialServiceEvent event = {
                            .event = SerialServiceEventTypesBleResetRequest,
                        };
                        serial_svc->callback(event, serial_svc->context);
                    }
                }
            }
        } else if(blecore_evt->ecode == ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE) {
            FURI_LOG_T(TAG, "Ack received");
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

static void serial_svc_update_rpc_char(SerialServiceRpcStatus status) {
    flipper_gatt_characteristic_update(
        serial_svc->svc_handle, &serial_svc->chars[SerialSvcGattCharacteristicStatus], &status);
}

void serial_svc_start() {
    UNUSED(serial_svc_chars);
    tBleStatus status;
    serial_svc = malloc(sizeof(SerialSvc));
    // Register event handler
    SVCCTL_RegisterSvcHandler(serial_svc_event_handler);

    // Add service
    status = aci_gatt_add_service(
        UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 12, &serial_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Serial service: %d", status);
    }

    // Add characteristics
    for(uint8_t i = 0; i < SerialSvcGattCharacteristicCount; i++) {
        flipper_gatt_characteristic_init(
            serial_svc->svc_handle, &serial_svc_chars[i], &serial_svc->chars[i]);
    }

    serial_svc_update_rpc_char(SerialServiceRpcStatusNotActive);
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
    flipper_gatt_characteristic_update(
        serial_svc->svc_handle,
        &serial_svc->chars[SerialSvcGattCharacteristicFlowCtrl],
        &buff_size_reversed);
}

void serial_svc_notify_buffer_is_empty() {
    furi_assert(serial_svc);
    furi_assert(serial_svc->buff_size_mtx);

    furi_check(furi_mutex_acquire(serial_svc->buff_size_mtx, FuriWaitForever) == FuriStatusOk);
    if(serial_svc->bytes_ready_to_receive == 0) {
        FURI_LOG_D(TAG, "Buffer is empty. Notifying client");
        serial_svc->bytes_ready_to_receive = serial_svc->buff_size;

        uint32_t buff_size_reversed = REVERSE_BYTES_U32(serial_svc->buff_size);
        flipper_gatt_characteristic_update(
            serial_svc->svc_handle,
            &serial_svc->chars[SerialSvcGattCharacteristicFlowCtrl],
            &buff_size_reversed);
    }
    furi_check(furi_mutex_release(serial_svc->buff_size_mtx) == FuriStatusOk);
}

void serial_svc_stop() {
    tBleStatus status;
    if(serial_svc) {
        for(uint8_t i = 0; i < SerialSvcGattCharacteristicCount; i++) {
            flipper_gatt_characteristic_delete(serial_svc->svc_handle, &serial_svc->chars[i]);
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
            serial_svc->chars[SerialSvcGattCharacteristicTx].handle,
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

void serial_svc_set_rpc_status(SerialServiceRpcStatus status) {
    furi_assert(serial_svc);
    serial_svc_update_rpc_char(status);
}
