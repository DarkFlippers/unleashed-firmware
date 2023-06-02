#include "fake_worker.h"

#include <furi.h>
#include <timer.h>

#include <lib/ibutton/ibutton_worker.h>
#include <lib/ibutton/ibutton_key.h>

#include <toolbox/stream/stream.h>

struct FuzzerWorker {
    iButtonWorker* proto_worker;
    iButtonProtocolId protocol_id;
    iButtonProtocols* protocols_items;
    iButtonKey* key;

    const FuzzerProtocol* protocol;
    FuzzerWorkerAttackType attack_type;
    uint8_t timeer_delay;

    uint8_t payload[MAX_PAYLOAD_SIZE];
    Stream* uids_stream;
    uint16_t index;

    bool treead_running;
    FuriTimer* timer;

    FuzzerWorkerUidChagedCallback tick_callback;
    void* tick_context;

    FuzzerWorkerEndCallback end_callback;
    void* end_context;
};

static bool fuzzer_worker_load_key(FuzzerWorker* worker, bool next) {
    furi_assert(worker);
    furi_assert(worker->protocol);
    bool res = false;

    const FuzzerProtocol* protocol = worker->protocol;

    if(next) {
        worker->index++;
    }

    switch(worker->attack_type) {
    case FuzzerWorkerAttackTypeDefaultDict:
        if(worker->index < protocol->dict.len) {
            memcpy(
                worker->payload,
                &protocol->dict.val[worker->index * protocol->data_size],
                protocol->data_size);
            res = true;
        }
        break;

    default:
        break;
    }

    return res;
}

static void fuzzer_worker_on_tick_callback(void* context) {
    furi_assert(context);

    FuzzerWorker* worker = context;

    if(!fuzzer_worker_load_key(worker, true)) {
        fuzzer_worker_stop(worker);
        if(worker->end_callback) {
            worker->end_callback(worker->end_context);
        }
    } else {
        if(worker->tick_callback) {
            worker->tick_callback(worker->tick_context);
        }
    }

    // TODO load ibutton key
}

void fuzzer_worker_get_current_key(FuzzerWorker* worker, uint8_t* key) {
    furi_assert(worker);
    furi_assert(worker->protocol);

    memcpy(key, worker->payload, worker->protocol->data_size);
}

bool fuzzer_worker_attack_dict(FuzzerWorker* worker, FuzzerProtos protocol_index) {
    furi_assert(worker);

    worker->attack_type = FuzzerWorkerAttackTypeDefaultDict;
    worker->protocol = &fuzzer_proto_items[protocol_index];
    worker->index = 0;

    return fuzzer_worker_load_key(worker, false);
}

FuzzerWorker* fuzzer_worker_alloc() {
    FuzzerWorker* worker = malloc(sizeof(FuzzerWorker));

    worker->protocols_items = ibutton_protocols_alloc();
    worker->key = ibutton_key_alloc(ibutton_protocols_get_max_data_size(worker->protocols_items));

    worker->proto_worker = ibutton_worker_alloc(worker->protocols_items);

    worker->index = 0;
    worker->treead_running = false;

    memset(worker->payload, 0x00, sizeof(worker->payload));

    worker->timeer_delay = FUZZ_TIME_DELAY_DEFAULT;

    worker->timer =
        furi_timer_alloc(fuzzer_worker_on_tick_callback, FuriTimerTypePeriodic, worker);

    return worker;
}

void fuzzer_worker_free(FuzzerWorker* worker) {
    furi_assert(worker);

    fuzzer_worker_stop(worker);

    furi_timer_free(worker->timer);

    ibutton_worker_free(worker->proto_worker);

    ibutton_key_free(worker->key);
    ibutton_protocols_free(worker->protocols_items);
    // TODO delete
    UNUSED(fuzzer_worker_on_tick_callback);
    free(worker);
}

void fuzzer_worker_start(FuzzerWorker* worker, uint8_t timer_dellay) {
    furi_assert(worker);

    worker->timeer_delay = timer_dellay;

    furi_timer_start(worker->timer, furi_ms_to_ticks(timer_dellay * 100));

    // TODO start timer
    // worker->treead_running = true;
    // ibutton_worker_start_thread(worker->proto_worker);

    // TODO load ibutton key

    // ibutton_worker_emulate_start(worker->proto_worker, worker->key);
}

void fuzzer_worker_stop(FuzzerWorker* worker) {
    furi_assert(worker);

    furi_timer_stop(worker->timer);

    if(worker->treead_running) {
        ibutton_worker_stop(worker->proto_worker);
        ibutton_worker_stop_thread(worker->proto_worker);
        worker->treead_running = false;
    }

    // TODO stop timer, anything else
}

void fuzzer_worker_set_uid_chaged_callback(
    FuzzerWorker* worker,
    FuzzerWorkerUidChagedCallback callback,
    void* context) {
    furi_assert(worker);
    worker->tick_callback = callback;
    worker->tick_context = context;
}

void fuzzer_worker_set_end_callback(
    FuzzerWorker* worker,
    FuzzerWorkerEndCallback callback,
    void* context) {
    furi_assert(worker);
    worker->end_callback = callback;
    worker->end_context = context;
}
