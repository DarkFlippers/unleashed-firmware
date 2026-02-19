#pragma GCC optimize("O3")

// TODO: More efficient dictionary bruteforce by scanning through hardcoded very common keys and previously found dictionary keys first?
//       (a cache for key_already_found_for_nonce_in_dict)
// TODO: Selectively unroll loops to reduce binary size
// TODO: Collect parity during Mfkey32 attacks to further optimize the attack
// TODO: Why different sscanf between Mfkey32 and Nested?
// TODO: "Read tag again with NFC app" message upon completion, "Complete. Keys added: <n>"
// TODO: Separate Mfkey32 and Nested functions where possible to reduce branch statements
// TODO: Find ~1 KB memory leak
// TODO: Use seednt16 to reduce static encrypted key candidates: https://gist.github.com/noproto/8102f8f32546564cd674256e62ff76ea
//       https://eprint.iacr.org/2024/1275.pdf section X

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include "mfkey_icons.h"
#include <inttypes.h>
#include <string.h>
#include <toolbox/keys_dict.h>
#include <bit_lib/bit_lib.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <dolphin/dolphin.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include "mfkey.h"
#include "crypto1.h"
#include "mfkey_attack.h"
#include "plugin_interface.h"
#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>

#define TAG "MFKey"

// TODO: Remove defines that are not needed
#define KEYS_DICT_SYSTEM_PATH          EXT_PATH("nfc/assets/mf_classic_dict.nfc")
#define KEYS_DICT_USER_PATH            EXT_PATH("nfc/assets/mf_classic_dict_user.nfc")
#define MAX_NAME_LEN                   32
#define MAX_PATH_LEN                   64
#define STATIC_ENCRYPTED_RAM_THRESHOLD 4096

#define LF_POLY_ODD  (0x29CE5C)
#define LF_POLY_EVEN (0x870804)

#define CONST_M1_1  (LF_POLY_EVEN << 1 | 1)
#define CONST_M2_1  (LF_POLY_ODD << 1)
#define CONST_M1_2  (LF_POLY_ODD)
#define CONST_M2_2  (LF_POLY_EVEN << 1 | 1)
#define BIT(x, n)   ((x) >> (n) & 1)
#define BEBIT(x, n) BIT(x, (n) ^ 24)
#define SWAP(a, b)          \
    do {                    \
        unsigned int t = a; \
        a = b;              \
        b = t;              \
    } while(0)
#define SWAPENDIAN(x) \
    ((x) = ((x) >> 8 & 0xff00ff) | ((x) & 0xff00ff) << 8, (x) = (x) >> 16 | (x) << 16)
// #define SIZEOF(arr) sizeof(arr) / sizeof(*arr)

// Reduced to 16-bit as these values are small and don't need 32-bit
static int16_t eta_round_time = 30;
static int16_t eta_total_time = 481;
// MSB_LIMIT: Chunk size (out of 256) - can be 8-bit as it's a small value
// Not static - referenced by mfkey_attack.c
uint8_t MSB_LIMIT = 16;

// Not static - referenced by mfkey_attack.c for static_encrypted attacks
void flush_key_buffer(ProgramState* program_state) {
    if(program_state->key_buffer && program_state->key_buffer_count > 0 &&
       program_state->cuid_dict) {
        // Pre-allocate exact size needed: 2 hex chars (key_idx) + 12 hex chars (key) + 1 newline per key
        size_t total_size = program_state->key_buffer_count * 15;
        char* batch_buffer = malloc(total_size + 1); // +1 for null terminator

        char* ptr = batch_buffer;
        const char hex_chars[] = "0123456789ABCDEF";

        for(size_t i = 0; i < program_state->key_buffer_count; i++) {
            // Write key_idx as 2 hex chars
            uint8_t key_idx = program_state->key_idx_buffer[i];
            *ptr++ = hex_chars[key_idx >> 4];
            *ptr++ = hex_chars[key_idx & 0x0F];

            // Convert key to hex string directly into buffer
            for(size_t j = 0; j < sizeof(MfClassicKey); j++) {
                uint8_t byte = program_state->key_buffer[i].data[j];
                *ptr++ = hex_chars[byte >> 4];
                *ptr++ = hex_chars[byte & 0x0F];
            }
            *ptr++ = '\n';
        }
        *ptr = '\0';

        // Write all keys at once by directly accessing the stream
        Stream* stream = program_state->cuid_dict->stream;
        uint32_t actual_pos = stream_tell(stream);

        if(stream_seek(stream, 0, StreamOffsetFromEnd) &&
           stream_write(stream, (uint8_t*)batch_buffer, total_size) == total_size) {
            // Update total key count
            program_state->cuid_dict->total_keys += program_state->key_buffer_count;
        }

        // May not be needed
        stream_seek(stream, actual_pos, StreamOffsetFromStart);
        free(batch_buffer);
        program_state->key_buffer_count = 0;
    }
}

