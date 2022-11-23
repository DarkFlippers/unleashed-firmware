#include "type_code.h"
#include "../../services/convert/convert.h"

static const uint8_t hid_number_keys[10] = {
    HID_KEYBOARD_0,
    HID_KEYBOARD_1,
    HID_KEYBOARD_2,
    HID_KEYBOARD_3,
    HID_KEYBOARD_4,
    HID_KEYBOARD_5,
    HID_KEYBOARD_6,
    HID_KEYBOARD_7,
    HID_KEYBOARD_8,
    HID_KEYBOARD_9};

static void totp_type_code_worker_restore_usb_mode(TotpTypeCodeWorkerContext* context) {
    if(context->usb_mode_prev != NULL) {
        furi_hal_usb_set_config(context->usb_mode_prev, NULL);
        context->usb_mode_prev = NULL;
    }
}

static inline bool totp_type_code_worker_stop_requested() {
    return furi_thread_flags_get() & TotpTypeCodeWorkerEventStop;
}

static void totp_type_code_worker_type_code(TotpTypeCodeWorkerContext* context) {
    context->usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);
    uint8_t i = 0;
    do {
        furi_delay_ms(500);
        i++;
    } while(!furi_hal_hid_is_connected() && i < 100 && !totp_type_code_worker_stop_requested());

    if(furi_hal_hid_is_connected() &&
       furi_mutex_acquire(context->string_sync, 500) == FuriStatusOk) {
        furi_delay_ms(500);
        i = 0;
        while(i < context->string_length && context->string[i] != 0) {
            uint8_t digit = CONVERT_CHAR_TO_DIGIT(context->string[i]);
            if(digit > 9) break;
            uint8_t hid_kb_key = hid_number_keys[digit];
            furi_hal_hid_kb_press(hid_kb_key);
            furi_delay_ms(30);
            furi_hal_hid_kb_release(hid_kb_key);
            i++;
        }

        furi_mutex_release(context->string_sync);

        furi_delay_ms(100);
    }

    totp_type_code_worker_restore_usb_mode(context);
}

static int32_t totp_type_code_worker_callback(void* context) {
    ValueMutex context_mutex;
    if(!init_mutex(&context_mutex, context, sizeof(TotpTypeCodeWorkerContext))) {
        return 251;
    }

    while(true) {
        uint32_t flags = furi_thread_flags_wait(
            TotpTypeCodeWorkerEventStop | TotpTypeCodeWorkerEventType,
            FuriFlagWaitAny,
            FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0); //-V562
        if(flags & TotpTypeCodeWorkerEventStop) break;

        TotpTypeCodeWorkerContext* h_context = acquire_mutex_block(&context_mutex);
        if(flags & TotpTypeCodeWorkerEventType) {
            totp_type_code_worker_type_code(h_context);
        }

        release_mutex(&context_mutex, h_context);
    }

    delete_mutex(&context_mutex);

    return 0;
}

TotpTypeCodeWorkerContext* totp_type_code_worker_start() {
    TotpTypeCodeWorkerContext* context = malloc(sizeof(TotpTypeCodeWorkerContext));
    furi_check(context != NULL);
    context->string_sync = furi_mutex_alloc(FuriMutexTypeNormal);
    context->thread = furi_thread_alloc();
    context->usb_mode_prev = NULL;
    furi_thread_set_name(context->thread, "TOTPHidWorker");
    furi_thread_set_stack_size(context->thread, 1024);
    furi_thread_set_context(context->thread, context);
    furi_thread_set_callback(context->thread, totp_type_code_worker_callback);
    furi_thread_start(context->thread);
    return context;
}

void totp_type_code_worker_stop(TotpTypeCodeWorkerContext* context) {
    furi_assert(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), TotpTypeCodeWorkerEventStop);
    furi_thread_join(context->thread);
    furi_thread_free(context->thread);
    furi_mutex_free(context->string_sync);
    totp_type_code_worker_restore_usb_mode(context);
    free(context);
}

void totp_type_code_worker_notify(
    TotpTypeCodeWorkerContext* context,
    TotpTypeCodeWorkerEvent event) {
    furi_assert(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), event);
}