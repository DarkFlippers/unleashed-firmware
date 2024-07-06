#include "nfc_poller.h"

#include <nfc/protocols/nfc_poller_defs.h>

#include <furi.h>

typedef enum {
    NfcPollerSessionStateIdle,
    NfcPollerSessionStateActive,
    NfcPollerSessionStateStopRequest,
} NfcPollerSessionState;

typedef struct NfcPollerListElement {
    NfcProtocol protocol;
    NfcGenericInstance* poller;
    const NfcPollerBase* poller_api;
    struct NfcPollerListElement* child;
} NfcPollerListElement;

typedef struct {
    NfcPollerListElement* head;
    NfcPollerListElement* tail;
} NfcPollerList;

struct NfcPoller {
    NfcProtocol protocol;
    Nfc* nfc;
    NfcPollerList list;
    NfcPollerSessionState session_state;
    bool protocol_detected;

    NfcGenericCallbackEx callback;
    void* context;
};

static void nfc_poller_list_alloc(NfcPoller* instance) {
    instance->list.head = malloc(sizeof(NfcPollerListElement));
    instance->list.head->protocol = instance->protocol;
    instance->list.head->poller_api = nfc_pollers_api[instance->protocol];
    instance->list.head->child = NULL;
    instance->list.tail = instance->list.head;

    do {
        NfcProtocol parent_protocol = nfc_protocol_get_parent(instance->list.head->protocol);
        if(parent_protocol == NfcProtocolInvalid) break;

        NfcPollerListElement* parent = malloc(sizeof(NfcPollerListElement));
        parent->protocol = parent_protocol;
        parent->poller_api = nfc_pollers_api[parent_protocol];
        parent->child = instance->list.head;
        instance->list.head = parent;
    } while(true);

    NfcPollerListElement* iter = instance->list.head;
    iter->poller = iter->poller_api->alloc(instance->nfc);

    do {
        if(iter->child == NULL) break;
        iter->child->poller = iter->child->poller_api->alloc(iter->poller);
        iter->poller_api->set_callback(
            iter->poller, iter->child->poller_api->run, iter->child->poller);

        iter = iter->child;
    } while(true);
}

static void nfc_poller_list_free(NfcPoller* instance) {
    do {
        instance->list.head->poller_api->free(instance->list.head->poller);
        NfcPollerListElement* child = instance->list.head->child;
        free(instance->list.head);
        if(child == NULL) break;
        instance->list.head = child;
    } while(true);
}

NfcPoller* nfc_poller_alloc(Nfc* nfc, NfcProtocol protocol) {
    furi_check(nfc);
    furi_check(protocol < NfcProtocolNum);

    NfcPoller* instance = malloc(sizeof(NfcPoller));
    instance->session_state = NfcPollerSessionStateIdle;
    instance->nfc = nfc;
    instance->protocol = protocol;
    nfc_poller_list_alloc(instance);

    return instance;
}

void nfc_poller_free(NfcPoller* instance) {
    furi_check(instance);

    nfc_poller_list_free(instance);
    free(instance);
}

static NfcCommand nfc_poller_start_callback(NfcEvent event, void* context) {
    furi_assert(context);

    NfcPoller* instance = context;

    NfcCommand command = NfcCommandContinue;
    NfcGenericEvent poller_event = {
        .protocol = NfcProtocolInvalid,
        .instance = instance->nfc,
        .event_data = &event,
    };

    if(event.type == NfcEventTypePollerReady) {
        NfcPollerListElement* head_poller = instance->list.head;
        command = head_poller->poller_api->run(poller_event, head_poller->poller);
    }

    if(instance->session_state == NfcPollerSessionStateStopRequest) {
        command = NfcCommandStop;
    }

    return command;
}

void nfc_poller_start(NfcPoller* instance, NfcGenericCallback callback, void* context) {
    furi_check(instance);
    furi_check(callback);
    furi_check(instance->session_state == NfcPollerSessionStateIdle);

    NfcPollerListElement* tail_poller = instance->list.tail;
    tail_poller->poller_api->set_callback(tail_poller->poller, callback, context);

    instance->session_state = NfcPollerSessionStateActive;
    nfc_start(instance->nfc, nfc_poller_start_callback, instance);
}

static NfcCommand nfc_poller_start_ex_tail_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol != NfcProtocolInvalid);

    NfcPoller* instance = context;
    NfcCommand command = NfcCommandContinue;

    NfcGenericEventEx poller_event = {
        .poller = instance->list.tail->poller,
        .parent_event_data = event.event_data,
    };

    command = instance->callback(poller_event, instance->context);

    return command;
}

