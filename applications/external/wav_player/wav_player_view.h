#pragma once
#include <gui/view.h>

#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include <stm32wbxx_ll_dma.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/view_dispatcher.h>
#include <toolbox/stream/file_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WavPlayerView WavPlayerView;

typedef enum {
    WavPlayerCtrlVolUp,
    WavPlayerCtrlVolDn,
    WavPlayerCtrlMoveL,
    WavPlayerCtrlMoveR,
    WavPlayerCtrlOk,
    WavPlayerCtrlBack,
} WavPlayerCtrl;

typedef void (*WavPlayerCtrlCallback)(WavPlayerCtrl ctrl, void* context);

#define DATA_COUNT 116

struct WavPlayerView {
    View* view;
    WavPlayerCtrlCallback callback;
    void* context;
};

typedef struct {
    bool play;
    float volume;
    size_t start;
    size_t end;
    size_t current;
    uint8_t data[DATA_COUNT];

    uint16_t bits_per_sample;
    uint16_t num_channels;
} WavPlayerViewModel;

WavPlayerView* wav_player_view_alloc();

void wav_player_view_free(WavPlayerView* wav_view);

View* wav_player_view_get_view(WavPlayerView* wav_view);

void wav_player_view_set_volume(WavPlayerView* wav_view, float volume);

void wav_player_view_set_start(WavPlayerView* wav_view, size_t start);

void wav_player_view_set_end(WavPlayerView* wav_view, size_t end);

void wav_player_view_set_current(WavPlayerView* wav_view, size_t current);

void wav_player_view_set_play(WavPlayerView* wav_view, bool play);

void wav_player_view_set_data(WavPlayerView* wav_view, uint16_t* data, size_t data_count);

void wav_player_view_set_bits(WavPlayerView* wav_view, uint16_t bit);
void wav_player_view_set_chans(WavPlayerView* wav_view, uint16_t chn);

void wav_player_view_set_ctrl_callback(WavPlayerView* wav_view, WavPlayerCtrlCallback callback);

void wav_player_view_set_context(WavPlayerView* wav_view, void* context);

#ifdef __cplusplus
}
#endif