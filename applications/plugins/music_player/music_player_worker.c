#include "music_player_worker.h"

#include <furi_hal.h>
#include <furi.h>

#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>

#include <m-array.h>

#define TAG "MusicPlayerWorker"

#define MUSIC_PLAYER_FILETYPE "Flipper Music Format"
#define MUSIC_PLAYER_VERSION 0

#define SEMITONE_PAUSE 0xFF

#define NOTE_C4 261.63f
#define NOTE_C4_SEMITONE (4.0f * 12.0f)
#define TWO_POW_TWELTH_ROOT 1.059463094359f

typedef struct {
    uint8_t semitone;
    uint8_t duration;
    uint8_t dots;
} NoteBlock;

ARRAY_DEF(NoteBlockArray, NoteBlock, M_POD_OPLIST);

struct MusicPlayerWorker {
    FuriThread* thread;
    bool should_work;

    MusicPlayerWorkerCallback callback;
    void* callback_context;

    float volume;
    uint32_t bpm;
    uint32_t duration;
    uint32_t octave;
    NoteBlockArray_t notes;
};

static int32_t music_player_worker_thread_callback(void* context) {
    furi_assert(context);
    MusicPlayerWorker* instance = context;

    NoteBlockArray_it_t it;
    NoteBlockArray_it(it, instance->notes);

    while(instance->should_work) {
        if(NoteBlockArray_end_p(it)) {
            NoteBlockArray_it(it, instance->notes);
            furi_delay_ms(10);
        } else {
            NoteBlock* note_block = NoteBlockArray_ref(it);

            float note_from_a4 = (float)note_block->semitone - NOTE_C4_SEMITONE;
            float frequency = NOTE_C4 * powf(TWO_POW_TWELTH_ROOT, note_from_a4);
            float duration =
                60.0 * furi_kernel_get_tick_frequency() * 4 / instance->bpm / note_block->duration;
            uint32_t dots = note_block->dots;
            while(dots > 0) {
                duration += duration / 2;
                dots--;
            }
            uint32_t next_tick = furi_get_tick() + duration;
            float volume = instance->volume;

            if(instance->callback) {
                instance->callback(
                    note_block->semitone,
                    note_block->dots,
                    note_block->duration,
                    0.0,
                    instance->callback_context);
            }

            furi_hal_speaker_stop();
            furi_hal_speaker_start(frequency, volume);
            while(instance->should_work && furi_get_tick() < next_tick) {
                volume *= 0.9945679;
                furi_hal_speaker_set_volume(volume);
                furi_delay_ms(2);
            }
            NoteBlockArray_next(it);
        }
    }

    furi_hal_speaker_stop();

    return 0;
}

MusicPlayerWorker* music_player_worker_alloc() {
    MusicPlayerWorker* instance = malloc(sizeof(MusicPlayerWorker));

    NoteBlockArray_init(instance->notes);

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "MusicPlayerWorker");
    furi_thread_set_stack_size(instance->thread, 1024);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, music_player_worker_thread_callback);

    instance->volume = 1.0f;

    return instance;
}

void music_player_worker_free(MusicPlayerWorker* instance) {
    furi_assert(instance);
    furi_thread_free(instance->thread);
    NoteBlockArray_clear(instance->notes);
    free(instance);
}

static bool is_digit(const char c) {
    return isdigit(c) != 0;
}

static bool is_letter(const char c) {
    return islower(c) != 0 || isupper(c) != 0;
}

static bool is_space(const char c) {
    return c == ' ' || c == '\t';
}

static size_t extract_number(const char* string, uint32_t* number) {
    size_t ret = 0;
    while(is_digit(*string)) {
        *number *= 10;
        *number += (*string - '0');
        string++;
        ret++;
    }
    return ret;
}

static size_t extract_dots(const char* string, uint32_t* number) {
    size_t ret = 0;
    while(*string == '.') {
        *number += 1;
        string++;
        ret++;
    }
    return ret;
}

