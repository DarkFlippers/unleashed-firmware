#include "wav_parser.h"

#define TAG "WavParser"

const char* format_text(FormatTag tag) {
    switch(tag) {
    case FormatTagPCM:
        return "PCM";
    case FormatTagIEEE_FLOAT:
        return "IEEE FLOAT";
    default:
        return "Unknown";
    }
};

struct WavParser {
    WavHeaderChunk header;
    WavFormatChunk format;
    WavDataChunk data;
    size_t wav_data_start;
    size_t wav_data_end;
};

WavParser* wav_parser_alloc() {
    return malloc(sizeof(WavParser));
}

void wav_parser_free(WavParser* parser) {
    free(parser);
}

bool wav_parser_parse(WavParser* parser, Stream* stream, WavPlayerApp* app) {
    stream_read(stream, (uint8_t*)&parser->header, sizeof(WavHeaderChunk));
    stream_read(stream, (uint8_t*)&parser->format, sizeof(WavFormatChunk));
    stream_read(stream, (uint8_t*)&parser->data, sizeof(WavDataChunk));

    if(memcmp(parser->header.riff, "RIFF", 4) != 0 ||
       memcmp(parser->header.wave, "WAVE", 4) != 0) {
        FURI_LOG_E(TAG, "WAV: wrong header");
        return false;
    }

    if(memcmp(parser->format.fmt, "fmt ", 4) != 0) {
        FURI_LOG_E(TAG, "WAV: wrong format");
        return false;
    }

    if(parser->format.tag != FormatTagPCM || memcmp(parser->data.data, "data", 4) != 0) {
        FURI_LOG_E(
            TAG,
            "WAV: non-PCM format %u, next '%lu'",
            parser->format.tag,
            (uint32_t)parser->data.data);
        return false;
    }

    FURI_LOG_I(
        TAG,
        "Format tag: %s, ch: %u, smplrate: %lu, bps: %lu, bits: %u",
        format_text(parser->format.tag),
        parser->format.channels,
        parser->format.sample_rate,
        parser->format.byte_per_sec,
        parser->format.bits_per_sample);

    app->sample_rate = parser->format.sample_rate;
    app->num_channels = parser->format.channels;
    app->bits_per_sample = parser->format.bits_per_sample;

    parser->wav_data_start = stream_tell(stream);
    parser->wav_data_end = parser->wav_data_start + parser->data.size;

    FURI_LOG_I(TAG, "data: %u - %u", parser->wav_data_start, parser->wav_data_end);

    return true;
}

size_t wav_parser_get_data_start(WavParser* parser) {
    return parser->wav_data_start;
}

size_t wav_parser_get_data_end(WavParser* parser) {
    return parser->wav_data_end;
}

size_t wav_parser_get_data_len(WavParser* parser) {
    return parser->wav_data_end - parser->wav_data_start;
}
