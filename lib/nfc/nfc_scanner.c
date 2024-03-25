#include "nfc_scanner.h"
#include "nfc_poller.h"

#include <nfc/protocols/nfc_poller_defs.h>

#include <furi/furi.h>

#define TAG "NfcScanner"

typedef enum {
    NfcScannerStateIdle,
    NfcScannerStateTryBasePollers,
    NfcScannerStateFindChildrenProtocols,
    NfcScannerStateDetectChildrenProtocols,
    NfcScannerStateComplete,

    NfcScannerStateNum,
} NfcScannerState;

typedef enum {
    NfcScannerSessionStateIdle,
    NfcScannerSessionStateActive,
    NfcScannerSessionStateStopRequest,
} NfcScannerSessionState;

struct NfcScanner {
    Nfc* nfc;
    NfcScannerState state;
    NfcScannerSessionState session_state;

    NfcScannerCallback callback;
    void* context;

    NfcEvent nfc_event;

    NfcProtocol first_detected_protocol;

    size_t base_protocols_num;
    size_t base_protocols_idx;
    NfcProtocol base_protocols[NfcProtocolNum];

    size_t detected_base_protocols_num;
    NfcProtocol detected_base_protocols[NfcProtocolNum];

    size_t children_protocols_num;
    size_t children_protocols_idx;
    NfcProtocol children_protocols[NfcProtocolNum];

    size_t detected_protocols_num;
    NfcProtocol detected_protocols[NfcProtocolNum];

    NfcProtocol current_protocol;

    FuriThread* scan_worker;
};

static void nfc_scanner_reset(NfcScanner* instance) {
    instance->base_protocols_idx = 0;
    instance->base_protocols_num = 0;

    instance->children_protocols_idx = 0;
    instance->children_protocols_num = 0;

    instance->detected_protocols_num = 0;
    instance->detected_base_protocols_num = 0;

    instance->current_protocol = 0;
}

typedef void (*NfcScannerStateHandler)(NfcScanner* instance);

void nfc_scanner_state_handler_idle(NfcScanner* instance) {
    for(size_t i = 0; i < NfcProtocolNum; i++) {
        NfcProtocol parent_protocol = nfc_protocol_get_parent(i);
        if(parent_protocol == NfcProtocolInvalid) {
            instance->base_protocols[instance->base_protocols_num] = i;
            instance->base_protocols_num++;
        }
    }
    FURI_LOG_D(TAG, "Found %zu base protocols", instance->base_protocols_num);

    instance->first_detected_protocol = NfcProtocolInvalid;
    instance->state = NfcScannerStateTryBasePollers;
}

void nfc_scanner_state_handler_try_base_pollers(NfcScanner* instance) {
    do {
        instance->current_protocol = instance->base_protocols[instance->base_protocols_idx];

        if(instance->first_detected_protocol == instance->current_protocol) {
            instance->state = NfcScannerStateFindChildrenProtocols;
            break;
        }

        NfcPoller* poller = nfc_poller_alloc(instance->nfc, instance->current_protocol);
        bool protocol_detected = nfc_poller_detect(poller);
        nfc_poller_free(poller);

        if(protocol_detected) {
            instance->detected_protocols[instance->detected_protocols_num] =
                instance->current_protocol;
            instance->detected_protocols_num++;

            instance->detected_base_protocols[instance->detected_base_protocols_num] =
                instance->current_protocol;
            instance->detected_base_protocols_num++;

            if(instance->first_detected_protocol == NfcProtocolInvalid) {
                instance->first_detected_protocol = instance->current_protocol;
                instance->current_protocol = NfcProtocolInvalid;
            }
        }

        instance->base_protocols_idx =
            (instance->base_protocols_idx + 1) % instance->base_protocols_num;
    } while(false);
}

void nfc_scanner_state_handler_find_children_protocols(NfcScanner* instance) {
    for(size_t i = 0; i < NfcProtocolNum; i++) {
        for(size_t j = 0; j < instance->detected_base_protocols_num; j++) {
            if(nfc_protocol_has_parent(i, instance->detected_base_protocols[j])) {
                instance->children_protocols[instance->children_protocols_num] = i;
                instance->children_protocols_num++;
            }
        }
    }

    if(instance->children_protocols_num > 0) {
        instance->state = NfcScannerStateDetectChildrenProtocols;
    } else {
        instance->state = NfcScannerStateComplete;
    }
    FURI_LOG_D(TAG, "Found %zu children", instance->children_protocols_num);
}

