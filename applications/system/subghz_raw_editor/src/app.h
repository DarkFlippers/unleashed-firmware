#pragma once

/*
 * Sub-GHz RAW Edit - a tiny waveform editor for Flipper Zero RAW .sub captures.
 * https://github.com/Lechnio/SubGHz-RAW-Edit
 */

#ifdef HOST_TEST
// tests/: the few firmware APIs the editing code needs, on the host
#include "host.h"
#else
#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUBGHZ_DIR          "/ext/subghz"
#define SYNTH_YIELD_CAP     4096
#define NAME_SCROLL_HOLD_MS 1500
#define NAME_SCROLL_STEP_MS 300
#define DUR_CLAMP           32000
#define LOAD_HEAP_RESERVE   10240 // Decode (DECODE_HEAP), Save (a 4 KB line) and dialogs
#define LINE_RESERVE        4096 // Sub-GHz writes up to 512 RAW values (~3.5 KB) per line

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

#define APP_VERSION "2.0"
#define APP_REPO    "github.com/Lechnio/SubGHz-RAW-Edit"
#define APP_NAME    "Sub-GHz RAW Edit"

// Custom buttons and keeping the rolling counter need a custom firmware's SDK
#ifdef FW_ORIGIN_Official
#define APP_CUSTOM_FW_FEATURES false
#else
#define APP_CUSTOM_FW_FEATURES true
#endif

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

#define DECODE_MAX         32
#define DECODE_TAIL_US     100000
#define DECODE_LEAD_MIN_US 2000
#define DECODE_LEAD_MAX_US DUR_CLAMP
#define DECODE_LEAD_US     18500
#define DECODE_START_TRIES 32 // Locate: bursts tried back from the end of a frame
/* RAM Decode takes, from the sizes in the firmware's debug info plus 8 bytes of
 * allocator header on each piece: list 1160, environment 64, text ~290, largest
 * decoder (Security+ 1.0) 144, a rainbow-table read 1.2 K (FIL 608, LFN 520);
 * then either the result list (32 items, ~5 K) or Save (~1.7 K). ~7 K in all,
 * in pieces of at most the list. */
#define DECODE_HEAP        8192
#define DECODE_BLOCK       2048
#define DECODE_RUN_HEAP    2048 // one decoder and a rainbow-table read
#define DECODE_RUN_BLOCK   1024 // the FIL
#define DECODE_KEYS_HEAP   32768
#define DECODE_KEYS_BLOCK  8192 // the key array grows by copying: old and new at once

/* The whole signal lives in a working file of int16 samples on the card; RAM
 * holds a window of it: samples [win_s0, win_s0 + count). Times are always
 * counted from the start of the signal, not of the window. */
typedef struct {
    int16_t* data;
    size_t count;
    size_t cap;
    size_t limit; // synthesis: sample_budget() when it started
    size_t file_count; // samples in the working file
    size_t win_s0;
    int32_t win_t0; // time of data[0]
    int32_t win_us; // time the window spans
    int32_t total_us;
    uint32_t frequency;
    char preset[48];
    bool truncated;
    bool out_of_memory;
    bool synthesized;
} SubData;

typedef struct {
    size_t file_count;
    size_t win_s0;
    int32_t win_t0;
    int32_t total_us;
    int32_t marker_a;
    int32_t marker_b;
    int32_t view_start;
    int32_t view_end;
    int active;
} WorkState;

// What the menu's third item does; Left/Right on it switch.
typedef enum {
    EditCut,
    EditCopy,
    EditPaste,
    EditOps,
} EditOp;

// One step of the Undo history: the state before an edit, whose working file is undo_<id>.bin.
typedef struct {
    WorkState st;
    uint32_t id;
    int32_t spl; // samples the edit added, negative when it removed them
    int32_t at_us; // where it happened
    uint8_t op; // EditOp
} UndoEntry;

