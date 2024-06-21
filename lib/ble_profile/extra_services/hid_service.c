#include "hid_service.h"
#include "app_common.h" // IWYU pragma: keep
#include <ble/ble.h>
#include <furi_ble/event_dispatcher.h>
#include <furi_ble/gatt.h>

#include <furi.h>
#include <stdint.h>

#define TAG "BleHid"

#define BLE_SVC_HID_REPORT_MAP_MAX_LEN (255)
#define BLE_SVC_HID_REPORT_MAX_LEN (255)
#define BLE_SVC_HID_REPORT_REF_LEN (2)
#define BLE_SVC_HID_INFO_LEN (4)
#define BLE_SVC_HID_CONTROL_POINT_LEN (1)

#define BLE_SVC_HID_INPUT_REPORT_COUNT (3)
#define BLE_SVC_HID_OUTPUT_REPORT_COUNT (0)
#define BLE_SVC_HID_FEATURE_REPORT_COUNT (0)
#define BLE_SVC_HID_REPORT_COUNT                                        \
    (BLE_SVC_HID_INPUT_REPORT_COUNT + BLE_SVC_HID_OUTPUT_REPORT_COUNT + \
     BLE_SVC_HID_FEATURE_REPORT_COUNT)

typedef enum {
    HidSvcGattCharacteristicProtocolMode = 0,
    HidSvcGattCharacteristicReportMap,
    HidSvcGattCharacteristicInfo,
    HidSvcGattCharacteristicCtrlPoint,
    HidSvcGattCharacteristicCount,
} HidSvcGattCharacteristicId;

typedef struct {
    uint8_t report_idx;
    uint8_t report_type;
} HidSvcReportId;

static_assert(sizeof(HidSvcReportId) == sizeof(uint16_t), "HidSvcReportId must be 2 bytes");

static const Service_UUID_t ble_svc_hid_uuid = {
    .Service_UUID_16 = HUMAN_INTERFACE_DEVICE_SERVICE_UUID,
};

static bool ble_svc_hid_char_desc_data_callback(
    const void* context,
    const uint8_t** data,
    uint16_t* data_len) {
    const HidSvcReportId* report_id = context;
    *data_len = sizeof(HidSvcReportId);
    if(data) {
        *data = (const uint8_t*)report_id;
    }
    return false;
}

typedef struct {
    const void* data_ptr;
    uint16_t data_len;
} HidSvcDataWrapper;

static bool ble_svc_hid_report_data_callback(
    const void* context,
    const uint8_t** data,
    uint16_t* data_len) {
    const HidSvcDataWrapper* report_data = context;
    if(data) {
        *data = report_data->data_ptr;
        *data_len = report_data->data_len;
    } else {
        *data_len = BLE_SVC_HID_REPORT_MAP_MAX_LEN;
    }
    return false;
}

static const BleGattCharacteristicParams ble_svc_hid_chars[HidSvcGattCharacteristicCount] = {
    [HidSvcGattCharacteristicProtocolMode] =
        {.name = "Protocol Mode",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = 1,
         .uuid.Char_UUID_16 = PROTOCOL_MODE_CHAR_UUID,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP,
         .security_permissions = ATTR_PERMISSION_NONE,
         .gatt_evt_mask = GATT_NOTIFY_ATTRIBUTE_WRITE,
         .is_variable = CHAR_VALUE_LEN_CONSTANT},
    [HidSvcGattCharacteristicReportMap] =
        {.name = "Report Map",
         .data_prop_type = FlipperGattCharacteristicDataCallback,
         .data.callback.fn = ble_svc_hid_report_data_callback,
         .data.callback.context = NULL,
         .uuid.Char_UUID_16 = REPORT_MAP_CHAR_UUID,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_READ,
         .security_permissions = ATTR_PERMISSION_NONE,
         .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
         .is_variable = CHAR_VALUE_LEN_VARIABLE},
    [HidSvcGattCharacteristicInfo] =
        {.name = "HID Information",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = BLE_SVC_HID_INFO_LEN,
         .data.fixed.ptr = NULL,
         .uuid.Char_UUID_16 = HID_INFORMATION_CHAR_UUID,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_READ,
         .security_permissions = ATTR_PERMISSION_NONE,
         .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
         .is_variable = CHAR_VALUE_LEN_CONSTANT},
    [HidSvcGattCharacteristicCtrlPoint] =
        {.name = "HID Control Point",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = BLE_SVC_HID_CONTROL_POINT_LEN,
         .uuid.Char_UUID_16 = HID_CONTROL_POINT_CHAR_UUID,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_WRITE_WITHOUT_RESP,
         .security_permissions = ATTR_PERMISSION_NONE,
         .gatt_evt_mask = GATT_NOTIFY_ATTRIBUTE_WRITE,
         .is_variable = CHAR_VALUE_LEN_CONSTANT},
};