static NfcCommand nfc_poller_start_ex_head_callback(NfcEvent event, void* context) {
    furi_assert(context);

    NfcCommand command = NfcCommandContinue;
    NfcPoller* instance = context;

    NfcProtocol parent_protocol = nfc_protocol_get_parent(instance->protocol);

    if(parent_protocol == NfcProtocolInvalid) {
        NfcGenericEventEx poller_event = {
            .poller = instance->list.tail->poller,
            .parent_event_data = &event,
        };

        command = instance->callback(poller_event, instance->context);
    } else {
        NfcGenericEvent poller_event = {
            .protocol = NfcProtocolInvalid,
            .instance = instance->nfc,
            .event_data = &event,
        };
        NfcPollerListElement* head_poller = instance->list.head;
        command = head_poller->poller_api->run(poller_event, head_poller->poller);
    }

    if(instance->session_state == NfcPollerSessionStateStopRequest) {
        command = NfcCommandStop;
    }

    return command;
}

void nfc_poller_start_ex(NfcPoller* instance, NfcGenericCallbackEx callback, void* context) {
    furi_check(instance);
    furi_check(callback);
    furi_check(instance->session_state == NfcPollerSessionStateIdle);

    instance->callback = callback;
    instance->context = context;

    NfcProtocol parent_protocol = nfc_protocol_get_parent(instance->protocol);
    if(parent_protocol != NfcProtocolInvalid) {
        NfcPollerListElement* iter = instance->list.head;
        while(iter->protocol != parent_protocol)
            iter = iter->child;

        iter->poller_api->set_callback(iter->poller, nfc_poller_start_ex_tail_callback, instance);
    }

    instance->session_state = NfcPollerSessionStateActive;
    nfc_start(instance->nfc, nfc_poller_start_ex_head_callback, instance);
}

void nfc_poller_stop(NfcPoller* instance) {
    furi_check(instance);
    furi_check(instance->nfc);

    instance->session_state = NfcPollerSessionStateStopRequest;
    nfc_stop(instance->nfc);
    instance->session_state = NfcPollerSessionStateIdle;
}

static NfcCommand nfc_poller_detect_tail_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);

    NfcPoller* instance = context;
    NfcPollerListElement* tail_poller = instance->list.tail;
    instance->protocol_detected = tail_poller->poller_api->detect(event, tail_poller->poller);

    return NfcCommandStop;
}

static NfcCommand nfc_poller_detect_head_callback(NfcEvent event, void* context) {
    furi_assert(context);

    NfcPoller* instance = context;
    NfcPollerListElement* tail_poller = instance->list.tail;
    NfcPollerListElement* head_poller = instance->list.head;

    NfcCommand command = NfcCommandContinue;
    NfcGenericEvent poller_event = {
        .protocol = NfcProtocolInvalid,
        .instance = instance->nfc,
        .event_data = &event,
    };

    if(event.type == NfcEventTypePollerReady) {
        if(tail_poller == head_poller) {
            instance->protocol_detected =
                tail_poller->poller_api->detect(poller_event, tail_poller->poller);
            command = NfcCommandStop;
        } else {
            command = head_poller->poller_api->run(poller_event, head_poller->poller);
        }
    }

    return command;
}

bool nfc_poller_detect(NfcPoller* instance) {
    furi_check(instance);
    furi_check(instance->session_state == NfcPollerSessionStateIdle);

    instance->session_state = NfcPollerSessionStateActive;
    NfcPollerListElement* tail_poller = instance->list.tail;
    NfcPollerListElement* iter = instance->list.head;

    if(tail_poller != instance->list.head) {
        while(iter->child != tail_poller)
            iter = iter->child;
        iter->poller_api->set_callback(iter->poller, nfc_poller_detect_tail_callback, instance);
    }

    nfc_start(instance->nfc, nfc_poller_detect_head_callback, instance);
    nfc_stop(instance->nfc);

    if(tail_poller != instance->list.head) {
        iter->poller_api->set_callback(
            iter->poller, tail_poller->poller_api->run, tail_poller->poller);
    }

    return instance->protocol_detected;
}

NfcProtocol nfc_poller_get_protocol(const NfcPoller* instance) {
    furi_check(instance);

    return instance->protocol;
}

const NfcDeviceData* nfc_poller_get_data(const NfcPoller* instance) {
    furi_check(instance);

    NfcPollerListElement* tail_poller = instance->list.tail;
    return tail_poller->poller_api->get_data(tail_poller->poller);
}
