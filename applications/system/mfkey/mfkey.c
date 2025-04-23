#pragma GCC optimize("O3")
#pragma GCC optimize("-funroll-all-loops")

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
// TODO: Static Encrypted: Minimum RAM for adding to keys dict (avoid crashes)
// TODO: Static Encrypted: Optimize KeysDict or buffer keys to write in chunks

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include "mfkey_icons.h"
#include <inttypes.h>
#include <toolbox/keys_dict.h>
#include <bit_lib/bit_lib.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <dolphin/dolphin.h>
#include <notification/notification_messages.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include "mfkey.h"
#include "crypto1.h"
#include "plugin_interface.h"
#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>

#define TAG "MFKey"

// TODO: Remove defines that are not needed
#define KEYS_DICT_SYSTEM_PATH EXT_PATH("nfc/assets/mf_classic_dict.nfc")
#define KEYS_DICT_USER_PATH   EXT_PATH("nfc/assets/mf_classic_dict_user.nfc")
#define MAX_NAME_LEN          32
#define MAX_PATH_LEN          64

#define LF_POLY_ODD  (0x29CE5C)
#define LF_POLY_EVEN (0x870804)
#define CONST_M1_1   (LF_POLY_EVEN << 1 | 1)
#define CONST_M2_1   (LF_POLY_ODD << 1)
#define CONST_M1_2   (LF_POLY_ODD)
#define CONST_M2_2   (LF_POLY_EVEN << 1 | 1)
#define BIT(x, n)    ((x) >> (n) & 1)
#define BEBIT(x, n)  BIT(x, (n) ^ 24)
#define SWAPENDIAN(x) \
    ((x) = ((x) >> 8 & 0xff00ff) | ((x) & 0xff00ff) << 8, (x) = (x) >> 16 | (x) << 16)
//#define SIZEOF(arr) sizeof(arr) / sizeof(*arr)

static int eta_round_time = 44;
static int eta_total_time = 705;
// MSB_LIMIT: Chunk size (out of 256)
static int MSB_LIMIT = 16;

static inline int
    check_state(struct Crypto1State* t, MfClassicNonce* n, ProgramState* program_state) {
    if(!(t->odd | t->even)) return 0;
    if(n->attack == mfkey32) {
        uint32_t rb = (napi_lfsr_rollback_word(t, 0, 0) ^ n->p64);
        if(rb != n->ar0_enc) {
            return 0;
        }
        rollback_word_noret(t, n->nr0_enc, 1);
        rollback_word_noret(t, n->uid_xor_nt0, 0);
        struct Crypto1State temp = {t->odd, t->even};
        crypt_word_noret(t, n->uid_xor_nt1, 0);
        crypt_word_noret(t, n->nr1_enc, 1);
        if(n->ar1_enc == (crypt_word(t) ^ n->p64b)) {
            crypto1_get_lfsr(&temp, &(n->key));
            return 1;
        }
    } else if(n->attack == static_nested) {
        struct Crypto1State temp = {t->odd, t->even};
        rollback_word_noret(t, n->uid_xor_nt1, 0);
        if(n->ks1_1_enc == crypt_word_ret(t, n->uid_xor_nt0, 0)) {
            rollback_word_noret(&temp, n->uid_xor_nt1, 0);
            crypto1_get_lfsr(&temp, &(n->key));
            return 1;
        }
    } else if(n->attack == static_encrypted) {
        // TODO: Parity bits from rollback_word?
        if(n->ks1_1_enc == napi_lfsr_rollback_word(t, n->uid_xor_nt0, 0)) {
            // Reduce with parity
            uint8_t local_parity_keystream_bits;
            struct Crypto1State temp = {t->odd, t->even};
            if((crypt_word_par(&temp, n->uid_xor_nt0, 0, n->nt0, &local_parity_keystream_bits) ==
                n->ks1_1_enc) &&
               (local_parity_keystream_bits == n->par_1)) {
                // Found key candidate
                crypto1_get_lfsr(t, &(n->key));
                program_state->num_candidates++;
                keys_dict_add_key(program_state->cuid_dict, n->key.data, sizeof(MfClassicKey));
            }
        }
    }
    return 0;
}

