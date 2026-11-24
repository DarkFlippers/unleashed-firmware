// Reading .sub files: RAW samples, decoded protocols turned into samples,
// custom buttons, jitter clean-up and the merge building blocks.

#include "app.h"

bool lr_read_line(LineReader* lr, FuriString* out) {
    furi_string_reset(out);
    bool any = false;
    while(true) {
        if(lr->pos >= lr->len) {
            if(lr->eof) break;

            lr->len = storage_file_read(lr->file, lr->buf, sizeof(lr->buf));
            lr->pos = 0;
            if(lr->len == 0) {
                lr->eof = true;
                break;
            }
        }

        char c = (char)lr->buf[lr->pos++];
        if(c == '\n') {
            any = true;
            break;
        }

        if(c == '\r') continue;

        furi_string_push_back(out, c);
        any = true;
    }

    return any;
}

void* safe_malloc(size_t size) {
    if(size == 0 || memmgr_get_free_heap() < size + LOAD_HEAP_RESERVE) return NULL;

    return malloc(size);
}

void* safe_realloc(void* ptr, size_t size) {
    if(size == 0 || memmgr_get_free_heap() < size + LOAD_HEAP_RESERVE) return NULL;

    return realloc(ptr, size);
}

// Most samples the RAM window may take: everything above LOAD_HEAP_RESERVE, which Decode and Save
// run in. The reserve has to stay contiguous too, so it is also kept back from the largest block
// the samples come out of; the rest of the heap may be only crumbs. Cut and Undo work on the
// working file, so they need no RAM of their own.
size_t sample_budget(void) {
    size_t free_heap = memmgr_get_free_heap();
    size_t block = memmgr_heap_get_max_free_block();
    if(free_heap <= LOAD_HEAP_RESERVE || block <= LOAD_HEAP_RESERVE) return 0;

    size_t usable = free_heap - LOAD_HEAP_RESERVE;
    if(usable > block - LOAD_HEAP_RESERVE) usable = block - LOAD_HEAP_RESERVE;

    return usable / sizeof(int16_t);
}

// The host tests (tests/) build this file without the Sub-GHz library: no decoded files there.
#ifndef HOST_TEST

/* Decoded protocols are synthesized by the firmware's own SubGhz encoder: the
 * key file is deserialized into a transmitter and we collect the level/duration
 * upload it would send on air. Every protocol the firmware supports works with
 * no per-protocol code here. Rolling codes (KeeLoq, Nice Flor-S, ...) bump and
 * re-encrypt their counter inside the encoder; custom firmware lets us zero that
 * step so the frame keeps its recorded counter, official firmware always yields
 * the NEXT one. The manufacturer keystores (system + user) are loaded so that
 * encryption works. */

SubGhzEnvironment* subghz_env_setup(void) {
    SubGhzEnvironment* env = subghz_environment_alloc();
    if(!env) return NULL;

    subghz_environment_set_protocol_registry(env, (void*)&subghz_protocol_registry);

    subghz_environment_set_came_atomo_rainbow_table_file_name(
        env, "/ext/subghz/assets/came_atomo");
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        env, "/ext/subghz/assets/alutech_at_4n");
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        env, "/ext/subghz/assets/nice_flor_s");

    return env;
}

static bool synth_grow(SubData* sd, size_t need) {
    if(sd->cap >= need) return true;

    size_t ncap = sd->cap ? sd->cap : 256;
    while(ncap < need)
        ncap *= 2;

    if(ncap > sd->limit) ncap = sd->limit;

    if(ncap < need) return false;

    int16_t* grown = safe_realloc(sd->data, ncap * sizeof(int16_t));
    if(!grown) {
        // What is already loaded stays usable, just truncated
        if(sd->count == 0) sd->out_of_memory = true;
        return false;
    }

    sd->data = grown;
    sd->cap = ncap;
    return true;
}

