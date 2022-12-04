#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "message.h"

typedef struct MidiParser MidiParser;

MidiParser* midi_parser_alloc(void);

void midi_parser_free(MidiParser* parser);

bool midi_parser_parse(MidiParser* parser, uint8_t data);

MidiEvent* midi_parser_get_message(MidiParser* parser);