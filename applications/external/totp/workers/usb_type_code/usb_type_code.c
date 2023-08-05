#include "usb_type_code.h"
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>
#include <furi/core/thread.h>
#include <furi/core/kernel.h>
#include <furi/core/check.h>
#include "../../services/convert/convert.h"
#include "../../types/token_info.h"
#include "../type_code_common.h"

struct TotpUsbTypeCodeWorkerContext {
    char* code_buffer;
    uint8_t code_buffer_size;
    uint8_t flags;
    FuriThread* thread;
    FuriMutex* code_buffer_sync;
    FuriHalUsbInterface* usb_mode_prev;
    AutomationKeyboardLayout keyboard_layout;
};

static void totp_type_code_worker_restore_usb_mode(TotpUsbTypeCodeWorkerContext* context) {
    if(context->usb_mode_prev != NULL) {
        furi_hal_usb_set_config(context->usb_mode_prev, NULL);
        context->usb_mode_prev = NULL;
    }
}

static inline bool totp_type_code_worker_stop_requested() {
    return furi_thread_flags_get() & TotpUsbTypeCodeWorkerEventStop;
}

static void totp_type_code_worker_type_code(TotpUsbTypeCodeWorkerContext* context) {
    context->usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);
    uint8_t i = 0;
    do {
        furi_delay_ms(500);
        i++;
    } while(!furi_hal_hid_is_connected() && i < 100 && !totp_type_code_worker_stop_requested());

    if(furi_hal_hid_is_connected() &&
       furi_mutex_acquire(context->code_buffer_sync, 500) == FuriStatusOk) {
        totp_type_code_worker_execute_automation(
            &furi_hal_hid_kb_press,
            &furi_hal_hid_kb_release,
            context->code_buffer,
            context->code_buffer_size,
            context->flags,
            context->keyboard_layout);
        furi_mutex_release(context->code_buffer_sync);

        furi_delay_ms(100);
    }

    totp_type_code_worker_restore_usb_mode(context);
}

static int32_t totp_type_code_worker_callback(void* context) {
    furi_check(context);
    FuriMutex* context_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    while(true) {
        uint32_t flags = furi_thread_flags_wait(
            TotpUsbTypeCodeWorkerEventStop | TotpUsbTypeCodeWorkerEventType,
            FuriFlagWaitAny,
            FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0); //-V562
        if(flags & TotpUsbTypeCodeWorkerEventStop) break;

        if(furi_mutex_acquire(context_mutex, FuriWaitForever) == FuriStatusOk) {
            if(flags & TotpUsbTypeCodeWorkerEventType) {
                totp_type_code_worker_type_code(context);
            }

            furi_mutex_release(context_mutex);
        }
    }

    furi_mutex_free(context_mutex);

    return 0;
}

TotpUsbTypeCodeWorkerContext* totp_usb_type_code_worker_start(
    char* code_buffer,
    uint8_t code_buffer_size,
    FuriMutex* code_buffer_sync,
    AutomationKeyboardLayout keyboard_layout) {
    TotpUsbTypeCodeWorkerContext* context = malloc(sizeof(TotpUsbTypeCodeWorkerContext));
    furi_check(context != NULL);
    context->code_buffer = code_buffer;
    context->code_buffer_size = code_buffer_size;
    context->code_buffer_sync = code_buffer_sync;
    context->thread = furi_thread_alloc();
    context->usb_mode_prev = NULL;
    context->keyboard_layout = keyboard_layout;
    furi_thread_set_name(context->thread, "TOTPUsbHidWorker");
    furi_thread_set_stack_size(context->thread, 1024);
    furi_thread_set_context(context->thread, context);
    furi_thread_set_callback(context->thread, totp_type_code_worker_callback);
    furi_thread_start(context->thread);
    return context;
}

void totp_usb_type_code_worker_stop(TotpUsbTypeCodeWorkerContext* context) {
    furi_check(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), TotpUsbTypeCodeWorkerEventStop);
    furi_thread_join(context->thread);
    furi_thread_free(context->thread);
    totp_type_code_worker_restore_usb_mode(context);
    free(context);
}

void totp_usb_type_code_worker_notify(
    TotpUsbTypeCodeWorkerContext* context,
    TotpUsbTypeCodeWorkerEvent event,
    uint8_t flags) {
    furi_check(context != NULL);
    context->flags = flags;
    furi_thread_flags_set(furi_thread_get_id(context->thread), event);
}