#include "subghz_file_encoder_worker.h"
#include <stream_buffer.h>

#include <lib/flipper_file/flipper_file.h>
#include <lib/flipper_file/file_helper.h>

#define SUBGHZ_FILE_ENCODER_LOAD 512

struct SubGhzFileEncoderWorker {
    FuriThread* thread;
    StreamBufferHandle_t stream;

    Storage* storage;
    FlipperFile* flipper_file;

    volatile bool worker_running;
    bool level;
    int32_t duration;
    string_t str_data;
    string_t file_path;
};

void subghz_file_encoder_worker_add_livel_duration(
    SubGhzFileEncoderWorker* instance,
    int32_t duration) {
    bool res = true;
    if(duration < 0 && !instance->level) {
        instance->duration += duration;
        res = false;
    } else if(duration > 0 && instance->level) {
        instance->duration += duration;
        res = false;
    } else if(duration == 0) {
        instance->duration = 0;
    }

    if(res) {
        instance->level = !instance->level;
        instance->duration += duration;
        xStreamBufferSend(instance->stream, &instance->duration, sizeof(int32_t), 10);
        instance->duration = 0;
    }
}

bool subghz_file_encoder_worker_data_parse(
    SubGhzFileEncoderWorker* instance,
    const char* strStart,
    size_t len) {
    char* str1;
    size_t ind_start = (size_t)strStart; //store the start address of the beginning of the line
    bool res = false;

    str1 = strstr(
        strStart, "RAW_Data: "); //looking for the beginning of the desired title in the line
    if(str1 != NULL) {
        str1 = strchr(
            str1,
            ' '); //if found, shift the pointer by 1 element per line "RAW_Data: -1, 2, -2..."
        while(
            strchr(str1, ' ') != NULL &&
            ((size_t)str1 <
             (len +
              ind_start))) { //check that there is still an element in the line and that it has not gone beyond the line
            str1 = strchr(str1, ' ');
            str1 += 1; //if found, shift the pointer by next element per line
            subghz_file_encoder_worker_add_livel_duration(instance, atoi(str1));
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
            FURI_LOG_I("SubGhzFileEncoderWorker", "Stop transmission");
        }
        return level_duration;
    } else {
        FURI_LOG_E("SubGhzFileEncoderWorker", "Slow flash read");
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
    FURI_LOG_I("SubGhzFileEncoderWorker", "Worker start");
    bool res = false;
    File* file = flipper_file_get_file(instance->flipper_file);
    do {
        if(!flipper_file_open_existing(
               instance->flipper_file, string_get_cstr(instance->file_path))) {
            FURI_LOG_E(
                "SubGhzFileEncoderWorker",
                "Unable to open file for read: %s",
                string_get_cstr(instance->file_path));
            break;
        }
        if(!flipper_file_read_string(instance->flipper_file, "Protocol", instance->str_data)) {
            FURI_LOG_E("SubGhzFileEncoderWorker", "Missing Protocol");
            break;
        }

        //skip the end of the previous line "\n"
        storage_file_seek(file, 1, false);
        res = true;
        FURI_LOG_I("SubGhzFileEncoderWorker", "Start transmission");
    } while(0);

    while(res && instance->worker_running) {
        size_t stream_free_byte = xStreamBufferSpacesAvailable(instance->stream);
        if((stream_free_byte / sizeof(int32_t)) >= SUBGHZ_FILE_ENCODER_LOAD) {
            if(file_helper_read_line(file, instance->str_data)) {
                //skip the end of the previous line "\n"
                storage_file_seek(file, 1, false);
                if(!subghz_file_encoder_worker_data_parse(
                       instance,
                       string_get_cstr(instance->str_data),
                       strlen(string_get_cstr(instance->str_data)))) {
                    //to stop DMA correctly
                    subghz_file_encoder_worker_add_livel_duration(instance, LEVEL_DURATION_RESET);
                    subghz_file_encoder_worker_add_livel_duration(instance, LEVEL_DURATION_RESET);

                    break;
                }
            } else {
                subghz_file_encoder_worker_add_livel_duration(instance, LEVEL_DURATION_RESET);
                subghz_file_encoder_worker_add_livel_duration(instance, LEVEL_DURATION_RESET);
                break;
            }
        }
    }
    //waiting for the end of the transfer
    FURI_LOG_I("SubGhzFileEncoderWorker", "End read file");
    while(instance->worker_running) {
        osDelay(50);
    }
    flipper_file_close(instance->flipper_file);

    FURI_LOG_I("SubGhzFileEncoderWorker", "Worker stop");
    return 0;
}

SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc() {
    SubGhzFileEncoderWorker* instance = furi_alloc(sizeof(SubGhzFileEncoderWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "subghz_file_encoder_worker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_file_encoder_worker_thread);
    instance->stream = xStreamBufferCreate(sizeof(int32_t) * 2048, sizeof(int32_t));

    instance->storage = furi_record_open("storage");
    instance->flipper_file = flipper_file_alloc(instance->storage);

    string_init(instance->str_data);
    string_init(instance->file_path);
    instance->level = false;

    return instance;
}

void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* instance) {
    furi_assert(instance);

    vStreamBufferDelete(instance->stream);
    furi_thread_free(instance->thread);

    string_clear(instance->str_data);
    string_clear(instance->file_path);

    flipper_file_free(instance->flipper_file);
    furi_record_close("storage");

    free(instance);
}

bool subghz_file_encoder_worker_start(SubGhzFileEncoderWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    xStreamBufferReset(instance->stream);
    string_set(instance->file_path, file_path);
    instance->worker_running = true;
    bool res = furi_thread_start(instance->thread);
    return res;
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
