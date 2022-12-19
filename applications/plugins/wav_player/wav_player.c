#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include <stm32wbxx_ll_dma.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/view_dispatcher.h>
#include <toolbox/stream/file_stream.h>
#include "wav_player_hal.h"
#include "wav_parser.h"
#include "wav_player_view.h"
#include <math.h>

#include <WAV_Player_icons.h>

#define TAG "WavPlayer"

#define WAVPLAYER_FOLDER "/ext/wav_player"

static bool open_wav_stream(Stream* stream) {
    DialogsApp* dialogs = furi_record_open("dialogs");
    bool result = false;
    FuriString* path;
    path = furi_string_alloc();
    furi_string_set(path, WAVPLAYER_FOLDER);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".wav", &I_music_10px);
    browser_options.base_path = WAVPLAYER_FOLDER;
    browser_options.hide_ext = false;

    bool ret = dialog_file_browser_show(dialogs, path, path, &browser_options);

    furi_record_close("dialogs");
    if(ret) {
        if(!file_stream_open(stream, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Cannot open file \"%s\"", furi_string_get_cstr(path));
        } else {
            result = true;
        }
    }
    furi_string_free(path);
    return result;
}

typedef enum {
    WavPlayerEventHalfTransfer,
    WavPlayerEventFullTransfer,
    WavPlayerEventCtrlVolUp,
    WavPlayerEventCtrlVolDn,
    WavPlayerEventCtrlMoveL,
    WavPlayerEventCtrlMoveR,
    WavPlayerEventCtrlOk,
    WavPlayerEventCtrlBack,
} WavPlayerEventType;

typedef struct {
    WavPlayerEventType type;
} WavPlayerEvent;

static void wav_player_dma_isr(void* ctx) {
    FuriMessageQueue* event_queue = ctx;

    // half of transfer
    if(LL_DMA_IsActiveFlag_HT1(DMA1)) {
        LL_DMA_ClearFlag_HT1(DMA1);
        // fill first half of buffer
        WavPlayerEvent event = {.type = WavPlayerEventHalfTransfer};
        furi_message_queue_put(event_queue, &event, 0);
    }

    // transfer complete
    if(LL_DMA_IsActiveFlag_TC1(DMA1)) {
        LL_DMA_ClearFlag_TC1(DMA1);
        // fill second half of buffer
        WavPlayerEvent event = {.type = WavPlayerEventFullTransfer};
        furi_message_queue_put(event_queue, &event, 0);
    }
}

typedef struct {
    Storage* storage;
    Stream* stream;
    WavParser* parser;
    uint16_t* sample_buffer;
    uint8_t* tmp_buffer;

    size_t samples_count_half;
    size_t samples_count;

    FuriMessageQueue* queue;

    float volume;
    bool play;

    WavPlayerView* view;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notification;
} WavPlayerApp;

static WavPlayerApp* app_alloc() {
    WavPlayerApp* app = malloc(sizeof(WavPlayerApp));
    app->samples_count_half = 1024 * 4;
    app->samples_count = app->samples_count_half * 2;
    app->storage = furi_record_open(RECORD_STORAGE);
    app->stream = file_stream_alloc(app->storage);
    app->parser = wav_parser_alloc();
    app->sample_buffer = malloc(sizeof(uint16_t) * app->samples_count);
    app->tmp_buffer = malloc(sizeof(uint8_t) * app->samples_count);
    app->queue = furi_message_queue_alloc(10, sizeof(WavPlayerEvent));

    app->volume = 10.0f;
    app->play = true;

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->view = wav_player_view_alloc();

    view_dispatcher_add_view(app->view_dispatcher, 0, wav_player_view_get_view(app->view));
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    app->notification = furi_record_open("notification");
    notification_message(app->notification, &sequence_display_backlight_enforce_on);

    return app;
}

static void app_free(WavPlayerApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    view_dispatcher_free(app->view_dispatcher);
    wav_player_view_free(app->view);
    furi_record_close(RECORD_GUI);

    furi_message_queue_free(app->queue);
    free(app->tmp_buffer);
    free(app->sample_buffer);
    wav_parser_free(app->parser);
    stream_free(app->stream);
    furi_record_close(RECORD_STORAGE);

    notification_message(app->notification, &sequence_display_backlight_enforce_auto);
    furi_record_close("notification");
    free(app);
}

// TODO: that works only with 8-bit 2ch audio
static bool fill_data(WavPlayerApp* app, size_t index) {
    uint16_t* sample_buffer_start = &app->sample_buffer[index];
    size_t count = stream_read(app->stream, app->tmp_buffer, app->samples_count);

    for(size_t i = count; i < app->samples_count; i++) {
        app->tmp_buffer[i] = 0;
    }

    for(size_t i = 0; i < app->samples_count; i += 2) {
        float data = app->tmp_buffer[i];
        data -= UINT8_MAX / 2; // to signed
        data /= UINT8_MAX / 2; // scale -1..1

        data *= app->volume; // volume
        data = tanhf(data); // hyperbolic tangent limiter

        data *= UINT8_MAX / 2; // scale -128..127
        data += UINT8_MAX / 2; // to unsigned

        if(data < 0) {
            data = 0;
        }

        if(data > 255) {
            data = 255;
        }

        sample_buffer_start[i / 2] = data;
    }

    wav_player_view_set_data(app->view, sample_buffer_start, app->samples_count_half);

    return count != app->samples_count;
}