// Not static - referenced by mfkey_attack.c
int sync_state(ProgramState* program_state) {
    int ts = furi_hal_rtc_get_timestamp();
    int elapsed_time = ts - program_state->eta_timestamp;
    if(elapsed_time < program_state->eta_round) {
        program_state->eta_round -= elapsed_time;
    } else {
        program_state->eta_round = 0;
    }
    if(elapsed_time < program_state->eta_total) {
        program_state->eta_total -= elapsed_time;
    } else {
        program_state->eta_total = 0;
    }
    program_state->eta_timestamp = ts;
    if(program_state->close_thread_please) {
        return 1;
    }
    return 0;
}

void** allocate_blocks(const size_t* block_sizes, int num_blocks) {
    void** block_pointers = malloc(num_blocks * sizeof(void*));
    if(!block_pointers) {
        return NULL;
    }

    for(int i = 0; i < num_blocks; i++) {
        if(memmgr_heap_get_max_free_block() < block_sizes[i]) {
            // Not enough memory, free previously allocated blocks
            for(int j = 0; j < i; j++) {
                free(block_pointers[j]);
            }
            free(block_pointers);
            return NULL;
        }

        block_pointers[i] = malloc(block_sizes[i]);
        if(!block_pointers[i]) {
            // Allocation failed
            for(int j = 0; j < i; j++) {
                free(block_pointers[j]);
            }
            free(block_pointers);
            return NULL;
        }
    }

    return block_pointers;
}

