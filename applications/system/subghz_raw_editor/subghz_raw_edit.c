/*
 * Sub-GHz RAW Edit - a tiny waveform editor for Flipper Zero RAW .sub captures.
 * https://github.com/Lechnio/SubGHz-RAW-Edit
 */

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
#include <gui/modules/number_input.h>
#include <gui/modules/variable_item_list.h>

#include <lib/toolbox/level_duration.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/toolbox/stream/file_stream.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/flipper_format/flipper_format_i.h>
#include <lib/subghz/environment.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/subghz_protocol_registry.h>
#ifndef FW_ORIGIN_Official
#include <lib/subghz/blocks/custom_btn.h>
#include <furi_hal_subghz.h>
#else
// Official firmware has no custom buttons: always send the recorded button.
#define SUBGHZ_CUSTOM_BTN_OK 0
#endif

#include "subghz_raw_edit_icons.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUBGHZ_DIR        "/ext/subghz"
#define MAX_SAMPLES       24000
#define SYNTH_YIELD_CAP   4096
#define DUR_CLAMP         32000
#define LOAD_HEAP_RESERVE 12288

#define GAP_KEEP_MAX_US 1500

#define MERGE_GAP_DEFAULT_MS 100
#define MERGE_GAP_MIN_MS     0
// A single sample is int16_t clamped to DUR_CLAMP (32000 us = 32 ms), so gaps
// longer than that are emitted as several consecutive silence samples.
#define MERGE_GAP_MAX_MS     1000
#define MERGE_REPEAT_DEFAULT 1
#define MERGE_REPEAT_MIN     1
#define MERGE_REPEAT_MAX     64
#define STR_(x)              #x
#define STR(x)               STR_(x)
#define MERGE_PATH_LEN       128
#define MERGE_BUF_CHUNK      512

#define APP_VERSION "1.9"
#define APP_REPO    "github.com/Lechnio/SubGHz-RAW-Edit"
#define APP_NAME    "Sub-GHz RAW Edit"

#define SCREEN_W_PX  128
#define SCREEN_H_PX  64
#define FONT_SIZE_PX 7
#define GAP_PX       1

#define TOP_TEXT_BASE FONT_SIZE_PX
#define OV_Y          (TOP_TEXT_BASE + GAP_PX)
#define OV_H          5
#define WAVE_H        32
#define WAVE_TOP      (OV_Y + OV_H + 1)
#define WAVE_BOT      (WAVE_TOP + WAVE_H)
#define SEP_Y         (WAVE_BOT + 1)
#define LINE1_BASE    (SEP_Y + 1 + GAP_PX + FONT_SIZE_PX)
#define LINE2_BASE    (LINE1_BASE + 1 + FONT_SIZE_PX)

static_assert(LINE2_BASE <= SCREEN_H_PX);

#define MENU_SEL_SAVE   0
#define MENU_SEL_DECODE 1
#define MENU_SEL_CUT    2
#define MENU_SEL_UNDO   3
#define MENU_ITEMS      4

#define DECODE_MAX          32
#define DECODE_TAIL_US      100000
#define DECODE_LEAD_MIN_US  2000
#define DECODE_LEAD_MAX_US  DUR_CLAMP
#define DECODE_LEAD_US      18500
#define DECODE_HEAP_RESERVE 4096
#define DECODE_KEYS_HEAP    32768

typedef struct {
    int16_t* data;
    size_t count;
    size_t cap;
    int32_t total_us;
    uint32_t frequency;
    char preset[48];
    bool truncated;
    bool out_of_memory;
    bool synthesized;
} SubData;

typedef enum {
    EditModeEdit = 0,
    EditModeMenu = 1,
} EditMode;

typedef struct {
    SubData sd;
    char basename[48];
    bool src_in_dir; // the open file is SUBGHZ_DIR/<basename>.sub

    int32_t view_start;
    int32_t view_end;
    int32_t marker_a;
    int32_t marker_b;
    int active;

    uint16_t activity[SCREEN_W_PX];
    uint16_t act_max;
    uint16_t overview[SCREEN_W_PX];
    uint16_t ov_max;
    bool wave_mode;

    char status[80];
    uint32_t status_until;

    bool loading;
    bool saving;

    int mode;
    int menu_sel;

    int16_t* undo_data;
    size_t undo_count;
    int32_t undo_total_us;
    int32_t undo_marker_a;
    int32_t undo_marker_b;
    int undo_active;
    int32_t undo_view_start;
    int32_t undo_view_end;
    bool has_undo;

    FuriMutex* mutex;
} App;

static inline int32_t iabs32(int32_t v) {
    return v < 0 ? -v : v;
}

static inline int32_t clamp_dur(int32_t v) {
    if(v > DUR_CLAMP) return DUR_CLAMP;

    if(v < -DUR_CLAMP) return -DUR_CLAMP;

    return v;
}

static void fmt_time(int32_t us, char* out, size_t n) {
    if(us < 0) us = 0;

    if(us < 1000) {
        snprintf(out, n, "%luus", (unsigned long)us);
    } else if(us < 1000000) {
        long ms = us / 1000;
        long t = (us % 1000) / 100;
        snprintf(out, n, "%ld.%ldms", ms, t);
    } else {
        long s = us / 1000000;
        long h = (us % 1000000) / 10000;
        snprintf(out, n, "%ld.%02lds", s, h);
    }
}

static int time_to_x(App* a, int32_t t) {
    int32_t span = a->view_end - a->view_start;
    if(span <= 0) span = 1;

    return (int)(((int64_t)(t - a->view_start) * SCREEN_W_PX) / span);
}

static void clamp_marker(App* a, int idx) {
    int32_t* m = idx ? &a->marker_b : &a->marker_a;
    if(*m < 0) *m = 0;
    if(*m > a->sd.total_us) *m = a->sd.total_us;
}

static void clamp_view(App* a) {
    int32_t total = a->sd.total_us;
    if(total < 1) total = 1;

    int32_t margin = total / 3 + 50000;
    int32_t lo = -margin;
    int32_t hi = total + margin;
    int32_t fullspan = hi - lo;

    int32_t span = a->view_end - a->view_start;
    if(span < 200) span = 200;

    if(span > fullspan) span = fullspan;

    if(a->view_start < lo) a->view_start = lo;

    a->view_end = a->view_start + span;

    if(a->view_end > hi) {
        a->view_end = hi;
        a->view_start = hi - span;
        if(a->view_start < lo) a->view_start = lo;
    }
}

static void ensure_visible(App* a, int32_t m) {
    int32_t span = a->view_end - a->view_start;
    if(m < a->view_start) {
        a->view_start = m - span / 5;
        a->view_end = a->view_start + span;
    } else if(m > a->view_end) {
        a->view_end = m + span / 5;
        a->view_start = a->view_end - span;
    }

    clamp_view(a);
}

// True when sample i is a real level change (an edge). The first sample is an
// edge only if it starts high; runs of same-sign samples - e.g. a long gap
// split into several silence samples - are one level, not repeated edges.
static bool sample_is_edge(const SubData* sd, size_t i) {
    return (i == 0) ? (sd->data[i] > 0) : ((sd->data[i] < 0) != (sd->data[i - 1] < 0));
}

static void recompute_activity(App* a) {
    memset(a->activity, 0, sizeof(a->activity));
    int32_t span = a->view_end - a->view_start;
    if(span <= 0) span = 1;

    a->wave_mode = (span <= 30000);

    int32_t run = 0;
    uint16_t mx = 0;
    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;

        if(s1 < a->view_start || s0 > a->view_end) continue;

        if(!sample_is_edge(&a->sd, i)) continue;

        int x = (int)(((int64_t)(s0 - a->view_start) * SCREEN_W_PX) / span);

        if(x < 0) x = 0;

        if(x > SCREEN_W_PX - 1) x = SCREEN_W_PX - 1;

        if(a->activity[x] < 65535) a->activity[x]++;

        if(a->activity[x] > mx) mx = a->activity[x];
    }

    a->act_max = mx ? mx : 1;
}

static void recompute_overview(App* a) {
    memset(a->overview, 0, sizeof(a->overview));
    int32_t total = a->sd.total_us;
    if(total < 1) total = 1;

    int32_t run = 0;
    uint16_t mx = 0;
    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;

        if(!sample_is_edge(&a->sd, i)) continue;

        int x = (int)(((int64_t)s0 * SCREEN_W_PX) / total);

        if(x < 0) x = 0;

        if(x > SCREEN_W_PX - 1) x = SCREEN_W_PX - 1;

        if(a->overview[x] < 65535) a->overview[x]++;

        if(a->overview[x] > mx) mx = a->overview[x];
    }

    a->ov_max = mx ? mx : 1;
}

static void auto_detect(App* a) {
    int32_t total = a->sd.total_us;

    if(a->sd.synthesized) {
        a->marker_a = 0;
        a->marker_b = total;
        a->view_start = 0;
        a->view_end = total > 0 ? total : 1;
        clamp_view(a);

        return;
    }

    if(total < 1 || a->sd.count < 2) {
        a->view_start = 0;
        a->view_end = total > 0 ? total : 1;
        a->marker_a = total / 4;
        a->marker_b = (total * 3) / 4;

        return;
    }

    const int32_t GAP_US = 15000;
    const int32_t MIN_FRAME_US = 30000;

    int32_t run = 0;
    int32_t seg_start = -1;
    int32_t last_edge = 0;
    int32_t best_a = -1, best_b = -1;
    int32_t best_len = 0;
    int32_t best_gap = 0;

    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;

        if(ad >= GAP_US) {
            if(seg_start >= 0) {
                int32_t seglen = last_edge - seg_start;
                if(seglen >= MIN_FRAME_US && seglen > best_len) {
                    best_len = seglen;
                    best_a = seg_start;
                    best_b = last_edge;
                    best_gap = ad;
                }

                seg_start = -1;
            }
        } else {
            if(seg_start < 0) seg_start = s0;

            last_edge = s1;
        }
    }

    if(seg_start >= 0) {
        int32_t seglen = last_edge - seg_start;
        if(seglen >= MIN_FRAME_US && seglen > best_len) {
            best_a = seg_start;
            best_b = last_edge;
            best_gap = 0;
        }
    }

    if(best_a < 0) {
        a->marker_a = 0;
        a->marker_b = total;
        a->view_start = 0;
        a->view_end = total;
        clamp_view(a);

        return;
    }

    /* Extend B into the trailing inter-frame gap so the auto-selected frame
     * keeps the end-of-frame silence that decoders (KeeLoq, ...) require to
     * finalize a frame. Without it the selection ends on the last bit and the
     * saved capture won't decode (it had to be widened by hand). Any positive
     * extension is enough: write_selection emits the whole gap sample once the
     * marker lands inside it. Limit it to GAP_KEEP_MAX_US so the marker/view
     * stay tidy and never reach into the next frame. */
    int32_t gap_keep = best_gap;
    if(gap_keep > GAP_KEEP_MAX_US) gap_keep = GAP_KEEP_MAX_US;

    int32_t pad = (best_b - best_a) / 6 + 500;
    a->marker_a = best_a;
    a->marker_b = best_b + gap_keep;
    a->view_start = best_a - pad * 3;
    a->view_end = a->marker_b + pad * 3;
    clamp_view(a);
}

typedef struct {
    File* file;
    uint8_t buf[128];
    size_t len;
    size_t pos;
    bool eof;
} LineReader;