static const BleGattCharacteristicDescriptorParams ble_svc_hid_char_descr_template = {
    .uuid_type = UUID_TYPE_16,
    .uuid.Char_UUID_16 = REPORT_REFERENCE_DESCRIPTOR_UUID,
    .max_length = BLE_SVC_HID_REPORT_REF_LEN,
    .data_callback.fn = ble_svc_hid_char_desc_data_callback,
    .security_permissions = ATTR_PERMISSION_NONE,
    .access_permissions = ATTR_ACCESS_READ_WRITE,
    .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
    .is_variable = CHAR_VALUE_LEN_CONSTANT,
};

static const BleGattCharacteristicParams ble_svc_hid_report_template = {
    .name = "Report",
    .data_prop_type = FlipperGattCharacteristicDataCallback,
    .data.callback.fn = ble_svc_hid_report_data_callback,
    .data.callback.context = NULL,
    .uuid.Char_UUID_16 = REPORT_CHAR_UUID,
    .uuid_type = UUID_TYPE_16,
    .char_properties = CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    .security_permissions = ATTR_PERMISSION_NONE,
    .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
    .is_variable = CHAR_VALUE_LEN_VARIABLE,
};

struct BleServiceHid {
    uint16_t svc_handle;
    BleGattCharacteristicInstance chars[HidSvcGattCharacteristicCount];
    BleGattCharacteristicInstance input_report_chars[BLE_SVC_HID_INPUT_REPORT_COUNT];
    BleGattCharacteristicInstance output_report_chars[BLE_SVC_HID_OUTPUT_REPORT_COUNT];
    BleGattCharacteristicInstance feature_report_chars[BLE_SVC_HID_FEATURE_REPORT_COUNT];
    GapSvcEventHandler* event_handler;
};

static BleEventAckStatus ble_svc_hid_event_handler(void* event, void* context) {
    UNUSED(context);

    BleEventAckStatus ret = BleEventNotAck;
    hci_event_pckt* event_pckt = (hci_event_pckt*)(((hci_uart_pckt*)event)->data);
    evt_blecore_aci* blecore_evt = (evt_blecore_aci*)event_pckt->data;
    // aci_gatt_attribute_modified_event_rp0* attribute_modified;
    if(event_pckt->evt == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
        if(blecore_evt->ecode == ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE) {
            // Process modification events
            ret = BleEventAckFlowEnable;
        } else if(blecore_evt->ecode == ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE) {
            // Process notification confirmation
            ret = BleEventAckFlowEnable;
        }
    }
    return ret;
}