bool recover(MfClassicNonce* n, int ks2, unsigned int in, ProgramState* program_state) {
    bool found = false;
    // Packed 24-bit Msb format: 16 buckets × 2312 bytes each = 36992
    // states_buffer needs 1024 elements × 4 bytes = 4096
    const size_t block_sizes[] = {36992, 36992, 5120, 5120, 4096};
    // Reduced: 8 buckets × 2312 bytes each = 18496
    const size_t reduced_block_sizes[] = {18496, 18496, 5120, 5120, 4096};
    const int num_blocks = sizeof(block_sizes) / sizeof(block_sizes[0]);
    // Reset globals each nonce
    eta_round_time = 30;
    eta_total_time = 481;
    MSB_LIMIT = 16;

    // Use half speed (reduced block sizes) for static encrypted nonces so we can buffer keys
    bool use_half_speed = (n->attack == static_encrypted);
    if(use_half_speed) {
        //eta_round_time *= 2;
        eta_total_time *= 2;
        MSB_LIMIT /= 2;
    }

    void** block_pointers =
        allocate_blocks(use_half_speed ? reduced_block_sizes : block_sizes, num_blocks);
    if(block_pointers == NULL) {
        if(n->attack != static_encrypted) {
            // System has less than the guaranteed amount of RAM (140 KB) - adjust some parameters to run anyway at half speed
            // eta_round_time *= 2;
            eta_total_time *= 2;
            MSB_LIMIT /= 2;
            block_pointers = allocate_blocks(reduced_block_sizes, num_blocks);
            if(block_pointers == NULL) {
                // System has less than 70 KB of RAM - should never happen so we don't reduce speed further
                program_state->err = InsufficientRAM;
                program_state->mfkey_state = Error;
                return false;
            }
        } else {
            program_state->err = InsufficientRAM;
            program_state->mfkey_state = Error;
            return false;
        }
    }
    struct Msb* odd_msbs = block_pointers[0];
    struct Msb* even_msbs = block_pointers[1];
    unsigned int* temp_states_odd = block_pointers[2];
    unsigned int* temp_states_even = block_pointers[3];
    unsigned int* states_buffer = block_pointers[4];

    // Allocate key buffer for static encrypted nonces
    if(n->attack == static_encrypted) {
        size_t available_ram = memmgr_heap_get_max_free_block();
        // Each key becomes 2 hex chars (key_idx) + 12 hex chars (key) + 1 newline = 15 bytes in the batch string
        // Plus original 6 bytes (key) + 1 byte (key_idx) in buffer = 22 bytes total per key
        // Add extra safety margin for string overhead and other allocations
        const size_t safety_threshold = STATIC_ENCRYPTED_RAM_THRESHOLD;
        const size_t bytes_per_key =
            sizeof(MfClassicKey) + sizeof(uint8_t) + 15; // buffer + string representation
        if(available_ram > safety_threshold) {
            program_state->key_buffer_size = (available_ram - safety_threshold) / bytes_per_key;
            program_state->key_buffer =
                malloc(program_state->key_buffer_size * sizeof(MfClassicKey));
            program_state->key_idx_buffer =
                malloc(program_state->key_buffer_size * sizeof(uint8_t));
            program_state->key_buffer_count = 0;
            if(!program_state->key_buffer || !program_state->key_idx_buffer) {
                // Free the allocated blocks before returning
                for(int i = 0; i < num_blocks; i++) {
                    free(block_pointers[i]);
                }
                free(block_pointers);
                program_state->err = InsufficientRAM;
                program_state->mfkey_state = Error;
                return false;
            }
        } else {
            // Free the allocated blocks before returning
            for(int i = 0; i < num_blocks; i++) {
                free(block_pointers[i]);
            }
            free(block_pointers);
            program_state->err = InsufficientRAM;
            program_state->mfkey_state = Error;
            return false;
        }
    } else {
        program_state->key_buffer = NULL;
        program_state->key_idx_buffer = NULL;
        program_state->key_buffer_size = 0;
        program_state->key_buffer_count = 0;
    }

    int oks = 0, eks = 0;
    int i = 0, msb = 0;
    for(i = 31; i >= 0; i -= 2) {
        oks = oks << 1 | BEBIT(ks2, i);
    }
    for(i = 30; i >= 0; i -= 2) {
        eks = eks << 1 | BEBIT(ks2, i);
    }
    int bench_start = furi_hal_rtc_get_timestamp();
    program_state->eta_total = eta_total_time;
    program_state->eta_timestamp = bench_start;
    for(msb = 0; msb <= ((256 / MSB_LIMIT) - 1); msb++) {
        program_state->search = msb;
        program_state->eta_round = eta_round_time;
        program_state->eta_total = eta_total_time - (eta_round_time * msb);
        if(calculate_msb_tables_optimized(
               oks,
               eks,
               msb,
               n,
               states_buffer,
               odd_msbs,
               even_msbs,
               temp_states_odd,
               temp_states_even,
               in,
               program_state)) {
            found = true;
            break;
        }
        if(program_state->close_thread_please) {
            break;
        }
    }

    // Final flush and cleanup for key buffer
    if(n->attack == static_encrypted && program_state->key_buffer) {
        flush_key_buffer(program_state);
        free(program_state->key_buffer);
        free(program_state->key_idx_buffer);
        program_state->key_buffer = NULL;
        program_state->key_idx_buffer = NULL;
        program_state->key_buffer_size = 0;
        program_state->key_buffer_count = 0;
    }

    // Free the allocated blocks
    for(int i = 0; i < num_blocks; i++) {
        free(block_pointers[i]);
    }
    free(block_pointers);
    return found;
}

