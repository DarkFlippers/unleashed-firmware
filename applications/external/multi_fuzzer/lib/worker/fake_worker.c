#include "fake_worker.h"
#include "protocol_i.h"

#include <timer.h>

#include <lib/toolbox/hex.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/buffered_file_stream.h>

#define TAG "Fuzzer worker"

#if defined(RFID_125_PROTOCOL)

#include <lib/lfrfid/lfrfid_dict_file.h>
#include <lib/lfrfid/lfrfid_worker.h>
#include <lfrfid/protocols/lfrfid_protocols.h>

#else

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
    uint16_t timer_idle_time_ms;
    uint16_t timer_emu_time_ms;

    uint8_t payload[MAX_PAYLOAD_SIZE];
    Stream* uids_stream;
    uint16_t index;
    uint8_t chusen_byte;

    bool treead_running;
    bool in_emu_phase;
    FuriTimer* timer;

    FuzzerWorkerUidChagedCallback tick_callback;
    void* tick_context;

    FuzzerWorkerEndCallback end_callback;
    void* end_context;
};

static bool fuzzer_worker_load_key(FuzzerWorker* instance, bool next) {
    furi_assert(instance);
    furi_assert(instance->protocol);
    bool res = false;

    const FuzzerProtocol* protocol = instance->protocol;

    switch(instance->attack_type) {
    case FuzzerWorkerAttackTypeDefaultDict:
        if(next) {
            instance->index++;
        }
        if(instance->index < protocol->dict.len) {
            memcpy(
                instance->payload,
                &protocol->dict.val[instance->index * protocol->data_size],
                protocol->data_size);
            res = true;
        }
        break;

    case FuzzerWorkerAttackTypeLoadFileCustomUids: {
        if(next) {
            instance->index++;
        }
        uint8_t str_len = protocol->data_size * 2 + 1;
        FuriString* data_str = furi_string_alloc();
        while(true) {
            furi_string_reset(data_str);
            if(!stream_read_line(instance->uids_stream, data_str)) {
                stream_rewind(instance->uids_stream);
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
                           &instance->payload[i])) {
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

    case FuzzerWorkerAttackTypeLoadFile:
        if(instance->payload[instance->index] != 0xFF) {
            instance->payload[instance->index]++;
            res = true;
        }

        break;

    default:
        break;
    }
#if defined(RFID_125_PROTOCOL)
    protocol_dict_set_data(
        instance->protocols_items, instance->protocol_id, instance->payload, MAX_PAYLOAD_SIZE);
#else
    ibutton_key_set_protocol_id(instance->key, instance->protocol_id);
    iButtonEditableData data;
    ibutton_protocols_get_editable_data(instance->protocols_items, instance->key, &data);

    //  TODO  check data.size logic
    data.size = MAX_PAYLOAD_SIZE;
    memcpy(data.ptr, instance->payload, MAX_PAYLOAD_SIZE); // data.size);
#endif
    return res;
}

static void fuzzer_worker_on_tick_callback(void* context) {
    furi_assert(context);

    FuzzerWorker* instance = context;

    if(instance->in_emu_phase) {
        if(instance->treead_running) {
#if defined(RFID_125_PROTOCOL)
            lfrfid_worker_stop(instance->proto_worker);
#else
            ibutton_worker_stop(instance->proto_worker);
#endif
        }
        instance->in_emu_phase = false;
        furi_timer_start(instance->timer, furi_ms_to_ticks(instance->timer_idle_time_ms));
    } else {
        if(!fuzzer_worker_load_key(instance, true)) {
            fuzzer_worker_pause(instance); // XXX
            if(instance->end_callback) {
                instance->end_callback(instance->end_context);
            }
        } else {
            if(instance->treead_running) {
#if defined(RFID_125_PROTOCOL)
                lfrfid_worker_emulate_start(instance->proto_worker, instance->protocol_id);
#else
                ibutton_worker_emulate_start(instance->proto_worker, instance->key);
#endif
            }
            instance->in_emu_phase = true;
            furi_timer_start(instance->timer, furi_ms_to_ticks(instance->timer_emu_time_ms));
            if(instance->tick_callback) {
                instance->tick_callback(instance->tick_context);
            }
        }
    }
}

void fuzzer_worker_get_current_key(FuzzerWorker* instance, FuzzerPayload* output_key) {
    furi_assert(instance);
    furi_assert(output_key);
    furi_assert(instance->protocol);

    output_key->data_size = instance->protocol->data_size;
    memcpy(output_key->data, instance->payload, instance->protocol->data_size);
}

static void fuzzer_worker_set_protocol(FuzzerWorker* instance, FuzzerProtocolsID protocol_index) {
    instance->protocol = &fuzzer_proto_items[protocol_index];

#if defined(RFID_125_PROTOCOL)
    instance->protocol_id =
        protocol_dict_get_protocol_by_name(instance->protocols_items, instance->protocol->name);
#else
    // TODO iButtonProtocolIdInvalid check
    instance->protocol_id =
        ibutton_protocols_get_id_by_name(instance->protocols_items, instance->protocol->name);
#endif
}

bool fuzzer_worker_init_attack_dict(FuzzerWorker* instance, FuzzerProtocolsID protocol_index) {
    furi_assert(instance);

    bool res = false;
    fuzzer_worker_set_protocol(instance, protocol_index);

    instance->attack_type = FuzzerWorkerAttackTypeDefaultDict;
    instance->index = 0;

    if(!fuzzer_worker_load_key(instance, false)) {
        instance->attack_type = FuzzerWorkerAttackTypeMax;
    } else {
        res = true;
    }

    return res;
}

bool fuzzer_worker_init_attack_file_dict(
    FuzzerWorker* instance,
    FuzzerProtocolsID protocol_index,
    FuriString* file_path) {
    furi_assert(instance);
    furi_assert(file_path);

    bool res = false;
    fuzzer_worker_set_protocol(instance, protocol_index);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    instance->uids_stream = buffered_file_stream_alloc(storage);

    if(!buffered_file_stream_open(
           instance->uids_stream, furi_string_get_cstr(file_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        buffered_file_stream_close(instance->uids_stream);
        return res;
    }

    instance->attack_type = FuzzerWorkerAttackTypeLoadFileCustomUids;
    instance->index = 0;

    if(!fuzzer_worker_load_key(instance, false)) {
        instance->attack_type = FuzzerWorkerAttackTypeMax;
        buffered_file_stream_close(instance->uids_stream);
        furi_record_close(RECORD_STORAGE);
    } else {
        res = true;
    }

    return res;
}

bool fuzzer_worker_init_attack_bf_byte(
    FuzzerWorker* instance,
    FuzzerProtocolsID protocol_index,
    const FuzzerPayload* new_uid,
    uint8_t chusen) {
    furi_assert(instance);

    bool res = false;
    fuzzer_worker_set_protocol(instance, protocol_index);

    instance->attack_type = FuzzerWorkerAttackTypeLoadFile;
    instance->index = chusen;

    memcpy(instance->payload, new_uid->data, instance->protocol->data_size);

    res = true;

    return res;
}

// TODO make it protocol independent
bool fuzzer_worker_load_key_from_file(
    FuzzerWorker* instance,
    FuzzerProtocolsID protocol_index,
    const char* filename) {
    furi_assert(instance);

    bool res = false;
    fuzzer_worker_set_protocol(instance, protocol_index);

#if defined(RFID_125_PROTOCOL)
    ProtocolId loaded_proto_id = lfrfid_dict_file_load(instance->protocols_items, filename);
    if(loaded_proto_id == PROTOCOL_NO) {
        // Err Cant load file
        FURI_LOG_W(TAG, "Cant load file");
    } else if(instance->protocol_id != loaded_proto_id) { // Err wrong protocol
        FURI_LOG_W(TAG, "Wrong protocol");
        FURI_LOG_W(
            TAG,
            "Selected: %s Loaded: %s",
            instance->protocol->name,
            protocol_dict_get_name(instance->protocols_items, loaded_proto_id));
    } else {
        protocol_dict_get_data(
            instance->protocols_items, instance->protocol_id, instance->payload, MAX_PAYLOAD_SIZE);
        res = true;
    }
#else
    if(!ibutton_protocols_load(instance->protocols_items, instance->key, filename)) {
        // Err Cant load file
        FURI_LOG_W(TAG, "Cant load file");
    } else {
        if(instance->protocol_id != ibutton_key_get_protocol_id(instance->key)) {
            // Err wrong protocol
            FURI_LOG_W(TAG, "Wrong protocol");
            FURI_LOG_W(
                TAG,
                "Selected: %s Loaded: %s",
                instance->protocol->name,
                ibutton_protocols_get_name(
                    instance->protocols_items, ibutton_key_get_protocol_id(instance->key)));
        } else {
            iButtonEditableData data;
            ibutton_protocols_get_editable_data(instance->protocols_items, instance->key, &data);
            memcpy(instance->payload, data.ptr, data.size);
            res = true;
        }
    }
#endif

    return res;
}

FuzzerWorker* fuzzer_worker_alloc() {
    FuzzerWorker* instance = malloc(sizeof(FuzzerWorker));

#if defined(RFID_125_PROTOCOL)
    instance->protocols_items = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    instance->proto_worker = lfrfid_worker_alloc(instance->protocols_items);
#else
    instance->protocols_items = ibutton_protocols_alloc();
    instance->key =
        ibutton_key_alloc(ibutton_protocols_get_max_data_size(instance->protocols_items));

    instance->proto_worker = ibutton_worker_alloc(instance->protocols_items);
#endif
    instance->attack_type = FuzzerWorkerAttackTypeMax;
    instance->index = 0;
    instance->treead_running = false;
    instance->in_emu_phase = false;

    memset(instance->payload, 0x00, sizeof(instance->payload));

    instance->timer_idle_time_ms = PROTOCOL_DEF_IDLE_TIME * 100;
    instance->timer_emu_time_ms = PROTOCOL_DEF_EMU_TIME * 100;

    instance->timer =
        furi_timer_alloc(fuzzer_worker_on_tick_callback, FuriTimerTypeOnce, instance);

    return instance;
}

void fuzzer_worker_free(FuzzerWorker* instance) {
    furi_assert(instance);

    fuzzer_worker_stop(instance);

    furi_timer_free(instance->timer);

#if defined(RFID_125_PROTOCOL)
    lfrfid_worker_free(instance->proto_worker);

    protocol_dict_free(instance->protocols_items);
#else
    ibutton_worker_free(instance->proto_worker);

    ibutton_key_free(instance->key);
    ibutton_protocols_free(instance->protocols_items);
#endif

    free(instance);
}

bool fuzzer_worker_start(FuzzerWorker* instance, uint8_t idle_time, uint8_t emu_time) {
    furi_assert(instance);

    if(instance->attack_type < FuzzerWorkerAttackTypeMax) {
        if(idle_time == 0) {
            instance->timer_idle_time_ms = 10;
        } else {
            instance->timer_idle_time_ms = idle_time * 100;
        }
        if(emu_time == 0) {
            instance->timer_emu_time_ms = 10;
        } else {
            instance->timer_emu_time_ms = emu_time * 100;
        }

        FURI_LOG_D(
            TAG,
            "Emu_time %u ms  Idle_time %u ms",
            instance->timer_emu_time_ms,
            instance->timer_idle_time_ms);

        if(!instance->treead_running) {
#if defined(RFID_125_PROTOCOL)
            lfrfid_worker_start_thread(instance->proto_worker);
#else
            ibutton_worker_start_thread(instance->proto_worker);
#endif
            FURI_LOG_D(TAG, "Worker Starting");
            instance->treead_running = true;
        } else {
            FURI_LOG_D(TAG, "Worker UnPaused");
        }

#if defined(RFID_125_PROTOCOL)
        // lfrfid_worker_start_thread(instance->proto_worker);
        lfrfid_worker_emulate_start(instance->proto_worker, instance->protocol_id);
#else
        // ibutton_worker_start_thread(instance->proto_worker);
        ibutton_worker_emulate_start(instance->proto_worker, instance->key);
#endif
        instance->in_emu_phase = true;
        furi_timer_start(instance->timer, furi_ms_to_ticks(instance->timer_emu_time_ms));
        return true;
    }
    return false;
}

void fuzzer_worker_pause(FuzzerWorker* instance) {
    furi_assert(instance);

    furi_timer_stop(instance->timer);

    if(instance->treead_running) {
#if defined(RFID_125_PROTOCOL)
        lfrfid_worker_stop(instance->proto_worker);
#else
        ibutton_worker_stop(instance->proto_worker);
#endif
        FURI_LOG_D(TAG, "Worker Paused");
    }
}

void fuzzer_worker_stop(FuzzerWorker* instance) {
    furi_assert(instance);

    furi_timer_stop(instance->timer);

    if(instance->treead_running) {
#if defined(RFID_125_PROTOCOL)
        lfrfid_worker_stop(instance->proto_worker);
        lfrfid_worker_stop_thread(instance->proto_worker);
#else
        ibutton_worker_stop(instance->proto_worker);
        ibutton_worker_stop_thread(instance->proto_worker);
#endif
        FURI_LOG_D(TAG, "Worker Stopping");
        instance->treead_running = false;
    }

    if(instance->attack_type == FuzzerWorkerAttackTypeLoadFileCustomUids) {
        buffered_file_stream_close(instance->uids_stream);
        furi_record_close(RECORD_STORAGE);
        instance->attack_type = FuzzerWorkerAttackTypeMax;
    }

    // TODO  anything else
}

void fuzzer_worker_set_uid_chaged_callback(
    FuzzerWorker* instance,
    FuzzerWorkerUidChagedCallback callback,
    void* context) {
    furi_assert(instance);
    instance->tick_callback = callback;
    instance->tick_context = context;
}

void fuzzer_worker_set_end_callback(
    FuzzerWorker* instance,
    FuzzerWorkerEndCallback callback,
    void* context) {
    furi_assert(instance);
    instance->end_callback = callback;
    instance->end_context = context;
}
