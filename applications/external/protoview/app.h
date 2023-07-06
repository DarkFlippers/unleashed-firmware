/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
#include <notification/notification_messages.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/registry.h>
#include "raw_samples.h"
#include "helpers/radio_device_loader.h"

#define TAG "ProtoView"
#define PROTOVIEW_RAW_VIEW_DEFAULT_SCALE 100 // 100us is 1 pixel by default
#define BITMAP_SEEK_NOT_FOUND UINT32_MAX // Returned by function as sentinel
#define PROTOVIEW_VIEW_PRIVDATA_LEN 64 // View specific private data len

#define DEBUG_MSG 0

/* Forward declarations. */

typedef struct ProtoViewApp ProtoViewApp;
typedef struct ProtoViewMsgInfo ProtoViewMsgInfo;
typedef struct ProtoViewFieldSet ProtoViewFieldSet;
typedef struct ProtoViewDecoder ProtoViewDecoder;

/* ============================== enumerations ============================== */

/* Subghz system state */
typedef enum {
    TxRxStateIDLE,
    TxRxStateRx,
    TxRxStateTx,
    TxRxStateSleep,
} TxRxState;

/* Currently active view. */
typedef enum {
    ViewRawPulses,
    ViewInfo,
    ViewFrequencySettings,
    ViewModulationSettings,
    ViewBuildMessage,
    ViewDirectSampling,
    ViewLast, /* Just a sentinel to wrap around. */

    /* The following are special views that are not iterated, but
     * have meaning for the API. */
    ViewGoNext,
    ViewGoPrev,
} ProtoViewCurrentView;

/* ================================== RX/TX ================================= */

typedef struct {
    const char* name; // Name to show to the user.
    const char* id; // Identifier in the Flipper API/file.
    FuriHalSubGhzPreset preset; // The preset ID.
    uint8_t* custom; /* If not null, a set of registers for
                                       the CC1101, specifying a custom preset.*/
    uint32_t duration_filter; /* Ignore pulses and gaps that are less
                                       than the specified microseconds. This
                                       depends on the data rate. */
} ProtoViewModulation;

extern ProtoViewModulation ProtoViewModulations[]; /* In app_subghz.c */

/* This is the context of our subghz worker and associated thread.
 * It receives data and we get our protocol "feed" callback called
 * with the level (1 or 0) and duration. */
struct ProtoViewTxRx {
    bool freq_mod_changed; /* The user changed frequency and/or modulation
                                   from the interface. There is to restart the
                                   radio with the right parameters. */
    TxRxState txrx_state; /* Receiving, idle or sleeping? */

    /* Timer sampling mode state. */
    bool debug_timer_sampling; /* Read data from GDO0 in a busy loop. Only
                                   for testing. */
    uint32_t last_g0_change_time; /* Last high->low (or reverse) switch. */
    bool last_g0_value; /* Current value (high or low): we are
                                     checking the duration in the timer
                                     handler. */
};

typedef struct ProtoViewTxRx ProtoViewTxRx;

/* ============================== Main app state ============================ */

#define ALERT_MAX_LEN 32
struct ProtoViewApp {
    /* GUI */
    Gui* gui;
    NotificationApp* notification;
    ViewPort* view_port; /* We just use a raw viewport and we render
                                everything into the low level canvas. */
    ProtoViewCurrentView current_view; /* Active left-right view ID. */
    FuriMutex* view_updating_mutex; /* The Flipper GUI calls the screen redraw
                                       callback in a different thread. We
                                       use this mutex to protect the redraw
                                       from changes in app->view_privdata. */
    int current_subview[ViewLast]; /* Active up-down subview ID. */
    FuriMessageQueue* event_queue; /* Keypress events go here. */