void nfc_scanner_state_handler_detect_children_protocols(NfcScanner* instance) {
    furi_assert(instance->children_protocols_num);

    instance->current_protocol = instance->children_protocols[instance->children_protocols_idx];

    NfcPoller* poller = nfc_poller_alloc(instance->nfc, instance->current_protocol);
    bool protocol_detected = nfc_poller_detect(poller);
    nfc_poller_free(poller);

    if(protocol_detected) {
        instance->detected_protocols[instance->detected_protocols_num] =
            instance->current_protocol;
        instance->detected_protocols_num++;
    }

    instance->children_protocols_idx++;
    if(instance->children_protocols_idx == instance->children_protocols_num) {
        instance->state = NfcScannerStateComplete;
    }
}

static void nfc_scanner_filter_detected_protocols(NfcScanner* instance) {
    size_t filtered_protocols_num = 0;
    NfcProtocol filtered_protocols[NfcProtocolNum] = {};

    for(size_t i = 0; i < instance->detected_protocols_num; i++) {
        bool is_parent = false;
        for(size_t j = i; j < instance->detected_protocols_num; j++) {
            is_parent = nfc_protocol_has_parent(
                instance->detected_protocols[j], instance->detected_protocols[i]);
            if(is_parent) break;
        }
        if(!is_parent) {
            filtered_protocols[filtered_protocols_num] = instance->detected_protocols[i];
            filtered_protocols_num++;
        }
    }

    instance->detected_protocols_num = filtered_protocols_num;
    memcpy(instance->detected_protocols, filtered_protocols, filtered_protocols_num);
}

void nfc_scanner_state_handler_complete(NfcScanner* instance) {
    if(instance->detected_protocols_num > 1) {
        nfc_scanner_filter_detected_protocols(instance);
    }
    FURI_LOG_I(TAG, "Detected %zu protocols", instance->detected_protocols_num);

    NfcScannerEvent event = {
        .type = NfcScannerEventTypeDetected,
        .data =
            {
                .protocol_num = instance->detected_protocols_num,
                .protocols = instance->detected_protocols,
            },
    };

    instance->callback(event, instance->context);
    furi_delay_ms(100);
}

static NfcScannerStateHandler nfc_scanner_state_handlers[NfcScannerStateNum] = {
    [NfcScannerStateIdle] = nfc_scanner_state_handler_idle,
    [NfcScannerStateTryBasePollers] = nfc_scanner_state_handler_try_base_pollers,
    [NfcScannerStateFindChildrenProtocols] = nfc_scanner_state_handler_find_children_protocols,
    [NfcScannerStateDetectChildrenProtocols] = nfc_scanner_state_handler_detect_children_protocols,
    [NfcScannerStateComplete] = nfc_scanner_state_handler_complete,
};

static int32_t nfc_scanner_worker(void* context) {
    furi_assert(context);

    NfcScanner* instance = context;

    while(instance->session_state == NfcScannerSessionStateActive) {
        nfc_scanner_state_handlers[instance->state](instance);
    }

    nfc_scanner_reset(instance);

    return 0;
}

NfcScanner* nfc_scanner_alloc(Nfc* nfc) {
    furi_check(nfc);

    NfcScanner* instance = malloc(sizeof(NfcScanner));
    instance->nfc = nfc;

    return instance;
}

void nfc_scanner_free(NfcScanner* instance) {
    furi_check(instance);
    furi_check(instance->state == NfcScannerStateIdle);

    free(instance);
}

void nfc_scanner_start(NfcScanner* instance, NfcScannerCallback callback, void* context) {
    furi_check(instance);
    furi_check(callback);
    furi_check(instance->state == NfcScannerStateIdle);
    furi_check(instance->scan_worker == NULL);

    instance->callback = callback;
    instance->context = context;
    instance->session_state = NfcScannerSessionStateActive;

    instance->scan_worker = furi_thread_alloc();
    furi_thread_set_name(instance->scan_worker, "NfcScanWorker");
    furi_thread_set_context(instance->scan_worker, instance);
    furi_thread_set_stack_size(instance->scan_worker, 4 * 1024);
    furi_thread_set_callback(instance->scan_worker, nfc_scanner_worker);

    furi_thread_start(instance->scan_worker);
}

void nfc_scanner_stop(NfcScanner* instance) {
    furi_check(instance);
    furi_check(instance->scan_worker);

    instance->session_state = NfcScannerSessionStateStopRequest;
    furi_thread_join(instance->scan_worker);
    instance->session_state = NfcScannerSessionStateIdle;

    furi_thread_free(instance->scan_worker);
    instance->scan_worker = NULL;
    instance->callback = NULL;
    instance->context = NULL;
    instance->state = NfcScannerStateIdle;
}