static inline int state_loop(
    unsigned int* states_buffer,
    int xks,
    int m1,
    int m2,
    unsigned int in,
    uint8_t and_val) {
    int states_tail = 0;
    int round = 0, s = 0, xks_bit = 0, round_in = 0;

    for(round = 1; round <= 12; round++) {
        xks_bit = BIT(xks, round);
        if(round > 4) {
            round_in = ((in >> (2 * (round - 4))) & and_val) << 24;
        }

        for(s = 0; s <= states_tail; s++) {
            states_buffer[s] <<= 1;

            if((filter(states_buffer[s]) ^ filter(states_buffer[s] | 1)) != 0) {
                states_buffer[s] |= filter(states_buffer[s]) ^ xks_bit;
                if(round > 4) {
                    update_contribution(states_buffer, s, m1, m2);
                    states_buffer[s] ^= round_in;
                }
            } else if(filter(states_buffer[s]) == xks_bit) {
                // TODO: Refactor
                if(round > 4) {
                    states_buffer[++states_tail] = states_buffer[s + 1];
                    states_buffer[s + 1] = states_buffer[s] | 1;
                    update_contribution(states_buffer, s, m1, m2);
                    states_buffer[s++] ^= round_in;
                    update_contribution(states_buffer, s, m1, m2);
                    states_buffer[s] ^= round_in;
                } else {
                    states_buffer[++states_tail] = states_buffer[++s];
                    states_buffer[s] = states_buffer[s - 1] | 1;
                }
            } else {
                states_buffer[s--] = states_buffer[states_tail--];
            }
        }
    }

    return states_tail;
}

int binsearch(unsigned int data[], int start, int stop) {
    int mid, val = data[stop] & 0xff000000;
    while(start != stop) {
        mid = (stop - start) >> 1;
        if((data[start + mid] ^ 0x80000000) > (val ^ 0x80000000))
            stop = start + mid;
        else
            start += mid + 1;
    }
    return start;
}
void quicksort(unsigned int array[], int low, int high) {
    //if (SIZEOF(array) == 0)
    //    return;
    if(low >= high) return;
    int middle = low + (high - low) / 2;
    unsigned int pivot = array[middle];
    int i = low, j = high;
    while(i <= j) {
        while(array[i] < pivot) {
            i++;
        }
        while(array[j] > pivot) {
            j--;
        }
        if(i <= j) { // swap
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            i++;
            j--;
        }
    }
    if(low < j) {
        quicksort(array, low, j);
    }
    if(high > i) {
        quicksort(array, i, high);
    }
}
int extend_table(unsigned int data[], int tbl, int end, int bit, int m1, int m2, unsigned int in) {
    in <<= 24;
    for(data[tbl] <<= 1; tbl <= end; data[++tbl] <<= 1) {
        if((filter(data[tbl]) ^ filter(data[tbl] | 1)) != 0) {
            data[tbl] |= filter(data[tbl]) ^ bit;
            update_contribution(data, tbl, m1, m2);
            data[tbl] ^= in;
        } else if(filter(data[tbl]) == bit) {
            data[++end] = data[tbl + 1];
            data[tbl + 1] = data[tbl] | 1;
            update_contribution(data, tbl, m1, m2);
            data[tbl++] ^= in;
            update_contribution(data, tbl, m1, m2);
            data[tbl] ^= in;
        } else {
            data[tbl--] = data[end--];
        }
    }
    return end;
}

int old_recover(
    unsigned int odd[],
    int o_head,
    int o_tail,
    int oks,
    unsigned int even[],
    int e_head,
    int e_tail,
    int eks,
    int rem,
    int s,
    MfClassicNonce* n,
    unsigned int in,
    int first_run,
    ProgramState* program_state) {
    int o, e, i;
    if(rem == -1) {
        for(e = e_head; e <= e_tail; ++e) {
            even[e] = (even[e] << 1) ^ evenparity32(even[e] & LF_POLY_EVEN) ^ (!!(in & 4));
            for(o = o_head; o <= o_tail; ++o, ++s) {
                struct Crypto1State temp = {0, 0};
                temp.even = odd[o];
                temp.odd = even[e] ^ evenparity32(odd[o] & LF_POLY_ODD);
                if(check_state(&temp, n, program_state)) {
                    return -1;
                }
            }
        }
        return s;
    }
    if(first_run == 0) {
        for(i = 0; (i < 4) && (rem-- != 0); i++) {
            oks >>= 1;
            eks >>= 1;
            in >>= 2;
            o_tail = extend_table(
                odd, o_head, o_tail, oks & 1, LF_POLY_EVEN << 1 | 1, LF_POLY_ODD << 1, 0);
            if(o_head > o_tail) return s;
            e_tail = extend_table(
                even, e_head, e_tail, eks & 1, LF_POLY_ODD, LF_POLY_EVEN << 1 | 1, in & 3);
            if(e_head > e_tail) return s;
        }
    }
    first_run = 0;
    quicksort(odd, o_head, o_tail);
    quicksort(even, e_head, e_tail);
    while(o_tail >= o_head && e_tail >= e_head) {
        if(((odd[o_tail] ^ even[e_tail]) >> 24) == 0) {
            o_tail = binsearch(odd, o_head, o = o_tail);
            e_tail = binsearch(even, e_head, e = e_tail);
            s = old_recover(
                odd,
                o_tail--,
                o,
                oks,
                even,
                e_tail--,
                e,
                eks,
                rem,
                s,
                n,
                in,
                first_run,
                program_state);
            if(s == -1) {
                break;
            }
        } else if((odd[o_tail] ^ 0x80000000) > (even[e_tail] ^ 0x80000000)) {
            o_tail = binsearch(odd, o_head, o_tail) - 1;
        } else {
            e_tail = binsearch(even, e_head, e_tail) - 1;
        }
    }
    return s;
}