    /* Input text state. */
    ViewDispatcher* view_dispatcher; /* Used only when we want to show
                                        the text_input view for a moment.
                                        Otherwise it is set to null. */
    TextInput* text_input;
    bool show_text_input;
    char* text_input_buffer;
    uint32_t text_input_buffer_len;
    void (*text_input_done_callback)(void*);

    /* Alert state. */
    uint32_t alert_dismiss_time; /* Millisecond when the alert will be
                                       no longer shown. Or zero if the alert
                                       is currently not set at all. */
    char alert_text[ALERT_MAX_LEN]; /* Alert content. */

    /* Radio related. */
    ProtoViewTxRx* txrx; /* Radio state. */
    SubGhzSetting* setting; /* A list of valid frequencies. */

    const SubGhzDevice* radio_device;

    /* Generic app state. */
    int running; /* Once false exists the app. */
    uint32_t signal_bestlen; /* Longest coherent signal observed so far. */
    uint32_t signal_last_scan_idx; /* Index of the buffer last time we
                                      performed the scan. */
    bool signal_decoded; /* Was the current signal decoded? */
    ProtoViewMsgInfo* msg_info; /* Decoded message info if not NULL. */
    bool direct_sampling_enabled; /* This special view needs an explicit
                                     acknowledge to work. */
    void* view_privdata; /* This is a piece of memory of total size
                               PROTOVIEW_VIEW_PRIVDATA_LEN that it is
                               initialized to zero when we switch to
                               a a new view. While the view we are using
                               is the same, it can be used by the view to
                               store any kind of info inside, just casting
                               the pointer to a few specific-data structure. */

    /* Raw view apps state. */
    uint32_t us_scale; /* microseconds per pixel. */
    uint32_t signal_offset; /* Long press left/right panning in raw view. */

    /* Configuration view app state. */
    uint32_t frequency; /* Current frequency. */
    uint8_t modulation; /* Current modulation ID, array index in the
                                ProtoViewModulations table. */
};

/* =========================== Protocols decoders =========================== */

/* This stucture is filled by the decoder for specific protocols with the
 * informations about the message. ProtoView will display such information
 * in the message info view. */
#define PROTOVIEW_MSG_STR_LEN 32
typedef struct ProtoViewMsgInfo {
    ProtoViewDecoder* decoder; /* The decoder that decoded the message. */
    ProtoViewFieldSet* fieldset; /* Decoded fields. */
    /* Low level information of the detected signal: the following are filled
     * by the protocol decoding function: */
    uint32_t start_off; /* Pulses start offset in the bitmap. */
    uint32_t pulses_count; /* Number of pulses of the full message. */
    /* The following are passed already filled to the decoder. */
    uint32_t short_pulse_dur; /* Microseconds duration of the short pulse. */
    /* The following are filled by ProtoView core after the decoder returned
     * success. */
    uint8_t* bits; /* Bitmap with the signal. */
    uint32_t bits_bytes; /* Number of full bytes in the bitmap, that
                                   is 'pulses_count/8' rounded to the next
                                   integer. */
} ProtoViewMsgInfo;

/* This structures describe a set of protocol fields. It is used by decoders
 * supporting message building to receive and return information about the
 * protocol. */
typedef enum {
    FieldTypeStr,
    FieldTypeSignedInt,
    FieldTypeUnsignedInt,
    FieldTypeBinary,
    FieldTypeHex,
    FieldTypeBytes,
    FieldTypeFloat,
} ProtoViewFieldType;

typedef struct {
    ProtoViewFieldType type;
    uint32_t len; // Depends on type:
        // Bits for integers (signed,unsigned,binary,hex).
        // Number of characters for strings.
        // Number of nibbles for bytes (1 for each 4 bits).
        // Number of digits after dot for floats.
    char* name; // Field name.
    union {
        char* str; // String type.
        int64_t value; // Signed integer type.
        uint64_t uvalue; // Unsigned integer type.
        uint8_t* bytes; // Raw bytes type.
        float fvalue; // Float type.
    };
} ProtoViewField;

