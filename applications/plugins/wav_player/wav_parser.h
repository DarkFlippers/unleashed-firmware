#pragma once
#include <toolbox/stream/stream.h>

#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include <stm32wbxx_ll_dma.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/view_dispatcher.h>
#include <toolbox/stream/file_stream.h>

#include "wav_player_view.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FormatTagPCM = 0x0001,
    FormatTagIEEE_FLOAT = 0x0003,
} FormatTag;

typedef struct {
    uint8_t riff[4];
    uint32_t size;
    uint8_t wave[4];
} WavHeaderChunk;

typedef struct {
    uint8_t fmt[4];
    uint32_t size;
    uint16_t tag;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WavFormatChunk;

typedef struct {
    uint8_t data[4];
    uint32_t size;
} WavDataChunk;

typedef struct WavParser WavParser;

typedef struct {
    Storage* storage;
    Stream* stream;
    WavParser* parser;
    uint16_t* sample_buffer;
    uint8_t* tmp_buffer;

    uint32_t sample_rate;

    uint16_t num_channels;
    uint16_t bits_per_sample;

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

WavParser* wav_parser_alloc();

void wav_parser_free(WavParser* parser);

bool wav_parser_parse(WavParser* parser, Stream* stream, WavPlayerApp* app);

size_t wav_parser_get_data_start(WavParser* parser);

size_t wav_parser_get_data_end(WavParser* parser);

size_t wav_parser_get_data_len(WavParser* parser);

#ifdef __cplusplus
}
#endif