BleServiceHid* ble_svc_hid_start(void) {
    BleServiceHid* hid_svc = malloc(sizeof(BleServiceHid));

    // Register event handler
    hid_svc->event_handler =
        ble_event_dispatcher_register_svc_handler(ble_svc_hid_event_handler, hid_svc);
    /**
     *  Add Human Interface Device Service
     */
    if(!ble_gatt_service_add(
           UUID_TYPE_16,
           &ble_svc_hid_uuid,
           PRIMARY_SERVICE,
           2 + /* protocol mode */
               (4 * BLE_SVC_HID_INPUT_REPORT_COUNT) + (3 * BLE_SVC_HID_OUTPUT_REPORT_COUNT) +
               (3 * BLE_SVC_HID_FEATURE_REPORT_COUNT) + 1 + 2 + 2 +
               2, /* Service + Report Map + HID Information + HID Control Point */
           &hid_svc->svc_handle)) {
        free(hid_svc);
        return NULL;
    }

    // Maintain previously defined characteristic order
    ble_gatt_characteristic_init(
        hid_svc->svc_handle,
        &ble_svc_hid_chars[HidSvcGattCharacteristicProtocolMode],
        &hid_svc->chars[HidSvcGattCharacteristicProtocolMode]);

    uint8_t protocol_mode = 1;
    ble_gatt_characteristic_update(
        hid_svc->svc_handle,
        &hid_svc->chars[HidSvcGattCharacteristicProtocolMode],
        &protocol_mode);

    // reports
    BleGattCharacteristicDescriptorParams ble_svc_hid_char_descr;
    BleGattCharacteristicParams report_char;
    HidSvcReportId report_id;

    memcpy(
        &ble_svc_hid_char_descr, &ble_svc_hid_char_descr_template, sizeof(ble_svc_hid_char_descr));
    memcpy(&report_char, &ble_svc_hid_report_template, sizeof(report_char));

    ble_svc_hid_char_descr.data_callback.context = &report_id;
    report_char.descriptor_params = &ble_svc_hid_char_descr;

    typedef struct {
        uint8_t report_type;
        uint8_t report_count;
        BleGattCharacteristicInstance* chars;
    } HidSvcReportCharProps;

    HidSvcReportCharProps hid_report_chars[] = {
        {0x01, BLE_SVC_HID_INPUT_REPORT_COUNT, hid_svc->input_report_chars},
        {0x02, BLE_SVC_HID_OUTPUT_REPORT_COUNT, hid_svc->output_report_chars},
        {0x03, BLE_SVC_HID_FEATURE_REPORT_COUNT, hid_svc->feature_report_chars},
    };

    for(size_t report_type_idx = 0; report_type_idx < COUNT_OF(hid_report_chars);
        report_type_idx++) {
        report_id.report_type = hid_report_chars[report_type_idx].report_type;
        for(size_t report_idx = 0; report_idx < hid_report_chars[report_type_idx].report_count;
            report_idx++) {
            report_id.report_idx = report_idx + 1;
            ble_gatt_characteristic_init(
                hid_svc->svc_handle,
                &report_char,
                &hid_report_chars[report_type_idx].chars[report_idx]);
        }
    }

    // Setup remaining characteristics
    for(size_t i = HidSvcGattCharacteristicReportMap; i < HidSvcGattCharacteristicCount; i++) {
        ble_gatt_characteristic_init(
            hid_svc->svc_handle, &ble_svc_hid_chars[i], &hid_svc->chars[i]);
    }

    return hid_svc;
}

bool ble_svc_hid_update_report_map(BleServiceHid* hid_svc, const uint8_t* data, uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);

    HidSvcDataWrapper report_data = {
        .data_ptr = data,
        .data_len = len,
    };
    return ble_gatt_characteristic_update(
        hid_svc->svc_handle, &hid_svc->chars[HidSvcGattCharacteristicReportMap], &report_data);
}

bool ble_svc_hid_update_input_report(
    BleServiceHid* hid_svc,
    uint8_t input_report_num,
    uint8_t* data,
    uint16_t len) {
    furi_assert(data);
    furi_assert(hid_svc);
    furi_assert(input_report_num < BLE_SVC_HID_INPUT_REPORT_COUNT);

    HidSvcDataWrapper report_data = {
        .data_ptr = data,
        .data_len = len,
    };
    return ble_gatt_characteristic_update(
        hid_svc->svc_handle, &hid_svc->input_report_chars[input_report_num], &report_data);
}

bool ble_svc_hid_update_info(BleServiceHid* hid_svc, uint8_t* data) {
    furi_assert(data);
    furi_assert(hid_svc);

    return ble_gatt_characteristic_update(
        hid_svc->svc_handle, &hid_svc->chars[HidSvcGattCharacteristicInfo], &data);
}

void ble_svc_hid_stop(BleServiceHid* hid_svc) {
    furi_assert(hid_svc);
    ble_event_dispatcher_unregister_svc_handler(hid_svc->event_handler);
    // Delete characteristics
    for(size_t i = 0; i < HidSvcGattCharacteristicCount; i++) {
        ble_gatt_characteristic_delete(hid_svc->svc_handle, &hid_svc->chars[i]);
    }

    typedef struct {
        uint8_t report_count;
        BleGattCharacteristicInstance* chars;
    } HidSvcReportCharProps;

    HidSvcReportCharProps hid_report_chars[] = {
        {BLE_SVC_HID_INPUT_REPORT_COUNT, hid_svc->input_report_chars},
        {BLE_SVC_HID_OUTPUT_REPORT_COUNT, hid_svc->output_report_chars},
        {BLE_SVC_HID_FEATURE_REPORT_COUNT, hid_svc->feature_report_chars},
    };

    for(size_t report_type_idx = 0; report_type_idx < COUNT_OF(hid_report_chars);
        report_type_idx++) {
        for(size_t report_idx = 0; report_idx < hid_report_chars[report_type_idx].report_count;
            report_idx++) {
            ble_gatt_characteristic_delete(
                hid_svc->svc_handle, &hid_report_chars[report_type_idx].chars[report_idx]);
        }
    }

    // Delete service
    ble_gatt_service_delete(hid_svc->svc_handle);
    free(hid_svc);
}