typedef struct ProtoViewFieldSet {
    ProtoViewField** fields;
    uint32_t numfields;
} ProtoViewFieldSet;

typedef struct ProtoViewDecoder {
    const char* name; /* Protocol name. */
    /* The decode function takes a buffer that is actually a bitmap, with
     * high and low levels represented as 0 and 1. The number of high/low
     * pulses represented by the bitmap is passed as the 'numbits' argument,
     * while 'numbytes' represents the total size of the bitmap pointed by
     * 'bits'. So 'numbytes' is mainly useful to pass as argument to other
     * functions that perform bit extraction with bound checking, such as
     * bitmap_get() and so forth. */
    bool (*decode)(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info);
    /* This method is used by the decoder to return the fields it needs
     * in order to build a new message. This way the message builder view
     * can ask the user to fill the right set of fields of the specified
     * type. */
    void (*get_fields)(ProtoViewFieldSet* fields);
    /* This method takes the fields supported by the decoder, and
     * renders a message in 'samples'. */
    void (*build_message)(RawSamplesBuffer* samples, ProtoViewFieldSet* fields);
} ProtoViewDecoder;

extern RawSamplesBuffer *RawSamples, *DetectedSamples;

/* app_subghz.c */
void radio_begin(ProtoViewApp* app);
uint32_t radio_rx(ProtoViewApp* app);
void radio_idle(ProtoViewApp* app);
void radio_rx_end(ProtoViewApp* app);
void radio_sleep(ProtoViewApp* app);
void raw_sampling_worker_start(ProtoViewApp* app);
void raw_sampling_worker_stop(ProtoViewApp* app);
void radio_tx_signal(ProtoViewApp* app, FuriHalSubGhzAsyncTxCallback data_feeder, void* ctx);
void protoview_rx_callback(bool level, uint32_t duration, void* context);

/* signal.c */
uint32_t duration_delta(uint32_t a, uint32_t b);
void reset_current_signal(ProtoViewApp* app);
void scan_for_signal(ProtoViewApp* app, RawSamplesBuffer* source, uint32_t min_duration);
bool bitmap_get(uint8_t* b, uint32_t blen, uint32_t bitpos);
void bitmap_set(uint8_t* b, uint32_t blen, uint32_t bitpos, bool val);
void bitmap_copy(
    uint8_t* d,
    uint32_t dlen,
    uint32_t doff,
    uint8_t* s,
    uint32_t slen,
    uint32_t soff,
    uint32_t count);
void bitmap_set_pattern(uint8_t* b, uint32_t blen, uint32_t off, const char* pat);
void bitmap_reverse_bytes_bits(uint8_t* p, uint32_t len);
bool bitmap_match_bits(uint8_t* b, uint32_t blen, uint32_t bitpos, const char* bits);
uint32_t bitmap_seek_bits(
    uint8_t* b,
    uint32_t blen,
    uint32_t startpos,
    uint32_t maxbits,
    const char* bits);
bool bitmap_match_bitmap(
    uint8_t* b1,
    uint32_t b1len,
    uint32_t b1off,
    uint8_t* b2,
    uint32_t b2len,
    uint32_t b2off,
    uint32_t cmplen);
void bitmap_to_string(char* dst, uint8_t* b, uint32_t blen, uint32_t off, uint32_t len);
uint32_t convert_from_line_code(
    uint8_t* buf,
    uint64_t buflen,
    uint8_t* bits,
    uint32_t len,
    uint32_t offset,
    const char* zero_pattern,
    const char* one_pattern);
uint32_t convert_from_diff_manchester(
    uint8_t* buf,
    uint64_t buflen,
    uint8_t* bits,
    uint32_t len,
    uint32_t off,
    bool previous);
void init_msg_info(ProtoViewMsgInfo* i, ProtoViewApp* app);
void free_msg_info(ProtoViewMsgInfo* i);