bool key_already_found_for_nonce_in_solved(
    MfClassicKey* keyarray,
    int keyarray_size,
    MfClassicNonce* nonce) {
    for(int k = 0; k < keyarray_size; k++) {
        uint64_t key_as_int = bit_lib_bytes_to_num_be(keyarray[k].data, sizeof(MfClassicKey));
        struct Crypto1State temp = {0, 0};
        for(int i = 0; i < 24; i++) {
            (&temp)->odd |= (BIT(key_as_int, 2 * i + 1) << (i ^ 3));
            (&temp)->even |= (BIT(key_as_int, 2 * i) << (i ^ 3));
        }
        if(nonce->attack == mfkey32) {
            crypt_word_noret(&temp, nonce->uid_xor_nt1, 0);
            crypt_word_noret(&temp, nonce->nr1_enc, 1);
            if(nonce->ar1_enc == (crypt_word(&temp) ^ nonce->p64b)) {
                return true;
            }
        } else if(nonce->attack == static_nested) {
            uint32_t expected_ks1 = crypt_word_ret(&temp, nonce->uid_xor_nt0, 0);
            if(nonce->ks1_1_enc == expected_ks1) {
                return true;
            }
        }
    }
    return false;
}

#pragma GCC push_options
#pragma GCC optimize("Os")
static void finished_beep() {
    // Beep to indicate completion
    NotificationApp* notification = furi_record_open("notification");
    notification_message(notification, &sequence_audiovisual_alert);
    notification_message(notification, &sequence_display_backlight_on);
    furi_record_close("notification");
}

