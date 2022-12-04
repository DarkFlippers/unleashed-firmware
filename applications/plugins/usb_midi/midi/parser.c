#include <stdlib.h>
#include "parser.h"

typedef enum {
    ParserEmpty,
    ParserHasStatus,
    ParserHasData0,
    ParserSysEx,
} ParserState;

const uint8_t kStatusByteMask = 0x80;
const uint8_t kMessageMask = 0x70;
const uint8_t kDataByteMask = 0x7F;
const uint8_t kSystemCommonMask = 0xF0;
const uint8_t kChannelMask = 0x0F;
const uint8_t kRealTimeMask = 0xF8;
const uint8_t kSystemRealTimeMask = 0x07;

struct MidiParser {
    MidiMessageType status;
    ParserState state;
    MidiEvent incoming_message;
};

MidiParser* midi_parser_alloc(void) {
    MidiParser* parser = malloc(sizeof(MidiParser));
    parser->incoming_message.type = MessageLast;
    parser->state = ParserEmpty;
    return parser;
}

void midi_parser_free(MidiParser* parser) {
    free(parser);
}

bool midi_parser_parse(MidiParser* parser, uint8_t byte) {
    bool parsed = false;
    MidiEvent* event = &parser->incoming_message;

    switch(parser->state) {
    case ParserEmpty:
        // check byte for valid Status Byte
        if(byte & kStatusByteMask) {
            // Get MessageType, and Channel
            event->channel = byte & kChannelMask;
            event->type = (MidiMessageType)((byte & kMessageMask) >> 4);

            // Validate, and move on.
            if(event->type < MessageLast) {
                parser->state = ParserHasStatus;
                // Mark this status byte as running_status
                parser->status = event->type;

                if(parser->status == SystemCommon) {
                    event->channel = 0;
                    //system real time = 1111 1xxx
                    if(byte & 0x08) {
                        event->type = SystemRealTime;
                        parser->status = SystemRealTime;
                        event->srt_type = (SystemRealTimeType)(byte & kSystemRealTimeMask);

                        //short circuit to start
                        parser->state = ParserEmpty;
                        //queue_.push(incoming_message_);
                        parsed = true;
                    }
                    //system common
                    else {
                        event->sc_type = (SystemCommonType)(byte & 0x07);
                        //sysex
                        if(event->sc_type == SystemExclusive) {
                            parser->state = ParserSysEx;
                            event->sysex_message_len = 0;
                        }
                        //short circuit
                        else if(event->sc_type > SongSelect) {
                            parser->state = ParserEmpty;
                            //queue_.push(incoming_message_);
                            parsed = true;
                        }
                    }
                }
            }
            // Else we'll keep waiting for a valid incoming status byte
        } else {
            // Handle as running status
            event->type = parser->status;
            event->data[0] = byte & kDataByteMask;
            parser->state = ParserHasData0;
        }
        break;
    case ParserHasStatus:
        if((byte & kStatusByteMask) == 0) {
            event->data[0] = byte & kDataByteMask;
            if(parser->status == ChannelPressure || parser->status == ProgramChange ||
               event->sc_type == MTCQuarterFrame || event->sc_type == SongSelect) {
                //these are just one data byte, so we short circuit back to start
                parser->state = ParserEmpty;
                //queue_.push(incoming_message_);
                parsed = true;
            } else {
                parser->state = ParserHasData0;
            }

            //ChannelModeMessages (reserved Control Changes)
            if(parser->status == ControlChange && event->data[0] > 119) {
                event->type = ChannelMode;
                parser->status = ChannelMode;
                event->cm_type = (ChannelModeType)(event->data[0] - 120);
            }
        } else {
            // invalid message go back to start ;p
            parser->state = ParserEmpty;
        }
        break;
    case ParserHasData0:
        if((byte & kStatusByteMask) == 0) {
            event->data[1] = byte & kDataByteMask;
            // At this point the message is valid, and we can add this MidiEvent to the queue
            //queue_.push(incoming_message_);
            parsed = true;
        }
        // Regardless, of whether the data was valid or not we go back to empty
        // because either the message is queued for handling or its not.
        parser->state = ParserEmpty;
        break;
    case ParserSysEx:
        // end of sysex
        if(byte == 0xf7) {
            parser->state = ParserEmpty;
            //queue_.push(incoming_message_);
            parsed = true;
        } else {
            if(event->sysex_message_len < SYSEX_BUFFER_LEN) {
                event->sysex_data[event->sysex_message_len] = byte;
                event->sysex_message_len++;
            }
        }
        break;
    default:
        break;
    }

    return parsed;
}

MidiEvent* midi_parser_get_message(MidiParser* parser) {
    return &parser->incoming_message;
}