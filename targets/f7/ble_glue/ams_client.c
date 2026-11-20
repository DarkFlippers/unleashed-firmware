#include "ams_client_i.h"

#include "app_common.h"
#include "furi_ble/event_dispatcher.h"
#include <ble/ble.h>

#include <furi.h>
#include <furi_hal_bt.h>
#include <string.h>
#include <stdlib.h>

#define TAG "AmsClient"

#define AMS_ENTITY_PLAYER 0
#define AMS_ENTITY_QUEUE  1
#define AMS_ENTITY_TRACK  2

#define AMS_PLAYER_NAME     0
#define AMS_PLAYER_PLAYBACK 1
#define AMS_QUEUE_INDEX     0
#define AMS_QUEUE_COUNT     1
#define AMS_TRACK_ARTIST    0
#define AMS_TRACK_TITLE     2

// UUIDs in little-endian byte order, as the stack expects
static const uint8_t ams_service_uuid[16] =
    {0xDC, 0xF8, 0x55, 0xAD, 0x02, 0xC5, 0xF4, 0x8E, 0x3A, 0x43, 0x36, 0x0F, 0x2B, 0x50, 0xD3, 0x89};
static const uint8_t ams_remote_command_uuid[16] =
    {0xC2, 0x51, 0xCA, 0xF7, 0x56, 0x0E, 0xDF, 0xB8, 0x8A, 0x4A, 0xB1, 0x57, 0xD8, 0x81, 0x3C, 0x9B};
static const uint8_t ams_entity_update_uuid[16] =
    {0x02, 0xC1, 0x96, 0xBA, 0x92, 0xBB, 0x0C, 0x9A, 0x1F, 0x41, 0x8D, 0x80, 0xCE, 0xAB, 0x7C, 0x2F};

typedef struct {
    GapSvcEventHandler* handler;
    FuriMutex* mutex;
    uint16_t conn;
    uint16_t svc_start;
    uint16_t svc_end;
    uint16_t remote_command;
    uint16_t entity_update;
    uint16_t entity_update_end;
    uint16_t entity_update_cccd;
    AmsInfo info;
} AmsClient;

static AmsClient* ams = NULL;

// Test mode: same layout under UUIDs that differ in the last byte, for bench tests
// against a phone simulator (macOS refuses to publish Apple's reserved AMS UUIDs).
static bool ams_test_uuids = false;

static const uint8_t* ams_uuid(const uint8_t* uuid, uint8_t* scratch) {
    memcpy(scratch, uuid, 16);
    if(ams_test_uuids) scratch[0] ^= 0x01;
    return scratch;
}

const char* ams_client_state_name(AmsState state) {
    switch(state) {
    case AmsStateIdle:
        return "idle";
    case AmsStateDiscService:
        return "finding service";
    case AmsStateDiscChars:
        return "finding characteristics";
    case AmsStateDiscDesc:
        return "finding descriptors";
    case AmsStateSubscribe:
        return "subscribing";
    case AmsStateRequestTrack:
        return "requesting track";
    case AmsStateRequestPlayer:
        return "requesting player";
    case AmsStateRequestQueue:
        return "requesting queue";
    case AmsStateReady:
        return "ready";
    case AmsStateCommand:
        return "sending command";
    case AmsStateNotFound:
        return "AMS not found";
    case AmsStateError:
        return "error";
    case AmsStateUnsupported:
        return "needs Full BLE stack";
    }
    return "?";
}

static tBleStatus ams_begin(uint16_t connection_handle);

static void ams_fail(uint8_t error) {
    ams->info.error = error;
    ams->info.state = AmsStateError;
    FURI_LOG_W(TAG, "Failed with 0x%02X", error);
}

static void ams_check(tBleStatus ret) {
    if(ret != BLE_STATUS_SUCCESS) ams_fail(ret);
}