static void ctrl_callback(WavPlayerCtrl ctrl, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    WavPlayerEvent event;

    switch(ctrl) {
    case WavPlayerCtrlVolUp:
        event.type = WavPlayerEventCtrlVolUp;
        furi_message_queue_put(event_queue, &event, 0);
        break;
    case WavPlayerCtrlVolDn:
        event.type = WavPlayerEventCtrlVolDn;
        furi_message_queue_put(event_queue, &event, 0);
        break;
    case WavPlayerCtrlMoveL:
        event.type = WavPlayerEventCtrlMoveL;
        furi_message_queue_put(event_queue, &event, 0);
        break;
    case WavPlayerCtrlMoveR:
        event.type = WavPlayerEventCtrlMoveR;
        furi_message_queue_put(event_queue, &event, 0);
        break;
    case WavPlayerCtrlOk:
        event.type = WavPlayerEventCtrlOk;
        furi_message_queue_put(event_queue, &event, 0);
        break;
    case WavPlayerCtrlBack:
        event.type = WavPlayerEventCtrlBack;
        furi_message_queue_put(event_queue, &event, 0);
        break;
    default:
        break;
    }
}

static void app_run(WavPlayerApp* app) {
    if(!open_wav_stream(app->stream)) return;
    if(!wav_parser_parse(app->parser, app->stream)) return;

    wav_player_view_set_volume(app->view, app->volume);
    wav_player_view_set_start(app->view, wav_parser_get_data_start(app->parser));
    wav_player_view_set_current(app->view, stream_tell(app->stream));
    wav_player_view_set_end(app->view, wav_parser_get_data_end(app->parser));
    wav_player_view_set_play(app->view, app->play);

    wav_player_view_set_context(app->view, app->queue);
    wav_player_view_set_ctrl_callback(app->view, ctrl_callback);

    bool eof = fill_data(app, 0);
    eof = fill_data(app, app->samples_count_half);

    wav_player_speaker_init();
    wav_player_dma_init((uint32_t)app->sample_buffer, app->samples_count);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdDma1Ch1, wav_player_dma_isr, app->queue);

    if(furi_hal_speaker_acquire(1000)) {
        wav_player_dma_start();
        wav_player_speaker_start();

        WavPlayerEvent event;

        while(1) {
            if(furi_message_queue_get(app->queue, &event, FuriWaitForever) == FuriStatusOk) {
                if(event.type == WavPlayerEventHalfTransfer) {
                    eof = fill_data(app, 0);
                    wav_player_view_set_current(app->view, stream_tell(app->stream));
                    if(eof) {
                        stream_seek(
                            app->stream,
                            wav_parser_get_data_start(app->parser),
                            StreamOffsetFromStart);
                    }

                } else if(event.type == WavPlayerEventFullTransfer) {
                    eof = fill_data(app, app->samples_count_half);
                    wav_player_view_set_current(app->view, stream_tell(app->stream));
                    if(eof) {
                        stream_seek(
                            app->stream,
                            wav_parser_get_data_start(app->parser),
                            StreamOffsetFromStart);
                    }
                } else if(event.type == WavPlayerEventCtrlVolUp) {
                    if(app->volume < 9.9) app->volume += 0.4;
                    wav_player_view_set_volume(app->view, app->volume);
                } else if(event.type == WavPlayerEventCtrlVolDn) {
                    if(app->volume > 0.01) app->volume -= 0.4;
                    wav_player_view_set_volume(app->view, app->volume);
                } else if(event.type == WavPlayerEventCtrlMoveL) {
                    int32_t seek =
                        stream_tell(app->stream) - wav_parser_get_data_start(app->parser);
                    seek =
                        MIN(seek, (int32_t)(wav_parser_get_data_len(app->parser) / (size_t)100));
                    stream_seek(app->stream, -seek, StreamOffsetFromCurrent);
                    wav_player_view_set_current(app->view, stream_tell(app->stream));
                } else if(event.type == WavPlayerEventCtrlMoveR) {
                    int32_t seek = wav_parser_get_data_end(app->parser) - stream_tell(app->stream);
                    seek =
                        MIN(seek, (int32_t)(wav_parser_get_data_len(app->parser) / (size_t)100));
                    stream_seek(app->stream, seek, StreamOffsetFromCurrent);
                    wav_player_view_set_current(app->view, stream_tell(app->stream));
                } else if(event.type == WavPlayerEventCtrlOk) {
                    app->play = !app->play;
                    wav_player_view_set_play(app->view, app->play);

                    if(!app->play) {
                        wav_player_speaker_stop();
                    } else {
                        wav_player_speaker_start();
                    }
                } else if(event.type == WavPlayerEventCtrlBack) {
                    break;
                }
            }
        }

        wav_player_speaker_stop();
        wav_player_dma_stop();
        furi_hal_speaker_release();
    }

    furi_hal_interrupt_set_isr(FuriHalInterruptIdDma1Ch1, NULL, NULL);
}

int32_t wav_player_app(void* p) {
    UNUSED(p);
    WavPlayerApp* app = app_alloc();

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, WAVPLAYER_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", WAVPLAYER_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    app_run(app);
    app_free(app);
    return 0;
}
