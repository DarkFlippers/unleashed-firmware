#include "hid_service.h"
#include "app_common.h"
#include <ble/ble.h>

#include <furi.h>

#define TAG "BtHid"

typedef struct {
    uint16_t svc_handle;
    uint16_t protocol_mode_char_handle;
    uint16_t report_char_handle[HID_SVC_REPORT_COUNT];
    uint16_t report_ref_desc_handle[HID_SVC_REPORT_COUNT];
    uint16_t report_map_char_handle;
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
    /**
     *  Add Human Interface Device Service
     */
    status = aci_gatt_add_service(
        UUID_TYPE_16,
        &svc_uuid,
        PRIMARY_SERVICE,
        2 + /* protocol mode */
            (4 * HID_SVC_INPUT_REPORT_COUNT) + (3 * HID_SVC_OUTPUT_REPORT_COUNT) +
            (3 * HID_SVC_FEATURE_REPORT_COUNT) + 1 + 2 + 2 +
            2, /* Service + Report Map + HID Information + HID Control Point */
        &hid_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add HID service: %d", status);
    }
    // Add Protocol mode characteristics
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

#if(HID_SVC_REPORT_COUNT != 0)
    for(uint8_t i = 0; i < HID_SVC_REPORT_COUNT; i++) {
        if(i < HID_SVC_INPUT_REPORT_COUNT) {
            uint8_t buf[2] = {i + 1, 1}; // 1 input
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
                &(hid_svc->report_char_handle[i]));
            if(status) {
                FURI_LOG_E(TAG, "Failed to add report characteristic: %d", status);
            }

            desc_uuid.Char_UUID_16 = REPORT_REFERENCE_DESCRIPTOR_UUID;
            status = aci_gatt_add_char_desc(
                hid_svc->svc_handle,
                hid_svc->report_char_handle[i],
                UUID_TYPE_16,
                &desc_uuid,
                HID_SVC_REPORT_REF_LEN,
                HID_SVC_REPORT_REF_LEN,
                buf,
                ATTR_PERMISSION_NONE,
                ATTR_ACCESS_READ_WRITE,
                GATT_DONT_NOTIFY_EVENTS,
                MIN_ENCRY_KEY_SIZE,
                CHAR_VALUE_LEN_CONSTANT,
                &(hid_svc->report_ref_desc_handle[i]));
            if(status) {
                FURI_LOG_E(TAG, "Failed to add report reference descriptor: %d", status);
            }
        } else if((i - HID_SVC_INPUT_REPORT_COUNT) < HID_SVC_OUTPUT_REPORT_COUNT) {
            uint8_t buf[2] = {i + 1, 2}; // 2 output
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
                &(hid_svc->report_char_handle[i]));
            if(status) {
                FURI_LOG_E(TAG, "Failed to add report characteristic: %d", status);
            }

            desc_uuid.Char_UUID_16 = REPORT_REFERENCE_DESCRIPTOR_UUID;
            status = aci_gatt_add_char_desc(
                hid_svc->svc_handle,
                hid_svc->report_char_handle[i],
                UUID_TYPE_16,
                &desc_uuid,
                HID_SVC_REPORT_REF_LEN,
                HID_SVC_REPORT_REF_LEN,
                buf,
                ATTR_PERMISSION_NONE,
                ATTR_ACCESS_READ_WRITE,
                GATT_DONT_NOTIFY_EVENTS,
                MIN_ENCRY_KEY_SIZE,
                CHAR_VALUE_LEN_CONSTANT,
                &(hid_svc->report_ref_desc_handle[i]));
            if(status) {
                FURI_LOG_E(TAG, "Failed to add report reference descriptor: %d", status);
            }
        } else {
            uint8_t buf[2] = {i + 1, 3}; // 3 feature
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
                &(hid_svc->report_char_handle[i]));
            if(status) {
                FURI_LOG_E(TAG, "Failed to add report characteristic: %d", status);
            }

            desc_uuid.Char_UUID_16 = REPORT_REFERENCE_DESCRIPTOR_UUID;
            status = aci_gatt_add_char_desc(
                hid_svc->svc_handle,
                hid_svc->report_char_handle[i],
                UUID_TYPE_16,
                &desc_uuid,
                HID_SVC_REPORT_REF_LEN,
                HID_SVC_REPORT_REF_LEN,
                buf,
                ATTR_PERMISSION_NONE,
                ATTR_ACCESS_READ_WRITE,
                GATT_DONT_NOTIFY_EVENTS,
                MIN_ENCRY_KEY_SIZE,
                CHAR_VALUE_LEN_CONSTANT,
                &(hid_svc->report_ref_desc_handle[i]));
            if(status) {
                FURI_LOG_E(TAG, "Failed to add report reference descriptor: %d", status);
            }
        }
    }
#endif
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

bool hid_svc_update_report_map(const uint8_t* data, uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);

    tBleStatus status = aci_gatt_update_char_value(
        hid_svc->svc_handle, hid_svc->report_map_char_handle, 0, len, data);
    if(status) {
        FURI_LOG_E(TAG, "Failed updating report map characteristic: %d", status);
        return false;
    }
    return true;
}

bool hid_svc_update_input_report(uint8_t input_report_num, uint8_t* data, uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);

    tBleStatus status = aci_gatt_update_char_value(
        hid_svc->svc_handle, hid_svc->report_char_handle[input_report_num], 0, len, data);
    if(status) {
        FURI_LOG_E(TAG, "Failed updating report characteristic: %d", status);
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
        FURI_LOG_E(TAG, "Failed updating info characteristic: %d", status);
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
#if(HID_SVC_INPUT_REPORT_COUNT != 0)
        for(uint8_t i = 0; i < HID_SVC_REPORT_COUNT; i++) {
            status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->report_char_handle[i]);
            if(status) {
                FURI_LOG_E(TAG, "Failed to delete Report characteristic: %d", status);
            }
        }
#endif
        status = aci_gatt_del_char(hid_svc->svc_handle, hid_svc->protocol_mode_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Protocol Mode characteristic: %d", status);
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
