#include "subghz_protocol_raw.h"
#include "file-worker.h"
#include "../subghz_file_encoder_worker.h"

#define SUBGHZ_DOWNLOAD_MAX_SIZE 512

struct SubGhzProtocolRAW {
    SubGhzProtocolCommon common;

    int16_t* upload_raw;
    uint16_t ind_write;
    FileWorker* file_worker;
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

    instance->file_worker = file_worker_alloc(false);
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
    file_worker_free(instance->file_worker);
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
            subghz_protocol_save_raw_to_file_write(instance);
        }
    }
}

void subghz_protocol_raw_to_str(SubGhzProtocolRAW* instance, string_t output) {
    string_cat_printf(output, "RAW Date");
}

const char* subghz_protocol_get_last_file_name(SubGhzProtocolRAW* instance) {
    return string_get_cstr(instance->file_name);
}

void subghz_protocol_set_last_file_name(SubGhzProtocolRAW* instance, const char* name) {
    string_printf(instance->file_name, "%s", name);
}

bool subghz_protocol_save_raw_to_file_init(
    SubGhzProtocolRAW* instance,
    const char* dev_name,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    furi_assert(instance);

    string_t dev_file_name;
    string_init(dev_file_name);
    string_t temp_str;
    string_init(temp_str);
    bool init = false;

    do {
        // Create subghz folder directory if necessary
        if(!file_worker_mkdir(instance->file_worker, SUBGHZ_RAW_FOLDER)) {
            break;
        }
        // Create saved directory if necessary
        if(!file_worker_mkdir(instance->file_worker, SUBGHZ_RAW_PATH_FOLDER)) {
            break;
        }
        //get the name of the next free file
        file_worker_get_next_filename(
            instance->file_worker,
            SUBGHZ_RAW_PATH_FOLDER,
            dev_name,
            SUBGHZ_APP_EXTENSION,
            temp_str);

        string_set(instance->file_name, temp_str);

        string_printf(
            dev_file_name,
            "%s/%s%s",
            SUBGHZ_RAW_PATH_FOLDER,
            string_get_cstr(temp_str),
            SUBGHZ_APP_EXTENSION);
        // Open file
        if(!file_worker_open(
               instance->file_worker,
               string_get_cstr(dev_file_name),
               FSAM_WRITE,
               FSOM_CREATE_ALWAYS)) {
            break;
        }
        //Get string frequency preset protocol
        string_printf(
            temp_str,
            "Frequency: %d\n"
            "Preset: %d\n"
            "Protocol: RAW\n",
            (int)frequency,
            (int)preset);

        if(!file_worker_write(
               instance->file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
            break;
        }

        instance->upload_raw = furi_alloc(SUBGHZ_DOWNLOAD_MAX_SIZE * sizeof(uint16_t));
        instance->file_is_open = RAWFileIsOpenWrite;
        instance->sample_write = 0;
        init = true;
    } while(0);

    string_clear(temp_str);
    string_clear(dev_file_name);

    return init;
}

void subghz_protocol_save_raw_to_file_stop(SubGhzProtocolRAW* instance) {
    furi_assert(instance);

    if(instance->file_is_open == RAWFileIsOpenWrite && instance->ind_write)
        subghz_protocol_save_raw_to_file_write(instance);
    if(instance->file_is_open != RAWFileIsOpenClose) {
        free(instance->upload_raw);
        instance->upload_raw = NULL;
    }

    file_worker_close(instance->file_worker);
    instance->file_is_open = RAWFileIsOpenClose;
}

bool subghz_protocol_save_raw_to_file_write(SubGhzProtocolRAW* instance) {
    furi_assert(instance);

    string_t temp_str;
    string_init(temp_str);
    bool is_write = false;
    if(instance->file_is_open == RAWFileIsOpenWrite) {
        do {
            string_printf(temp_str, "RAW_Data: ");

            if(!file_worker_write(
                   instance->file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
                break;
            }

            for(size_t i = 0; i < instance->ind_write - 1; i++) {
                string_printf(temp_str, "%d, ", instance->upload_raw[i]);
                if(!file_worker_write(
                       instance->file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
                    break;
                }
            }

            string_printf(temp_str, "%d\n", instance->upload_raw[instance->ind_write - 1]);
            if(!file_worker_write(
                   instance->file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
                break;
            }

            instance->sample_write += instance->ind_write;
            instance->ind_write = 0;
            is_write = true;
        } while(0);
        string_clear(temp_str);
    }
    return is_write;
}

size_t subghz_save_protocol_raw_get_sample_write(SubGhzProtocolRAW* instance) {
    return instance->sample_write + instance->ind_write;
}

bool subghz_protocol_raw_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolRAW* instance,
    const char* file_path) {
    subghz_protocol_set_last_file_name(instance, file_path);
    return true;
}