#define WORK_DIR            APP_DATA_PATH("tmp")
#define WORK_PATH           WORK_DIR "/work.bin"
#define WORK_NEW_PATH       WORK_DIR "/work.new"
#define CLIP_PATH           WORK_DIR "/clip.bin"
#define UNDO_DIR            WORK_DIR "/undos"
#define UNDO_MAX            32 // the oldest step is dropped past this
#define WORK_CHUNK          128 // samples per buffered file read or write
#define WINDOW_WARN_SAMPLES 15000 // a fresh boot fits ~38000

// Builds the working file sample by sample.
typedef struct {
    File* f;
    size_t count;
    int64_t total;
    size_t tail; // first sample of the trailing silence (count when it ends on a pulse)
    int32_t tail_us;
    bool skip_lead; // drop silence until the first pulse
    bool fail;
    size_t n;
    int16_t buf[WORK_CHUNK];
} WorkWriter;

typedef enum {
    EditModeEdit = 0,
    EditModeMenu = 1,
} EditMode;

typedef struct {
    SubData sd;
    char basename[48];
    bool src_in_dir; // the open file is SUBGHZ_DIR/<basename>.sub
    uint32_t opened_tick; // a name too long for the top bar scrolls from here

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

    const char* busy; // full-screen message while loading, decoding or saving

    int mode;
    int menu_sel;

    // A..B details shown beside the menu, filled when it opens
    uint32_t info_spl;
    uint32_t info_te;
    const char* info_enc;

    uint8_t edit_op; // EditOp
    size_t clip_count; // samples copied to CLIP_PATH
    int32_t clip_us;

    UndoEntry* undo; // the Undo history, oldest first
    size_t undo_n;
    uint32_t undo_seq; // id of the next step

    Storage* st;
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

typedef struct {
    File* file;
    uint8_t buf[128];
    size_t len;
    size_t pos;
    bool eof;
} LineReader;

#ifndef FW_ORIGIN_Official
#define BTN_SLOTS     5
#define BTN_LABEL_LEN 20

typedef struct {
    bool available;
    char protocol[32];
    char label[BTN_SLOTS][BTN_LABEL_LEN];
} BtnPick;
#endif

// How a signal being appended is joined to what's already in the buffer.
typedef enum {
    MergeJoinNone, // first signal of the whole merge; nothing precedes it
    MergeJoinManual, // start of a new file: separate with the manual gap
    MergeJoinNative, // a repetition of the same signal: keep its native gap
} MergeJoin;

typedef struct {
    const SubGhzProtocol* proto;
    uint32_t hash;
    uint16_t frames;
    char key[18];
    uint32_t first, last; // samples of the entry's first frame: its first and last pulse
} Decoded;

typedef struct {
    SubGhzEnvironment* env;
    size_t i0, i1;
    uint32_t lead_us;
    Decoded* list;
    size_t n;
    const Decoded* want; // replay mode: stop at the first frame matching this entry
    FuriString* text;
    const int16_t* data;
    size_t run, run_end; // the run being fed: samples [run, run_end)
    uint32_t run_gap; // its length when it is a gap, else 0
    bool hit;
    size_t hit_last; // replay: last pulse of the frame that hit
    bool oom;
} DecodeCtx;

typedef struct {
    ViewDispatcher* vd;
    bool confirmed;
    uint32_t index;
} PromptCtx;

/* ---- Shared between modules ---- */

/* The SDK builds apps with -mlong-calls so they can reach the firmware, but
 * calls between these modules stay inside the app: plain BL calls are enough
 * and save the literal and register a long call costs at every call site. */
#pragma no_long_calls

// view.c
extern const char busy_loading[];
void fmt_time(int32_t us, char* out, size_t n);
void clamp_marker(App* a, int idx);
void clamp_view(App* a);
void ensure_visible(App* a, int32_t m);
void recompute_activity(App* a);
void recompute_overview(App* a);
void select_frame(App* a, int32_t start, int32_t end, int32_t gap);
void auto_detect(App* a);
void draw_busy(Canvas* c, const char* text, const char* name);
void draw_cb(Canvas* c, void* ctx);
void input_cb(InputEvent* e, void* ctx);
void zoom(App* a, bool in);

// sel_info.c
void selection_info(App* a);

// load.c
extern int32_t g_merge_repeat;
extern int32_t g_merge_gap_ms;
extern bool g_normalize_jitter;
extern uint8_t g_custom_btn_id;
bool lr_read_line(LineReader* lr, FuriString* out);
void* safe_malloc(size_t size);
void* safe_realloc(void* ptr, size_t size);
size_t sample_budget(void);
SubGhzEnvironment* subghz_env_setup(void);
bool push_duration(SubData* sd, int32_t v);
size_t duration_samples(int32_t mag);
#ifndef FW_ORIGIN_Official
void btn_probe(Storage* storage, const char* path, const char* protocol, BtnPick* out);
#endif
float jitter_te(const int16_t* d, size_t n);
void jitter_apply(int16_t* d, size_t n, float te);
bool parse_sub_meta(
    const char* s,
    uint32_t* freq,
    char* preset,
    size_t preset_sz,
    char* protocol,
    size_t proto_sz);
size_t count_sub_samples(
    Storage* storage,
    const char* path,
    uint32_t* freq,
    char* preset,
    size_t preset_sz);
WorkWriter* work_begin(Storage* st);
bool work_add_sub(Storage* st, const char* path, WorkWriter* w, SubData* meta, MergeJoin join);
bool work_end(WorkWriter* w, SubData* sd);

// work.c
int32_t samples_us(const int16_t* d, size_t n);
bool work_windowed(const SubData* sd);
bool work_load(App* a, size_t s0);
bool work_slide(App* a, int dir);
bool work_open(App* a);
bool work_cut(App* a);
bool clip_store(App* a);
bool work_paste(App* a);
bool work_undo(App* a, size_t k);
void undo_label(const App* a, size_t k, char* out, size_t n);
void work_cleanup(Storage* st);

// edit_ops.c
void propose_name(
    Storage* st,
    const char* base,
    const char* sep,
    bool bare,
    char* out,
    size_t outlen);
void propose_edit_name(Storage* st, const App* a, char* out, size_t outlen);
bool selection_range(const App* a, size_t* i0, size_t* i1);
bool write_selection(Storage* st, App* a, const char* path);
size_t cut_run(App* a, bool apply);
void cut_compact(App* a);

// decode.c
bool write_decoded(Storage* st, App* a, DecodeCtx* dc, const Decoded* d, const char* path);
void decoded_base_name(const DecodeCtx* dc, const Decoded* d, char* out, size_t outlen);
void do_decode_action(Gui* gui, ViewPort* vp, FuriMessageQueue* queue, Storage* st, App* a);

// ui.c
bool btn_select_for_file(Gui* gui, Storage* storage, const char* path);
void pick_index_cb(void* ctx, uint32_t index);
void button_ok_cb(GuiButtonType button, InputType type, void* ctx);
bool prompt_run(Gui* gui, PromptCtx* pc, View* view);
bool prompt_filename(Gui* gui, char* namebuf, size_t buflen);
bool confirm(Gui* gui, const char* question);
void show_message(DialogsApp* dialogs, const char* header, const char* text, uint8_t text_y);

// editor.c
void prompt_and_save(
    Gui* gui,
    ViewPort* vp,
    FuriMessageQueue* queue,
    Storage* st,
    App* a,
    DecodeCtx* dc,
    const Decoded* d);
void run_editor_session(
    Gui* gui,
    ViewPort* vp,
    FuriMessageQueue* queue,
    Storage* storage,
    DialogsApp* dialogs,
    App* app,
    bool auto_select);
bool open_work(DialogsApp* dialogs, App* app);
void run_editor(Storage* storage, DialogsApp* dialogs, const char* file);

// merge.c
void run_merge(Storage* storage, DialogsApp* dialogs);

#pragma long_calls_off
