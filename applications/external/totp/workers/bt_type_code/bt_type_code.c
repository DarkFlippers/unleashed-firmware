#include "bt_type_code.h"
#include <furi_hal_bt_hid.h>
#include <storage/storage.h>
#include "../../types/common.h"
#include "../../services/convert/convert.h"
#include "../constants.h"

#define HID_BT_KEYS_STORAGE_PATH EXT_PATH("authenticator/.bt_hid.keys")

static inline bool totp_type_code_worker_stop_requested() {
    return furi_thread_flags_get() & TotpBtTypeCodeWorkerEventStop;
}

static void totp_type_code_worker_type_code(TotpBtTypeCodeWorkerContext* context) {
    uint8_t i = 0;
    do {
        furi_delay_ms(500);
        i++;
    } while(!furi_hal_bt_is_active() && i < 100 && !totp_type_code_worker_stop_requested());

    if(furi_hal_bt_is_active() && furi_mutex_acquire(context->string_sync, 500) == FuriStatusOk) {
        furi_delay_ms(500);
        i = 0;
        while(i < context->string_length && context->string[i] != 0) {
            uint8_t digit = CONVERT_CHAR_TO_DIGIT(context->string[i]);
            if(digit > 9) break;
            uint8_t hid_kb_key = hid_number_keys[digit];
            furi_hal_bt_hid_kb_press(hid_kb_key);
            furi_delay_ms(30);
            furi_hal_bt_hid_kb_release(hid_kb_key);
            i++;
        }

        furi_mutex_release(context->string_sync);
    }
}

static int32_t totp_type_code_worker_callback(void* context) {
    furi_assert(context);
    FuriMutex* context_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(context_mutex == NULL) {
        return 251;
    }

    TotpBtTypeCodeWorkerContext* bt_context = context;

    furi_hal_bt_start_advertising();
    bt_context->is_advertising = true;

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

    furi_hal_bt_stop_advertising();

    bt_context->is_advertising = false;

    furi_mutex_free(context_mutex);

    return 0;
}

void totp_bt_type_code_worker_start(
    TotpBtTypeCodeWorkerContext* context,
    char* code_buf,
    uint8_t code_buf_length,
    FuriMutex* code_buf_update_sync) {
    furi_assert(context != NULL);
    context->string = code_buf;
    context->string_length = code_buf_length;
    context->string_sync = code_buf_update_sync;
    context->thread = furi_thread_alloc();
    furi_thread_set_name(context->thread, "TOTPBtHidWorker");
    furi_thread_set_stack_size(context->thread, 1024);
    furi_thread_set_context(context->thread, context);
    furi_thread_set_callback(context->thread, totp_type_code_worker_callback);
    furi_thread_start(context->thread);
}

void totp_bt_type_code_worker_stop(TotpBtTypeCodeWorkerContext* context) {
    furi_assert(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), TotpBtTypeCodeWorkerEventStop);
    furi_thread_join(context->thread);
    furi_thread_free(context->thread);
    context->thread = NULL;
}

void totp_bt_type_code_worker_notify(
    TotpBtTypeCodeWorkerContext* context,
    TotpBtTypeCodeWorkerEvent event) {
    furi_assert(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), event);
}

TotpBtTypeCodeWorkerContext* totp_bt_type_code_worker_init() {
    TotpBtTypeCodeWorkerContext* context = malloc(sizeof(TotpBtTypeCodeWorkerContext));
    furi_check(context != NULL);

    context->bt = furi_record_open(RECORD_BT);
    context->is_advertising = false;
    bt_disconnect(context->bt);
    furi_delay_ms(200);
    bt_keys_storage_set_storage_path(context->bt, HID_BT_KEYS_STORAGE_PATH);
    if(!bt_set_profile(context->bt, BtProfileHidKeyboard)) {
        FURI_LOG_E(LOGGING_TAG, "Failed to switch BT to keyboard HID profile");
    }

    return context;
}

void totp_bt_type_code_worker_free(TotpBtTypeCodeWorkerContext* context) {
    furi_assert(context != NULL);

    if(context->thread != NULL) {
        totp_bt_type_code_worker_stop(context);
    }

    bt_disconnect(context->bt);
    bt_keys_storage_set_default_path(context->bt);

    if(!bt_set_profile(context->bt, BtProfileSerial)) {
        FURI_LOG_E(LOGGING_TAG, "Failed to switch BT to Serial profile");
    }
    furi_record_close(RECORD_BT);
    context->bt = NULL;

    free(context);
}