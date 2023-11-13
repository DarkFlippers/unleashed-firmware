#pragma once

#include "../../signal_reader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso15693Parser Iso15693Parser;

typedef enum {
    Iso15693ParserEventDataReceived,
} Iso15693ParserEvent;

typedef void (*Iso15693ParserCallback)(Iso15693ParserEvent event, void* context);

Iso15693Parser* iso15693_parser_alloc(const GpioPin* pin, size_t max_frame_size);

void iso15693_parser_free(Iso15693Parser* instance);

void iso15693_parser_reset(Iso15693Parser* instance);

void iso15693_parser_start(
    Iso15693Parser* instance,
    Iso15693ParserCallback callback,
    void* context);

void iso15693_parser_stop(Iso15693Parser* instance);

bool iso15693_parser_run(Iso15693Parser* instance);

size_t iso15693_parser_get_data_size_bytes(Iso15693Parser* instance);

void iso15693_parser_get_data(
    Iso15693Parser* instance,
    uint8_t* buff,
    size_t buff_size,
    size_t* data_bits);

#ifdef __cplusplus
}
#endif