/* signal_file.c */
bool save_signal(ProtoViewApp* app, const char* filename);

/* view_*.c */
void render_view_raw_pulses(Canvas* const canvas, ProtoViewApp* app);
void process_input_raw_pulses(ProtoViewApp* app, InputEvent input);
void render_view_settings(Canvas* const canvas, ProtoViewApp* app);
void process_input_settings(ProtoViewApp* app, InputEvent input);
void render_view_info(Canvas* const canvas, ProtoViewApp* app);
void process_input_info(ProtoViewApp* app, InputEvent input);
void render_view_direct_sampling(Canvas* const canvas, ProtoViewApp* app);
void process_input_direct_sampling(ProtoViewApp* app, InputEvent input);
void render_view_build_message(Canvas* const canvas, ProtoViewApp* app);
void process_input_build_message(ProtoViewApp* app, InputEvent input);
void view_enter_build_message(ProtoViewApp* app);
void view_exit_build_message(ProtoViewApp* app);
void view_enter_direct_sampling(ProtoViewApp* app);
void view_exit_direct_sampling(ProtoViewApp* app);
void view_exit_settings(ProtoViewApp* app);
void view_exit_info(ProtoViewApp* app);
void adjust_raw_view_scale(ProtoViewApp* app, uint32_t short_pulse_dur);

/* ui.c */
int ui_get_current_subview(ProtoViewApp* app);
void ui_show_available_subviews(Canvas* canvas, ProtoViewApp* app, int last_subview);
bool ui_process_subview_updown(ProtoViewApp* app, InputEvent input, int last_subview);
void ui_show_keyboard(
    ProtoViewApp* app,
    char* buffer,
    uint32_t buflen,
    void (*done_callback)(void*));
void ui_dismiss_keyboard(ProtoViewApp* app);
void ui_show_alert(ProtoViewApp* app, const char* text, uint32_t ttl);
void ui_dismiss_alert(ProtoViewApp* app);
void ui_draw_alert_if_needed(Canvas* canvas, ProtoViewApp* app);
void canvas_draw_str_with_border(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    const char* str,
    Color text_color,
    Color border_color);

/* fields.c */
void fieldset_free(ProtoViewFieldSet* fs);
ProtoViewFieldSet* fieldset_new(void);
void fieldset_add_int(ProtoViewFieldSet* fs, const char* name, int64_t val, uint8_t bits);
void fieldset_add_uint(ProtoViewFieldSet* fs, const char* name, uint64_t uval, uint8_t bits);
void fieldset_add_hex(ProtoViewFieldSet* fs, const char* name, uint64_t uval, uint8_t bits);
void fieldset_add_bin(ProtoViewFieldSet* fs, const char* name, uint64_t uval, uint8_t bits);
void fieldset_add_str(ProtoViewFieldSet* fs, const char* name, const char* s, size_t len);
void fieldset_add_bytes(
    ProtoViewFieldSet* fs,
    const char* name,
    const uint8_t* bytes,
    uint32_t count);
void fieldset_add_float(
    ProtoViewFieldSet* fs,
    const char* name,
    float val,
    uint32_t digits_after_dot);
const char* field_get_type_name(ProtoViewField* f);
int field_to_string(char* buf, size_t len, ProtoViewField* f);
bool field_set_from_string(ProtoViewField* f, char* buf, size_t len);
bool field_incr_value(ProtoViewField* f, int incr);
void fieldset_copy_matching_fields(ProtoViewFieldSet* dst, ProtoViewFieldSet* src);
void field_set_from_field(ProtoViewField* dst, ProtoViewField* src);

/* crc.c */
uint8_t crc8(const uint8_t* data, size_t len, uint8_t init, uint8_t poly);
uint8_t sum_bytes(const uint8_t* data, size_t len, uint8_t init);
uint8_t xor_bytes(const uint8_t* data, size_t len, uint8_t init);
