#include "fake_worker.h"

#include <timer.h>

#include <lib/toolbox/hex.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/buffered_file_stream.h>

#define TAG "Fuzzer worker"
#define FUZZ_TIME_DELAY_DEFAULT (10)

#if defined(RFID_125_PROTOCOL)

#define MAX_PAYLOAD_SIZE 6
#include <lib/lfrfid/lfrfid_worker.h>
#include <lfrfid/protocols/lfrfid_protocols.h>

#else

#define MAX_PAYLOAD_SIZE 8
#include <lib/ibutton/ibutton_worker.h>
#include <lib/ibutton/ibutton_key.h>

#endif

#include <toolbox/stream/stream.h>

struct FuzzerWorker {
#if defined(RFID_125_PROTOCOL)
    LFRFIDWorker* proto_worker;
    ProtocolId protocol_id;
    ProtocolDict* protocols_items;
#else
    iButtonWorker* proto_worker;
    iButtonProtocolId protocol_id; // TODO
    iButtonProtocols* protocols_items;
    iButtonKey* key;
#endif

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

    case FuzzerWorkerAttackTypeLoadFileCustomUids: {
        uint8_t str_len = protocol->data_size * 2 + 1;
        FuriString* data_str = furi_string_alloc();
        while(true) {
            furi_string_reset(data_str);
            if(!stream_read_line(worker->uids_stream, data_str)) {
                stream_rewind(worker->uids_stream);
                // TODO Check empty file & close stream and storage
                break;
            } else if(furi_string_get_char(data_str, 0) == '#') {
                // Skip comment string
                continue;
            } else if(furi_string_size(data_str) != str_len) {
                // Ignore strin with bad length
                FURI_LOG_W(TAG, "Bad string length");
                continue;
            } else {
                FURI_LOG_D(TAG, "Uid candidate: \"%s\"", furi_string_get_cstr(data_str));
                bool parse_ok = true;
                for(uint8_t i = 0; i < protocol->data_size; i++) {
                    if(!hex_char_to_uint8(
                           furi_string_get_cstr(data_str)[i * 2],
                           furi_string_get_cstr(data_str)[i * 2 + 1],
                           &worker->payload[i])) {
                        parse_ok = false;
                        break;
                    }
                }
                res = parse_ok;
            }
            break;
        }
    }

    break;

    default:
        break;
    }
#if defined(RFID_125_PROTOCOL)
    protocol_dict_set_data(
        worker->protocols_items, worker->protocol_id, worker->payload, MAX_PAYLOAD_SIZE);
#else
    ibutton_key_set_protocol_id(worker->key, worker->protocol_id);
    iButtonEditableData data;
    ibutton_protocols_get_editable_data(worker->protocols_items, worker->key, &data);

    //  TODO  check data.size logic
    data.size = MAX_PAYLOAD_SIZE;
    memcpy(data.ptr, worker->payload, MAX_PAYLOAD_SIZE); // data.size);
#endif
    return res;
}

static void fuzzer_worker_on_tick_callback(void* context) {
    furi_assert(context);

    FuzzerWorker* worker = context;

    if(worker->treead_running) {
#if defined(RFID_125_PROTOCOL)
        lfrfid_worker_stop(worker->proto_worker);
#else
        ibutton_worker_stop(worker->proto_worker);
#endif
    }

    if(!fuzzer_worker_load_key(worker, true)) {
        fuzzer_worker_stop(worker);
        if(worker->end_callback) {
            worker->end_callback(worker->end_context);
        }
    } else {
        if(worker->treead_running) {
#if defined(RFID_125_PROTOCOL)
            lfrfid_worker_emulate_start(worker->proto_worker, worker->protocol_id);
#else
            ibutton_worker_emulate_start(worker->proto_worker, worker->key);
#endif
        }
        if(worker->tick_callback) {
            worker->tick_callback(worker->tick_context);
        }
    }
}

void fuzzer_worker_get_current_key(FuzzerWorker* worker, uint8_t* key) {
    furi_assert(worker);
    furi_assert(worker->protocol);

    memcpy(key, worker->payload, worker->protocol->data_size);
}