void mfkey(ProgramState* program_state) {
    uint32_t ks_enc = 0, nt_xor_uid = 0;
    MfClassicKey found_key; // Recovered key
    size_t keyarray_size = 0;
    MfClassicKey* keyarray = malloc(sizeof(MfClassicKey) * 1);
    if(!keyarray) {
        program_state->err = InsufficientRAM;
        program_state->mfkey_state = Error;
        return;
    }

    uint32_t i = 0, j = 0;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);
    flipper_application_preload(app, APP_ASSETS_PATH("plugins/mfkey_init_plugin.fal"));
    flipper_application_map_to_memory(app);
    const FlipperAppPluginDescriptor* app_descriptor =
        flipper_application_plugin_get_descriptor(app);
    const MfkeyPlugin* init_plugin = app_descriptor->entry_point;
    // Check for nonces
    program_state->mfkey32_present = init_plugin->napi_mf_classic_mfkey32_nonces_check_presence();
    program_state->nested_present = init_plugin->napi_mf_classic_nested_nonces_check_presence();
    if(!(program_state->mfkey32_present) && !(program_state->nested_present)) {
        program_state->err = MissingNonces;
        program_state->mfkey_state = Error;
        flipper_application_free(app);
        furi_record_close(RECORD_STORAGE);
        free(keyarray);
        return;
    }
    // Read dictionaries (optional)
    KeysDict* system_dict = {0};
    bool system_dict_exists = keys_dict_check_presence(KEYS_DICT_SYSTEM_PATH);
    KeysDict* user_dict = {0};
    bool user_dict_exists = keys_dict_check_presence(KEYS_DICT_USER_PATH);
    uint32_t total_dict_keys = 0;
    if(system_dict_exists) {
        system_dict =
            keys_dict_alloc(KEYS_DICT_SYSTEM_PATH, KeysDictModeOpenExisting, sizeof(MfClassicKey));
        total_dict_keys += keys_dict_get_total_keys(system_dict);
    }
    user_dict = keys_dict_alloc(KEYS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));
    if(user_dict_exists) {
        total_dict_keys += keys_dict_get_total_keys(user_dict);
    }
    user_dict_exists = true;
    program_state->dict_count = total_dict_keys;
    program_state->mfkey_state = DictionaryAttack;
    // Read nonces
    MfClassicNonceArray* nonce_arr;
    nonce_arr = init_plugin->napi_mf_classic_nonce_array_alloc(
        system_dict, system_dict_exists, user_dict, program_state);
    if(system_dict_exists) {
        keys_dict_free(system_dict);
    }
    if(nonce_arr->total_nonces == 0) {
        // Nothing to crack
        program_state->err = ZeroNonces;
        program_state->mfkey_state = Error;
        init_plugin->napi_mf_classic_nonce_array_free(nonce_arr);
        flipper_application_free(app);
        furi_record_close(RECORD_STORAGE);
        keys_dict_free(user_dict);
        free(keyarray);
        return;
    }
    flipper_application_free(app);
    furi_record_close(RECORD_STORAGE);
    // TODO: Track free state at the time this is called to ensure double free does not happen
    furi_assert(nonce_arr);
    furi_assert(nonce_arr->stream);
    // TODO: Already closed?
    buffered_file_stream_close(nonce_arr->stream);
    stream_free(nonce_arr->stream);
    program_state->mfkey_state = MFKeyAttack;
    // TODO: Work backwards on this array and free memory
    for(i = 0; i < nonce_arr->total_nonces; i++) {
        MfClassicNonce next_nonce = nonce_arr->remaining_nonce_array[i];
        if(key_already_found_for_nonce_in_solved(keyarray, keyarray_size, &next_nonce)) {
            nonce_arr->remaining_nonces--;
            (program_state->cracked)++;
            (program_state->num_completed)++;
            continue;
        }
        FuriString* cuid_dict_path;
        switch(next_nonce.attack) {
        case mfkey32:
            ks_enc = next_nonce.ar0_enc ^ next_nonce.p64;
            nt_xor_uid = 0;
            break;
        case static_nested:
            ks_enc = next_nonce.ks1_2_enc;
            nt_xor_uid = next_nonce.uid_xor_nt1;
            break;
        case static_encrypted:
            ks_enc = next_nonce.ks1_1_enc;
            nt_xor_uid = next_nonce.uid_xor_nt0;
            cuid_dict_path = furi_string_alloc_printf(
                "%s/mf_classic_dict_%08lx.nfc", EXT_PATH("nfc/assets"), next_nonce.uid);
            // May need RECORD_STORAGE?
            program_state->cuid_dict = keys_dict_alloc(
                furi_string_get_cstr(cuid_dict_path),
                KeysDictModeOpenAlways,
                sizeof(MfClassicKey));
            furi_string_free(cuid_dict_path);
            break;
        }

        if(!recover(&next_nonce, ks_enc, nt_xor_uid, program_state)) {
            // Check for non-recoverable errors and break the loop
            if(program_state->mfkey_state == Error) {
                if((next_nonce.attack == static_encrypted) && (program_state->cuid_dict)) {
                    keys_dict_free(program_state->cuid_dict);
                    program_state->cuid_dict = NULL;
                }
                break;
            }
            if(program_state->close_thread_please) {
                if((next_nonce.attack == static_encrypted) && (program_state->cuid_dict)) {
                    keys_dict_free(program_state->cuid_dict);
                    program_state->cuid_dict = NULL;
                }
                break;
            }
            // No key found in recover() or static encrypted
            (program_state->num_completed)++;
            // Free CUID dictionary after each static_encrypted nonce processing
            if((next_nonce.attack == static_encrypted) && (program_state->cuid_dict)) {
                keys_dict_free(program_state->cuid_dict);
                program_state->cuid_dict = NULL;
            }
            continue;
        }
        (program_state->cracked)++;
        (program_state->num_completed)++;
        found_key = next_nonce.key;
        bool already_found = false;
        for(j = 0; j < keyarray_size; j++) {
            if(memcmp(keyarray[j].data, found_key.data, MF_CLASSIC_KEY_SIZE) == 0) {
                already_found = true;
                break;
            }
        }
        if(already_found == false) {
            // New key
            MfClassicKey* new_keyarray =
                realloc(keyarray, sizeof(MfClassicKey) * (keyarray_size + 1));
            if(!new_keyarray) {
                // Realloc failed - continue with existing keyarray
                FURI_LOG_E(TAG, "Failed to realloc keyarray");
            } else {
                keyarray = new_keyarray;
                keyarray_size += 1;
                keyarray[keyarray_size - 1] = found_key;
                (program_state->unique_cracked)++;
            }
        }
    }
    // TODO: Update display to show all keys were found
    // TODO: Prepend found key(s) to user dictionary file
    for(i = 0; i < keyarray_size; i++) {
        keys_dict_add_key(user_dict, keyarray[i].data, sizeof(MfClassicKey));
    }
    if(keyarray_size > 0) {
        dolphin_deed(DolphinDeedNfcKeyAdd);
    }
    free(nonce_arr);
    keys_dict_free(user_dict);
    free(keyarray);
    if(program_state->mfkey_state == Error) {
        return;
    }
    program_state->mfkey_state = Complete;
    // No need to alert the user if they asked it to stop
    if(!(program_state->close_thread_please)) {
        finished_beep();
    }
    return;
}