static size_t extract_char(const char* string, char* symbol) {
    if(is_letter(*string)) {
        *symbol = *string;
        return 1;
    } else {
        return 0;
    }
}

static size_t extract_sharp(const char* string, char* symbol) {
    if(*string == '#' || *string == '_') {
        *symbol = '#';
        return 1;
    } else {
        return 0;
    }
}

static size_t skip_till(const char* string, const char symbol) {
    size_t ret = 0;
    while(*string != '\0' && *string != symbol) {
        string++;
        ret++;
    }
    if(*string != symbol) {
        ret = 0;
    }
    return ret;
}

static bool music_player_worker_add_note(
    MusicPlayerWorker* instance,
    uint8_t semitone,
    uint8_t duration,
    uint8_t dots) {
    NoteBlock note_block;

    note_block.semitone = semitone;
    note_block.duration = duration;
    note_block.dots = dots;

    NoteBlockArray_push_back(instance->notes, note_block);

    return true;
}

static int8_t note_to_semitone(const char note) {
    switch(note) {
    case 'C':
        return 0;
    // C#
    case 'D':
        return 2;
    // D#
    case 'E':
        return 4;
    case 'F':
        return 5;
    // F#
    case 'G':
        return 7;
    // G#
    case 'A':
        return 9;
    // A#
    case 'B':
        return 11;
    default:
        return 0;
    }
}

static bool music_player_worker_parse_notes(MusicPlayerWorker* instance, const char* string) {
    const char* cursor = string;
    bool result = true;

    while(*cursor != '\0') {
        if(!is_space(*cursor)) {
            uint32_t duration = 0;
            char note_char = '\0';
            char sharp_char = '\0';
            uint32_t octave = 0;
            uint32_t dots = 0;

            // Parsing
            cursor += extract_number(cursor, &duration);
            cursor += extract_char(cursor, &note_char);
            cursor += extract_sharp(cursor, &sharp_char);
            cursor += extract_number(cursor, &octave);
            cursor += extract_dots(cursor, &dots);

            // Post processing
            note_char = toupper(note_char);
            if(!duration) {
                duration = instance->duration;
            }
            if(!octave) {
                octave = instance->octave;
            }

            // Validation
            bool is_valid = true;
            is_valid &= (duration >= 1 && duration <= 128);
            is_valid &= ((note_char >= 'A' && note_char <= 'G') || note_char == 'P');
            is_valid &= (sharp_char == '#' || sharp_char == '\0');
            is_valid &= (octave <= 16);
            is_valid &= (dots <= 16);
            if(!is_valid) {
                FURI_LOG_E(
                    TAG,
                    "Invalid note: %u%c%c%u.%u",
                    duration,
                    note_char == '\0' ? '_' : note_char,
                    sharp_char == '\0' ? '_' : sharp_char,
                    octave,
                    dots);
                result = false;
                break;
            }

            // Note to semitones
            uint8_t semitone = 0;
            if(note_char == 'P') {
                semitone = SEMITONE_PAUSE;
            } else {
                semitone += octave * 12;
                semitone += note_to_semitone(note_char);
                semitone += sharp_char == '#' ? 1 : 0;
            }

            if(music_player_worker_add_note(instance, semitone, duration, dots)) {
                FURI_LOG_D(
                    TAG,
                    "Added note: %c%c%u.%u = %u %u",
                    note_char == '\0' ? '_' : note_char,
                    sharp_char == '\0' ? '_' : sharp_char,
                    octave,
                    dots,
                    semitone,
                    duration);
            } else {
                FURI_LOG_E(
                    TAG,
                    "Invalid note: %c%c%u.%u = %u %u",
                    note_char == '\0' ? '_' : note_char,
                    sharp_char == '\0' ? '_' : sharp_char,
                    octave,
                    dots,
                    semitone,
                    duration);
            }
            cursor += skip_till(cursor, ',');
        }

        if(*cursor != '\0') cursor++;
    }

    return result;
}

