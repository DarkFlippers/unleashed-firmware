#include "raw.h"
#include <lib/flipper_format/flipper_format.h>
#include "../subghz_file_encoder_worker.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>

#define TAG "SubGhzProtocolRAW"
#define SUBGHZ_DOWNLOAD_MAX_SIZE 512

static const SubGhzBlockConst subghz_protocol_raw_const = {
    .te_short = 50,
    .te_long = 32700,
    .te_delta = 0,
    .min_count_bit_for_found = 0,
};

struct SubGhzProtocolDecoderRAW {
    SubGhzProtocolDecoderBase base;

    int32_t* upload_raw;
    uint16_t ind_write;
    Storage* storage;
    FlipperFormat* flipper_file;
    uint32_t file_is_open;
    FuriString* file_name;
    size_t sample_write;
    bool last_level;
    bool pause;
};

struct SubGhzProtocolEncoderRAW {
    SubGhzProtocolEncoderBase base;

    bool is_running;
    FuriString* file_name;
    SubGhzFileEncoderWorker* file_worker_encoder;
};

typedef enum {
    RAWFileIsOpenClose = 0,
    RAWFileIsOpenWrite,
    RAWFileIsOpenRead,
} RAWFilIsOpen;

const SubGhzProtocolDecoder subghz_protocol_raw_decoder = {
    .alloc = subghz_protocol_decoder_raw_alloc,
    .free = subghz_protocol_decoder_raw_free,

    .feed = subghz_protocol_decoder_raw_feed,
    .reset = subghz_protocol_decoder_raw_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = subghz_protocol_decoder_raw_deserialize,
    .get_string = subghz_protocol_decoder_raw_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_raw_encoder = {
    .alloc = subghz_protocol_encoder_raw_alloc,
    .free = subghz_protocol_encoder_raw_free,

    .deserialize = subghz_protocol_encoder_raw_deserialize,
    .stop = subghz_protocol_encoder_raw_stop,
    .yield = subghz_protocol_encoder_raw_yield,
};

const SubGhzProtocol subghz_protocol_raw = {
    .name = SUBGHZ_PROTOCOL_RAW_NAME,
    .type = SubGhzProtocolTypeRAW,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_RAW |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_raw_decoder,
    .encoder = &subghz_protocol_raw_encoder,
};

bool subghz_protocol_raw_save_to_file_init(
    SubGhzProtocolDecoderRAW* instance,
    const char* dev_name,
    SubGhzRadioPreset* preset) {
    furi_assert(instance);

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->flipper_file = flipper_format_file_alloc(instance->storage);

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    bool init = false;

    do {
        // Create subghz folder directory if necessary
        if(!storage_simply_mkdir(instance->storage, SUBGHZ_RAW_FOLDER)) {
            break;
        }
        // Create saved directory if necessary
        if(!storage_simply_mkdir(instance->storage, SUBGHZ_RAW_FOLDER)) {
            break;
        }

        furi_string_set(instance->file_name, dev_name);
        // First remove subghz device file if it was saved
        furi_string_printf(temp_str, "%s/%s%s", SUBGHZ_RAW_FOLDER, dev_name, SUBGHZ_APP_EXTENSION);

        if(!storage_simply_remove(instance->storage, furi_string_get_cstr(temp_str))) {
            break;
        }

        // Open file
        if(!flipper_format_file_open_always(
               instance->flipper_file, furi_string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Unable to open file for write: %s", furi_string_get_cstr(temp_str));
            break;
        }

        if(!flipper_format_write_header_cstr(
               instance->flipper_file, SUBGHZ_RAW_FILE_TYPE, SUBGHZ_RAW_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            break;
        }

        if(!flipper_format_write_uint32(
               instance->flipper_file, "Frequency", &preset->frequency, 1)) {
            FURI_LOG_E(TAG, "Unable to add Frequency");
            break;
        }

        subghz_block_generic_get_preset_name(furi_string_get_cstr(preset->name), temp_str);
        if(!flipper_format_write_string_cstr(
               instance->flipper_file, "Preset", furi_string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Unable to add Preset");
            break;
        }
        if(!strcmp(furi_string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            if(!flipper_format_write_string_cstr(
                   instance->flipper_file, "Custom_preset_module", "CC1101")) {
                FURI_LOG_E(TAG, "Unable to add Custom_preset_module");
                break;
            }
            if(!flipper_format_write_hex(
                   instance->flipper_file, "Custom_preset_data", preset->data, preset->data_size)) {
                FURI_LOG_E(TAG, "Unable to add Custom_preset_data");
                break;
            }
        }
        if(!flipper_format_write_string_cstr(
               instance->flipper_file, "Protocol", instance->base.protocol->name)) {
            FURI_LOG_E(TAG, "Unable to add Protocol");
            break;
        }

        instance->upload_raw = malloc(SUBGHZ_DOWNLOAD_MAX_SIZE * sizeof(int32_t));
        instance->file_is_open = RAWFileIsOpenWrite;
        instance->sample_write = 0;
        instance->last_level = false;
        instance->pause = false;
        init = true;
    } while(0);

    furi_string_free(temp_str);

    return init;
}

static bool subghz_protocol_raw_save_to_file_write(SubGhzProtocolDecoderRAW* instance) {
    furi_assert(instance);

    bool is_write = false;
    if(instance->file_is_open == RAWFileIsOpenWrite) {
        if(!flipper_format_write_int32(
               instance->flipper_file, "RAW_Data", instance->upload_raw, instance->ind_write)) {
            FURI_LOG_E(TAG, "Unable to add RAW_Data");
        } else {
            instance->sample_write += instance->ind_write;
            instance->ind_write = 0;
            is_write = true;
        }
    }
    return is_write;
}

void subghz_protocol_raw_save_to_file_stop(SubGhzProtocolDecoderRAW* instance) {
    furi_assert(instance);

    if(instance->file_is_open == RAWFileIsOpenWrite && instance->ind_write)
        subghz_protocol_raw_save_to_file_write(instance);
    if(instance->file_is_open != RAWFileIsOpenClose) {
        free(instance->upload_raw);
        instance->upload_raw = NULL;
        flipper_format_file_close(instance->flipper_file);
        flipper_format_free(instance->flipper_file);
        furi_record_close(RECORD_STORAGE);
    }

    instance->file_is_open = RAWFileIsOpenClose;
}

void subghz_protocol_raw_save_to_file_pause(SubGhzProtocolDecoderRAW* instance, bool pause) {
    furi_assert(instance);

    if(instance->pause != pause) {
        instance->pause = pause;
    }
}

size_t subghz_protocol_raw_get_sample_write(SubGhzProtocolDecoderRAW* instance) {
    return instance->sample_write + instance->ind_write;
}

void* subghz_protocol_decoder_raw_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderRAW* instance = malloc(sizeof(SubGhzProtocolDecoderRAW));
    instance->base.protocol = &subghz_protocol_raw;
    instance->upload_raw = NULL;
    instance->ind_write = 0;
    instance->last_level = false;
    instance->file_is_open = RAWFileIsOpenClose;
    instance->file_name = furi_string_alloc();

    return instance;
}

void subghz_protocol_decoder_raw_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRAW* instance = context;
    furi_string_free(instance->file_name);
    free(instance);
}

void subghz_protocol_decoder_raw_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRAW* instance = context;
    instance->ind_write = 0;
    instance->last_level = false;
}

void subghz_protocol_decoder_raw_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderRAW* instance = context;

    if(!instance->pause && (instance->upload_raw != NULL)) {
        if(duration > subghz_protocol_raw_const.te_short) {
            if(instance->last_level != level) {
                instance->last_level = (level ? true : false);
                instance->upload_raw[instance->ind_write++] = (level ? duration : -duration);
            }
        }

        if(instance->ind_write == SUBGHZ_DOWNLOAD_MAX_SIZE) {
            subghz_protocol_raw_save_to_file_write(instance);
        }
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_raw_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    UNUSED(context);
    UNUSED(flipper_format);
    //ToDo stub, for backwards compatibility
    return SubGhzProtocolStatusOk;
}

void subghz_protocol_decoder_raw_get_string(void* context, FuriString* output) {
    furi_assert(context);
    //SubGhzProtocolDecoderRAW* instance = context;
    UNUSED(context);
    //ToDo no use
    furi_string_cat_printf(output, "RAW Date");
}

void* subghz_protocol_encoder_raw_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderRAW* instance = malloc(sizeof(SubGhzProtocolEncoderRAW));

    instance->base.protocol = &subghz_protocol_raw;
    instance->file_name = furi_string_alloc();
    instance->is_running = false;
    return instance;
}

void subghz_protocol_encoder_raw_stop(void* context) {
    SubGhzProtocolEncoderRAW* instance = context;
    instance->is_running = false;
    if(subghz_file_encoder_worker_is_running(instance->file_worker_encoder)) {
        subghz_file_encoder_worker_stop(instance->file_worker_encoder);
        subghz_file_encoder_worker_free(instance->file_worker_encoder);
    }
}

void subghz_protocol_encoder_raw_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderRAW* instance = context;
    subghz_protocol_encoder_raw_stop(instance);
    furi_string_free(instance->file_name);
    free(instance);
}

