#include "bt_type_code.h"
#include <furi_hal_bt_hid.h>
#include <furi_hal_version.h>
#include <bt/bt_service/bt_i.h>
#include <furi/core/thread.h>
#include <furi/core/mutex.h>
#include <furi/core/string.h>
#include <furi/core/kernel.h>
#include <bt/bt_service/bt.h>
#include <storage/storage.h>
#include "../../types/common.h"
#include "../../types/token_info.h"
#include "../type_code_common.h"
#include "../../features_config.h"

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
#define TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN FURI_HAL_BT_ADV_NAME_LENGTH
#define TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN GAP_MAC_ADDR_SIZE
#endif

#define HID_BT_KEYS_STORAGE_PATH EXT_PATH("authenticator/.bt_hid.keys")

struct TotpBtTypeCodeWorkerContext {
    char* code_buffer;
    uint8_t code_buffer_size;
    uint8_t flags;
    FuriThread* thread;
    FuriMutex* code_buffer_sync;
    Bt* bt;
    bool is_advertising;
    bool is_connected;
#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
    char previous_bt_name[TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN];
    uint8_t previous_bt_mac[TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN];
#endif
};

static inline bool totp_type_code_worker_stop_requested() {
    return furi_thread_flags_get() & TotpBtTypeCodeWorkerEventStop;
}

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
static void totp_type_code_worker_bt_set_app_mac(uint8_t* mac) {
    uint8_t max_i;
    size_t uid_size = furi_hal_version_uid_size();
    if(uid_size < TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN) {
        max_i = uid_size;
    } else {
        max_i = TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN;
    }

    const uint8_t* uid = furi_hal_version_uid();
    memcpy(mac, uid, max_i);
    for(uint8_t i = max_i; i < TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN; i++) {
        mac[i] = 0;
    }

    mac[0] = 0b10;
}
#endif

static void totp_type_code_worker_type_code(TotpBtTypeCodeWorkerContext* context) {
    uint8_t i = 0;
    do {
        furi_delay_ms(500);
        i++;
    } while(!context->is_connected && i < 100 && !totp_type_code_worker_stop_requested());

    if(context->is_connected &&
       furi_mutex_acquire(context->code_buffer_sync, 500) == FuriStatusOk) {
        totp_type_code_worker_execute_automation(
            &furi_hal_bt_hid_kb_press,
            &furi_hal_bt_hid_kb_release,
            context->code_buffer,
            context->code_buffer_size,
            context->flags);
        furi_mutex_release(context->code_buffer_sync);
    }
}

static int32_t totp_type_code_worker_callback(void* context) {
    furi_check(context);
    FuriMutex* context_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    TotpBtTypeCodeWorkerContext* bt_context = context;

    while(true) {
        uint32_t flags = furi_thread_flags_wait(
            TotpBtTypeCodeWorkerEventStop | TotpBtTypeCodeWorkerEventType,
            FuriFlagWaitAny,
            FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0); //-V562
        if(flags & TotpBtTypeCodeWorkerEventStop) break;

        if(furi_mutex_acquire(context_mutex, FuriWaitForever) == FuriStatusOk) {
            if(flags & TotpBtTypeCodeWorkerEventType) {
                totp_type_code_worker_type_code(bt_context);
            }

            furi_mutex_release(context_mutex);
        }
    }

    furi_mutex_free(context_mutex);

    return 0;
}

static void connection_status_changed_callback(BtStatus status, void* context) {
    TotpBtTypeCodeWorkerContext* bt_context = context;
    if(status == BtStatusConnected) {
        bt_context->is_connected = true;
    } else if(status < BtStatusConnected) {
        bt_context->is_connected = false;
    }
}

