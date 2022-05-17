#pragma once
#include <gui/view.h>

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

WavPlayerView* wav_player_view_alloc();

void wav_player_view_free(WavPlayerView* wav_view);

View* wav_player_view_get_view(WavPlayerView* wav_view);

void wav_player_view_set_volume(WavPlayerView* wav_view, float volume);

void wav_player_view_set_start(WavPlayerView* wav_view, size_t start);

void wav_player_view_set_end(WavPlayerView* wav_view, size_t end);

void wav_player_view_set_current(WavPlayerView* wav_view, size_t current);

void wav_player_view_set_play(WavPlayerView* wav_view, bool play);

void wav_player_view_set_data(WavPlayerView* wav_view, uint16_t* data, size_t data_count);

void wav_player_view_set_ctrl_callback(WavPlayerView* wav_view, WavPlayerCtrlCallback callback);

void wav_player_view_set_context(WavPlayerView* wav_view, void* context);

#ifdef __cplusplus
}
#endif