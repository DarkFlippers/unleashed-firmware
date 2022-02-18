#include "hid_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define TAG "BtHid"

typedef struct {
    uint16_t svc_handle;
    uint16_t protocol_mode_char_handle;
    uint16_t report_char_handle;
    uint16_t report_ref_desc_handle;
    uint16_t report_map_char_handle;
    uint16_t keyboard_boot_char_handle;
    uint16_t info_char_handle;
    uint16_t ctrl_point_char_handle;
} HIDSvc;

static HIDSvc* hid_svc = NULL;

static SVCCTL_EvtAckStatus_t hid_svc_event_handler(void* event) {
    SVCCTL_EvtAckStatus_t ret = SVCCTL_EvtNotAck;
    hci_event_pckt* event_pckt = (hci_event_pckt*)(((hci_uart_pckt*)event)->data);
    evt_blecore_aci* blecore_evt = (evt_blecore_aci*)event_pckt->data;
    // aci_gatt_attribute_modified_event_rp0* attribute_modified;
    if(event_pckt->evt == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
        if(blecore_evt->ecode == ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE) {
            // Process modification events
            ret = SVCCTL_EvtAckFlowEnable;
        } else if(blecore_evt->ecode == ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE) {
            // Process notification confirmation
            ret = SVCCTL_EvtAckFlowEnable;
        }
    }
    return ret;
}

void hid_svc_start() {
    tBleStatus status;
    hid_svc = malloc(sizeof(HIDSvc));
    Service_UUID_t svc_uuid = {};
    Char_Desc_Uuid_t desc_uuid = {};
    Char_UUID_t char_uuid = {};

    // Register event handler
    SVCCTL_RegisterSvcHandler(hid_svc_event_handler);
    // Add service
    svc_uuid.Service_UUID_16 = HUMAN_INTERFACE_DEVICE_SERVICE_UUID;
    status =
        aci_gatt_add_service(UUID_TYPE_16, &svc_uuid, PRIMARY_SERVICE, 30, &hid_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add HID service: %d", status);
    }
    // Add Protocol mode characterstics
    char_uuid.Char_UUID_16 = PROTOCOL_MODE_CHAR_UUID;
    status = aci_gatt_add_char(
        hid_svc->svc_handle,
        UUID_TYPE_16,
        &char_uuid,
        1,
        CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP,
        ATTR_PERMISSION_NONE,
        GATT_NOTIFY_ATTRIBUTE_WRITE,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &hid_svc->protocol_mode_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add protocol mode characteristic: %d", status);
    }
    // Update Protocol mode characteristic
    uint8_t protocol_mode = 1;
    status = aci_gatt_update_char_value(
        hid_svc->svc_handle, hid_svc->protocol_mode_char_handle, 0, 1, &protocol_mode);
    if(status) {
        FURI_LOG_E(TAG, "Failed to update protocol mode characteristic: %d", status);
    }
    // Add Report characterstics
    char_uuid.Char_UUID_16 = REPORT_CHAR_UUID;
    status = aci_gatt_add_char(
        hid_svc->svc_handle,
        UUID_TYPE_16,
        &char_uuid,
        HID_SVC_REPORT_MAX_LEN,
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        ATTR_PERMISSION_NONE,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_VARIABLE,
        &hid_svc->report_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add report characteristic: %d", status);
    }
    // Add Report descriptor
    uint8_t desc_val[] = {0x00, 0x01};
    desc_uuid.Char_UUID_16 = REPORT_REFERENCE_DESCRIPTOR_UUID;
    status = aci_gatt_add_char_desc(
        hid_svc->svc_handle,
        hid_svc->report_char_handle,
        UUID_TYPE_16,
        &desc_uuid,
        HID_SVC_REPORT_REF_LEN,
        HID_SVC_REPORT_REF_LEN,
        desc_val,
        ATTR_PERMISSION_NONE,
        ATTR_ACCESS_READ_ONLY,
        GATT_DONT_NOTIFY_EVENTS,
        MIN_ENCRY_KEY_SIZE,
        CHAR_VALUE_LEN_CONSTANT,
        &hid_svc->report_ref_desc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add report reference descriptor: %d", status);
    }
    // Add Report Map characteristic
    char_uuid.Char_UUID_16 = REPORT_MAP_CHAR_UUID;
    status = aci_gatt_add_char(
        hid_svc->svc_handle,
        UUID_TYPE_16,
        &char_uuid,
        HID_SVC_REPORT_MAP_MAX_LEN,
        CHAR_PROP_READ,
        ATTR_PERMISSION_NONE,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_VARIABLE,
        &hid_svc->report_map_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add report map characteristic: %d", status);
    }
    // Add Boot Keyboard characteristic
    char_uuid.Char_UUID_16 = BOOT_KEYBOARD_INPUT_REPORT_CHAR_UUID;
    status = aci_gatt_add_char(
        hid_svc->svc_handle,
        UUID_TYPE_16,
        &char_uuid,
        HID_SVC_BOOT_KEYBOARD_INPUT_REPORT_MAX_LEN,
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        ATTR_PERMISSION_NONE,
        GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP,
        10,
        CHAR_VALUE_LEN_VARIABLE,
        &hid_svc->keyboard_boot_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add report map characteristic: %d", status);
    }
    // Add Information characteristic
    char_uuid.Char_UUID_16 = HID_INFORMATION_CHAR_UUID;
    status = aci_gatt_add_char(
        hid_svc->svc_handle,
        UUID_TYPE_16,
        &char_uuid,
        HID_SVC_INFO_LEN,
        CHAR_PROP_READ,
        ATTR_PERMISSION_NONE,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &hid_svc->info_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add information characteristic: %d", status);
    }
    // Add Control Point characteristic
    char_uuid.Char_UUID_16 = HID_CONTROL_POINT_CHAR_UUID;
    status = aci_gatt_add_char(
        hid_svc->svc_handle,
        UUID_TYPE_16,
        &char_uuid,
        HID_SVC_CONTROL_POINT_LEN,
        CHAR_PROP_WRITE_WITHOUT_RESP,
        ATTR_PERMISSION_NONE,
        GATT_NOTIFY_ATTRIBUTE_WRITE,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &hid_svc->ctrl_point_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add control point characteristic: %d", status);
    }
}