void totp_bt_type_code_worker_start(
    TotpBtTypeCodeWorkerContext* context,
    char* code_buffer,
    uint8_t code_buffer_size,
    FuriMutex* code_buffer_sync) {
    furi_check(context != NULL);
    context->code_buffer = code_buffer;
    context->code_buffer_size = code_buffer_size;
    context->code_buffer_sync = code_buffer_sync;
    context->thread = furi_thread_alloc();
    furi_thread_set_name(context->thread, "TOTPBtHidWorker");
    furi_thread_set_stack_size(context->thread, 1024);
    furi_thread_set_context(context->thread, context);
    furi_thread_set_callback(context->thread, totp_type_code_worker_callback);
    furi_thread_start(context->thread);
}

void totp_bt_type_code_worker_stop(TotpBtTypeCodeWorkerContext* context) {
    furi_check(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), TotpBtTypeCodeWorkerEventStop);
    furi_thread_join(context->thread);
    furi_thread_free(context->thread);
    context->thread = NULL;
}

void totp_bt_type_code_worker_notify(
    TotpBtTypeCodeWorkerContext* context,
    TotpBtTypeCodeWorkerEvent event,
    uint8_t flags) {
    furi_check(context != NULL);
    context->flags = flags;
    furi_thread_flags_set(furi_thread_get_id(context->thread), event);
}

TotpBtTypeCodeWorkerContext* totp_bt_type_code_worker_init() {
    TotpBtTypeCodeWorkerContext* context = malloc(sizeof(TotpBtTypeCodeWorkerContext));
    furi_check(context != NULL);

    context->bt = furi_record_open(RECORD_BT);
    context->is_advertising = false;
    context->is_connected = false;
    bt_disconnect(context->bt);
    furi_hal_bt_reinit();
    furi_delay_ms(200);
    bt_keys_storage_set_storage_path(context->bt, HID_BT_KEYS_STORAGE_PATH);

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
    memcpy(
        &context->previous_bt_name[0],
        furi_hal_bt_get_profile_adv_name(FuriHalBtProfileHidKeyboard),
        TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN);
    memcpy(
        &context->previous_bt_mac[0],
        furi_hal_bt_get_profile_mac_addr(FuriHalBtProfileHidKeyboard),
        TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN);
    char new_name[TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN];
    snprintf(new_name, sizeof(new_name), "%s TOTP Auth", furi_hal_version_get_name_ptr());
    uint8_t new_bt_mac[TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN];
    totp_type_code_worker_bt_set_app_mac(new_bt_mac);
    furi_hal_bt_set_profile_adv_name(FuriHalBtProfileHidKeyboard, new_name);
    furi_hal_bt_set_profile_mac_addr(FuriHalBtProfileHidKeyboard, new_bt_mac);
#endif

    if(!bt_set_profile(context->bt, BtProfileHidKeyboard)) {
        FURI_LOG_E(LOGGING_TAG, "Failed to switch BT to keyboard HID profile");
    }

    furi_hal_bt_start_advertising();

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
    bt_enable_peer_key_update(context->bt);
#endif

    context->is_advertising = true;
    bt_set_status_changed_callback(context->bt, connection_status_changed_callback, context);

    return context;
}

void totp_bt_type_code_worker_free(TotpBtTypeCodeWorkerContext* context) {
    furi_check(context != NULL);

    if(context->thread != NULL) {
        totp_bt_type_code_worker_stop(context);
    }

    bt_set_status_changed_callback(context->bt, NULL, NULL);

    furi_hal_bt_stop_advertising();
    context->is_advertising = false;
    context->is_connected = false;

    bt_disconnect(context->bt);
    furi_delay_ms(200);
    bt_keys_storage_set_default_path(context->bt);

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
    furi_hal_bt_set_profile_adv_name(FuriHalBtProfileHidKeyboard, context->previous_bt_name);
    furi_hal_bt_set_profile_mac_addr(FuriHalBtProfileHidKeyboard, context->previous_bt_mac);
#endif

    if(!bt_set_profile(context->bt, BtProfileSerial)) {
        FURI_LOG_E(LOGGING_TAG, "Failed to switch BT to Serial profile");
    }
    furi_record_close(RECORD_BT);
    context->bt = NULL;

    free(context);
}

bool totp_bt_type_code_worker_is_advertising(const TotpBtTypeCodeWorkerContext* context) {
    return context->is_advertising;
}