// Screen is 128x64 px
static void render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    ProgramState* program_state = ctx;
    furi_mutex_acquire(program_state->mutex, FuriWaitForever);
    char draw_str[44] = {};

    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_draw_frame(canvas, 0, 15, 128, 64);

    // FontSecondary by default, title is drawn at the end
    snprintf(draw_str, sizeof(draw_str), "RAM: %zub", memmgr_get_free_heap());
    canvas_draw_str_aligned(canvas, 48, 5, AlignLeft, AlignTop, draw_str);
    canvas_draw_icon(canvas, 114, 4, &I_mfkey);
    if(program_state->mfkey_state == MFKeyAttack) {
        float eta_round = (float)1 - ((float)program_state->eta_round / (float)eta_round_time);
        float eta_total = (float)1 - ((float)program_state->eta_total / (float)eta_total_time);
        float progress = (float)program_state->num_completed / (float)program_state->total;
        if(eta_round < 0 || eta_round > 1) {
            // Round ETA miscalculated
            eta_round = 1;
            program_state->eta_round = 0;
        }
        if(eta_total < 0 || eta_round > 1) {
            // Total ETA miscalculated
            eta_total = 1;
            program_state->eta_total = 0;
        }
        snprintf(
            draw_str,
            sizeof(draw_str),
            "Cracking: %d/%d - in prog.",
            program_state->num_completed,
            program_state->total);
        elements_progress_bar_with_text(canvas, 5, 18, 118, progress, draw_str);
        snprintf(
            draw_str,
            sizeof(draw_str),
            "Round: %d/%d - ETA %02d Sec",
            (program_state->search) + 1, // Zero indexed
            256 / MSB_LIMIT,
            program_state->eta_round);
        elements_progress_bar_with_text(canvas, 5, 31, 118, eta_round, draw_str);
        snprintf(draw_str, sizeof(draw_str), "Total ETA %03d Sec", program_state->eta_total);
        elements_progress_bar_with_text(canvas, 5, 44, 118, eta_total, draw_str);
    } else if(program_state->mfkey_state == DictionaryAttack) {
        snprintf(
            draw_str, sizeof(draw_str), "Dict solves: %d (in progress)", program_state->cracked);
        canvas_draw_str_aligned(canvas, 10, 18, AlignLeft, AlignTop, draw_str);
        snprintf(draw_str, sizeof(draw_str), "Keys in dict: %d", program_state->dict_count);
        canvas_draw_str_aligned(canvas, 26, 28, AlignLeft, AlignTop, draw_str);
    } else if(program_state->mfkey_state == Complete) {
        // TODO: Scrollable list view to see cracked keys if user presses down
        elements_progress_bar(canvas, 5, 18, 118, 1);
        canvas_draw_str_aligned(canvas, 64, 31, AlignCenter, AlignTop, "Complete");
        snprintf(
            draw_str,
            sizeof(draw_str),
            "Keys added to user dict: %d",
            program_state->unique_cracked);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignTop, draw_str);
        if(program_state->num_candidates > 0) {
            snprintf(
                draw_str,
                sizeof(draw_str),
                "SEN key candidates: %d",
                program_state->num_candidates);
            canvas_draw_str_aligned(canvas, 64, 51, AlignCenter, AlignTop, draw_str);
        }
    } else if(program_state->mfkey_state == Ready) {
        canvas_draw_str_aligned(canvas, 50, 30, AlignLeft, AlignTop, "Ready");
        elements_button_center(canvas, "Start");
        elements_button_right(canvas, "Help");
    } else if(program_state->mfkey_state == Help) {
        canvas_draw_str_aligned(canvas, 7, 20, AlignLeft, AlignTop, "Collect nonces by reading");
        canvas_draw_str_aligned(canvas, 7, 30, AlignLeft, AlignTop, "tag or reader in NFC app:");
        canvas_draw_str_aligned(canvas, 7, 40, AlignLeft, AlignTop, "https://docs.flipper.net/");
        canvas_draw_str_aligned(canvas, 7, 50, AlignLeft, AlignTop, "nfc/mfkey32");
    } else if(program_state->mfkey_state == Error) {
        canvas_draw_str_aligned(canvas, 50, 25, AlignLeft, AlignTop, "Error");
        if(program_state->err == MissingNonces) {
            canvas_draw_str_aligned(canvas, 25, 36, AlignLeft, AlignTop, "No nonces found");
        } else if(program_state->err == ZeroNonces) {
            canvas_draw_str_aligned(canvas, 15, 36, AlignLeft, AlignTop, "Nonces already cracked");
        } else if(program_state->err == InsufficientRAM) {
            canvas_draw_str_aligned(canvas, 35, 36, AlignLeft, AlignTop, "No free RAM");
        } else {
            // Unhandled error
        }
    } else {
        // Unhandled program state
    }
    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 5, 4, AlignLeft, AlignTop, "MFKey");
    furi_mutex_release(program_state->mutex);
}