bool hid_svc_update_report_map(uint8_t* data, uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);

    tBleStatus status = aci_gatt_update_char_value(
        hid_svc->svc_handle, hid_svc->report_map_char_handle, 0, len, data);
    if(status) {
        FURI_LOG_E(TAG, "Failed updating report map characteristic");
        return false;
    }
    return true;
}

bool hid_svc_update_input_report(uint8_t* data, uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);

    tBleStatus status =
        aci_gatt_update_char_value(hid_svc->svc_handle, hid_svc->report_char_handle, 0, len, data);
    if(status) {
        FURI_LOG_E(TAG, "Failed updating report characteristic");
        return false;
    }
    return true;
}

bool hid_svc_update_info(uint8_t* data, uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);

    tBleStatus status =
        aci_gatt_update_char_value(hid_svc->svc_handle, hid_svc->info_char_handle, 0, len, data);
    if(status) {
        FURI_LOG_E(TAG, "Failed updating info characteristic");
        return false;
    }
    return true;
}

bool hid_svc_is_started() {
    return hid_svc != NULL;
}

void hid_svc_stop() {
    tBleStatus status;
    if(hid_svc) {
        // Delete characteristics
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->report_map_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Report Map characteristic: %d", status);
        }
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->report_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Report characteristic: %d", status);
        }
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->protocol_mode_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Protocol Mode characteristic: %d", status);
        }
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->keyboard_boot_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Keyboard Boot characteristic: %d", status);
        }
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->info_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Information characteristic: %d", status);
        }
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->ctrl_point_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Control Point characteristic: %d", status);
        }
        // Delete service
        status = aci_gatt_del_service(hid_svc->svc_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete HID service: %d", status);
        }
        // Delete buffer size mutex
        free(hid_svc);
        hid_svc = NULL;
    }
}
