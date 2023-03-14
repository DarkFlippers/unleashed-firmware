#include "morse_code_worker.h"
#include <furi_hal.h>
#include <lib/flipper_format/flipper_format.h>

#define TAG "MorseCodeWorker"

#define MORSE_CODE_VERSION 0

//A-Z0-1
const char morse_array[36][6] = {".-",    "-...",  "-.-.",  "-..",   ".",     "..-.",
                                 "--.",   "....",  "..",    ".---",  "-.-",   ".-..",
                                 "--",    "-.",    "---",   ".--.",  "--.-",  ".-.",
                                 "...",   "-",     "..-",   "...-",  ".--",   "-..-",
                                 "-.--",  "--..",  ".----", "..---", "...--", "....-",
                                 ".....", "-....", "--...", "---..", "----.", "-----"};
const char symbol_array[36] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                               'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                               'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};

struct MorseCodeWorker {
    FuriThread* thread;
    MorseCodeWorkerCallback callback;
    void* callback_context;
    bool is_running;
    bool play;
    float volume;
    uint32_t dit_delta;
    FuriString* buffer;
    FuriString* words;
};

void morse_code_worker_fill_buffer(MorseCodeWorker* instance, uint32_t duration) {
    FURI_LOG_D("MorseCode: Duration", "%ld", duration);
    if(duration <= instance->dit_delta)
        furi_string_push_back(instance->buffer, *DOT);
    else if(duration <= (instance->dit_delta * 3))
        furi_string_push_back(instance->buffer, *LINE);
    else
        furi_string_reset(instance->buffer);
    if(furi_string_size(instance->buffer) > 5) furi_string_reset(instance->buffer);
    FURI_LOG_D("MorseCode: Buffer", "%s", furi_string_get_cstr(instance->buffer));
}

void morse_code_worker_fill_letter(MorseCodeWorker* instance) {
    if(furi_string_size(instance->words) > 63) furi_string_reset(instance->words);
    for(size_t i = 0; i < sizeof(morse_array); i++) {
        if(furi_string_cmp_str(instance->buffer, morse_array[i]) == 0) {
            furi_string_push_back(instance->words, symbol_array[i]);
            break;
        }
    }
    furi_string_reset(instance->buffer);
    FURI_LOG_D("MorseCode: Words", "%s", furi_string_get_cstr(instance->words));
}

static int32_t morse_code_worker_thread_callback(void* context) {
    furi_assert(context);
    MorseCodeWorker* instance = context;
    bool was_playing = false;
    uint32_t start_tick = 0;
    uint32_t end_tick = 0;
    bool pushed = true;
    bool spaced = true;
    while(instance->is_running) {
        furi_delay_ms(SLEEP);

        if(instance->play) {
            if(!was_playing) {
                start_tick = furi_get_tick();
                if(furi_hal_speaker_acquire(1000)) {
                    furi_hal_speaker_start(FREQUENCY, instance->volume);
                }
                was_playing = true;
            }
        } else {
            if(was_playing) {
                pushed = false;
                spaced = false;
                if(furi_hal_speaker_is_mine()) {
                    furi_hal_speaker_stop();
                    furi_hal_speaker_release();
                }
                end_tick = furi_get_tick();
                was_playing = false;
                morse_code_worker_fill_buffer(instance, end_tick - start_tick);
                start_tick = 0;
            }
        }
        if(!pushed) {
            if(end_tick + (instance->dit_delta * 3) < furi_get_tick()) {
                //NEW LETTER
                if(!furi_string_empty(instance->buffer)) {
                    morse_code_worker_fill_letter(instance);
                    if(instance->callback)
                        instance->callback(instance->words, instance->callback_context);
                } else {
                    spaced = true;
                }
                pushed = true;
            }
        }
        if(!spaced) {
            if(end_tick + (instance->dit_delta * 7) < furi_get_tick()) {
                //NEW WORD
                furi_string_push_back(instance->words, *SPACE);
                if(instance->callback)
                    instance->callback(instance->words, instance->callback_context);
                spaced = true;
            }
        }
    }
    return 0;
}

MorseCodeWorker* morse_code_worker_alloc() {
    MorseCodeWorker* instance = malloc(sizeof(MorseCodeWorker));
    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "MorseCodeWorker");
    furi_thread_set_stack_size(instance->thread, 1024);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, morse_code_worker_thread_callback);
    instance->play = false;
    instance->volume = 1.0f;
    instance->dit_delta = 150;
    instance->buffer = furi_string_alloc_set_str("");
    instance->words = furi_string_alloc_set_str("");
    return instance;
}

void morse_code_worker_free(MorseCodeWorker* instance) {
    furi_assert(instance);
    furi_string_free(instance->buffer);
    furi_string_free(instance->words);
    furi_thread_free(instance->thread);
    free(instance);
}

void morse_code_worker_set_callback(
    MorseCodeWorker* instance,
    MorseCodeWorkerCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void morse_code_worker_play(MorseCodeWorker* instance, bool play) {
    furi_assert(instance);
    instance->play = play;
}

void morse_code_worker_set_volume(MorseCodeWorker* instance, float level) {
    furi_assert(instance);
    instance->volume = level;
}

void morse_code_worker_set_dit_delta(MorseCodeWorker* instance, uint32_t delta) {
    furi_assert(instance);
    instance->dit_delta = delta;
}

void morse_code_worker_reset_text(MorseCodeWorker* instance) {
    furi_assert(instance);
    furi_string_reset(instance->buffer);
    furi_string_reset(instance->words);
}

void morse_code_worker_start(MorseCodeWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->is_running == false);
    instance->is_running = true;
    furi_thread_start(instance->thread);
    FURI_LOG_D("MorseCode: Start", "is Running");
}

void morse_code_worker_stop(MorseCodeWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->is_running == true);
    instance->play = false;
    instance->is_running = false;
    furi_thread_join(instance->thread);
    FURI_LOG_D("MorseCode: Stop", "Stop");
}