bool fuzzer_worker_attack_dict(FuzzerWorker* worker, FuzzerProtos protocol_index) {
    furi_assert(worker);

    bool res = false;

    worker->protocol = &fuzzer_proto_items[protocol_index];

#if defined(RFID_125_PROTOCOL)
    worker->protocol_id =
        protocol_dict_get_protocol_by_name(worker->protocols_items, worker->protocol->name);
#else
    // TODO iButtonProtocolIdInvalid check
    worker->protocol_id =
        ibutton_protocols_get_id_by_name(worker->protocols_items, worker->protocol->name);
#endif
    worker->attack_type = FuzzerWorkerAttackTypeDefaultDict;
    worker->index = 0;

    if(!fuzzer_worker_load_key(worker, false)) {
        worker->attack_type = FuzzerWorkerAttackTypeMax;
    } else {
        res = true;
    }

    return res;
}

bool fuzzer_worker_attack_file_dict(
    FuzzerWorker* worker,
    FuzzerProtos protocol_index,
    FuriString* file_path) {
    furi_assert(worker);
    furi_assert(file_path);

    bool res = false;

    worker->protocol = &fuzzer_proto_items[protocol_index];

#if defined(RFID_125_PROTOCOL)
    worker->protocol_id =
        protocol_dict_get_protocol_by_name(worker->protocols_items, worker->protocol->name);
#else
    // TODO iButtonProtocolIdInvalid check
    worker->protocol_id =
        ibutton_protocols_get_id_by_name(worker->protocols_items, worker->protocol->name);
#endif

    Storage* storage = furi_record_open(RECORD_STORAGE);
    worker->uids_stream = buffered_file_stream_alloc(storage);

    if(!buffered_file_stream_open(
           worker->uids_stream, furi_string_get_cstr(file_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        buffered_file_stream_close(worker->uids_stream);
        return res;
    }

    worker->attack_type = FuzzerWorkerAttackTypeLoadFileCustomUids;
    worker->index = 0;

    if(!fuzzer_worker_load_key(worker, false)) {
        worker->attack_type = FuzzerWorkerAttackTypeMax;
        buffered_file_stream_close(worker->uids_stream);
        furi_record_close(RECORD_STORAGE);
    } else {
        res = true;
    }

    return res;
}

FuzzerWorker* fuzzer_worker_alloc() {
    FuzzerWorker* worker = malloc(sizeof(FuzzerWorker));

#if defined(RFID_125_PROTOCOL)
    worker->protocols_items = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    worker->proto_worker = lfrfid_worker_alloc(worker->protocols_items);
#else
    worker->protocols_items = ibutton_protocols_alloc();
    worker->key = ibutton_key_alloc(ibutton_protocols_get_max_data_size(worker->protocols_items));

    worker->proto_worker = ibutton_worker_alloc(worker->protocols_items);
#endif
    worker->attack_type = FuzzerWorkerAttackTypeMax;
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

#if defined(RFID_125_PROTOCOL)
    lfrfid_worker_free(worker->proto_worker);

    protocol_dict_free(worker->protocols_items);
#else
    ibutton_worker_free(worker->proto_worker);

    ibutton_key_free(worker->key);
    ibutton_protocols_free(worker->protocols_items);
#endif

    free(worker);
}

bool fuzzer_worker_start(FuzzerWorker* worker, uint8_t timer_dellay) {
    furi_assert(worker);

    if(worker->attack_type < FuzzerWorkerAttackTypeMax) {
        worker->timeer_delay = timer_dellay;

        furi_timer_start(worker->timer, furi_ms_to_ticks(timer_dellay * 100));

        worker->treead_running = true;
#if defined(RFID_125_PROTOCOL)
        lfrfid_worker_start_thread(worker->proto_worker);
        lfrfid_worker_emulate_start(worker->proto_worker, worker->protocol_id);
#else
        ibutton_worker_start_thread(worker->proto_worker);
        ibutton_worker_emulate_start(worker->proto_worker, worker->key);
#endif
        return true;
    }
    return false;
}

void fuzzer_worker_stop(FuzzerWorker* worker) {
    furi_assert(worker);

    furi_timer_stop(worker->timer);

    if(worker->treead_running) {
#if defined(RFID_125_PROTOCOL)
        lfrfid_worker_stop(worker->proto_worker);
        lfrfid_worker_stop_thread(worker->proto_worker);
#else
        ibutton_worker_stop(worker->proto_worker);
        ibutton_worker_stop_thread(worker->proto_worker);
#endif
        worker->treead_running = false;
    }

    if(worker->attack_type == FuzzerWorkerAttackTypeLoadFileCustomUids) {
        buffered_file_stream_close(worker->uids_stream);
        furi_record_close(RECORD_STORAGE);
        worker->attack_type = FuzzerWorkerAttackTypeMax;
    }

    // TODO  anything else
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