static void input_callback(InputEvent* input_event, void* event_queue) {
    furi_assert(event_queue);
    furi_message_queue_put((FuriMessageQueue*)event_queue, input_event, FuriWaitForever);
}

static void mfkey_state_init(ProgramState* program_state) {
    program_state->mfkey_state = Ready;
    program_state->cracked = 0;
    program_state->unique_cracked = 0;
    program_state->num_completed = 0;
    program_state->num_candidates = 0;
    program_state->total = 0;
    program_state->dict_count = 0;
}

// Entrypoint for worker thread
static int32_t mfkey_worker_thread(void* ctx) {
    ProgramState* program_state = ctx;
    program_state->mfkey_state = Initializing;
    mfkey(program_state);
    return 0;
}

int32_t mfkey_main() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ProgramState* program_state = malloc(sizeof(ProgramState));

    mfkey_state_init(program_state);

    program_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, program_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    program_state->mfkeythread =
        furi_thread_alloc_ex("MFKeyWorker", 4096, mfkey_worker_thread, program_state);

    InputEvent input_event;
    for(bool main_loop = true; main_loop;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &input_event, 100);

        furi_mutex_acquire(program_state->mutex, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            if(input_event.type == InputTypePress) {
                switch(input_event.key) {
                case InputKeyRight:
                    if(program_state->mfkey_state == Ready) {
                        program_state->mfkey_state = Help;
                    }
                    break;
                case InputKeyOk:
                    if(program_state->mfkey_state == Ready) {
                        furi_thread_start(program_state->mfkeythread);
                    }
                    break;
                case InputKeyBack:
                    if(program_state->mfkey_state == Help) {
                        program_state->mfkey_state = Ready;
                    } else {
                        program_state->close_thread_please = true;
                        // Wait until thread is finished
                        furi_thread_join(program_state->mfkeythread);
                        main_loop = false;
                    }
                    break;
                default:
                    break;
                }
            }
        }

        furi_mutex_release(program_state->mutex);
        view_port_update(view_port);
    }

    // Thread joined in back event handler
    furi_thread_free(program_state->mfkeythread);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(program_state->mutex);
    free(program_state);

    return 0;
}
#pragma GCC pop_options
