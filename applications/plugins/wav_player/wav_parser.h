#pragma once
#include <toolbox/stream/stream.h>

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

WavParser* wav_parser_alloc();

void wav_parser_free(WavParser* parser);

bool wav_parser_parse(WavParser* parser, Stream* stream);

size_t wav_parser_get_data_start(WavParser* parser);

size_t wav_parser_get_data_end(WavParser* parser);

size_t wav_parser_get_data_len(WavParser* parser);

#ifdef __cplusplus
}
#endif