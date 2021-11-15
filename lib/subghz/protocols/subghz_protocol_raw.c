#include "subghz_protocol_raw.h"
#include "../subghz_file_encoder_worker.h"

#define TAG "SubGhzRaw"

#define SUBGHZ_DOWNLOAD_MAX_SIZE 512

struct SubGhzProtocolRAW {
    SubGhzProtocolCommon common;

    int32_t* upload_raw;
    uint16_t ind_write;
    Storage* storage;
    FlipperFile* flipper_file;
    SubGhzFileEncoderWorker* file_worker_encoder;
    uint32_t file_is_open;
    string_t file_name;
    size_t sample_write;
    bool last_level;
};

typedef enum {
    RAWFileIsOpenClose = 0,
    RAWFileIsOpenWrite,
    RAWFileIsOpenRead,
} RAWFilIsOpen;

SubGhzProtocolRAW* subghz_protocol_raw_alloc(void) {
    SubGhzProtocolRAW* instance = furi_alloc(sizeof(SubGhzProtocolRAW));

    instance->upload_raw = NULL;
    instance->ind_write = 0;

    instance->last_level = false;

    instance->storage = furi_record_open("storage");
    instance->flipper_file = flipper_file_alloc(instance->storage);
    instance->file_is_open = RAWFileIsOpenClose;
    string_init(instance->file_name);

    instance->common.name = "RAW";
    instance->common.code_min_count_bit_for_found = 0;
    instance->common.te_short = 80;
    instance->common.te_long = 32700;
    instance->common.te_delta = 0;
    instance->common.type_protocol = SubGhzProtocolCommonTypeRAW;
    instance->common.to_load_protocol_from_file =
        (SubGhzProtocolCommonLoadFromFile)subghz_protocol_raw_to_load_protocol_from_file;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_raw_to_str;
    //instance->common.to_load_protocol =
    //    (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_raw_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_raw_send_key;

    return instance;
}

void subghz_protocol_raw_free(SubGhzProtocolRAW* instance) {
    furi_assert(instance);
    string_clear(instance->file_name);

    flipper_file_free(instance->flipper_file);
    furi_record_close("storage");

    free(instance);
}

void subghz_protocol_raw_file_encoder_worker_stop(void* context) {
    furi_assert(context);
    SubGhzProtocolRAW* instance = context;
    if(subghz_file_encoder_worker_is_running(instance->file_worker_encoder)) {
        subghz_file_encoder_worker_stop(instance->file_worker_encoder);
        subghz_file_encoder_worker_free(instance->file_worker_encoder);
        instance->file_is_open = RAWFileIsOpenClose;
    }
}

bool subghz_protocol_raw_send_key(
    SubGhzProtocolRAW* instance,
    SubGhzProtocolCommonEncoder* encoder) {
    furi_assert(instance);
    furi_assert(encoder);

    bool loaded = false;

    instance->file_worker_encoder = subghz_file_encoder_worker_alloc();

    if(subghz_file_encoder_worker_start(
           instance->file_worker_encoder, string_get_cstr(instance->file_name))) {
        //the worker needs a file in order to open and read part of the file
        osDelay(100);
        instance->file_is_open = RAWFileIsOpenRead;
        subghz_protocol_encoder_common_set_callback(
            encoder, subghz_file_encoder_worker_get_level_duration, instance->file_worker_encoder);
        subghz_protocol_encoder_common_set_callback_end(
            encoder, subghz_protocol_raw_file_encoder_worker_stop, instance);

        loaded = true;
    } else {
        subghz_protocol_raw_file_encoder_worker_stop(instance);
    }
    return loaded;
}

void subghz_protocol_raw_reset(SubGhzProtocolRAW* instance) {
    instance->ind_write = 0;
}

