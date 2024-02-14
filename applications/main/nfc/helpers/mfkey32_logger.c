#include "mfkey32_logger.h"

#include <m-array.h>

#include <bit_lib/bit_lib.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>

#define MFKEY32_LOGGER_MAX_NONCES_SAVED (100)

typedef struct {
    bool is_filled;
    uint32_t cuid;
    uint8_t sector_num;
    MfClassicKeyType key_type;
    uint32_t nt0;
    uint32_t nr0;
    uint32_t ar0;
    uint32_t nt1;
    uint32_t nr1;
    uint32_t ar1;
} Mfkey32LoggerParams;

ARRAY_DEF(Mfkey32LoggerParams, Mfkey32LoggerParams, M_POD_OPLIST);

struct Mfkey32Logger {
    uint32_t cuid;
    Mfkey32LoggerParams_t params_arr;
    size_t nonces_saves;
    size_t params_collected;
};

Mfkey32Logger* mfkey32_logger_alloc(uint32_t cuid) {
    Mfkey32Logger* instance = malloc(sizeof(Mfkey32Logger));
    instance->cuid = cuid;
    Mfkey32LoggerParams_init(instance->params_arr);

    return instance;
}

void mfkey32_logger_free(Mfkey32Logger* instance) {
    furi_assert(instance);
    furi_assert(instance->params_arr);

    Mfkey32LoggerParams_clear(instance->params_arr);
    free(instance);
}

static bool mfkey32_logger_add_nonce_to_existing_params(
    Mfkey32Logger* instance,
    MfClassicAuthContext* auth_context) {
    bool nonce_added = false;
    do {
        if(Mfkey32LoggerParams_size(instance->params_arr) == 0) break;

        Mfkey32LoggerParams_it_t it;
        for(Mfkey32LoggerParams_it(it, instance->params_arr); !Mfkey32LoggerParams_end_p(it);
            Mfkey32LoggerParams_next(it)) {
            Mfkey32LoggerParams* params = Mfkey32LoggerParams_ref(it);
            if(params->is_filled) continue;

            uint8_t sector_num = mf_classic_get_sector_by_block(auth_context->block_num);
            if(params->sector_num != sector_num) continue;
            if(params->key_type != auth_context->key_type) continue;

            params->nt1 = bit_lib_bytes_to_num_be(auth_context->nt.data, sizeof(MfClassicNt));
            params->nr1 = bit_lib_bytes_to_num_be(auth_context->nr.data, sizeof(MfClassicNr));
            params->ar1 = bit_lib_bytes_to_num_be(auth_context->ar.data, sizeof(MfClassicAr));
            params->is_filled = true;

            instance->params_collected++;
            nonce_added = true;
            break;
        }

    } while(false);

    return nonce_added;
}

void mfkey32_logger_add_nonce(Mfkey32Logger* instance, MfClassicAuthContext* auth_context) {
    furi_assert(instance);
    furi_assert(auth_context);

    bool nonce_added = mfkey32_logger_add_nonce_to_existing_params(instance, auth_context);
    if(!nonce_added && (instance->nonces_saves < MFKEY32_LOGGER_MAX_NONCES_SAVED)) {
        uint8_t sector_num = mf_classic_get_sector_by_block(auth_context->block_num);
        Mfkey32LoggerParams params = {
            .is_filled = false,
            .cuid = instance->cuid,
            .sector_num = sector_num,
            .key_type = auth_context->key_type,
            .nt0 = bit_lib_bytes_to_num_be(auth_context->nt.data, sizeof(MfClassicNt)),
            .nr0 = bit_lib_bytes_to_num_be(auth_context->nr.data, sizeof(MfClassicNr)),
            .ar0 = bit_lib_bytes_to_num_be(auth_context->ar.data, sizeof(MfClassicAr)),
        };
        Mfkey32LoggerParams_push_back(instance->params_arr, params);
        instance->nonces_saves++;
    }
}

size_t mfkey32_logger_get_params_num(Mfkey32Logger* instance) {
    furi_assert(instance);

    return instance->params_collected;
}

bool mfkey32_logger_save_params(Mfkey32Logger* instance, const char* path) {
    furi_assert(instance);
    furi_assert(path);
    furi_assert(instance->params_collected > 0);
    furi_assert(instance->params_arr);

    bool params_saved = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = buffered_file_stream_alloc(storage);
    FuriString* temp_str = furi_string_alloc();

    do {
        if(!buffered_file_stream_open(stream, path, FSAM_WRITE, FSOM_OPEN_APPEND)) break;

        bool params_write_success = true;
        Mfkey32LoggerParams_it_t it;
        for(Mfkey32LoggerParams_it(it, instance->params_arr); !Mfkey32LoggerParams_end_p(it);
            Mfkey32LoggerParams_next(it)) {
            Mfkey32LoggerParams* params = Mfkey32LoggerParams_ref(it);
            if(!params->is_filled) continue;
            furi_string_printf(
                temp_str,
                "Sec %d key %c cuid %08lx nt0 %08lx nr0 %08lx ar0 %08lx nt1 %08lx nr1 %08lx ar1 %08lx\n",
                params->sector_num,
                params->key_type == MfClassicKeyTypeA ? 'A' : 'B',
                params->cuid,
                params->nt0,
                params->nr0,
                params->ar0,
                params->nt1,
                params->nr1,
                params->ar1);
            if(!stream_write_string(stream, temp_str)) {
                params_write_success = false;
                break;
            }
        }
        if(!params_write_success) break;

        params_saved = true;
    } while(false);

    furi_string_free(temp_str);
    buffered_file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);

    return params_saved;
}

void mfkey32_logger_get_params_data(Mfkey32Logger* instance, FuriString* str) {
    furi_assert(instance);
    furi_assert(str);
    furi_assert(instance->params_collected > 0);

    furi_string_reset(str);
    Mfkey32LoggerParams_it_t it;
    for(Mfkey32LoggerParams_it(it, instance->params_arr); !Mfkey32LoggerParams_end_p(it);
        Mfkey32LoggerParams_next(it)) {
        Mfkey32LoggerParams* params = Mfkey32LoggerParams_ref(it);
        if(!params->is_filled) continue;

        char key_char = params->key_type == MfClassicKeyTypeA ? 'A' : 'B';
        furi_string_cat_printf(str, "Sector %d, key %c\n", params->sector_num, key_char);
    }
}