static inline int sync_state(ProgramState* program_state) {
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

int calculate_msb_tables(
    int oks,
    int eks,
    int msb_round,
    MfClassicNonce* n,
    unsigned int* states_buffer,
    struct Msb* odd_msbs,
    struct Msb* even_msbs,
    unsigned int* temp_states_odd,
    unsigned int* temp_states_even,
    unsigned int in,
    ProgramState* program_state) {
    //FURI_LOG_I(TAG, "MSB GO %i", msb_iter); // DEBUG
    unsigned int msb_head = (MSB_LIMIT * msb_round); // msb_iter ranges from 0 to (256/MSB_LIMIT)-1
    unsigned int msb_tail = (MSB_LIMIT * (msb_round + 1));
    int states_tail = 0, tail = 0;
    int i = 0, j = 0, semi_state = 0, found = 0;
    unsigned int msb = 0;
    in = ((in >> 16 & 0xff) | (in << 16) | (in & 0xff00)) << 1;
    // TODO: Why is this necessary?
    memset(odd_msbs, 0, MSB_LIMIT * sizeof(struct Msb));
    memset(even_msbs, 0, MSB_LIMIT * sizeof(struct Msb));

    for(semi_state = 1 << 20; semi_state >= 0; semi_state--) {
        if(semi_state % 32768 == 0) {
            if(sync_state(program_state) == 1) {
                return 0;
            }
        }

        if(filter(semi_state) == (oks & 1)) { //-V547
            states_buffer[0] = semi_state;
            states_tail = state_loop(states_buffer, oks, CONST_M1_1, CONST_M2_1, 0, 0);

            for(i = states_tail; i >= 0; i--) {
                msb = states_buffer[i] >> 24;
                if((msb >= msb_head) && (msb < msb_tail)) {
                    found = 0;
                    for(j = 0; j < odd_msbs[msb - msb_head].tail - 1; j++) {
                        if(odd_msbs[msb - msb_head].states[j] == states_buffer[i]) {
                            found = 1;
                            break;
                        }
                    }

                    if(!found) {
                        tail = odd_msbs[msb - msb_head].tail++;
                        odd_msbs[msb - msb_head].states[tail] = states_buffer[i];
                    }
                }
            }
        }

        if(filter(semi_state) == (eks & 1)) { //-V547
            states_buffer[0] = semi_state;
            states_tail = state_loop(states_buffer, eks, CONST_M1_2, CONST_M2_2, in, 3);

            for(i = 0; i <= states_tail; i++) {
                msb = states_buffer[i] >> 24;
                if((msb >= msb_head) && (msb < msb_tail)) {
                    found = 0;

                    for(j = 0; j < even_msbs[msb - msb_head].tail; j++) {
                        if(even_msbs[msb - msb_head].states[j] == states_buffer[i]) {
                            found = 1;
                            break;
                        }
                    }

                    if(!found) {
                        tail = even_msbs[msb - msb_head].tail++;
                        even_msbs[msb - msb_head].states[tail] = states_buffer[i];
                    }
                }
            }
        }
    }

    oks >>= 12;
    eks >>= 12;

    for(i = 0; i < MSB_LIMIT; i++) {
        if(sync_state(program_state) == 1) {
            return 0;
        }
        // TODO: Why is this necessary?
        memset(temp_states_even, 0, sizeof(unsigned int) * (1280));
        memset(temp_states_odd, 0, sizeof(unsigned int) * (1280));
        memcpy(temp_states_odd, odd_msbs[i].states, odd_msbs[i].tail * sizeof(unsigned int));
        memcpy(temp_states_even, even_msbs[i].states, even_msbs[i].tail * sizeof(unsigned int));
        int res = old_recover(
            temp_states_odd,
            0,
            odd_msbs[i].tail,
            oks,
            temp_states_even,
            0,
            even_msbs[i].tail,
            eks,
            3,
            0,
            n,
            in >> 16,
            1,
            program_state);
        if(res == -1) {
            return 1;
        }
        //odd_msbs[i].tail = 0;
        //even_msbs[i].tail = 0;
    }

    return 0;
}

void** allocate_blocks(const size_t* block_sizes, int num_blocks) {
    void** block_pointers = malloc(num_blocks * sizeof(void*));

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
    }

    return block_pointers;
}

