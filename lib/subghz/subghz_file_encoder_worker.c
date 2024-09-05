#include "subghz_file_encoder_worker.h"

#include <toolbox/stream/stream.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#include <lib/subghz/devices/devices.h>
#include <lib/toolbox/strint.h>

#define TAG "SubGhzFileEncoderWorker"

#define SUBGHZ_FILE_ENCODER_LOAD 512

struct SubGhzFileEncoderWorker {
    FuriThread* thread;
    FuriStreamBuffer* stream;

    Storage* storage;
    FlipperFormat* flipper_format;

    volatile bool worker_running;
    volatile bool worker_stoping;
    bool is_storage_slow;
    FuriString* str_data;
    FuriString* file_path;
    const SubGhzDevice* device;

    SubGhzFileEncoderWorkerCallbackEnd callback_end;
    void* context_end;
};

void subghz_file_encoder_worker_callback_end(
    SubGhzFileEncoderWorker* instance,
    SubGhzFileEncoderWorkerCallbackEnd callback_end,
    void* context_end) {
    furi_assert(instance);
    furi_assert(callback_end);
    instance->callback_end = callback_end;
    instance->context_end = context_end;
}

void subghz_file_encoder_worker_add_level_duration(
    SubGhzFileEncoderWorker* instance,
    int32_t duration) {
    size_t ret = furi_stream_buffer_send(instance->stream, &duration, sizeof(int32_t), 100);
    if(sizeof(int32_t) != ret) FURI_LOG_E(TAG, "Invalid add duration in the stream");
}

bool subghz_file_encoder_worker_data_parse(SubGhzFileEncoderWorker* instance, const char* strStart) {
    // Line sample: "RAW_Data: -1, 2, -2..."

    // Look for the key in the line
    char* str = strstr(strStart, "RAW_Data: ");
    bool res = false;

    if(str) {
        // Skip key
        str = strchr(str, ' ');

        // Parse next element
        int32_t duration;
        while(strint_to_int32(str, &str, &duration, 10) == StrintParseNoError) {
            subghz_file_encoder_worker_add_level_duration(instance, duration);
            if(*str == ',') str++; // could also be `\0`
        }

        res = true;
    }

    return res;
}

LevelDuration subghz_file_encoder_worker_get_level_duration(void* context) {
    furi_assert(context);
    SubGhzFileEncoderWorker* instance = context;
    int32_t duration;
    int ret = furi_stream_buffer_receive(instance->stream, &duration, sizeof(int32_t), 0);
    if(ret == sizeof(int32_t)) {
        LevelDuration level_duration = {.level = LEVEL_DURATION_RESET};
        if(duration < 0) {
            level_duration = level_duration_make(false, -duration);
        } else if(duration > 0) {
            level_duration = level_duration_make(true, duration);
        } else if(duration == 0) { //-V547
            level_duration = level_duration_reset();
            FURI_LOG_I(TAG, "Stop transmission");
            instance->worker_stoping = true;
        }
        return level_duration;
    } else {
        instance->is_storage_slow = true;
        return level_duration_wait();
    }
}

/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_file_encoder_worker_thread(void* context) {
    SubGhzFileEncoderWorker* instance = context;
    FURI_LOG_I(TAG, "Worker start");
    bool res = false;
    instance->is_storage_slow = false;
    Stream* stream = flipper_format_get_raw_stream(instance->flipper_format);
    do {
        if(!flipper_format_file_open_existing(
               instance->flipper_format, furi_string_get_cstr(instance->file_path))) {
            FURI_LOG_E(
                TAG,
                "Unable to open file for read: %s",
                furi_string_get_cstr(instance->file_path));
            break;
        }
        if(!flipper_format_read_string(instance->flipper_format, "Protocol", instance->str_data)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }

        //skip the end of the previous line "\n"
        stream_seek(stream, 1, StreamOffsetFromCurrent);
        res = true;
        instance->worker_stoping = false;
        FURI_LOG_I(TAG, "Start transmission");
    } while(0);

    while(res && instance->worker_running) {
        size_t stream_free_byte = furi_stream_buffer_spaces_available(instance->stream);
        if((stream_free_byte / sizeof(int32_t)) >= SUBGHZ_FILE_ENCODER_LOAD) {
            if(stream_read_line(stream, instance->str_data)) {
                furi_string_trim(instance->str_data);
                if(!subghz_file_encoder_worker_data_parse(
                       instance, furi_string_get_cstr(instance->str_data))) {
                    subghz_file_encoder_worker_add_level_duration(instance, LEVEL_DURATION_RESET);
                    break;
                }
            } else {
                subghz_file_encoder_worker_add_level_duration(instance, LEVEL_DURATION_RESET);
                break;
            }
        } else {
            furi_delay_ms(1);
        }
    }
    //waiting for the end of the transfer
    if(instance->is_storage_slow) {
        FURI_LOG_E(TAG, "Storage is slow");
    }

    FURI_LOG_I(TAG, "End read file");
    while(instance->device && !subghz_devices_is_async_complete_tx(instance->device) &&
          instance->worker_running) {
        furi_delay_ms(5);
    }

    FURI_LOG_I(TAG, "End transmission");
    while(instance->worker_running) {
        if(instance->worker_stoping) {
            if(instance->callback_end) instance->callback_end(instance->context_end);
        }
        furi_delay_ms(50);
    }
    flipper_format_file_close(instance->flipper_format);

    FURI_LOG_I(TAG, "Worker stop");
    return 0;
}

SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc(void) {
    SubGhzFileEncoderWorker* instance = malloc(sizeof(SubGhzFileEncoderWorker));

    instance->thread =
        furi_thread_alloc_ex("SubGhzFEWorker", 2048, subghz_file_encoder_worker_thread, instance);
    instance->stream = furi_stream_buffer_alloc(sizeof(int32_t) * 2048, sizeof(int32_t));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->flipper_format = flipper_format_file_alloc(instance->storage);

    instance->str_data = furi_string_alloc();
    instance->file_path = furi_string_alloc();
    instance->worker_stoping = true;

    return instance;
}

void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* instance) {
    furi_assert(instance);

    furi_stream_buffer_free(instance->stream);
    furi_thread_free(instance->thread);

    furi_string_free(instance->str_data);
    furi_string_free(instance->file_path);

    flipper_format_free(instance->flipper_format);
    furi_record_close(RECORD_STORAGE);

    free(instance);
}

bool subghz_file_encoder_worker_start(
    SubGhzFileEncoderWorker* instance,
    const char* file_path,
    const char* radio_device_name) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    furi_stream_buffer_reset(instance->stream);
    furi_string_set(instance->file_path, file_path);
    if(radio_device_name) {
        instance->device = subghz_devices_get_by_name(radio_device_name);
    }
    instance->worker_running = true;
    furi_thread_start(instance->thread);

    return true;
}

void subghz_file_encoder_worker_stop(SubGhzFileEncoderWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    instance->worker_running = false;
    furi_thread_join(instance->thread);
}

bool subghz_file_encoder_worker_is_running(SubGhzFileEncoderWorker* instance) {
    furi_assert(instance);
    return instance->worker_running;
}