bool music_player_worker_load(MusicPlayerWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(file_path);

    bool ret = false;
    if(strcasestr(file_path, ".fmf")) {
        ret = music_player_worker_load_fmf_from_file(instance, file_path);
    } else {
        ret = music_player_worker_load_rtttl_from_file(instance, file_path);
    }
    return ret;
}

bool music_player_worker_load_fmf_from_file(MusicPlayerWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(file_path);

    bool result = false;
    string_t temp_str;
    string_init(temp_str);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_existing(file, file_path)) break;

        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(string_cmp_str(temp_str, MUSIC_PLAYER_FILETYPE) || (version != MUSIC_PLAYER_VERSION)) {
            FURI_LOG_E(TAG, "Incorrect file format or version");
            break;
        }

        if(!flipper_format_read_uint32(file, "BPM", &instance->bpm, 1)) {
            FURI_LOG_E(TAG, "BPM is missing");
            break;
        }
        if(!flipper_format_read_uint32(file, "Duration", &instance->duration, 1)) {
            FURI_LOG_E(TAG, "Duration is missing");
            break;
        }
        if(!flipper_format_read_uint32(file, "Octave", &instance->octave, 1)) {
            FURI_LOG_E(TAG, "Octave is missing");
            break;
        }

        if(!flipper_format_read_string(file, "Notes", temp_str)) {
            FURI_LOG_E(TAG, "Notes is missing");
            break;
        }

        if(!music_player_worker_parse_notes(instance, string_get_cstr(temp_str))) {
            break;
        }

        result = true;
    } while(false);

    furi_record_close(RECORD_STORAGE);
    flipper_format_free(file);
    string_clear(temp_str);

    return result;
}

bool music_player_worker_load_rtttl_from_file(MusicPlayerWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(file_path);

    bool result = false;
    string_t content;
    string_init(content);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Unable to open file");
            break;
        };

        uint16_t ret = 0;
        do {
            uint8_t buffer[65] = {0};
            ret = storage_file_read(file, buffer, sizeof(buffer) - 1);
            for(size_t i = 0; i < ret; i++) {
                string_push_back(content, buffer[i]);
            }
        } while(ret > 0);

        string_strim(content);
        if(!string_size(content)) {
            FURI_LOG_E(TAG, "Empty file");
            break;
        }

        if(!music_player_worker_load_rtttl_from_string(instance, string_get_cstr(content))) {
            FURI_LOG_E(TAG, "Invalid file content");
            break;
        }

        result = true;
    } while(0);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    string_clear(content);

    return result;
}

bool music_player_worker_load_rtttl_from_string(MusicPlayerWorker* instance, const char* string) {
    furi_assert(instance);

    const char* cursor = string;

    // Skip name
    cursor += skip_till(cursor, ':');
    if(*cursor != ':') {
        return false;
    }

    // Duration
    cursor += skip_till(cursor, '=');
    if(*cursor != '=') {
        return false;
    }
    cursor++;
    cursor += extract_number(cursor, &instance->duration);

    // Octave
    cursor += skip_till(cursor, '=');
    if(*cursor != '=') {
        return false;
    }
    cursor++;
    cursor += extract_number(cursor, &instance->octave);

    // BPM
    cursor += skip_till(cursor, '=');
    if(*cursor != '=') {
        return false;
    }
    cursor++;
    cursor += extract_number(cursor, &instance->bpm);

    // Notes
    cursor += skip_till(cursor, ':');
    if(*cursor != ':') {
        return false;
    }
    cursor++;
    if(!music_player_worker_parse_notes(instance, cursor)) {
        return false;
    }

    return true;
}

void music_player_worker_set_callback(
    MusicPlayerWorker* instance,
    MusicPlayerWorkerCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void music_player_worker_set_volume(MusicPlayerWorker* instance, float volume) {
    furi_assert(instance);
    instance->volume = volume;
}

void music_player_worker_start(MusicPlayerWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->should_work == false);

    instance->should_work = true;
    furi_thread_start(instance->thread);
}

void music_player_worker_stop(MusicPlayerWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->should_work == true);

    instance->should_work = false;
    furi_thread_join(instance->thread);
}