bool is_full_speed() {
    return MSB_LIMIT == 16;
}

bool recover(MfClassicNonce* n, int ks2, unsigned int in, ProgramState* program_state) {
    bool found = false;
    const size_t block_sizes[] = {49216, 49216, 5120, 5120, 4096};
    const size_t reduced_block_sizes[] = {24608, 24608, 5120, 5120, 4096};
    const int num_blocks = sizeof(block_sizes) / sizeof(block_sizes[0]);
    void** block_pointers = allocate_blocks(block_sizes, num_blocks);
    if(block_pointers == NULL) {
        // System has less than the guaranteed amount of RAM (140 KB) - adjust some parameters to run anyway at half speed
        if(is_full_speed()) {
            //eta_round_time *= 2;
            eta_total_time *= 2;
            MSB_LIMIT /= 2;
        }
        block_pointers = allocate_blocks(reduced_block_sizes, num_blocks);
        if(block_pointers == NULL) {
            // System has less than 70 KB of RAM - should never happen so we don't reduce speed further
            program_state->err = InsufficientRAM;
            program_state->mfkey_state = Error;
            return false;
        }
    }
    // Adjust estimates for static encrypted attacks
    if(n->attack == static_encrypted) {
        eta_round_time *= 4;
        eta_total_time *= 4;
        if(is_full_speed()) {
            eta_round_time *= 4;
            eta_total_time *= 4;
        }
    }
    struct Msb* odd_msbs = block_pointers[0];
    struct Msb* even_msbs = block_pointers[1];
    unsigned int* temp_states_odd = block_pointers[2];
    unsigned int* temp_states_even = block_pointers[3];
    unsigned int* states_buffer = block_pointers[4];
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
        if(calculate_msb_tables(
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
            //int bench_stop = furi_hal_rtc_get_timestamp();
            //FURI_LOG_I(TAG, "Cracked in %i seconds", bench_stop - bench_start);
            found = true;
            break;
        }
        if(program_state->close_thread_please) {
            break;
        }
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
    uint32_t i = 0, j = 0;
    //FURI_LOG_I(TAG, "Free heap before alloc(): %zub", memmgr_get_free_heap());
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
    //FURI_LOG_I(TAG, "Free heap after free(): %zub", memmgr_get_free_heap());
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
        //FURI_LOG_I(TAG, "Beginning recovery for %8lx", next_nonce.uid);
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
            break;
        }

        if(!recover(&next_nonce, ks_enc, nt_xor_uid, program_state)) {
            if((next_nonce.attack == static_encrypted) && (program_state->cuid_dict)) {
                keys_dict_free(program_state->cuid_dict);
            }
            if(program_state->close_thread_please) {
                break;
            }
            // No key found in recover() or static encrypted
            (program_state->num_completed)++;
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
            keyarray = realloc(keyarray, sizeof(MfClassicKey) * (keyarray_size + 1)); //-V701
            keyarray_size += 1;
            keyarray[keyarray_size - 1] = found_key;
            (program_state->unique_cracked)++;
        }
    }
    // TODO: Update display to show all keys were found
    // TODO: Prepend found key(s) to user dictionary file
    //FURI_LOG_I(TAG, "Unique keys found:");
    for(i = 0; i < keyarray_size; i++) {
        //FURI_LOG_I(TAG, "%012" PRIx64, keyarray[i]);
        keys_dict_add_key(user_dict, keyarray[i].data, sizeof(MfClassicKey));
    }
    if(keyarray_size > 0) {
        dolphin_deed(DolphinDeedNfcMfcAdd);
    }
    free(nonce_arr);
    keys_dict_free(user_dict);
    free(keyarray);
    if(program_state->mfkey_state == Error) {
        return;
    }
    //FURI_LOG_I(TAG, "mfkey function completed normally"); // DEBUG
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
        furi_thread_alloc_ex("MFKeyWorker", 2048, mfkey_worker_thread, program_state);

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
