#include "subghz_file_encoder_worker.h"
#include <stream_buffer.h>

#include <toolbox/stream/stream.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>

#define TAG "SubGhzFileEncoderWorker"

#define SUBGHZ_FILE_ENCODER_LOAD 512

struct SubGhzFileEncoderWorker {
    FuriThread* thread;
    StreamBufferHandle_t stream;

    Storage* storage;
    FlipperFormat* flipper_format;

    volatile bool worker_running;
    volatile bool worker_stoping;
    bool level;
    string_t str_data;
    string_t file_path;

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
    bool res = true;
    if(duration < 0 && !instance->level) {
        res = false;
    } else if(duration > 0 && instance->level) {
        res = false;
    }

    if(res) {
        instance->level = !instance->level;
        xStreamBufferSend(instance->stream, &duration, sizeof(int32_t), 100);
    } else {
        FURI_LOG_E(TAG, "Invalid level in the stream");
    }
}

bool subghz_file_encoder_worker_data_parse(SubGhzFileEncoderWorker* instance, const char* strStart) {
    char* str1;
    bool res = false;
    // Line sample: "RAW_Data: -1, 2, -2..."

    // Look for a key in the line
    str1 = strstr(strStart, "RAW_Data: ");

    if(str1 != NULL) {
        // Skip key
        str1 = strchr(str1, ' ');

        // Check that there is still an element in the line
        while(strchr(str1, ' ') != NULL) {
            str1 = strchr(str1, ' ');

            // Skip space
            str1 += 1;
            subghz_file_encoder_worker_add_level_duration(instance, atoi(str1));
        }
        res = true;
    }
    return res;
}

LevelDuration subghz_file_encoder_worker_get_level_duration(void* context) {
    furi_assert(context);
    SubGhzFileEncoderWorker* instance = context;
    int32_t duration;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int ret = xStreamBufferReceiveFromISR(
        instance->stream, &duration, sizeof(int32_t), &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    if(ret == sizeof(int32_t)) {
        LevelDuration level_duration = {.level = LEVEL_DURATION_RESET};
        if(duration < 0) {
            level_duration = level_duration_make(false, duration * -1);
        } else if(duration > 0) {
            level_duration = level_duration_make(true, duration);
        } else if(duration == 0) {
            level_duration = level_duration_reset();
            FURI_LOG_I(TAG, "Stop transmission");
            instance->worker_stoping = true;
        }
        return level_duration;
    } else {
        FURI_LOG_E(TAG, "Slow flash read");
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
    Stream* stream = flipper_format_get_raw_stream(instance->flipper_format);
    do {
        if(!flipper_format_file_open_existing(
               instance->flipper_format, string_get_cstr(instance->file_path))) {
            FURI_LOG_E(
                TAG, "Unable to open file for read: %s", string_get_cstr(instance->file_path));
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
        size_t stream_free_byte = xStreamBufferSpacesAvailable(instance->stream);
        if((stream_free_byte / sizeof(int32_t)) >= SUBGHZ_FILE_ENCODER_LOAD) {
            if(stream_read_line(stream, instance->str_data)) {
                string_strim(instance->str_data);
                if(!subghz_file_encoder_worker_data_parse(
                       instance, string_get_cstr(instance->str_data))) {
                    //to stop DMA correctly
                    subghz_file_encoder_worker_add_level_duration(instance, LEVEL_DURATION_RESET);
                    subghz_file_encoder_worker_add_level_duration(instance, LEVEL_DURATION_RESET);

                    break;
                }
            } else {
                subghz_file_encoder_worker_add_level_duration(instance, LEVEL_DURATION_RESET);
                subghz_file_encoder_worker_add_level_duration(instance, LEVEL_DURATION_RESET);
                break;
            }
        }
        furi_delay_ms(5);
    }
    //waiting for the end of the transfer
    FURI_LOG_I(TAG, "End read file");
    while(!furi_hal_subghz_is_async_tx_complete() && instance->worker_running) {
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

SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc() {
    SubGhzFileEncoderWorker* instance = malloc(sizeof(SubGhzFileEncoderWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubGhzFEWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_file_encoder_worker_thread);
    instance->stream = xStreamBufferCreate(sizeof(int32_t) * 2048, sizeof(int32_t));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->flipper_format = flipper_format_file_alloc(instance->storage);

    string_init(instance->str_data);
    string_init(instance->file_path);
    instance->level = false;
    instance->worker_stoping = true;

    return instance;
}

void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* instance) {
    furi_assert(instance);

    vStreamBufferDelete(instance->stream);
    furi_thread_free(instance->thread);

    string_clear(instance->str_data);
    string_clear(instance->file_path);

    flipper_format_free(instance->flipper_format);
    furi_record_close(RECORD_STORAGE);

    free(instance);
}

bool subghz_file_encoder_worker_start(SubGhzFileEncoderWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    xStreamBufferReset(instance->stream);
    string_set(instance->file_path, file_path);
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
