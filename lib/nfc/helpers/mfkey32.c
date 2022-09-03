#include "mfkey32.h"

#include <furi/furi.h>
#include <storage/storage.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include <m-array.h>

#include <lib/nfc/protocols/mifare_classic.h>
#include <lib/nfc/protocols/nfc_util.h>

#define TAG "Mfkey32"

#define MFKEY32_LOGS_PATH EXT_PATH("nfc/.mfkey32.log")

typedef enum {
    Mfkey32StateIdle,
    Mfkey32StateAuthReceived,
    Mfkey32StateAuthNtSent,
    Mfkey32StateAuthArNrReceived,
} Mfkey32State;

typedef struct {
    uint32_t cuid;
    uint8_t sector;
    MfClassicKey key;
    uint32_t nt0;
    uint32_t nr0;
    uint32_t ar0;
    uint32_t nt1;
    uint32_t nr1;
    uint32_t ar1;
} Mfkey32Params;

ARRAY_DEF(Mfkey32Params, Mfkey32Params, M_POD_OPLIST);

typedef struct {
    uint8_t sector;
    MfClassicKey key;
    uint32_t nt;
    uint32_t nr;
    uint32_t ar;
} Mfkey32Nonce;

struct Mfkey32 {
    Mfkey32State state;
    Stream* file_stream;
    Mfkey32Params_t params_arr;
    Mfkey32Nonce nonce;
    uint32_t cuid;
    Mfkey32ParseDataCallback callback;
    void* context;
};

Mfkey32* mfkey32_alloc(uint32_t cuid) {
    Mfkey32* instance = malloc(sizeof(Mfkey32));
    instance->cuid = cuid;
    instance->state = Mfkey32StateIdle;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    instance->file_stream = buffered_file_stream_alloc(storage);
    if(!buffered_file_stream_open(
           instance->file_stream, MFKEY32_LOGS_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        buffered_file_stream_close(instance->file_stream);
        stream_free(instance->file_stream);
        free(instance);
        instance = NULL;
    } else {
        Mfkey32Params_init(instance->params_arr);
    }

    furi_record_close(RECORD_STORAGE);

    return instance;
}

void mfkey32_free(Mfkey32* instance) {
    furi_assert(instance != NULL);

    Mfkey32Params_clear(instance->params_arr);
    buffered_file_stream_close(instance->file_stream);
    stream_free(instance->file_stream);
    free(instance);
}

void mfkey32_set_callback(Mfkey32* instance, Mfkey32ParseDataCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static bool mfkey32_write_params(Mfkey32* instance, Mfkey32Params* params) {
    string_t str;
    string_init_printf(
        str,
        "Sector %d key %c cuid %08x nt0 %08x nr0 %08x ar0 %08x nt1 %08x nr1 %08x ar1 %08x\n",
        params->sector,
        params->key == MfClassicKeyA ? 'A' : 'B',
        params->cuid,
        params->nt0,
        params->nr0,
        params->ar0,
        params->nt1,
        params->nr1,
        params->ar1);
    bool write_success = stream_write_string(instance->file_stream, str);
    string_clear(str);
    return write_success;
}

static void mfkey32_add_params(Mfkey32* instance) {
    Mfkey32Nonce* nonce = &instance->nonce;
    bool nonce_added = false;
    // Search if we partially collected params
    if(Mfkey32Params_size(instance->params_arr)) {
        Mfkey32Params_it_t it;
        for(Mfkey32Params_it(it, instance->params_arr); !Mfkey32Params_end_p(it);
            Mfkey32Params_next(it)) {
            Mfkey32Params* params = Mfkey32Params_ref(it);
            if((params->sector == nonce->sector) && (params->key == nonce->key)) {
                params->nt1 = nonce->nt;
                params->nr1 = nonce->nr;
                params->ar1 = nonce->ar;
                nonce_added = true;
                FURI_LOG_I(
                    TAG,
                    "Params for sector %d key %c collected",
                    params->sector,
                    params->key == MfClassicKeyA ? 'A' : 'B');
                // Write on sd card
                if(mfkey32_write_params(instance, params)) {
                    Mfkey32Params_remove(instance->params_arr, it);
                    if(instance->callback) {
                        instance->callback(Mfkey32EventParamCollected, instance->context);
                    }
                }
            }
        }
    }
    if(!nonce_added) {
        Mfkey32Params params = {
            .sector = nonce->sector,
            .key = nonce->key,
            .cuid = instance->cuid,
            .nt0 = nonce->nt,
            .nr0 = nonce->nr,
            .ar0 = nonce->ar,
        };
        Mfkey32Params_push_back(instance->params_arr, params);
    }
}

void mfkey32_process_data(
    Mfkey32* instance,
    uint8_t* data,
    uint16_t len,
    bool reader_to_tag,
    bool crc_dropped) {
    furi_assert(instance);
    furi_assert(data);

    Mfkey32Nonce* nonce = &instance->nonce;
    uint16_t data_len = len;
    if((data_len > 3) && !crc_dropped) {
        data_len -= 2;
    }

    bool data_processed = false;
    if(instance->state == Mfkey32StateIdle) {
        if(reader_to_tag) {
            if((data[0] == 0x60) || (data[0] == 0x61)) {
                nonce->key = data[0] == 0x60 ? MfClassicKeyA : MfClassicKeyB;
                nonce->sector = mf_classic_get_sector_by_block(data[1]);
                instance->state = Mfkey32StateAuthReceived;
                data_processed = true;
            }
        }
    } else if(instance->state == Mfkey32StateAuthReceived) {
        if(!reader_to_tag) {
            if(len == 4) {
                nonce->nt = nfc_util_bytes2num(data, 4);
                instance->state = Mfkey32StateAuthNtSent;
                data_processed = true;
            }
        }
    } else if(instance->state == Mfkey32StateAuthNtSent) {
        if(reader_to_tag) {
            if(len == 8) {
                nonce->nr = nfc_util_bytes2num(data, 4);
                nonce->ar = nfc_util_bytes2num(&data[4], 4);
                mfkey32_add_params(instance);
                instance->state = Mfkey32StateIdle;
            }
        }
    }
    if(!data_processed) {
        instance->state = Mfkey32StateIdle;
    }
}

uint16_t mfkey32_get_auth_sectors(string_t data_str) {
    furi_assert(data_str);

    uint16_t nonces_num = 0;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* file_stream = buffered_file_stream_alloc(storage);
    string_t temp_str;
    string_init(temp_str);

    do {
        if(!buffered_file_stream_open(
               file_stream, MFKEY32_LOGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING))
            break;
        while(true) {
            if(!stream_read_line(file_stream, temp_str)) break;
            size_t uid_pos = string_search_str(temp_str, "cuid");
            string_left(temp_str, uid_pos);
            string_push_back(temp_str, '\n');
            string_cat(data_str, temp_str);
            nonces_num++;
        }
    } while(false);

    buffered_file_stream_close(file_stream);
    stream_free(file_stream);
    string_clear(temp_str);

    return nonces_num;
}