bool push_duration(SubData* sd, int32_t v) {
    int32_t sign = (v < 0) ? -1 : 1;
    int32_t mag = iabs32(v);

    do {
        int32_t chunk = (mag > DUR_CLAMP) ? DUR_CLAMP : mag;
        if(!synth_grow(sd, sd->count + 1)) {
            sd->truncated = true;
            return false;
        }
        sd->data[sd->count++] = (int16_t)(sign * chunk);
        mag -= chunk;
    } while(mag > 0);

    return true;
}

// How many int16 samples a duration of magnitude |mag| µs occupies once split to
// fit the per-sample clamp (the inverse count of what push_duration emits).
size_t duration_samples(int32_t mag) {
    return (mag > DUR_CLAMP) ? (size_t)((mag + DUR_CLAMP - 1) / DUR_CLAMP) : 1;
}

static FlipperFormat* fff_load_copy(Storage* storage, const char* path) {
    FlipperFormat* fff = flipper_format_string_alloc();
    if(!fff) return NULL;

    Stream* src = file_stream_alloc(storage);
    bool ok = false;

    if(file_stream_open(src, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        Stream* dst = flipper_format_get_raw_stream(fff);
        ok = stream_copy_full(src, dst) > 0;
        stream_rewind(dst);
    }

    stream_free(src);

    if(!ok) {
        flipper_format_free(fff);
        return NULL;
    }

    return fff;
}

/* ---- Custom buttons ---------------------------------------------------- */

uint8_t g_custom_btn_id = SUBGHZ_CUSTOM_BTN_OK;

#ifndef FW_ORIGIN_Official

static void btn_extract_label(const char* info, char* out, size_t outlen) {
    out[0] = '\0';

    const char* p = strstr(info, "Btn");
    if(!p) return;

    const char* eol = p;
    while(*eol && *eol != '\r' && *eol != '\n')
        eol++;

    const char* dash = strstr(p, " - ");
    const char* start;

    if(dash && dash < eol) {
        start = dash + 3;
    } else {
        start = strchr(p, ':');
        if(!start || start > eol) return;

        start++;
        while(*start == ' ')
            start++;
    }

    size_t n = 0;

    if(dash && dash < eol) {
        while(start[n] && start[n] != '\r' && start[n] != '\n' && n < outlen - 1)
            n++;

        memcpy(out, start, n);
        out[n] = '\0';
        return;
    }

    while(start[n] && start[n] != ' ' && start[n] != '\t' && start[n] != '\r' &&
          start[n] != '\n' && n < outlen - 1)
        n++;

    if(n == 0) return;

    if(n >= 2 && start[0] == '0' && (start[1] == 'x' || start[1] == 'X'))
        snprintf(out, outlen, "%.*s", (int)n, start);
    else
        snprintf(out, outlen, "0x%.*s", (int)n, start);
}

void btn_probe(Storage* storage, const char* path, const char* protocol, BtnPick* out) {
    memset(out, 0, sizeof(*out));
    strncpy(out->protocol, protocol, sizeof(out->protocol) - 1);

    SubGhzEnvironment* env = subghz_env_setup();
    if(!env) return;

    if(strstr(protocol, "KeeLoq")) {
        subghz_environment_load_keystore(env, "/ext/subghz/assets/keeloq_mfcodes");
        subghz_environment_load_keystore(env, "/ext/subghz/assets/keeloq_mfcodes_user");
    }

    SubGhzReceiver* rx = subghz_receiver_alloc_init(env);
    SubGhzProtocolDecoderBase* dec =
        rx ? subghz_receiver_search_decoder_base_by_name(rx, protocol) : NULL;

    subghz_custom_btns_reset();

    FuriString* info = furi_string_alloc();
    int named = 0;

    for(uint8_t slot = 0; slot < BTN_SLOTS; slot++) {
        if(slot > 0 && !subghz_custom_btn_set(slot)) continue;

        FlipperFormat* fff = fff_load_copy(storage, path);
        if(!fff) break;

        SubGhzTransmitter* tx = subghz_transmitter_alloc_init(env, protocol);
        bool encoded = tx && subghz_transmitter_deserialize(tx, fff) == SubGhzProtocolStatusOk;

        if(tx) subghz_transmitter_free(tx);

        if(slot == 0 && (!encoded || !subghz_custom_btn_is_allowed())) {
            flipper_format_free(fff);
            break;
        }

        if(encoded && dec && flipper_format_rewind(fff) &&
           subghz_protocol_decoder_base_deserialize(dec, fff) == SubGhzProtocolStatusOk) {
            furi_string_reset(info);
            subghz_protocol_decoder_base_get_string(dec, info);
            btn_extract_label(furi_string_get_cstr(info), out->label[slot], BTN_LABEL_LEN);
        }

        flipper_format_free(fff);

        if(out->label[slot][0]) named++;
    }

    furi_string_free(info);

    if(rx) subghz_receiver_free(rx);

    subghz_environment_free(env);

    out->available = named > 1;
    subghz_custom_btn_set(SUBGHZ_CUSTOM_BTN_OK);
}

#endif // FW_ORIGIN_Official

static bool synthesize_via_transmitter(
    Storage* storage,
    const char* path,
    const char* protocol,
    SubData* sd) {
    if(!protocol[0]) return false;

    SubGhzEnvironment* env = subghz_env_setup();
    if(!env) return false;

    if(strstr(protocol, "KeeLoq")) {
        subghz_environment_load_keystore(env, "/ext/subghz/assets/keeloq_mfcodes");
        subghz_environment_load_keystore(env, "/ext/subghz/assets/keeloq_mfcodes_user");
    }

#ifndef FW_ORIGIN_Official
    /* Rolling-code encoders add this multiplier to the counter before
     * re-encrypting. Zero loads the frame with the counter as recorded, not
     * the next one; the Sub-GHz app's setting is restored right after. */
    int32_t cnt_mult = furi_hal_subghz_get_rolling_counter_mult();
    furi_hal_subghz_set_rolling_counter_mult(0);

    if(g_custom_btn_id != SUBGHZ_CUSTOM_BTN_OK) {
        subghz_custom_btns_reset();

        FlipperFormat* prime = fff_load_copy(storage, path);
        if(prime) {
            SubGhzTransmitter* ptx = subghz_transmitter_alloc_init(env, protocol);
            if(ptx) {
                subghz_transmitter_deserialize(ptx, prime);
                subghz_transmitter_free(ptx);
            }

            flipper_format_free(prime);
        }

        subghz_custom_btn_set(g_custom_btn_id);
    }
#endif

    FlipperFormat* fff = fff_load_copy(storage, path);
    SubGhzTransmitter* tx = NULL;
    bool ok = false;

    do {
        if(!fff) break;

        tx = subghz_transmitter_alloc_init(env, protocol);
        if(!tx) break;

        if(subghz_transmitter_deserialize(tx, fff) != SubGhzProtocolStatusOk) break;

        sd->count = 0;
        for(;;) {
            LevelDuration ld = subghz_transmitter_yield(tx);
            if(level_duration_is_reset(ld)) break;

            int32_t dur = (int32_t)level_duration_get_duration(ld);
            int32_t v = level_duration_get_level(ld) ? dur : -dur;

            if(!push_duration(sd, v)) break; // out of room; keep what we have

            if(sd->count >= SYNTH_YIELD_CAP) break;
        }

        /* The encoder repeats the same upload `repeat` times. Find the true
         * period P (smallest P for which the whole buffer is P-periodic - this
         * rejects the short sub-period of a repetitive preamble like KeeLoq's,
         * unlike a header-only match) and keep ONE period plus the next frame's
         * header as its trailing sync. Without this, N duplicates bloat the
         * buffer and OOM on large frames (e.g. Nice Flor-S). */
        size_t n = sd->count;
        for(size_t p = 4; p * 2 <= n; p++) {
            bool periodic = true;
            for(size_t i = p; i < n; i++) {
                if(sd->data[i] != sd->data[i - p]) {
                    periodic = false;
                    break;
                }
            }

            if(periodic) {
                /* Keep one period. Append the next frame's first sample as a
                 * trailing delimiter only when it is a gap (gap-first protocols
                 * like Dooya/CAME, whose period otherwise ends on a pulse and
                 * leaves the last bit unframed). Pulse-first protocols (KeeLoq)
                 * already end the period on their inter-frame guard gap, so
                 * appending data[p] there would dangle a stray pulse. */
                sd->count = (sd->data[p] < 0) ? p + 1 : p;
                break;
            }
        }

        ok = sd->count > 0;
    } while(0);

    if(tx) subghz_transmitter_free(tx);

    if(fff) flipper_format_free(fff);

#ifndef FW_ORIGIN_Official
    furi_hal_subghz_set_rolling_counter_mult(cnt_mult);
#endif

    subghz_environment_free(env);
    return ok;
}

#endif // HOST_TEST

/* Snap each pulse to the nearest multiple of a base unit Te, removing timing
 * jitter and cumulative drift. Te is found by an iterative least-squares fit
 * |dur| ~ m*Te; gaps and noise (outside the data window) are left alone. */
bool g_normalize_jitter = false;

/* Manual silence separator inserted between independent signals (different
 * files) during a merge, in milliseconds. Repetitions of the same signal are
 * separated by the signal's own native gap instead. */
int32_t g_merge_gap_ms = MERGE_GAP_DEFAULT_MS;

/* How many times each loaded signal is repeated in the merged output. */
int32_t g_merge_repeat = MERGE_REPEAT_DEFAULT;

#define JITTER_DATA_MIN_US 60
#define JITTER_DATA_MAX_US 3000
#define JITTER_FIT_ITERS   8

/* The jitter clean-up snaps every data-length pulse to a whole multiple of the
 * signal's TE. jitter_te fits TE (0 when there is nothing to fit) on the RAM
 * window - the start of a signal bigger than RAM - and jitter_apply is then run
 * over the whole working file. */
float jitter_te(const int16_t* d, size_t n) {
    if(n < 8) return 0.0f;

    int64_t sum_all = 0;
    size_t n_all = 0;
    for(size_t i = 0; i < n; i++) {
        int32_t a = iabs32(d[i]);
        if(a >= JITTER_DATA_MIN_US && a <= JITTER_DATA_MAX_US) {
            sum_all += a;
            n_all++;
        }
    }

    if(n_all == 0) return 0.0f;

    int32_t mean_all = (int32_t)(sum_all / (int64_t)n_all);
    int64_t sum_short = 0;
    size_t n_short = 0;
    for(size_t i = 0; i < n; i++) {
        int32_t a = iabs32(d[i]);
        if(a >= JITTER_DATA_MIN_US && a <= mean_all) {
            sum_short += a;
            n_short++;
        }
    }

    float te = (n_short > 0) ? (float)sum_short / (float)n_short : (float)mean_all;
    if(te < 1.0f) return 0.0f;

    for(int it = 0; it < JITTER_FIT_ITERS; it++) {
        float num = 0.0f, den = 0.0f;
        for(size_t i = 0; i < n; i++) {
            int32_t a = iabs32(d[i]);
            if(a < JITTER_DATA_MIN_US || a > JITTER_DATA_MAX_US) continue;

            int m = (int)((float)a / te + 0.5f);
            if(m < 1) m = 1;

            num += (float)m * (float)a;
            den += (float)m * (float)m;
        }
        if(den <= 0.0f) return 0.0f;

        te = num / den;
    }

    return te;
}

void jitter_apply(int16_t* d, size_t n, float te) {
    for(size_t i = 0; i < n; i++) {
        int32_t a = iabs32(d[i]);
        if(a < JITTER_DATA_MIN_US || a > JITTER_DATA_MAX_US) continue;

        int m = (int)((float)a / te + 0.5f);
        if(m < 1) m = 1;

        int32_t mag = (int32_t)((float)m * te + 0.5f);
        if(mag > DUR_CLAMP) mag = DUR_CLAMP;

        d[i] = (int16_t)(d[i] < 0 ? -mag : mag);
    }
}

// Parse a .sub metadata line (Frequency/Preset/Protocol) into the given fields.
// Returns true if the line matched one of them; RAW_Data is handled by callers.
bool parse_sub_meta(
    const char* s,
    uint32_t* freq,
    char* preset,
    size_t preset_sz,
    char* protocol,
    size_t proto_sz) {
    if(strncmp(s, "Frequency:", 10) == 0) {
        *freq = (uint32_t)strtoul(s + 10, NULL, 10);
        return true;
    }

    if(strncmp(s, "Preset:", 7) == 0) {
        const char* p = s + 7;
        while(*p == ' ')
            p++;

        strncpy(preset, p, preset_sz - 1);
        preset[preset_sz - 1] = '\0';
        return true;
    }

    if(strncmp(s, "Protocol:", 9) == 0) {
        const char* p = s + 9;
        while(*p == ' ')
            p++;

        strncpy(protocol, p, proto_sz - 1);
        protocol[proto_sz - 1] = '\0';
        return true;
    }

    return false;
}

// Pull the next integer from a RAW_Data value list, advancing *p. Returns false
// at end of line. Skips stray non-numeric characters between values.
static bool next_raw_value(const char** p, long* out) {
    char* end;
    while(**p) {
        long v = strtol(*p, &end, 10);
        if(end == *p) {
            if(**p == '\0') break;

            (*p)++;
            continue;
        }

        *p = end;
        *out = v;

        return true;
    }

    return false;
}

/* ---- The working file ----
 * Every signal - a RAW capture, a synthesized decoded file, a repetition or a
 * merge - is written sample by sample into WORK_PATH, int16 like the RAM copy,
 * so a window of it is read back with one seek. */

static void work_flush(WorkWriter* w) {
    size_t bytes = w->n * sizeof(int16_t);
    if(w->n && storage_file_write(w->f, w->buf, bytes) != bytes) w->fail = true;

    w->n = 0;
}

static void work_put(WorkWriter* w, int32_t v) {
    if(w->skip_lead) {
        if(v < 0) return;

        w->skip_lead = false;
    }

    int32_t sign = (v < 0) ? -1 : 1;
    int32_t mag = iabs32(v);
    w->total += mag;
    if(v < 0) w->tail_us += mag;

    do {
        int32_t chunk = (mag > DUR_CLAMP) ? DUR_CLAMP : mag;
        if(w->n == WORK_CHUNK) work_flush(w);

        w->buf[w->n++] = (int16_t)(sign * chunk);
        w->count++;
        mag -= chunk;
    } while(mag > 0);

    if(v >= 0) {
        w->tail = w->count;
        w->tail_us = 0;
    }
}

WorkWriter* work_begin(Storage* st) {
    WorkWriter* w = safe_malloc(sizeof(WorkWriter));
    if(!w) return NULL;

    memset(w, 0, sizeof(*w));
    storage_simply_mkdir(st, WORK_DIR);
    w->f = storage_file_alloc(st);
    if(!storage_file_open(w->f, WORK_PATH, FSAM_READ_WRITE, FSOM_CREATE_ALWAYS)) w->fail = true;

    return w;
}

/* Prepare the join before the next signal so gaps don't stack: signals end and
 * start with their own guard silence.
 *  - Manual (a new file): the previous trailing silence becomes the configured gap.
 *  - Native (a repetition): the previous trailing silence stays as the gap.
 * Either way the appended signal's leading silence is dropped. */
static void work_join(WorkWriter* w, MergeJoin join) {
    w->skip_lead = false;
    if(join == MergeJoinManual) {
        if(g_merge_gap_ms) {
            work_flush(w);
            if(w->tail < w->count && !(storage_file_seek(w->f, w->tail * sizeof(int16_t), true) &&
                                       storage_file_truncate(w->f)))
                w->fail = true;

            w->count = w->tail;
            w->total -= w->tail_us;
            w->tail_us = 0;
            work_put(w, -(g_merge_gap_ms * 1000));
        }

        w->skip_lead = true;
    } else if(join == MergeJoinNative)
        w->skip_lead = w->tail < w->count;
}

// Write the RAW samples of a .sub file; protocol gets its Protocol: line.
static void work_raw(Storage* st, const char* path, WorkWriter* w, SubData* meta, char* protocol) {
    File* f = storage_file_alloc(st);
    if(storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FuriString* line = furi_string_alloc();
        furi_string_reserve(line, LINE_RESERVE);
        LineReader lr = {.file = f};
        while(!w->fail && lr_read_line(&lr, line)) {
            const char* s = furi_string_get_cstr(line);
            if(strncmp(s, "RAW_Data:", 9) == 0) {
                const char* p = s + 9;
                long v;
                while(next_raw_value(&p, &v))
                    work_put(w, (int32_t)v);
            } else {
                parse_sub_meta(
                    s, &meta->frequency, meta->preset, sizeof(meta->preset), protocol, 32);
            }
        }

        furi_string_free(line);
        storage_file_close(f);
    }

    storage_file_free(f);
}

/* Append path's signal g_merge_repeat times, joined to what is before it by
 * join and to itself natively. A decoded file is synthesized once and its
 * frame repeated, so a rolling code keeps one counter. */
bool work_add_sub(Storage* st, const char* path, WorkWriter* w, SubData* meta, MergeJoin join) {
    // After the join: its gap alone must not pass for RAW data of this file
    char protocol[32] = {0};
    work_join(w, join);
    size_t before = w->count;
    work_raw(st, path, w, meta, protocol);

    if(w->count > before) {
        for(int r = 1; r < g_merge_repeat && !w->fail; r++) {
            work_join(w, MergeJoinNative);
            work_raw(st, path, w, meta, protocol);
        }
    }
#ifndef HOST_TEST
    else if(protocol[0] && strcmp(protocol, "RAW") != 0) {
        SubData tmp = {.limit = sample_budget()};
        if(synthesize_via_transmitter(st, path, protocol, &tmp)) {
            meta->synthesized = true;
            for(int r = 0; r < g_merge_repeat && !w->fail; r++) {
                if(r) work_join(w, MergeJoinNative);

                for(size_t i = 0; i < tmp.count; i++)
                    work_put(w, tmp.data[i]);
            }
        }

        free(tmp.data);
    }
#endif

    return w->count > before && !w->fail;
}

// Close the working file; sd gets its sample count and length.
bool work_end(WorkWriter* w, SubData* sd) {
    work_flush(w);
    storage_file_close(w->f);
    storage_file_free(w->f);

    sd->file_count = w->count;
    sd->total_us = w->total > 0x7FFFFFFF ? 0x7FFFFFFF : (int32_t)w->total;
    bool ok = !w->fail && w->count >= 2;
    free(w);
    return ok;
}

#ifndef HOST_TEST
size_t count_sub_samples(
    Storage* storage,
    const char* path,
    uint32_t* freq,
    char* preset,
    size_t preset_sz) {
    File* f = storage_file_alloc(storage);
    if(!storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(f);
        return 0;
    }

    LineReader lr = {.file = f, .len = 0, .pos = 0, .eof = false};
    FuriString* line = furi_string_alloc();
    size_t raw_count = 0;
    char protocol[32] = {0};

    while(lr_read_line(&lr, line)) {
        const char* s = furi_string_get_cstr(line);
        if(strncmp(s, "RAW_Data:", 9) == 0) {
            const char* p = s + 9;
            long v;
            while(next_raw_value(&p, &v)) {
                // Must match work_put: durations beyond the clamp are split into
                // several samples, so count them the same way.
                raw_count += duration_samples(iabs32((int32_t)v));
            }
        } else {
            parse_sub_meta(s, freq, preset, preset_sz, protocol, sizeof(protocol));
        }
    }

    furi_string_free(line);
    storage_file_close(f);
    storage_file_free(f);

    if(raw_count > 0) return raw_count;

    /* Decoded protocol: the frame length isn't predictable from the file alone,
     * so synthesize it once via the firmware encoder and count the samples. */
    if(protocol[0] && strcmp(protocol, "RAW") != 0) {
        SubData tmp = {.limit = sample_budget()};
        size_t n = 0;
        if(synthesize_via_transmitter(storage, path, protocol, &tmp)) n = tmp.count;

        if(tmp.data) free(tmp.data);

        if(n > 0) return n;
    }

    return 0;
}
#endif