void subghz_protocol_raw_file_encoder_worker_set_callback_end(
    SubGhzProtocolEncoderRAW* instance,
    SubGhzProtocolEncoderRAWCallbackEnd callback_end,
    void* context_end) {
    furi_assert(instance);
    furi_assert(callback_end);
    subghz_file_encoder_worker_callback_end(
        instance->file_worker_encoder, callback_end, context_end);
}

static bool subghz_protocol_encoder_raw_worker_init(SubGhzProtocolEncoderRAW* instance) {
    furi_assert(instance);

    instance->file_worker_encoder = subghz_file_encoder_worker_alloc();
    if(subghz_file_encoder_worker_start(
           instance->file_worker_encoder, furi_string_get_cstr(instance->file_name))) {
        //the worker needs a file in order to open and read part of the file
        furi_delay_ms(100);
        instance->is_running = true;
    } else {
        subghz_protocol_encoder_raw_stop(instance);
    }
    return instance->is_running;
}

void subghz_protocol_raw_gen_fff_data(FlipperFormat* flipper_format, const char* file_path) {
    do {
        stream_clean(flipper_format_get_raw_stream(flipper_format));
        if(!flipper_format_write_string_cstr(flipper_format, "Protocol", "RAW")) {
            FURI_LOG_E(TAG, "Unable to add Protocol");
            break;
        }

        if(!flipper_format_write_string_cstr(flipper_format, "File_name", file_path)) {
            FURI_LOG_E(TAG, "Unable to add File_name");
            break;
        }
    } while(false);
}

SubGhzProtocolStatus
    subghz_protocol_encoder_raw_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderRAW* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    do {
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            res = SubGhzProtocolStatusErrorParserOthers;
            break;
        }

        if(!flipper_format_read_string(flipper_format, "File_name", temp_str)) {
            FURI_LOG_E(TAG, "Missing File_name");
            res = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        furi_string_set(instance->file_name, temp_str);

        if(!subghz_protocol_encoder_raw_worker_init(instance)) {
            res = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        res = SubGhzProtocolStatusOk;
    } while(false);
    furi_string_free(temp_str);
    return res;
}

LevelDuration subghz_protocol_encoder_raw_yield(void* context) {
    SubGhzProtocolEncoderRAW* instance = context;

    if(!instance->is_running) return level_duration_reset();
    return subghz_file_encoder_worker_get_level_duration(instance->file_worker_encoder);
}