static void ams_copy_value(char* dst, size_t size, const uint8_t* src, size_t len) {
    if(len >= size) len = size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void ams_on_entity_update(const uint8_t* data, uint8_t len) {
    if(len < 3) return;
    uint8_t entity = data[0];
    uint8_t attribute = data[1];
    const uint8_t* value = &data[3];
    size_t value_len = len - 3;
    AmsInfo* info = &ams->info;

    if(entity == AMS_ENTITY_TRACK && attribute == AMS_TRACK_TITLE) {
        ams_copy_value(info->title, sizeof(info->title), value, value_len);
    } else if(entity == AMS_ENTITY_TRACK && attribute == AMS_TRACK_ARTIST) {
        ams_copy_value(info->artist, sizeof(info->artist), value, value_len);
    } else if(entity == AMS_ENTITY_PLAYER && attribute == AMS_PLAYER_NAME) {
        ams_copy_value(info->player, sizeof(info->player), value, value_len);
    } else if(entity == AMS_ENTITY_PLAYER && attribute == AMS_PLAYER_PLAYBACK) {
        ams_copy_value(info->playback, sizeof(info->playback), value, value_len);
    } else if(
        entity == AMS_ENTITY_QUEUE &&
        (attribute == AMS_QUEUE_INDEX || attribute == AMS_QUEUE_COUNT)) {
        char number[8];
        ams_copy_value(number, sizeof(number), value, value_len);
        uint16_t n = (uint16_t)atoi(number);
        if(attribute == AMS_QUEUE_INDEX) {
            info->queue_index = n;
        } else {
            info->queue_count = n;
        }
    }
    info->updates++;
}

static void ams_on_characteristics(const aci_att_read_by_type_resp_event_rp0* rp) {
    uint8_t pair_len = rp->Handle_Value_Pair_Length;
    if(pair_len < 7) return;
    for(uint16_t i = 0; i + pair_len <= rp->Data_Length; i += pair_len) {
        const uint8_t* p = &rp->Handle_Value_Pair_Data[i];
        uint16_t decl = p[0] | (p[1] << 8);
        uint16_t value = p[3] | (p[4] << 8);
        // A characteristic declared after Entity Update closes its descriptor range
        if(ams->entity_update && !ams->entity_update_end && decl > ams->entity_update) {
            ams->entity_update_end = decl - 1;
        }
        if(pair_len != 21) continue;
        uint8_t eu[16], rc[16];
        if(memcmp(&p[5], ams_uuid(ams_entity_update_uuid, eu), 16) == 0) {
            ams->entity_update = value;
        } else if(memcmp(&p[5], ams_uuid(ams_remote_command_uuid, rc), 16) == 0) {
            ams->remote_command = value;
        }
    }
}

static void ams_on_descriptors(const aci_att_find_info_resp_event_rp0* rp) {
    if(rp->Format != 1) return;
    for(uint16_t i = 0; i + 4 <= rp->Event_Data_Length; i += 4) {
        const uint8_t* p = &rp->Handle_UUID_Pair[i];
        uint16_t handle = p[0] | (p[1] << 8);
        uint16_t uuid = p[2] | (p[3] << 8);
        if(uuid == 0x2902) ams->entity_update_cccd = handle;
    }
}

static void ams_on_procedure_complete(uint8_t error) {
    AmsInfo* info = &ams->info;
    if(error) {
        ams_fail(error);
        return;
    }
    switch(info->state) {
    case AmsStateDiscService:
        if(!ams->svc_start) {
            info->state = AmsStateNotFound;
            break;
        }
        info->state = AmsStateDiscChars;
        ams_check(aci_gatt_disc_all_char_of_service(ams->conn, ams->svc_start, ams->svc_end));
        break;
    case AmsStateDiscChars:
        if(!ams->entity_update) {
            info->state = AmsStateNotFound;
            break;
        }
        if(!ams->entity_update_end) ams->entity_update_end = ams->svc_end;
        info->state = AmsStateDiscDesc;
        ams_check(
            aci_gatt_disc_all_char_desc(ams->conn, ams->entity_update, ams->entity_update_end));
        break;
    case AmsStateDiscDesc: {
        if(!ams->entity_update_cccd) {
            ams_fail(0xF0);
            break;
        }
        static const uint8_t enable[2] = {0x01, 0x00};
        info->state = AmsStateSubscribe;
        ams_check(aci_gatt_write_char_desc(ams->conn, ams->entity_update_cccd, 2, enable));
        break;
    }
    case AmsStateSubscribe: {
        static const uint8_t track[3] = {AMS_ENTITY_TRACK, AMS_TRACK_ARTIST, AMS_TRACK_TITLE};
        info->state = AmsStateRequestTrack;
        ams_check(aci_gatt_write_char_value(ams->conn, ams->entity_update, 3, track));
        break;
    }
    case AmsStateRequestTrack: {
        static const uint8_t player[3] = {AMS_ENTITY_PLAYER, AMS_PLAYER_NAME, AMS_PLAYER_PLAYBACK};
        info->state = AmsStateRequestPlayer;
        ams_check(aci_gatt_write_char_value(ams->conn, ams->entity_update, 3, player));
        break;
    }
    case AmsStateRequestPlayer: {
        static const uint8_t queue[3] = {AMS_ENTITY_QUEUE, AMS_QUEUE_INDEX, AMS_QUEUE_COUNT};
        info->state = AmsStateRequestQueue;
        ams_check(aci_gatt_write_char_value(ams->conn, ams->entity_update, 3, queue));
        break;
    }
    case AmsStateRequestQueue:
    case AmsStateCommand:
        info->state = AmsStateReady;
        break;
    default:
        break;
    }
}

static BleEventAckStatus ams_event_handler(void* event, void* context) {
    UNUSED(context);
    hci_event_pckt* event_pckt = (hci_event_pckt*)(((hci_uart_pckt*)event)->data);

    if(event_pckt->evt == HCI_DISCONNECTION_COMPLETE_EVT_CODE) {
        hci_disconnection_complete_event_rp0* rp = (void*)event_pckt->data;
        if(rp->Connection_Handle == ams->conn) {
            furi_mutex_acquire(ams->mutex, FuriWaitForever);
            ams->conn = 0xFFFF;
            ams->info.state = AmsStateIdle;
            furi_mutex_release(ams->mutex);
        }
        return BleEventNotAck;
    }

    if(event_pckt->evt == HCI_ENCRYPTION_CHANGE_EVT_CODE) {
        hci_encryption_change_event_rp0* rp = (void*)event_pckt->data;
        if(rp->Status == 0 && rp->Encryption_Enabled) {
            furi_mutex_acquire(ams->mutex, FuriWaitForever);
            if(ams->info.state == AmsStateIdle || ams->info.state == AmsStateNotFound ||
               ams->info.state == AmsStateError) {
                FURI_LOG_I(TAG, "Link 0x%04X encrypted, looking for AMS", rp->Connection_Handle);
                ams_test_uuids = false;
                ams_begin(rp->Connection_Handle);
            }
            furi_mutex_release(ams->mutex);
        }
        return BleEventNotAck;
    }

    if(event_pckt->evt != HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) return BleEventNotAck;
    if(ams->info.state == AmsStateIdle) return BleEventNotAck;

    evt_blecore_aci* blue_evt = (evt_blecore_aci*)event_pckt->data;
    BleEventAckStatus ack = BleEventNotAck;

    furi_mutex_acquire(ams->mutex, FuriWaitForever);
    switch(blue_evt->ecode) {
    case ACI_ATT_FIND_BY_TYPE_VALUE_RESP_VSEVT_CODE: {
        aci_att_find_by_type_value_resp_event_rp0* rp = (void*)blue_evt->data;
        if(rp->Connection_Handle != ams->conn) break;
        if(rp->Num_of_Handle_Pair) {
            ams->svc_start = rp->Attribute_Group_Handle_Pair[0].Found_Attribute_Handle;
            ams->svc_end = rp->Attribute_Group_Handle_Pair[0].Group_End_Handle;
        }
        ack = BleEventAckFlowEnable;
        break;
    }
    case ACI_ATT_READ_BY_TYPE_RESP_VSEVT_CODE: {
        aci_att_read_by_type_resp_event_rp0* rp = (void*)blue_evt->data;
        if(rp->Connection_Handle != ams->conn) break;
        ams_on_characteristics(rp);
        ack = BleEventAckFlowEnable;
        break;
    }
    case ACI_ATT_FIND_INFO_RESP_VSEVT_CODE: {
        aci_att_find_info_resp_event_rp0* rp = (void*)blue_evt->data;
        if(rp->Connection_Handle != ams->conn) break;
        ams_on_descriptors(rp);
        ack = BleEventAckFlowEnable;
        break;
    }
    case ACI_GATT_NOTIFICATION_VSEVT_CODE: {
        aci_gatt_notification_event_rp0* rp = (void*)blue_evt->data;
        if(rp->Connection_Handle != ams->conn || rp->Attribute_Handle != ams->entity_update) break;
        ams_on_entity_update(rp->Attribute_Value, rp->Attribute_Value_Length);
        ack = BleEventAckFlowEnable;
        break;
    }
    case ACI_GATT_ERROR_RESP_VSEVT_CODE: {
        aci_gatt_error_resp_event_rp0* rp = (void*)blue_evt->data;
        if(rp->Connection_Handle != ams->conn) break;
        // "Attribute not found" just ends a discovery; anything else is a real error
        if(rp->Error_Code != 0x0A) ams->info.error = rp->Error_Code;
        ack = BleEventAckFlowEnable;
        break;
    }
    case ACI_GATT_PROC_COMPLETE_VSEVT_CODE: {
        aci_gatt_proc_complete_event_rp0* rp = (void*)blue_evt->data;
        if(rp->Connection_Handle != ams->conn) break;
        ams_on_procedure_complete(rp->Error_Code);
        ack = BleEventAckFlowEnable;
        break;
    }
    default:
        break;
    }
    furi_mutex_release(ams->mutex);
    return ack;
}

void ams_client_init(void) {
    furi_check(ams == NULL);
    ams = malloc(sizeof(AmsClient));
    memset(ams, 0, sizeof(AmsClient));
    ams->conn = 0xFFFF;
    ams->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    ams->handler = ble_event_dispatcher_register_svc_handler(ams_event_handler, ams);
}

void ams_client_deinit(void) {
    if(!ams) return;
    ble_event_dispatcher_unregister_svc_handler(ams->handler);
    furi_mutex_free(ams->mutex);
    free(ams);
    ams = NULL;
}

static tBleStatus ams_begin(uint16_t connection_handle) {
    ams->conn = connection_handle;
    ams->svc_start = 0;
    ams->svc_end = 0;
    ams->remote_command = 0;
    ams->entity_update = 0;
    ams->entity_update_end = 0;
    ams->entity_update_cccd = 0;
    memset(&ams->info, 0, sizeof(AmsInfo));
    if(furi_hal_bt_get_radio_stack() != FuriHalBtStackFull) {
        ams->info.state = AmsStateUnsupported;
        return BLE_STATUS_FAILED;
    }
    ams->info.state = AmsStateDiscService;

    UUID_t uuid;
    ams_uuid(ams_service_uuid, uuid.UUID_128);
    tBleStatus ret = aci_gatt_disc_primary_service_by_uuid(ams->conn, UUID_TYPE_128, &uuid);
    if(ret != BLE_STATUS_SUCCESS) ams_fail(ret);
    return ret;
}

static bool ams_start_locked(uint16_t connection_handle, bool test_uuids) {
    if(!ams || connection_handle == 0xFFFF) return false;
    furi_mutex_acquire(ams->mutex, FuriWaitForever);
    ams_test_uuids = test_uuids;
    tBleStatus ret = ams_begin(connection_handle);
    furi_mutex_release(ams->mutex);
    return ret == BLE_STATUS_SUCCESS;
}

bool ams_client_start(uint16_t connection_handle) {
    return ams_start_locked(connection_handle, false);
}

bool ams_client_start_test(uint16_t connection_handle) {
    return ams_start_locked(connection_handle, true);
}

void ams_client_get(AmsInfo* info) {
    if(!ams) {
        memset(info, 0, sizeof(AmsInfo));
        return;
    }
    furi_mutex_acquire(ams->mutex, FuriWaitForever);
    *info = ams->info;
    furi_mutex_release(ams->mutex);
}

bool ams_client_command(AmsCommand command) {
    if(!ams) return false;
    bool sent = false;
    furi_mutex_acquire(ams->mutex, FuriWaitForever);
    if(ams->info.state == AmsStateReady && ams->remote_command) {
        uint8_t id = command;
        ams->info.state = AmsStateCommand;
        tBleStatus ret = aci_gatt_write_char_value(ams->conn, ams->remote_command, 1, &id);
        if(ret == BLE_STATUS_SUCCESS) {
            sent = true;
        } else {
            ams_fail(ret);
        }
    }
    furi_mutex_release(ams->mutex);
    return sent;
}