void subghz_protocol_raw_parse(SubGhzProtocolRAW* instance, bool level, uint32_t duration) {
    if(instance->upload_raw != NULL) {
        if(duration > instance->common.te_short) {
            if(duration > instance->common.te_long) duration = instance->common.te_long;
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

void subghz_protocol_raw_to_str(SubGhzProtocolRAW* instance, string_t output) {
    string_cat_printf(output, "RAW Date");
}

const char* subghz_protocol_raw_get_last_file_name(SubGhzProtocolRAW* instance) {
    return string_get_cstr(instance->file_name);
}

void subghz_protocol_raw_set_last_file_name(SubGhzProtocolRAW* instance, const char* name) {
    string_printf(instance->file_name, "%s", name);
}

bool subghz_protocol_raw_save_to_file_init(
    SubGhzProtocolRAW* instance,
    const char* dev_name,
    uint32_t frequency,
    const char* preset) {
    furi_assert(instance);

    //instance->flipper_file = flipper_file_alloc(instance->storage);
    string_t dev_file_name;
    string_init(dev_file_name);
    bool init = false;

    do {
        // Create subghz folder directory if necessary
        if(!storage_simply_mkdir(instance->storage, SUBGHZ_RAW_FOLDER)) {
            break;
        }
        // Create saved directory if necessary
        if(!storage_simply_mkdir(instance->storage, SUBGHZ_RAW_PATH_FOLDER)) {
            break;
        }

        string_set(instance->file_name, dev_name);
        // First remove subghz device file if it was saved
        string_printf(
            dev_file_name, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, dev_name, SUBGHZ_APP_EXTENSION);

        if(!storage_simply_remove(instance->storage, string_get_cstr(dev_file_name))) {
            break;
        }

        // Open file
        if(!flipper_file_open_always(instance->flipper_file, string_get_cstr(dev_file_name))) {
            FURI_LOG_E(TAG, "Unable to open file for write: %s", dev_file_name);
            break;
        }

        if(!flipper_file_write_header_cstr(
               instance->flipper_file, SUBGHZ_RAW_FILE_TYPE, SUBGHZ_RAW_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            break;
        }

        if(!flipper_file_write_uint32(instance->flipper_file, "Frequency", &frequency, 1)) {
            FURI_LOG_E(TAG, "Unable to add Frequency");
            break;
        }

        if(!flipper_file_write_string_cstr(instance->flipper_file, "Preset", preset)) {
            FURI_LOG_E(TAG, "Unable to add Preset");
            break;
        }

        if(!flipper_file_write_string_cstr(instance->flipper_file, "Protocol", instance->common.name)) {
            FURI_LOG_E(TAG, "Unable to add Protocol");
            break;
        }

        instance->upload_raw = furi_alloc(SUBGHZ_DOWNLOAD_MAX_SIZE * sizeof(int32_t));
        instance->file_is_open = RAWFileIsOpenWrite;
        instance->sample_write = 0;
        init = true;
    } while(0);

    string_clear(dev_file_name);

    return init;
}

void subghz_protocol_raw_save_to_file_stop(SubGhzProtocolRAW* instance) {
    furi_assert(instance);

    if(instance->file_is_open == RAWFileIsOpenWrite && instance->ind_write)
        subghz_protocol_raw_save_to_file_write(instance);
    if(instance->file_is_open != RAWFileIsOpenClose) {
        free(instance->upload_raw);
        instance->upload_raw = NULL;
    }

    flipper_file_close(instance->flipper_file);
    instance->file_is_open = RAWFileIsOpenClose;
}

bool subghz_protocol_raw_save_to_file_write(SubGhzProtocolRAW* instance) {
    furi_assert(instance);

    bool is_write = false;
    if(instance->file_is_open == RAWFileIsOpenWrite) {
        if(!flipper_file_write_int32(
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

size_t subghz_protocol_raw_get_sample_write(SubGhzProtocolRAW* instance) {
    return instance->sample_write + instance->ind_write;
}

bool subghz_protocol_raw_to_load_protocol_from_file(
    FlipperFile* flipper_file,
    SubGhzProtocolRAW* instance,
    const char* file_path) {
    furi_assert(file_path);
    subghz_protocol_raw_set_last_file_name(instance, file_path);
    return true;
}