static bool lr_read_line(LineReader* lr, FuriString* out) {
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

static bool append_sample(SubData* sd, int32_t v) {
    if(sd->count >= sd->cap) {
        sd->truncated = true;
        return false;
    }

    sd->data[sd->count++] = (int16_t)clamp_dur(v);

    return true;
}

static void* safe_malloc(size_t size) {
    if(size == 0 || memmgr_get_free_heap() < size + LOAD_HEAP_RESERVE) return NULL;

    return malloc(size);
}

static void* safe_realloc(void* ptr, size_t size) {
    if(size == 0 || memmgr_get_free_heap() < size + LOAD_HEAP_RESERVE) return NULL;

    return realloc(ptr, size);
}

/* Decoded protocols are synthesized by the firmware's own SubGhz encoder: the
 * key file is deserialized into a transmitter and we collect the level/duration
 * upload it would send on air. Every protocol the firmware supports works with
 * no per-protocol code here. Rolling codes (KeeLoq, Nice Flor-S, ...) bump and
 * re-encrypt their counter inside the encoder; custom firmware lets us zero that
 * step so the frame keeps its recorded counter, official firmware always yields
 * the NEXT one. The manufacturer keystores (system + user) are loaded so that
 * encryption works. */

static SubGhzEnvironment* subghz_env_setup(void) {
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

    if(ncap > MAX_SAMPLES) ncap = MAX_SAMPLES;

    if(ncap < need) return false;

    int16_t* grown = safe_realloc(sd->data, ncap * sizeof(int16_t));
    if(!grown) {
        sd->out_of_memory = true;
        return false;
    }

    sd->data = grown;
    sd->cap = ncap;
    return true;
}

static bool push_duration(SubData* sd, int32_t v) {
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
static size_t duration_samples(int32_t mag) {
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

static uint8_t g_custom_btn_id = SUBGHZ_CUSTOM_BTN_OK;

#ifndef FW_ORIGIN_Official

#define BTN_SLOTS     5
#define BTN_LABEL_LEN 20

typedef struct {
    bool available;
    char protocol[32];
    char label[BTN_SLOTS][BTN_LABEL_LEN];
} BtnPick;

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

static void btn_probe(Storage* storage, const char* path, const char* protocol, BtnPick* out) {
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

/* Snap each pulse to the nearest multiple of a base unit Te, removing timing
 * jitter and cumulative drift. Te is found by an iterative least-squares fit
 * |dur| ~ m*Te; gaps and noise (outside the data window) are left alone. */
static bool g_normalize_jitter = false;

/* Manual silence separator inserted between independent signals (different
 * files) during a merge, in milliseconds. Repetitions of the same signal are
 * separated by the signal's own native gap instead. */
static int32_t g_merge_gap_ms = MERGE_GAP_DEFAULT_MS;

/* How many times each loaded signal is repeated in the merged output. */
static int32_t g_merge_repeat = MERGE_REPEAT_DEFAULT;

#define JITTER_DATA_MIN_US 60
#define JITTER_DATA_MAX_US 3000
#define JITTER_FIT_ITERS   8

static void normalize_jitter(SubData* sd) {
    if(!sd || !sd->data || sd->count < 8) return;

    int64_t sum_all = 0;
    size_t n_all = 0;
    for(size_t i = 0; i < sd->count; i++) {
        int32_t a = iabs32(sd->data[i]);
        if(a >= JITTER_DATA_MIN_US && a <= JITTER_DATA_MAX_US) {
            sum_all += a;
            n_all++;
        }
    }

    if(n_all == 0) return;

    int32_t mean_all = (int32_t)(sum_all / (int64_t)n_all);
    int64_t sum_short = 0;
    size_t n_short = 0;
    for(size_t i = 0; i < sd->count; i++) {
        int32_t a = iabs32(sd->data[i]);
        if(a >= JITTER_DATA_MIN_US && a <= mean_all) {
            sum_short += a;
            n_short++;
        }
    }

    float te = (n_short > 0) ? (float)sum_short / (float)n_short : (float)mean_all;
    if(te < 1.0f) return;

    for(int it = 0; it < JITTER_FIT_ITERS; it++) {
        float num = 0.0f, den = 0.0f;
        for(size_t i = 0; i < sd->count; i++) {
            int32_t a = iabs32(sd->data[i]);
            if(a < JITTER_DATA_MIN_US || a > JITTER_DATA_MAX_US) continue;

            int m = (int)((float)a / te + 0.5f);
            if(m < 1) m = 1;

            num += (float)m * (float)a;
            den += (float)m * (float)m;
        }
        if(den <= 0.0f) return;

        te = num / den;
    }

    for(size_t i = 0; i < sd->count; i++) {
        int32_t a = iabs32(sd->data[i]);
        if(a < JITTER_DATA_MIN_US || a > JITTER_DATA_MAX_US) continue;

        int m = (int)((float)a / te + 0.5f);
        if(m < 1) m = 1;

        int32_t mag = (int32_t)((float)m * te + 0.5f);
        if(mag > DUR_CLAMP) mag = DUR_CLAMP;

        sd->data[i] = (int16_t)(sd->data[i] < 0 ? -mag : mag);
    }
}

// Sum of all sample durations, saturated to int32, cached on the SubData.
static void recompute_total_us(SubData* sd) {
    int64_t run = 0;
    for(size_t i = 0; i < sd->count; i++)
        run += iabs32(sd->data[i]);

    if(run > 0x7FFFFFFF) run = 0x7FFFFFFF;

    sd->total_us = (int32_t)run;
}

// Parse a .sub metadata line (Frequency/Preset/Protocol) into the given fields.
// Returns true if the line matched one of them; RAW_Data is handled by callers.
static bool parse_sub_meta(
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

static bool load_sub(Storage* storage, const char* path, SubData* sd) {
    memset(sd, 0, sizeof(*sd));
    sd->frequency = 433920000;
    strncpy(sd->preset, "FuriHalSubGhzPresetOok650Async", sizeof(sd->preset) - 1);

    File* f = storage_file_alloc(storage);
    if(!storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(f);
        return false;
    }

    uint64_t fsize = storage_file_size(f);

    // Predict sample count from file weight. Measured RAW .sub captures run
    // ~4.4-5.4 bytes per sample (number text + sign + space), so fsize/4 is a
    // good first guess that fits normal captures without over-allocating. It is
    // only a hint: push_duration grows the buffer on demand (e.g. when long gaps
    // split into several samples), and decoded files synthesize into it.
    size_t est = (size_t)(fsize / 4) + 64;
    if(est > MAX_SAMPLES) est = MAX_SAMPLES;

    sd->data = safe_malloc(est * sizeof(int16_t));
    if(!sd->data) {
        sd->out_of_memory = true;
        storage_file_close(f);
        storage_file_free(f);
        return false;
    }

    sd->cap = est;

    char protocol[32] = {0};

    LineReader lr = {.file = f, .len = 0, .pos = 0, .eof = false};
    FuriString* line = furi_string_alloc();
    bool stop = false;

    while(!stop && lr_read_line(&lr, line)) {
        const char* s = furi_string_get_cstr(line);
        if(strncmp(s, "RAW_Data:", 9) == 0) {
            const char* p = s + 9;
            long v;
            while(next_raw_value(&p, &v)) {
                if(!push_duration(sd, (int32_t)v)) {
                    stop = true;
                    break;
                }
            }
        } else {
            parse_sub_meta(
                s, &sd->frequency, sd->preset, sizeof(sd->preset), protocol, sizeof(protocol));
        }
    }

    furi_string_free(line);
    storage_file_close(f);
    storage_file_free(f);

    if(sd->count == 0 && protocol[0] && strcmp(protocol, "RAW") != 0)
        sd->synthesized = synthesize_via_transmitter(storage, path, protocol, sd);

    if(sd->data && sd->count > 0 && sd->count < sd->cap) {
        int16_t* shrunk = safe_realloc(sd->data, sd->count * sizeof(int16_t));
        if(shrunk) {
            sd->data = shrunk;
            sd->cap = sd->count;
        }
    }

    if(g_normalize_jitter) normalize_jitter(sd);

    recompute_total_us(sd);
    return sd->count >= 2;
}

/* Insert a clean inter-signal separator so the silence at the join equals
 * exactly the configured gap. Signals (especially synthesized KeeLoq) end and
 * start with their own guard silence. Adding the gap on top of it would double
 * (or worse) the visible gap. So we replace the previous
 * signal's trailing silence with the gap here and the next signal's leading
 * silence is dropped by the caller when it is appended. */
static void merge_separator(SubData* dst) {
    // Absorb the previous signal's trailing silence so the gap isn't added on
    // top of it, then emit the gap via push_duration (which splits it into
    // several samples when it exceeds the per-sample clamp).
    if(g_merge_gap_ms == 0) return;

    while(dst->count > 0 && dst->data[dst->count - 1] < 0)
        dst->count--;

    push_duration(dst, -(g_merge_gap_ms * 1000));
}

static size_t count_sub_samples(
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
                // Must match load_sub: durations beyond the clamp are split into
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
        SubData tmp;
        memset(&tmp, 0, sizeof(tmp));
        size_t n = 0;
        if(synthesize_via_transmitter(storage, path, protocol, &tmp)) n = tmp.count;

        if(tmp.data) free(tmp.data);

        if(n > 0) return n;
    }

    return 0;
}

// How a signal being appended is joined to what's already in the buffer.
typedef enum {
    MergeJoinNone, // first signal of the whole merge; nothing precedes it
    MergeJoinManual, // start of a new file: separate with the manual gap
    MergeJoinNative, // a repetition of the same signal: keep its native gap
} MergeJoin;

// Prepare the boundary before appending a signal and report whether its own
// leading silence must be dropped so gaps don't stack into a doubled one.
//  - Manual join: replace the previous signal's trailing silence with our gap.
//  - Native join: keep the previous signal's native trailing silence as the gap.
// In both cases the appended signal's leading silence is dropped.
static bool merge_prepare_join(SubData* dst, MergeJoin join) {
    if(join == MergeJoinManual) {
        merge_separator(dst);
        return true;
    }

    if(join == MergeJoinNative && dst->count > 0 && dst->data[dst->count - 1] < 0) return true;

    return false;
}

static bool load_signal(Storage* storage, const char* path, SubData* out) {
    return load_sub(storage, path, out);
}

// Append a preloaded signal to dst, inserting the appropriate join gap and
// dropping the signal's own leading silence so gaps don't stack.
static void append_signal(SubData* dst, const SubData* sig, MergeJoin join) {
    bool skip_lead = merge_prepare_join(dst, join);
    for(size_t i = 0; i < sig->count; i++) {
        int32_t v = sig->data[i];
        if(skip_lead) {
            if(v < 0) continue;

            skip_lead = false;
        }

        if(!append_sample(dst, v)) break;
    }
}

static bool load_sub_repeated(Storage* storage, const char* path, SubData* out) {
    SubData sig = {0};
    bool ok = load_sub(storage, path, &sig);
    if(!ok || !sig.data || sig.count == 0) {
        *out = sig; // carries out_of_memory / flags for the caller
        return ok;
    }

    // Metadata (frequency, preset, flags) carries over; rebuild the samples.
    *out = sig;
    out->data = NULL;
    out->count = 0;
    out->cap = 0;

    size_t want = sig.count * (size_t)g_merge_repeat;
    size_t need = want > MAX_SAMPLES ? MAX_SAMPLES : want;
    out->data = safe_malloc(need * sizeof(int16_t));
    if(!out->data) {
        out->out_of_memory = true;
        free(sig.data);
        return false;
    }
    out->cap = need;
    if(want > MAX_SAMPLES) out->truncated = true;

    for(int r = 0; r < g_merge_repeat; r++) {
        MergeJoin join = (r == 0) ? MergeJoinNone : MergeJoinNative;
        append_signal(out, &sig, join);
    }

    recompute_total_us(out);
    free(sig.data);
    return out->count >= 2;
}

// First free name: <base> itself when bare, then <base><sep>1, <base><sep>2, ...
static void propose_name(
    Storage* st,
    const char* base,
    const char* sep,
    bool bare,
    char* out,
    size_t outlen) {
    FuriString* path = furi_string_alloc();
    for(int n = bare ? 0 : 1; n < 1000; n++) {
        if(n)
            snprintf(out, outlen, "%s%s%d", base, sep, n);
        else
            strlcpy(out, base, outlen);
        furi_string_printf(path, "%s/%s.sub", SUBGHZ_DIR, out);
        if(!storage_file_exists(st, furi_string_get_cstr(path))) break;
    }

    furi_string_free(path);
}

// <name>_editN for a RAW save; an open <name>_editK continues the same series.
static void propose_edit_name(Storage* st, const App* a, char* out, size_t outlen) {
    char base[sizeof(a->basename)];
    strlcpy(base, a->basename, sizeof(base));

    char* hit = NULL;
    for(char* s = base; (s = strstr(s, "_edit")); s++)
        hit = s;

    if(hit) {
        const char* t = hit + 5;
        while(*t >= '0' && *t <= '9')
            t++;
        if(!*t) *hit = '\0';
    }

    propose_name(st, base, "_edit", false, out, outlen);
}

// Sample indices touched by A..B. False when nothing falls inside.
static bool selection_range(const App* a, size_t* i0, size_t* i1) {
    int32_t lo = a->marker_a, hi = a->marker_b;
    if(lo > hi) {
        int32_t t = lo;
        lo = hi;
        hi = t;
    }

    int32_t run = 0;
    bool started = false;
    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;
        if(s1 > lo && s0 < hi) {
            if(!started) {
                *i0 = i;
                started = true;
            }
            *i1 = i;
        }
    }

    return started;
}

static bool write_selection(Storage* st, App* a, const char* path) {
    size_t i0 = 0, i1 = 0;
    if(!selection_range(a, &i0, &i1)) {
        snprintf(a->status, sizeof(a->status), "Nothing in A..B");
        return false;
    }

    /* A synthesized frame is exact - keep its leading guard (a low gap) and
     * trailing guard verbatim. The trims below would strip the leading silence
     * (making preamble-less protocols like Princeton undecodable) so skip them. */
    if(!a->sd.synthesized) {
        while(i0 < i1 && a->sd.data[i0] < 0)
            i0++;

        while(i1 > i0 && a->sd.data[i1] > 0)
            i1--;
    }

    if(i0 >= i1) {
        snprintf(a->status, sizeof(a->status), "Range too small");
        return false;
    }

    File* f = storage_file_alloc(st);
    if(!storage_file_open(f, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_free(f);
        snprintf(a->status, sizeof(a->status), "Write failed");
        return false;
    }

    char hdr[192];
    int n = snprintf(
        hdr,
        sizeof(hdr),
        "Filetype: Flipper SubGhz RAW File\nVersion: 1\nFrequency: %lu\nPreset: %s\nProtocol: RAW\n",
        (unsigned long)a->sd.frequency,
        a->sd.preset);

    storage_file_write(f, hdr, n);

    FuriString* lbuf = furi_string_alloc();
    furi_string_set(lbuf, "RAW_Data:");

    int cnt = 0;
    for(size_t i = i0; i <= i1; i++) {
        char vb[16];
        snprintf(vb, sizeof(vb), " %ld", (long)a->sd.data[i]);
        furi_string_cat_str(lbuf, vb);
        if(++cnt >= 512) {
            furi_string_push_back(lbuf, '\n');
            storage_file_write(f, furi_string_get_cstr(lbuf), furi_string_size(lbuf));
            furi_string_set(lbuf, "RAW_Data:");
            cnt = 0;
        }
    }

    if(cnt > 0) {
        furi_string_push_back(lbuf, '\n');
        storage_file_write(f, furi_string_get_cstr(lbuf), furi_string_size(lbuf));
    }

    furi_string_free(lbuf);
    storage_file_close(f);
    storage_file_free(f);
    return true;
}

/* Decode: A..B is replayed through the firmware's own Sub-GHz decoders - the
 * same ones the Sub-GHz app uses while reading - so every protocol it knows
 * works here with no per-protocol code. Decoders are allocated one at a time
 * instead of through a SubGhzReceiver (which holds all of them at once,
 * BinRAW's big buffers included), keeping peak RAM to a single instance.
 * Identical frames from one protocol collapse into one entry; only a short
 * fingerprint is kept per entry and the full frame is recovered on demand by
 * replaying that one protocol. */

typedef struct {
    const SubGhzProtocol* proto;
    uint32_t hash;
    uint16_t frames;
    char key[18];
} Decoded;

typedef struct {
    SubGhzEnvironment* env;
    size_t i0, i1;
    uint32_t lead_us;
    Decoded* list;
    size_t n;
    const Decoded* want; // replay mode: stop at the first frame matching this entry
    FuriString* text;
    bool hit;
    bool oom;
} DecodeCtx;

// FNV-1a of a frame description, skipping measured timings ("Te:350us",
// "GT:Te*31") that jitter between otherwise identical frames.
static uint32_t decoded_hash(const char* s) {
    uint32_t h = 2166136261u;
    while(*s) {
        if(s[0] == 'T' && s[1] == 'e' && (s[2] == ':' || s[2] == '*')) {
            for(s += 3; *s >= '0' && *s <= '9'; s++)
                ;
            continue;
        }

        h = (h ^ (uint8_t)*s++) * 16777619u;
    }

    return h;
}

static void decoded_key(const char* s, char* out, size_t n) {
    size_t len = 0;
    const char* k = strstr(s, "Key:");
    if(k) {
        for(k += 4; *k == ' '; k++)
            ;

        if(k[0] == '0' && (k[1] == 'x' || k[1] == 'X')) k += 2;

        while(len + 1 < n && *k && strchr("0123456789ABCDEFabcdef", *k))
            out[len++] = *k++;
    }

    out[len] = '\0';
}

static void decode_rx_cb(SubGhzProtocolDecoderBase* dec, void* ctx) {
    DecodeCtx* dc = ctx;
    furi_string_reset(dc->text);
    subghz_protocol_decoder_base_get_string(dec, dc->text);
    uint32_t h = decoded_hash(furi_string_get_cstr(dc->text));

    if(dc->want) {
        if(h == dc->want->hash) dc->hit = true;

        return;
    }

    for(size_t i = 0; i < dc->n; i++) {
        if(dc->list[i].proto == dec->protocol && dc->list[i].hash == h) {
            if(dc->list[i].frames < UINT16_MAX) dc->list[i].frames++;

            return;
        }
    }

    if(dc->n >= DECODE_MAX) return;

    Decoded* d = &dc->list[dc->n++];
    *d = (Decoded){.proto = dec->protocol, .hash = h, .frames = 1};
    decoded_key(furi_string_get_cstr(dc->text), d->key, sizeof(d->key));
}

// Feed A..B to one protocol's decoder as level/duration runs. Stops right after
// a replay hit, so the returned (still allocated) decoder carries that frame.
static SubGhzProtocolDecoderBase*
    decode_run(const App* a, const SubGhzProtocol* p, DecodeCtx* dc) {
    SubGhzProtocolDecoderBase* dec = p->decoder->alloc(dc->env);
    dec->callback = decode_rx_cb;
    dec->context = dc;
    p->decoder->reset(dec);
    p->decoder->feed(dec, false, dc->lead_us);

    bool level = a->sd.data[dc->i0] > 0;
    uint32_t dur = 0;
    for(size_t i = dc->i0; i <= dc->i1 && !dc->hit; i++) {
        int16_t v = a->sd.data[i];
        if((v > 0) != level) {
            p->decoder->feed(dec, level, dur);
            level = v > 0;
            dur = 0;
        }

        dur += iabs32(v);
    }

    // Close the last frame with a long silence so end-of-packet logic fires.
    if(!dc->hit && level) {
        p->decoder->feed(dec, true, dur);
        dur = 0;
    }

    if(!dc->hit) p->decoder->feed(dec, false, dur + DECODE_TAIL_US);

    return dec;
}

/* Shadow silence fed ahead of A..B (never shown or saved), standing in for
 * whatever comes before the first pulse. Preamble-less protocols (Princeton,
 * CAME, Nice FLO, ...) only start on a guard gap inside a protocol-specific
 * window around N x Te, and those windows do not all overlap, so no fixed
 * value fits every protocol. The signal's own guard does: take the longest
 * gap in the selection that could be one, else a value most windows accept. */
static uint32_t decode_lead_us(const App* a, size_t i0, size_t i1) {
    uint32_t best = 0, run = 0;
    for(size_t i = i0; i <= i1 + 1; i++) {
        if(i <= i1 && a->sd.data[i] <= 0) {
            run += -a->sd.data[i];
            continue;
        }

        if(run >= DECODE_LEAD_MIN_US && run <= DECODE_LEAD_MAX_US && run > best) best = run;

        run = 0;
    }

    return best ? best : DECODE_LEAD_US;
}

static void decode_selection(const App* a, DecodeCtx* dc) {
    size_t count = subghz_protocol_registry_count(&subghz_protocol_registry);
    for(size_t k = 0; k < count; k++) {
        const SubGhzProtocol* p =
            subghz_protocol_registry_get_by_index(&subghz_protocol_registry, k);
        if(!(p->flag & SubGhzProtocolFlag_Decodable) || !p->decoder || !p->decoder->alloc)
            continue;

        if(memmgr_get_free_heap() < DECODE_HEAP_RESERVE) {
            dc->oom = true;
            break;
        }

        p->decoder->free(decode_run(a, p, dc));
    }
}

// Replay A..B through d's protocol until d's frame comes out again. On success
// dc->text describes it and the returned decoder (caller frees) still holds it.
static SubGhzProtocolDecoderBase* decoded_replay(const App* a, DecodeCtx* dc, const Decoded* d) {
    dc->want = d;
    dc->hit = false;
    SubGhzProtocolDecoderBase* dec = decode_run(a, d->proto, dc);
    if(dc->hit) return dec;

    d->proto->decoder->free(dec);
    return NULL;
}

// The firmware serializer takes the short preset names the Sub-GHz app uses.
static const char* preset_short_name(const char* full) {
    static const char* const map[][2] = {
        {"FuriHalSubGhzPresetOok270Async", "AM270"},
        {"FuriHalSubGhzPresetOok650Async", "AM650"},
        {"FuriHalSubGhzPreset2FSKDev238Async", "FM238"},
        {"FuriHalSubGhzPreset2FSKDev12KAsync", "FM12K"},
        {"FuriHalSubGhzPreset2FSKDev476Async", "FM476"},
    };

    for(size_t i = 0; i < COUNT_OF(map); i++)
        if(!strcmp(full, map[i][0])) return map[i][1];

    return full;
}

// Save a decoded entry as a regular Sub-GHz key file (the same format the
// Sub-GHz app writes after Read), ready to be emulated.
static bool write_decoded(Storage* st, App* a, DecodeCtx* dc, const Decoded* d, const char* path) {
    SubGhzProtocolDecoderBase* dec = decoded_replay(a, dc, d);
    bool ok = false;
    if(dec) {
        SubGhzRadioPreset preset = {
            .name = furi_string_alloc_set(preset_short_name(a->sd.preset)),
            .frequency = a->sd.frequency,
        };

        FlipperFormat* ff = flipper_format_file_alloc(st);
        bool opened = flipper_format_file_open_always(ff, path);
        ok = opened &&
             subghz_protocol_decoder_base_serialize(dec, ff, &preset) == SubGhzProtocolStatusOk;

        flipper_format_free(ff);
        furi_string_free(preset.name);
        dec->protocol->decoder->free(dec);

        if(opened && !ok) storage_simply_remove(st, path);
    }

    if(!ok) snprintf(a->status, sizeof(a->status), "Write failed");

    return ok;
}

static bool snapshot_take(App* a) {
    if(a->sd.count == 0) return false;

    size_t need = a->sd.count * sizeof(int16_t);
    int16_t* copy = safe_malloc(need);
    if(!copy) return false;

    memcpy(copy, a->sd.data, need);

    if(a->undo_data) free(a->undo_data);

    a->undo_data = copy;
    a->undo_count = a->sd.count;
    a->undo_total_us = a->sd.total_us;
    a->undo_marker_a = a->marker_a;
    a->undo_marker_b = a->marker_b;
    a->undo_active = a->active;
    a->undo_view_start = a->view_start;
    a->undo_view_end = a->view_end;
    a->has_undo = true;
    return true;
}

static void snapshot_restore(App* a) {
    if(!a->undo_data) return;

    free(a->sd.data);
    a->sd.data = a->undo_data;
    a->sd.count = a->undo_count;
    a->sd.cap = a->undo_count;
    a->sd.total_us = a->undo_total_us;
    a->marker_a = a->undo_marker_a;
    a->marker_b = a->undo_marker_b;
    a->active = a->undo_active;
    a->view_start = a->undo_view_start;
    a->view_end = a->undo_view_end;

    a->undo_data = NULL;
    a->has_undo = false;

    recompute_overview(a);
    recompute_activity(a);
}

static bool cut_compact(App* a) {
    int32_t lo = a->marker_a, hi = a->marker_b;
    if(lo > hi) {
        int32_t t = lo;
        lo = hi;
        hi = t;
    }

    int16_t* d = a->sd.data;
    size_t n = a->sd.count;
    size_t w = 0;
    int32_t run = 0;
    bool seam = false; // true once something was dropped before the current sample
    for(size_t i = 0; i < n; i++) {
        int32_t v = d[i];
        int32_t ad = iabs32(v);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;
        int sign = (v >= 0) ? 1 : -1;
        int32_t keep = 0;

        if(s0 < lo) keep += (s1 < lo ? s1 : lo) - s0;

        if(s1 > hi) keep += s1 - (s0 > hi ? s0 : hi);

        if(keep < ad) seam = true; // part (or all) of this sample falls inside A..B

        if(keep <= 0) continue;

        int32_t val = sign * keep;
        // Merge only across the cut seam. Consecutive same-sign samples elsewhere
        // are long gaps split by push_duration() and must stay as they are.
        if(seam && w > 0 && ((d[w - 1] >= 0) == (val >= 0))) {
            int32_t sum = (int32_t)d[w - 1] + val;
            if(iabs32(sum) <= DUR_CLAMP) {
                d[w - 1] = (int16_t)sum;
            } else {
                // Keep the full duration: fill the previous sample, spill the rest.
                // Safe in place: at most one extra sample, written at index <= i.
                d[w - 1] = (int16_t)(sign * DUR_CLAMP);
                d[w++] = (int16_t)(sum - sign * DUR_CLAMP);
            }
        } else {
            d[w++] = (int16_t)val;
        }

        if(keep == ad && s0 >= hi) seam = false; // past the cut, back to verbatim copying
    }

    a->sd.count = w;
    if(w < 2) return true;

    recompute_total_us(&a->sd);

    a->marker_a = lo;
    a->marker_b = lo;
    clamp_marker(a, 0);
    clamp_marker(a, 1);
    a->active = 0;

    int32_t span = a->view_end - a->view_start;
    if(span < 200) span = 200;

    a->view_start = a->marker_a - span / 2;
    a->view_end = a->view_start + span;

    clamp_view(a);

    recompute_overview(a);
    recompute_activity(a);
    return false;
}

static void draw_marker(Canvas* c, int x, bool active) {
    if(x < 0 || x > SCREEN_W_PX - 1) return;

    if(active) {
        for(int y = WAVE_TOP; y <= WAVE_BOT; y++)
            canvas_draw_dot(c, x, y);

        canvas_draw_box(c, x - 1, WAVE_TOP - 4, 3, 3);
    } else {
        for(int y = WAVE_TOP; y <= WAVE_BOT; y += 2)
            canvas_draw_dot(c, x, y);
    }
}

static void draw_cb(Canvas* c, void* ctx) {
    App* a = ctx;
    furi_mutex_acquire(a->mutex, FuriWaitForever);

    canvas_clear(c);
    canvas_set_color(c, ColorBlack);
    canvas_set_font(c, FontSecondary);

    if(a->loading) {
        canvas_draw_str_aligned(c, SCREEN_H_PX, 28, AlignCenter, AlignCenter, "Loading...");
        char ln[20];
        strncpy(ln, a->basename, sizeof(ln) - 1);
        ln[sizeof(ln) - 1] = '\0';
        canvas_draw_str_aligned(c, 64, 40, AlignCenter, AlignCenter, ln);
        furi_mutex_release(a->mutex);
        return;
    }

    if(a->saving) {
        canvas_draw_str_aligned(c, 64, 34, AlignCenter, AlignCenter, "Saving...");
        furi_mutex_release(a->mutex);
        return;
    }

    char nm[15];
    strncpy(nm, a->basename, sizeof(nm) - 1);
    nm[sizeof(nm) - 1] = '\0';
    canvas_draw_str(c, 0, FONT_SIZE_PX, nm);

    char total_time_str[14];
    fmt_time(a->sd.total_us, total_time_str, sizeof(total_time_str));
    canvas_draw_str_aligned(
        c, SCREEN_W_PX - 1, FONT_SIZE_PX, AlignRight, AlignBottom, total_time_str);

    for(int x = 0; x < SCREEN_W_PX; x++) {
        if(a->overview[x] > a->ov_max / 6) canvas_draw_dot(c, x, OV_Y + OV_H - 1);
    }

    int32_t total = a->sd.total_us > 0 ? a->sd.total_us : 1;
    int vx0 = (int)(((int64_t)a->view_start * SCREEN_W_PX) / total);
    int vx1 = (int)(((int64_t)a->view_end * SCREEN_W_PX) / total);

    if(vx0 < 0) vx0 = 0;

    if(vx0 > SCREEN_W_PX - 1) vx0 = SCREEN_W_PX - 1;

    if(vx1 <= vx0) vx1 = vx0 + 1;

    if(vx1 > SCREEN_W_PX - 1) vx1 = SCREEN_W_PX - 1;

    canvas_draw_frame(c, vx0, OV_Y, vx1 - vx0 + 1, OV_H);
    canvas_draw_line(c, 0, SEP_Y, SCREEN_W_PX - 1, SEP_Y);

    if(a->wave_mode) {
        int yhi = WAVE_TOP + 2;
        int ylo = WAVE_BOT - 2;

        int32_t run = 0;
        int prev_x = -1;
        int prev_y = ylo;

        for(size_t i = 0; i < a->sd.count; i++) {
            int32_t ad = iabs32(a->sd.data[i]);
            int32_t t0 = run;
            run += ad;
            int32_t t1 = run;

            if(t1 < a->view_start || t0 > a->view_end) continue;

            int x0 = time_to_x(a, t0);
            int x1 = time_to_x(a, t1);

            if(x0 < 0) x0 = 0;

            if(x1 > SCREEN_W_PX - 1) x1 = SCREEN_W_PX - 1;

            int y = (a->sd.data[i] > 0) ? yhi : ylo;

            if(prev_x >= 0 && y != prev_y && x0 >= 0 && x0 <= SCREEN_W_PX - 1)
                canvas_draw_line(c, x0, yhi, x0, ylo);

            if(x1 >= x0) canvas_draw_line(c, x0, y, x1, y);

            prev_x = x1;
            prev_y = y;
        }
    } else {
        for(int x = 0; x < SCREEN_W_PX; x++) {
            if(a->activity[x] == 0) continue;

            int h = (int)(((int32_t)a->activity[x] * (WAVE_BOT - WAVE_TOP - 1)) / a->act_max);
            if(h < 1) h = 1;

            canvas_draw_line(c, x, WAVE_BOT, x, WAVE_BOT - h);
        }
    }

    int xa = time_to_x(a, a->marker_a);
    int xb = time_to_x(a, a->marker_b);
    int xlo = xa < xb ? xa : xb;
    int xhi = xa < xb ? xb : xa;

    if(xlo < 0) xlo = 0;

    if(xhi > SCREEN_W_PX - 1) xhi = SCREEN_W_PX - 1;

    for(int x = xlo; x <= xhi; x += 2)
        canvas_draw_dot(c, x, WAVE_TOP - 1);

    draw_marker(c, time_to_x(a, a->marker_a), a->active == 0);
    draw_marker(c, time_to_x(a, a->marker_b), a->active == 1);

    if(a->view_start > 0) {
        canvas_draw_str_aligned(c, 0, (WAVE_TOP + WAVE_BOT) / 2, AlignLeft, AlignCenter, "<");
    }

    if(a->view_end < a->sd.total_us || a->sd.truncated) {
        canvas_draw_str_aligned(
            c, SCREEN_W_PX - 1, (WAVE_TOP + WAVE_BOT) / 2, AlignRight, AlignCenter, ">");
    }

    char sa[14], sb[14], zb[14], sl[14];
    fmt_time(a->marker_a, sa, sizeof(sa));
    fmt_time(a->marker_b, sb, sizeof(sb));

    int32_t len = a->marker_b - a->marker_a;
    if(len < 0) len = -len;

    int32_t vis_lo = a->view_start < 0 ? 0 : a->view_start;
    int32_t vis_hi = a->view_end > a->sd.total_us ? a->sd.total_us : a->view_end;
    int32_t vis = vis_hi - vis_lo;
    if(vis < 0) vis = 0;

    fmt_time(vis, zb, sizeof(zb));
    fmt_time(len, sl, sizeof(sl));

    char l1[SCREEN_W_PX / 4];
    char l2[SCREEN_W_PX / 4];

    snprintf(l1, sizeof(l1), "%sA: %s", a->active == 0 ? ">" : "  ", sa);
    snprintf(l2, sizeof(l2), "%sB: %s", a->active == 1 ? ">" : "  ", sb);
    canvas_draw_str(c, 0, LINE1_BASE, l1);
    canvas_draw_str(c, 0, LINE2_BASE, l2);

    snprintf(l1, sizeof(l1), "View: %s", zb);
    snprintf(l2, sizeof(l2), "Marked: %s", sl);
    canvas_draw_str_aligned(c, SCREEN_W_PX - 1, 56, AlignRight, AlignBottom, l1);
    canvas_draw_str_aligned(
        c, SCREEN_W_PX - 1, 56 + FONT_SIZE_PX + 1, AlignRight, AlignBottom, l2);

    if(a->mode != EditModeMenu && a->status[0] && furi_get_tick() < a->status_until) {
        int w = 124;
        canvas_set_color(c, ColorWhite);
        canvas_draw_box(c, 2, 22, w, 22);
        canvas_set_color(c, ColorBlack);
        canvas_draw_frame(c, 2, 22, w, 22);

        const char* msg = a->status;
        if(strncmp(msg, "Saved ", 6) == 0) {
            canvas_draw_str_aligned(c, 64, 29, AlignCenter, AlignCenter, "Saved");
            canvas_draw_str_aligned(c, 64, 38, AlignCenter, AlignCenter, msg + 6);
        } else {
            canvas_draw_str_aligned(c, 64, 33, AlignCenter, AlignCenter, msg);
        }
    }

    if(a->mode == EditModeMenu) {
        const int bx = 36, bw = 56, bh = MENU_ITEMS * 14 + 4, by = (SCREEN_H_PX - bh) / 2;
        canvas_set_color(c, ColorWhite);
        canvas_draw_box(c, bx, by, bw, bh);
        canvas_set_color(c, ColorBlack);
        canvas_draw_frame(c, bx, by, bw, bh);

        const char* labels[MENU_ITEMS] = {"Save", "Decode", "Cut", "Undo"};
        for(int i = 0; i < MENU_ITEMS; i++) {
            int ry = by + 2 + i * 14;
            int rh = 13;
            bool disabled = (i == MENU_SEL_UNDO && !a->has_undo);

            if(a->menu_sel == i) {
                canvas_set_color(c, ColorBlack);
                canvas_draw_box(c, bx + 2, ry, bw - 4, rh);
                canvas_set_color(c, ColorWhite);
            } else {
                canvas_set_color(c, ColorBlack);
            }

            int cx = bx + bw / 2;
            canvas_draw_str_aligned(c, cx, ry + rh / 2 + 1, AlignCenter, AlignCenter, labels[i]);

            if(disabled) canvas_draw_line(c, cx - 13, ry + rh / 2, cx + 13, ry + rh / 2);
        }

        canvas_set_color(c, ColorBlack);
    }

    furi_mutex_release(a->mutex);
}

static void input_cb(InputEvent* e, void* ctx) {
    FuriMessageQueue* q = ctx;
    furi_message_queue_put(q, e, FuriWaitForever);
}

/* ---- Custom button picker UI ------------------------------------------- */

#ifndef FW_ORIGIN_Official

static void btn_pick_fit(Canvas* c, const char* s, int maxw, char* out, size_t outlen) {
    strncpy(out, s, outlen - 1);
    out[outlen - 1] = '\0';

    if(canvas_string_width(c, out) <= (size_t)maxw) return;

    size_t n = strlen(out);
    while(n > 1) {
        out[--n] = '\0';

        char tmp[BTN_LABEL_LEN + 8];
        snprintf(tmp, sizeof(tmp), "%s..", out);
        if(canvas_string_width(c, tmp) <= (size_t)maxw) {
            strncpy(out, tmp, outlen - 1);
            out[outlen - 1] = '\0';
            return;
        }
    }
}

static void btn_pick_draw_cb(Canvas* c, void* ctx) {
    const BtnPick* bp = ctx;

    canvas_clear(c);
    canvas_set_font(c, FontSecondary);
    canvas_draw_str(c, 0, 7, bp->protocol);
    canvas_draw_str_aligned(c, 127, 7, AlignRight, AlignBottom, "Back: cancel");
    canvas_draw_line(c, 0, 9, 127, 9);

    const int cx = 21, cy = 38;

    if(bp->label[SUBGHZ_CUSTOM_BTN_OK][0]) canvas_draw_circle(c, cx, cy, 5);

    if(bp->label[SUBGHZ_CUSTOM_BTN_UP][0])
        canvas_draw_triangle(c, cx, cy - 8, 9, 6, CanvasDirectionBottomToTop);

    if(bp->label[SUBGHZ_CUSTOM_BTN_DOWN][0])
        canvas_draw_triangle(c, cx, cy + 8, 9, 6, CanvasDirectionTopToBottom);

    if(bp->label[SUBGHZ_CUSTOM_BTN_LEFT][0])
        canvas_draw_triangle(c, cx - 8, cy, 9, 6, CanvasDirectionRightToLeft);

    if(bp->label[SUBGHZ_CUSTOM_BTN_RIGHT][0])
        canvas_draw_triangle(c, cx + 8, cy, 9, 6, CanvasDirectionLeftToRight);

    static const uint8_t order[BTN_SLOTS] = {
        SUBGHZ_CUSTOM_BTN_OK,
        SUBGHZ_CUSTOM_BTN_UP,
        SUBGHZ_CUSTOM_BTN_DOWN,
        SUBGHZ_CUSTOM_BTN_LEFT,
        SUBGHZ_CUSTOM_BTN_RIGHT,
    };

    for(int row = 0; row < BTN_SLOTS; row++) {
        uint8_t slot = order[row];
        int y = 19 + row * 9;
        int gx = 49, gy = y - 2;

        switch(slot) {
        case SUBGHZ_CUSTOM_BTN_UP:
            canvas_draw_triangle(c, gx, gy + 2, 5, 4, CanvasDirectionBottomToTop);
            break;
        case SUBGHZ_CUSTOM_BTN_DOWN:
            canvas_draw_triangle(c, gx, gy - 2, 5, 4, CanvasDirectionTopToBottom);
            break;
        case SUBGHZ_CUSTOM_BTN_LEFT:
            canvas_draw_triangle(c, gx + 2, gy, 5, 4, CanvasDirectionRightToLeft);
            break;
        case SUBGHZ_CUSTOM_BTN_RIGHT:
            canvas_draw_triangle(c, gx - 2, gy, 5, 4, CanvasDirectionLeftToRight);
            break;
        default:
            canvas_draw_disc(c, gx, gy, 2);
            break;
        }

        char buf[BTN_LABEL_LEN + 4];
        btn_pick_fit(c, bp->label[slot][0] ? bp->label[slot] : "-", 127 - 56, buf, sizeof(buf));
        canvas_draw_str(c, 56, y, buf);
    }
}

static int btn_pick_dialog(Gui* gui, const BtnPick* bp) {
    ViewPort* vp = view_port_alloc();
    FuriMessageQueue* q = furi_message_queue_alloc(8, sizeof(InputEvent));

    view_port_draw_callback_set(vp, btn_pick_draw_cb, (void*)bp);
    view_port_input_callback_set(vp, input_cb, q);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    view_port_update(vp);

    int sel = -1;
    InputEvent e;

    while(true) {
        if(furi_message_queue_get(q, &e, FuriWaitForever) != FuriStatusOk) continue;

        if(e.type != InputTypeShort && e.type != InputTypeLong) continue;

        if(e.key == InputKeyBack) break;

        uint8_t slot;
        if(e.key == InputKeyOk)
            slot = SUBGHZ_CUSTOM_BTN_OK;
        else if(e.key == InputKeyUp)
            slot = SUBGHZ_CUSTOM_BTN_UP;
        else if(e.key == InputKeyDown)
            slot = SUBGHZ_CUSTOM_BTN_DOWN;
        else if(e.key == InputKeyLeft)
            slot = SUBGHZ_CUSTOM_BTN_LEFT;
        else if(e.key == InputKeyRight)
            slot = SUBGHZ_CUSTOM_BTN_RIGHT;
        else
            continue;

        if(!bp->label[slot][0]) continue;

        sel = (int)slot;
        break;
    }

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(q);
    return sel;
}

static bool read_protocol(Storage* storage, const char* path, char* out, size_t outlen) {
    File* f = storage_file_alloc(storage);
    if(!storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(f);
        return false;
    }

    LineReader lr = {.file = f, .len = 0, .pos = 0, .eof = false};
    FuriString* line = furi_string_alloc();
    uint32_t freq = 0;
    char preset[48] = {0};

    out[0] = '\0';
    while(lr_read_line(&lr, line)) {
        const char* s = furi_string_get_cstr(line);
        if(strncmp(s, "RAW_Data:", 9) == 0) break;

        parse_sub_meta(s, &freq, preset, sizeof(preset), out, outlen);
    }

    furi_string_free(line);
    storage_file_close(f);
    storage_file_free(f);
    return true;
}

#endif // FW_ORIGIN_Official

static bool btn_select_for_file(Gui* gui, Storage* storage, const char* path) {
    g_custom_btn_id = SUBGHZ_CUSTOM_BTN_OK;

#ifdef FW_ORIGIN_Official
    UNUSED(gui);
    UNUSED(storage);
    UNUSED(path);
    return true;
#else
    subghz_custom_btns_reset();

    char protocol[32] = {0};
    if(!read_protocol(storage, path, protocol, sizeof(protocol))) return true;

    if(!protocol[0] || strcmp(protocol, "RAW") == 0) return true;

    BtnPick bp;
    btn_probe(storage, path, protocol, &bp);
    if(!bp.available) return true;

    int sel = btn_pick_dialog(gui, &bp);
    if(sel < 0) return false;

    g_custom_btn_id = (uint8_t)sel;
    return true;
#endif
}

static void zoom(App* a, bool in) {
    int32_t m = a->active ? a->marker_b : a->marker_a;
    int32_t span = a->view_end - a->view_start;
    int32_t nspan = in ? (span * 2 / 3) : (span * 3 / 2);
    if(nspan < 200) nspan = 200;

    int32_t fullspan = a->sd.total_us + 2 * (a->sd.total_us / 3 + 50000);
    if(nspan > fullspan) nspan = fullspan;

    if(nspan < 1) nspan = 1;

    int32_t off = (int32_t)(((int64_t)(m - a->view_start) * nspan) / (span > 0 ? span : 1));
    a->view_start = m - off;
    a->view_end = a->view_start + nspan;
    clamp_view(a);
}

typedef struct {
    ViewDispatcher* vd;
    bool confirmed;
    uint32_t index;
} PromptCtx;

static void prompt_result_cb(void* ctx) {
    PromptCtx* p = ctx;
    p->confirmed = true;
    view_dispatcher_stop(p->vd);
}

static void pick_index_cb(void* ctx, uint32_t index) {
    PromptCtx* p = ctx;
    p->index = index;
    prompt_result_cb(p);
}

// Widget buttons: Left declines, any other confirms.
static void button_ok_cb(GuiButtonType button, InputType type, void* ctx) {
    PromptCtx* p = ctx;
    if(type != InputTypeShort) return;

    p->confirmed = button != GuiButtonTypeLeft;
    view_dispatcher_stop(p->vd);
}

static uint32_t prompt_back_cb(void* ctx) {
    UNUSED(ctx);
    return VIEW_NONE;
}

// Run one view modally until it confirms (true) or Back leaves it (false).
static bool prompt_run(Gui* gui, PromptCtx* pc, View* view) {
    pc->vd = view_dispatcher_alloc();
    view_set_previous_callback(view, prompt_back_cb);
    view_dispatcher_attach_to_gui(pc->vd, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(pc->vd, 0, view);
    view_dispatcher_switch_to_view(pc->vd, 0);
    view_dispatcher_run(pc->vd);
    view_dispatcher_remove_view(pc->vd, 0);
    view_dispatcher_free(pc->vd);
    return pc->confirmed;
}

static bool prompt_filename(Gui* gui, char* namebuf, size_t buflen) {
    PromptCtx pc = {0};
    TextInput* ti = text_input_alloc();
    text_input_set_header_text(ti, "Name the file");
    text_input_set_result_callback(ti, prompt_result_cb, &pc, namebuf, buflen, false);
    bool ok = prompt_run(gui, &pc, text_input_get_view(ti));
    text_input_free(ti);
    return ok;
}

static bool confirm_overwrite(Gui* gui) {
    PromptCtx pc = {0};
    Widget* w = widget_alloc();
    widget_add_string_multiline_element(
        w, 64, 24, AlignCenter, AlignCenter, FontPrimary, "Name exists,\noverwrite?");
    widget_add_button_element(w, GuiButtonTypeLeft, "No", button_ok_cb, &pc);
    widget_add_button_element(w, GuiButtonTypeRight, "Yes", button_ok_cb, &pc);
    bool ok = prompt_run(gui, &pc, widget_get_view(w));
    widget_free(w);
    return ok;
}

// <Protocol>[_<Manufacturer>] from the decoded text, made filename-safe.
static void decoded_base_name(const DecodeCtx* dc, const Decoded* d, char* out, size_t outlen) {
    size_t n = strlcpy(out, d->proto->name, outlen);
    const char* mf = strstr(furi_string_get_cstr(dc->text), "MF:");
    if(mf) {
        mf += 3;
        int len = strcspn(mf, " \n");
        if(len && strncmp(mf, "Unknown", len) != 0 && n < outlen)
            snprintf(out + n, outlen - n, "_%.*s", len, mf);
    }

    char* w = out;
    for(const char* r = out; *r; r++) {
        char l = *r | 0x20;
        char c = ((*r >= '0' && *r <= '9') || (l >= 'a' && l <= 'z') || *r == '-') ? *r : '_';
        if(c != '_' || (w > out && w[-1] != '_')) *w++ = c;
    }

    while(w > out && w[-1] == '_')
        w--;
    *w = '\0';
}

// Save A..B as RAW, or with dc/d the decoded entry d as a key file.
static void prompt_and_save(
    Gui* gui,
    ViewPort* vp,
    FuriMessageQueue* queue,
    Storage* st,
    App* a,
    DecodeCtx* dc,
    const Decoded* d) {
    char namebuf[64];
    if(d) {
        char base[48];
        decoded_base_name(dc, d, base, sizeof(base));
        propose_name(st, base, "_", true, namebuf, sizeof(namebuf));
    } else
        propose_edit_name(st, a, namebuf, sizeof(namebuf));

    gui_remove_view_port(gui, vp);

    FuriString* path = furi_string_alloc();
    bool write = false;
    snprintf(a->status, sizeof(a->status), "Cancelled");
    while(prompt_filename(gui, namebuf, sizeof(namebuf)) && namebuf[0] != '\0') {
        furi_string_printf(path, "%s/%s.sub", SUBGHZ_DIR, namebuf);
        if(!storage_file_exists(st, furi_string_get_cstr(path)))
            write = true;
        else if(a->src_in_dir && strcasecmp(namebuf, a->basename) == 0)
            snprintf(a->status, sizeof(a->status), "Can't overwrite open file");
        else if(!confirm_overwrite(gui))
            continue;
        else
            write = true;

        break;
    }

    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    if(write) {
        a->saving = true;
        view_port_update(vp);

        const char* p = furi_string_get_cstr(path);
        if(d ? write_decoded(st, a, dc, d, p) : write_selection(st, a, p))
            snprintf(a->status, sizeof(a->status), "Saved %s.sub", namebuf);

        a->saving = false;
    }

    furi_string_free(path);

    a->status_until = furi_get_tick() + 2000;
    furi_message_queue_reset(queue);
    view_port_update(vp);
}

// Full description of one decoded entry. Returns true when Save was pressed.
static bool decoded_show(Gui* gui, const App* a, DecodeCtx* dc, const Decoded* d) {
    SubGhzProtocolDecoderBase* dec = decoded_replay(a, dc, d);
    if(!dec) return false;

    dec->protocol->decoder->free(dec);
    furi_string_replace_all(dc->text, "\r", "");

    PromptCtx pc = {0};
    Widget* w = widget_alloc();
    widget_add_text_scroll_element(w, 0, 0, 128, 50, furi_string_get_cstr(dc->text));
    widget_add_button_element(w, GuiButtonTypeRight, "Save", button_ok_cb, &pc);
    bool ok = prompt_run(gui, &pc, widget_get_view(w));
    widget_free(w);
    return ok;
}

// List of decoded entries (skipped for a single one). Returns the entry to save or -1.
static int32_t decoded_browse(Gui* gui, const App* a, DecodeCtx* dc) {
    uint32_t sel = 0;
    while(true) {
        if(dc->n > 1) {
            PromptCtx pc = {0};
            Submenu* sm = submenu_alloc();
            char label[48];
            snprintf(label, sizeof(label), "Decoded: %u", (unsigned)dc->n);
            submenu_set_header(sm, label);

            for(size_t i = 0; i < dc->n; i++) {
                const Decoded* d = &dc->list[i];
                int len = snprintf(label, sizeof(label), "%s %s", d->proto->name, d->key);
                if(d->frames > 1 && len > 0 && (size_t)len < sizeof(label))
                    snprintf(label + len, sizeof(label) - len, " x%u", (unsigned)d->frames);

                submenu_add_item(sm, label, i, pick_index_cb, &pc);
            }

            submenu_set_selected_item(sm, sel);
            bool ok = prompt_run(gui, &pc, submenu_get_view(sm));
            submenu_free(sm);

            if(!ok) return -1;

            sel = pc.index;
        }

        if(decoded_show(gui, a, dc, &dc->list[sel])) return (int32_t)sel;

        if(dc->n == 1) return -1;
    }
}

static void
    do_decode_action(Gui* gui, ViewPort* vp, FuriMessageQueue* queue, Storage* st, App* a) {
    DecodeCtx dc = {0};
    a->status_until = furi_get_tick() + 1500;

    if(!selection_range(a, &dc.i0, &dc.i1)) {
        snprintf(a->status, sizeof(a->status), "Nothing in A..B");
        return;
    }

    // The shadow silence replaces any leading gap: one that is too long or cut
    // short by A would fall outside the start window.
    while(dc.i0 < dc.i1 && a->sd.data[dc.i0] <= 0)
        dc.i0++;

    dc.lead_us = decode_lead_us(a, dc.i0, dc.i1);

    if(memmgr_get_free_heap() < DECODE_HEAP_RESERVE + DECODE_MAX * sizeof(Decoded)) {
        snprintf(a->status, sizeof(a->status), "Low memory");
        return;
    }

    snprintf(a->status, sizeof(a->status), "Decoding...");
    a->status_until = furi_get_tick() + 60000;
    view_port_update(vp);

    dc.env = subghz_env_setup();
    // Rolling codes (KeeLoq & co.) only show their manufacturer and counter
    // with the keystore; without spare RAM they still decode, just less.
    if(memmgr_get_free_heap() > DECODE_KEYS_HEAP) {
        subghz_environment_load_keystore(dc.env, "/ext/subghz/assets/keeloq_mfcodes");
        subghz_environment_load_keystore(dc.env, "/ext/subghz/assets/keeloq_mfcodes_user");
    }

    dc.list = malloc(DECODE_MAX * sizeof(Decoded));
    dc.text = furi_string_alloc();
    decode_selection(a, &dc);

    a->status_until = 0;
    int32_t save = -1;
    if(dc.n) {
        gui_remove_view_port(gui, vp);
        save = decoded_browse(gui, a, &dc);
        gui_add_view_port(gui, vp, GuiLayerFullscreen);
        furi_message_queue_reset(queue);
    } else {
        snprintf(a->status, sizeof(a->status), dc.oom ? "Low memory" : "No known protocol");
        a->status_until = furi_get_tick() + 1500;
    }

    if(save >= 0) prompt_and_save(gui, vp, queue, st, a, &dc, &dc.list[save]);

    furi_string_free(dc.text);
    free(dc.list);
    subghz_environment_free(dc.env);
}

static void
    do_cut_action(Gui* gui, ViewPort* vp, FuriMessageQueue* queue, DialogsApp* dialogs, App* a) {
    furi_mutex_acquire(a->mutex, FuriWaitForever);

    int32_t lo = a->marker_a, hi = a->marker_b;
    if(lo > hi) {
        int32_t t = lo;
        lo = hi;
        hi = t;
    }

    if(hi - lo < 1) {
        snprintf(a->status, sizeof(a->status), "Range too small");
        a->status_until = furi_get_tick() + 1500;
        furi_mutex_release(a->mutex);
        return;
    }

    if(!snapshot_take(a)) {
        snprintf(a->status, sizeof(a->status), "Low memory, no cut");
        a->status_until = furi_get_tick() + 1500;
        furi_mutex_release(a->mutex);
        return;
    }

    bool emptied = cut_compact(a);
    if(!emptied) {
        snprintf(a->status, sizeof(a->status), "Cut");
        a->status_until = furi_get_tick() + 1200;
        furi_mutex_release(a->mutex);
        return;
    }

    furi_mutex_release(a->mutex);

    gui_remove_view_port(gui, vp);

    DialogMessage* m = dialog_message_alloc();
    dialog_message_set_header(m, "Cannot cut", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(
        m, "That would leave the\nsignal empty.", 64, 30, AlignCenter, AlignCenter);
    dialog_message_set_buttons(m, NULL, NULL, "OK");
    dialog_message_show(dialogs, m);
    dialog_message_free(m);

    furi_mutex_acquire(a->mutex, FuriWaitForever);
    snapshot_restore(a);
    snprintf(a->status, sizeof(a->status), "Cut cancelled");
    a->status_until = furi_get_tick() + 1500;
    furi_mutex_release(a->mutex);

    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    furi_message_queue_reset(queue);
}

static void run_editor_session(
    Gui* gui,
    ViewPort* vp,
    FuriMessageQueue* queue,
    Storage* storage,
    DialogsApp* dialogs,
    App* app,
    bool auto_select) {
    app->loading = false;
    if(auto_select) {
        auto_detect(app);
    } else {
        app->marker_a = 0;
        app->marker_b = app->sd.total_us;
        app->view_start = 0;
        app->view_end = app->sd.total_us > 0 ? app->sd.total_us : 1;
        clamp_view(app);
    }
    recompute_overview(app);
    recompute_activity(app);
    furi_message_queue_reset(queue);
    view_port_update(vp);

    bool running = true;
    InputEvent e;
    while(running) {
        if(furi_message_queue_get(queue, &e, 100) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            bool changed = false;
            bool want_save = false;
            bool want_decode = false;
            bool want_cut = false;

            if(app->mode == EditModeMenu) {
                if(e.type == InputTypeShort) {
                    if(e.key == InputKeyDown) {
                        app->menu_sel = (app->menu_sel + 1) % MENU_ITEMS;
                    } else if(e.key == InputKeyUp) {
                        app->menu_sel = (app->menu_sel + MENU_ITEMS - 1) % MENU_ITEMS;
                    } else if(e.key == InputKeyOk) {
                        app->mode = EditModeEdit;
                        if(app->menu_sel == MENU_SEL_SAVE) {
                            want_save = true;
                        } else if(app->menu_sel == MENU_SEL_DECODE) {
                            want_decode = true;
                        } else if(app->menu_sel == MENU_SEL_CUT) {
                            want_cut = true;
                        } else if(app->menu_sel == MENU_SEL_UNDO) {
                            if(app->has_undo) {
                                snapshot_restore(app);
                                snprintf(app->status, sizeof(app->status), "Undone");
                            } else {
                                snprintf(app->status, sizeof(app->status), "Nothing to undo");
                            }

                            app->status_until = furi_get_tick() + 1500;
                        }
                    } else if(e.key == InputKeyBack) {
                        app->mode = EditModeEdit;
                    }
                }
            } else if(e.type == InputTypePress || e.type == InputTypeRepeat) {
                int32_t span = app->view_end - app->view_start;
                int32_t step = span / SCREEN_W_PX;
                if(step < 1) step = 1;

                if(e.type == InputTypeRepeat) {
                    int32_t fast = span / 24;
                    if(fast > step) step = fast;
                }

                int32_t* m = app->active ? &app->marker_b : &app->marker_a;
                switch(e.key) {
                case InputKeyLeft:
                    *m -= step;
                    clamp_marker(app, app->active);
                    ensure_visible(app, *m);
                    changed = true;
                    break;
                case InputKeyRight:
                    *m += step;
                    clamp_marker(app, app->active);
                    ensure_visible(app, *m);
                    changed = true;
                    break;
                case InputKeyUp:
                    zoom(app, true);
                    changed = true;
                    break;
                case InputKeyDown:
                    zoom(app, false);
                    changed = true;
                    break;
                default:
                    break;
                }
            } else if(e.type == InputTypeShort) {
                if(e.key == InputKeyOk) {
                    app->active ^= 1;
                    ensure_visible(app, app->active ? app->marker_b : app->marker_a);
                    changed = true;
                } else if(e.key == InputKeyBack) {
                    running = false;
                }
            } else if(e.type == InputTypeLong) {
                if(e.key == InputKeyOk) {
                    app->mode = EditModeMenu;
                    app->menu_sel = 0;
                } else if(e.key == InputKeyBack) {
                    running = false;
                }
            }

            if(changed) recompute_activity(app);

            furi_mutex_release(app->mutex);

            if(want_save) {
                prompt_and_save(gui, vp, queue, storage, app, NULL, NULL);
            } else if(want_decode) {
                do_decode_action(gui, vp, queue, storage, app);
            } else if(want_cut) {
                do_cut_action(gui, vp, queue, dialogs, app);
            }

            view_port_update(vp);
        } else {
            view_port_update(vp);
        }
    }
}

static void show_oom_dialog(DialogsApp* dialogs, const char* text) {
    DialogMessage* m = dialog_message_alloc();
    dialog_message_set_header(m, "Out of memory", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(m, text, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(m, NULL, NULL, "OK");
    dialog_message_show(dialogs, m);
    dialog_message_free(m);
}

// Edit `file`, or a file picked in the browser when NULL.
static void run_editor(Storage* storage, DialogsApp* dialogs, const char* file) {
    App* app = safe_malloc(sizeof(App));
    if(!app) {
        show_oom_dialog(dialogs, "Not enough RAM to\nopen the editor.\nReboot Flipper.");
        return;
    }

    memset(app, 0, sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    FuriString* path = furi_string_alloc_set(file ? file : SUBGHZ_DIR);
    if(!file) {
        DialogsFileBrowserOptions br;
        dialog_file_browser_set_basic_options(&br, ".sub", &I_sub1_10px);
        br.base_path = SUBGHZ_DIR;
        if(!dialog_file_browser_show(dialogs, path, path, &br)) goto cleanup;
    }

    {
        const char* full = furi_string_get_cstr(path);
        const char* slash = strrchr(full, '/');
        const char* nm = slash ? slash + 1 : full;
        strncpy(app->basename, nm, sizeof(app->basename) - 1);
        char* dot = strrchr(app->basename, '.');
        if(dot) *dot = '\0';

        size_t dl = strlen(SUBGHZ_DIR);
        app->src_in_dir = slash && (size_t)(slash - full) == dl &&
                          strncmp(full, SUBGHZ_DIR, dl) == 0 && strlen(nm) < sizeof(app->basename);
    }

    Gui* gui = furi_record_open(RECORD_GUI);

    if(!btn_select_for_file(gui, storage, furi_string_get_cstr(path))) {
        furi_record_close(RECORD_GUI);
        goto cleanup;
    }

    ViewPort* vp = view_port_alloc();
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    view_port_draw_callback_set(vp, draw_cb, app);
    view_port_input_callback_set(vp, input_cb, queue);
    app->loading = true;
    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    view_port_update(vp);

    bool loaded = (g_merge_repeat > 1) ?
                      load_sub_repeated(storage, furi_string_get_cstr(path), &app->sd) :
                      load_sub(storage, furi_string_get_cstr(path), &app->sd);

    if(app->sd.out_of_memory || !loaded) {
        gui_remove_view_port(gui, vp);
        view_port_free(vp);
        furi_message_queue_free(queue);
        furi_record_close(RECORD_GUI);

        DialogMessage* m = dialog_message_alloc();
        if(app->sd.out_of_memory) {
            dialog_message_set_header(m, "Out of memory", 64, 2, AlignCenter, AlignTop);
            dialog_message_set_text(
                m,
                "Not enough free RAM to\nload this capture.\nReboot Flipper, then\nopen the app first.",
                64,
                34,
                AlignCenter,
                AlignCenter);
        } else {
            dialog_message_set_header(m, APP_NAME, 64, 4, AlignCenter, AlignTop);
            dialog_message_set_text(
                m, "Unsupported protocol or file is empty", 64, 32, AlignCenter, AlignCenter);
        }

        dialog_message_set_buttons(m, NULL, NULL, "OK");
        dialog_message_show(dialogs, m);
        dialog_message_free(m);
        goto cleanup;
    }

    run_editor_session(gui, vp, queue, storage, dialogs, app, true);

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);

cleanup:
    furi_string_free(path);

    if(app->sd.data) free(app->sd.data);

    if(app->undo_data) free(app->undo_data);

    furi_mutex_free(app->mutex);
    free(app);
}

static void merge_loading_draw_cb(Canvas* c, void* ctx) {
    UNUSED(ctx);
    canvas_clear(c);
    canvas_set_font(c, FontPrimary);
    canvas_draw_str_aligned(c, 64, 32, AlignCenter, AlignCenter, "Loading...");
}

static DialogMessageButton merge_ask_more(DialogsApp* dialogs, int n, size_t total) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Added %d file(s).\n%u samples.", n, (unsigned)total);
    DialogMessage* m = dialog_message_alloc();
    dialog_message_set_header(m, "Merge .sub files", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(m, msg, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(m, "Done", NULL, "Add more");
    DialogMessageButton btn = dialog_message_show(dialogs, m);
    dialog_message_free(m);
    return btn;
}

static void run_merge(Storage* storage, DialogsApp* dialogs) {
    App* app = safe_malloc(sizeof(App));
    if(!app) {
        show_oom_dialog(dialogs, "Not enough RAM to\nstart merging.\nReboot Flipper.");
        return;
    }

    memset(app, 0, sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    // Packed buffer of file paths: <len_byte><path_bytes><len_byte>...<0>.
    // A 0 length byte marks the end (paths are never empty). Grows on demand so
    // the only real limit is available RAM.
    size_t pcap = MERGE_BUF_CHUNK;
    size_t pused = 0;
    uint8_t* pbuf = safe_malloc(pcap);
    if(!pbuf) {
        show_oom_dialog(dialogs, "Not enough RAM to\nstart merging.\nReboot Flipper.");
        furi_mutex_free(app->mutex);
        free(app);
        return;
    }
    pbuf[0] = 0;

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* load_vp = view_port_alloc();
    view_port_draw_callback_set(load_vp, merge_loading_draw_cb, NULL);

    int n = 0;
    size_t total = 0;
    bool aborted = false;
    uint32_t freq = 433920000;
    char preset[48];
    strncpy(preset, "FuriHalSubGhzPresetOok650Async", sizeof(preset) - 1);
    preset[sizeof(preset) - 1] = '\0';

    FuriString* path = furi_string_alloc_set(SUBGHZ_DIR);
    DialogsFileBrowserOptions br;
    dialog_file_browser_set_basic_options(&br, ".sub", &I_sub1_10px);
    br.base_path = SUBGHZ_DIR;

    while(true) {
        if(!dialog_file_browser_show(dialogs, path, path, &br)) {
            if(n == 0) break;

            DialogMessageButton b = merge_ask_more(dialogs, n, total);
            if(b == DialogMessageButtonBack) {
                aborted = true;
                break;
            }

            if(b != DialogMessageButtonRight) break;

            continue;
        }

        if(!btn_select_for_file(gui, storage, furi_string_get_cstr(path))) continue;

        gui_add_view_port(gui, load_vp, GuiLayerFullscreen);
        view_port_update(load_vp);
        uint32_t f = freq;
        char pr[48];
        strncpy(pr, preset, sizeof(pr) - 1);
        pr[sizeof(pr) - 1] = '\0';
        size_t cnt = count_sub_samples(storage, furi_string_get_cstr(path), &f, pr, sizeof(pr));
        gui_remove_view_port(gui, load_vp);

        if(cnt == 0) {
            DialogMessage* m = dialog_message_alloc();
            dialog_message_set_header(m, "Unsupported file", 64, 2, AlignCenter, AlignTop);
            dialog_message_set_text(
                m, "Unsupported protocol or file is empty", 64, 34, AlignCenter, AlignCenter);
            dialog_message_set_buttons(m, NULL, NULL, "OK");
            dialog_message_show(dialogs, m);
            dialog_message_free(m);
            continue;
        }

        if(n > 0 && f != freq) {
            char msg[128];
            snprintf(
                msg,
                sizeof(msg),
                "Merge: %lu.%02lu MHz\nFile: %lu.%02lu MHz",
                (unsigned long)(freq / 1000000),
                (unsigned long)((freq % 1000000) / 10000),
                (unsigned long)(f / 1000000),
                (unsigned long)((f % 1000000) / 10000));
            DialogMessage* m = dialog_message_alloc();
            dialog_message_set_header(m, "Frequency mismatch", 64, 2, AlignCenter, AlignTop);
            dialog_message_set_text(m, msg, 64, 32, AlignCenter, AlignCenter);
            dialog_message_set_buttons(m, NULL, NULL, "OK");
            dialog_message_show(dialogs, m);
            dialog_message_free(m);
            continue;
        }

        // Each copy contributes cnt samples. Repetitions reuse the signal's own
        // native trailing silence (no extra samples), while a new file inserts
        // the manual gap, which spans several samples once it exceeds DUR_CLAMP.
        size_t copies = (size_t)g_merge_repeat;
        size_t gap_chunks = duration_samples(g_merge_gap_ms * 1000);
        size_t extra = cnt * copies + (n > 0 ? gap_chunks : 0);
        size_t newtotal = total + extra;
        bool over_cap = newtotal > MAX_SAMPLES;
        bool over_ram = newtotal * sizeof(int16_t) + LOAD_HEAP_RESERVE > memmgr_get_free_heap();
        if(over_cap || over_ram) {
            char msg[128];
            snprintf(
                msg,
                sizeof(msg),
                "Have %u, +%u.\n%s %d.",
                (unsigned)total,
                (unsigned)cnt,
                over_cap ? "Limit" : "RAM fits ~",
                over_cap ? MAX_SAMPLES : (int)(memmgr_get_free_heap() / sizeof(int16_t)));

            DialogMessage* m = dialog_message_alloc();
            dialog_message_set_header(
                m, over_cap ? "Won't fit" : "Out of memory", 64, 2, AlignCenter, AlignTop);
            dialog_message_set_text(m, msg, 64, 34, AlignCenter, AlignCenter);
            dialog_message_set_buttons(m, NULL, NULL, "OK");
            dialog_message_show(dialogs, m);
            dialog_message_free(m);
            continue;
        }

        const char* cstr = furi_string_get_cstr(path);
        size_t plen = strlen(cstr);
        if(plen > MERGE_PATH_LEN - 1) plen = MERGE_PATH_LEN - 1;

        // Append <len_byte><path><btn_byte> to the packed buffer, growing if
        // needed. +3 keeps room for this entry's len byte, the chosen custom
        // button and the final terminator.
        size_t need = pused + 1 + plen + 1 + 1;
        if(need > pcap) {
            size_t newcap = pcap;
            while(newcap < need)
                newcap *= 2;

            uint8_t* np = safe_realloc(pbuf, newcap);
            if(!np) {
                DialogMessage* m = dialog_message_alloc();
                dialog_message_set_header(m, "Merge .sub files", 64, 2, AlignCenter, AlignTop);
                dialog_message_set_text(
                    m,
                    "Out of RAM for more\nfiles. Merging what\nwas added so far.",
                    64,
                    32,
                    AlignCenter,
                    AlignCenter);
                dialog_message_set_buttons(m, NULL, "OK", NULL);
                dialog_message_show(dialogs, m);
                dialog_message_free(m);
                break;
            }

            pbuf = np;
            pcap = newcap;
        }

        pbuf[pused++] = (uint8_t)plen;
        memcpy(pbuf + pused, cstr, plen);
        pused += plen;
        pbuf[pused++] = g_custom_btn_id;
        pbuf[pused] = 0;

        if(n == 0) {
            freq = f;
            strncpy(preset, pr, sizeof(preset) - 1);
            preset[sizeof(preset) - 1] = '\0';
        }

        total = newtotal;
        n++;

        DialogMessageButton b = merge_ask_more(dialogs, n, total);
        if(b == DialogMessageButtonBack) {
            aborted = true;
            break;
        }

        if(b != DialogMessageButtonRight) break;
    }

    furi_string_free(path);

    if(aborted || n < 1 || total < 2) {
        free(pbuf);
        goto cleanup;
    }

    app->sd.data = safe_malloc(total * sizeof(int16_t));
    if(!app->sd.data) {
        free(pbuf);
        show_oom_dialog(dialogs, "Not enough RAM to\nbuild the merge.\nReboot Flipper.");
        goto cleanup;
    }

    app->sd.cap = total;
    app->sd.count = 0;
    app->sd.frequency = freq;
    strncpy(app->sd.preset, preset, sizeof(app->sd.preset) - 1);
    app->sd.preset[sizeof(app->sd.preset) - 1] = '\0';

    gui_add_view_port(gui, load_vp, GuiLayerFullscreen);
    view_port_update(load_vp);

    char pathbuf[MERGE_PATH_LEN];
    size_t off = 0;
    for(int i = 0; i < n; i++) {
        size_t plen = pbuf[off++];
        memcpy(pathbuf, pbuf + off, plen);
        pathbuf[plen] = '\0';
        off += plen;
        g_custom_btn_id = pbuf[off++];

        SubData sig = {0};
        if(load_signal(storage, pathbuf, &sig) && sig.data) {
            app->sd.synthesized |= sig.synthesized;

            for(int r = 0; r < g_merge_repeat; r++) {
                MergeJoin join = (i == 0 && r == 0) ? MergeJoinNone :
                                 (r == 0)           ? MergeJoinManual :
                                                      MergeJoinNative;
                append_signal(&app->sd, &sig, join);
            }
        }

        if(sig.data) free(sig.data);
    }

    if(g_normalize_jitter) normalize_jitter(&app->sd);

    gui_remove_view_port(gui, load_vp);
    free(pbuf);

    recompute_total_us(&app->sd);
    strncpy(app->basename, "merged", sizeof(app->basename) - 1);

    ViewPort* vp = view_port_alloc();
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    view_port_draw_callback_set(vp, draw_cb, app);
    view_port_input_callback_set(vp, input_cb, queue);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    run_editor_session(gui, vp, queue, storage, dialogs, app, false);

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(queue);

cleanup:
    view_port_free(load_vp);
    furi_record_close(RECORD_GUI);

    if(app->sd.data) free(app->sd.data);

    if(app->undo_data) free(app->undo_data);

    furi_mutex_free(app->mutex);
    free(app);
}

typedef enum {
    MenuViewSubmenu,
    MenuViewAbout,
    MenuViewConfig,
    MenuViewGapInput,
} MenuViewId;

typedef enum {
    MenuItemSelectFile,
    MenuItemMergeFiles,
    MenuItemConfig,
    MenuItemAbout,
} MenuItemId;

typedef enum {
    MenuActionNone = 0,
    MenuActionEdit,
    MenuActionMerge,
} MenuAction;

typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    VariableItemList* config_list;
    NumberInput* num_input;
    VariableItem* gap_item;
    VariableItem* repeat_item;
    int editing;
    Storage* storage;
    DialogsApp* dialogs;
    MenuAction action;
} Menu;

static void menu_submenu_cb(void* context, uint32_t index) {
    Menu* menu = context;
    if(index == MenuItemSelectFile) {
        menu->action = MenuActionEdit;
        view_dispatcher_stop(menu->view_dispatcher);
    } else if(index == MenuItemMergeFiles) {
        menu->action = MenuActionMerge;
        view_dispatcher_stop(menu->view_dispatcher);
    } else if(index == MenuItemConfig) {
        view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewConfig);
    } else if(index == MenuItemAbout) {
        view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewAbout);
    }
}

static uint32_t about_back_cb(void* context) {
    UNUSED(context);
    return MenuViewSubmenu;
}

static uint32_t config_back_cb(void* context) {
    UNUSED(context);
    return MenuViewSubmenu;
}

static void config_norm_changed_cb(VariableItem* item) {
    uint8_t idx = variable_item_get_current_value_index(item);
    g_normalize_jitter = (idx != 0);
    variable_item_set_current_value_text(item, idx ? "ON" : "OFF");
}

typedef enum {
    ConfigItemNormalize = 0,
    ConfigItemGap,
    ConfigItemRepeat,
} ConfigItem;

static void config_gap_sync_item(VariableItem* item) {
    // The gap uses a 3-slot stepper (down / hold / up) recentred on every change
    // so ±1 ms stepping works across the whole 1..1000 range, which is far
    // beyond a VariableItem's 255-value index limit.
    variable_item_set_current_value_index(item, 1);

    char buf[8];
    snprintf(buf, sizeof(buf), "%ld ms", (long)g_merge_gap_ms);
    variable_item_set_current_value_text(item, buf);
}

static void config_repeat_sync_item(VariableItem* item) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%ldx", (long)g_merge_repeat);
    variable_item_set_current_value_index(item, g_merge_repeat - MERGE_REPEAT_MIN);
    variable_item_set_current_value_text(item, buf);
}

static void config_gap_changed_cb(VariableItem* item) {
    uint8_t idx = variable_item_get_current_value_index(item);
    if(idx == 0 && g_merge_gap_ms > MERGE_GAP_MIN_MS)
        g_merge_gap_ms--;
    else if(idx == 2 && g_merge_gap_ms < MERGE_GAP_MAX_MS)
        g_merge_gap_ms++;

    config_gap_sync_item(item);
}

static void config_repeat_changed_cb(VariableItem* item) {
    g_merge_repeat = MERGE_REPEAT_MIN + variable_item_get_current_value_index(item);
    config_repeat_sync_item(item);
}

static void config_num_input_cb(void* context, int32_t v) {
    Menu* menu = context;

    if(menu->editing == ConfigItemGap) {
        if(v < MERGE_GAP_MIN_MS) v = MERGE_GAP_MIN_MS;
        if(v > MERGE_GAP_MAX_MS) v = MERGE_GAP_MAX_MS;

        g_merge_gap_ms = v;
        config_gap_sync_item(menu->gap_item);
    } else if(menu->editing == ConfigItemRepeat) {
        if(v < MERGE_REPEAT_MIN) v = MERGE_REPEAT_MIN;
        if(v > MERGE_REPEAT_MAX) v = MERGE_REPEAT_MAX;

        g_merge_repeat = v;
        config_repeat_sync_item(menu->repeat_item);
    }

    view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewConfig);
}

static uint32_t config_num_input_back_cb(void* context) {
    UNUSED(context);
    return MenuViewConfig;
}

// OK on a numeric config item opens a number picker.
static void config_enter_cb(void* context, uint32_t index) {
    Menu* menu = context;

    const char* header;
    int32_t current, min_val, max_val;
    if(index == ConfigItemGap) {
        header = "Merge gap [ms]";
        current = g_merge_gap_ms;
        min_val = MERGE_GAP_MIN_MS;
        max_val = MERGE_GAP_MAX_MS;
    } else if(index == ConfigItemRepeat) {
        header = "Repeat each (max: " STR(MERGE_REPEAT_MAX) ")";
        current = g_merge_repeat;
        min_val = MERGE_REPEAT_MIN;
        max_val = MERGE_REPEAT_MAX;
    } else {
        return; // non-numeric item (e.g. Normalize jitter)
    }

    menu->editing = index;
    number_input_set_header_text(menu->num_input, header);
    number_input_set_result_callback(
        menu->num_input, config_num_input_cb, menu, current, min_val, max_val);
    view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewGapInput);
}

static void menu_build_config(Menu* menu) {
    VariableItem* it = variable_item_list_add(
        menu->config_list, "Normalize jitter", 2, config_norm_changed_cb, menu);
    variable_item_set_current_value_index(it, g_normalize_jitter ? 1 : 0);
    variable_item_set_current_value_text(it, g_normalize_jitter ? "ON" : "OFF");

    menu->gap_item =
        variable_item_list_add(menu->config_list, "Merge gap", 3, config_gap_changed_cb, menu);
    config_gap_sync_item(menu->gap_item);

    menu->repeat_item = variable_item_list_add(
        menu->config_list,
        "Repeat",
        MERGE_REPEAT_MAX - MERGE_REPEAT_MIN + 1,
        config_repeat_changed_cb,
        menu);
    config_repeat_sync_item(menu->repeat_item);

    variable_item_list_set_enter_callback(menu->config_list, config_enter_cb, menu);
}

static uint32_t submenu_back_cb(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static void menu_build_about(Menu* menu) {
    int title_h = FONT_SIZE_PX * 2;
    widget_add_text_box_element(
        menu->widget,
        0,
        0,
        SCREEN_W_PX,
        title_h,
        AlignCenter,
        AlignBottom,
        "\e#\e!     Sub-GHz RAW Edit     \e!\n",
        false);

    int scroll_element_offset = title_h + GAP_PX;
    widget_add_text_scroll_element(
        menu->widget,
        0,
        scroll_element_offset,
        SCREEN_W_PX,
        SCREEN_H_PX - scroll_element_offset,
        "\e#Version " APP_VERSION "\n"
        "Enjoyed? Leave a star:\n"
        "" APP_REPO "\n"
        "\n"
        "A tiny on-device waveform\n"
        "editor for Flipper Zero that\n"
        "trims RAW .sub captures\n"
        "down to just the part\n"
        "you care about.\n"
        "\n"
        "Full documentation available on the GitHub.");
}

int32_t subghz_raw_edit_app(void* p) {
    // Launched with a file (e.g. Sub-GHz -> More -> Edit): open it directly.
    const char* file = p;
    bool direct = file && *file;

    Menu* menu = safe_malloc(sizeof(Menu));
    if(!menu) return 0;

    memset(menu, 0, sizeof(Menu));

    menu->storage = furi_record_open(RECORD_STORAGE);
    menu->dialogs = furi_record_open(RECORD_DIALOGS);
    Gui* gui = furi_record_open(RECORD_GUI);

    menu->view_dispatcher = view_dispatcher_alloc();
    menu->submenu = submenu_alloc();
    menu->widget = widget_alloc();
    menu->config_list = variable_item_list_alloc();
    menu->num_input = number_input_alloc();

    submenu_set_header(menu->submenu, APP_NAME);
    submenu_add_item(menu->submenu, "Select .sub file", MenuItemSelectFile, menu_submenu_cb, menu);
    submenu_add_item(menu->submenu, "Merge .sub files", MenuItemMergeFiles, menu_submenu_cb, menu);
    submenu_add_item(menu->submenu, "Config", MenuItemConfig, menu_submenu_cb, menu);
    submenu_add_item(menu->submenu, "About", MenuItemAbout, menu_submenu_cb, menu);

    menu_build_about(menu);
    menu_build_config(menu);

    view_set_previous_callback(submenu_get_view(menu->submenu), submenu_back_cb);
    view_set_previous_callback(widget_get_view(menu->widget), about_back_cb);
    view_set_previous_callback(variable_item_list_get_view(menu->config_list), config_back_cb);
    view_set_previous_callback(number_input_get_view(menu->num_input), config_num_input_back_cb);

    view_dispatcher_attach_to_gui(menu->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(
        menu->view_dispatcher, MenuViewSubmenu, submenu_get_view(menu->submenu));
    view_dispatcher_add_view(menu->view_dispatcher, MenuViewAbout, widget_get_view(menu->widget));
    view_dispatcher_add_view(
        menu->view_dispatcher, MenuViewConfig, variable_item_list_get_view(menu->config_list));
    view_dispatcher_add_view(
        menu->view_dispatcher, MenuViewGapInput, number_input_get_view(menu->num_input));

    if(direct) run_editor(menu->storage, menu->dialogs, file);

    bool running = !direct;
    while(running) {
        menu->action = MenuActionNone;
        view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewSubmenu);
        view_dispatcher_run(menu->view_dispatcher);

        if(menu->action == MenuActionEdit) {
            run_editor(menu->storage, menu->dialogs, NULL);
        } else if(menu->action == MenuActionMerge) {
            run_merge(menu->storage, menu->dialogs);
        } else {
            running = false;
        }
    }

    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewSubmenu);
    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewAbout);
    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewConfig);
    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewGapInput);
    submenu_free(menu->submenu);
    widget_free(menu->widget);
    variable_item_list_free(menu->config_list);
    number_input_free(menu->num_input);
    view_dispatcher_free(menu->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);

    free(menu);
    return 0;
}
