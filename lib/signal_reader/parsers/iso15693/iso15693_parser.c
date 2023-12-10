#include "iso15693_parser.h"

#include <toolbox/bit_buffer.h>

#include <furi/furi.h>

#define ISO15693_PARSER_SIGNAL_READER_BUFF_SIZE (2)
#define ISO15693_PARSER_BITSTREAM_BUFF_SIZE (32)
#define ISO15693_PARSER_BITRATE_F64MHZ (603U)

#define TAG "Iso15693Parser"

typedef enum {
    Iso15693ParserStateParseSoF,
    Iso15693ParserStateParseFrame,
    Iso15693ParserStateFail,
} Iso15693ParserState;

typedef enum {
    Iso15693ParserMode1OutOf4,
    Iso15693ParserMode1OutOf256,

    Iso15693ParserModeNum,
} Iso15693ParserMode;

struct Iso15693Parser {
    Iso15693ParserState state;
    Iso15693ParserMode mode;

    SignalReader* signal_reader;

    uint8_t bitstream_buff[ISO15693_PARSER_BITSTREAM_BUFF_SIZE];
    size_t bitstream_idx;
    uint8_t last_byte;

    bool signal_detected;
    bool bit_offset_calculated;
    uint8_t bit_offset;
    size_t byte_idx;
    size_t bytes_to_process;

    uint8_t next_byte;
    uint16_t next_byte_part;
    bool zero_found;

    BitBuffer* parsed_frame;
    bool eof_received;
    bool frame_parsed;

    Iso15693ParserCallback callback;
    void* context;
};

typedef enum {
    Iso15693ParserCommandProcessed,
    Iso15693ParserCommandWaitData,
    Iso15693ParserCommandFail,
    Iso15693ParserCommandSuccess,
} Iso15693ParserCommand;

typedef Iso15693ParserCommand (*Iso15693ParserStateHandler)(Iso15693Parser* instance);

Iso15693Parser* iso15693_parser_alloc(const GpioPin* pin, size_t max_frame_size) {
    Iso15693Parser* instance = malloc(sizeof(Iso15693Parser));
    instance->parsed_frame = bit_buffer_alloc(max_frame_size);

    instance->signal_reader = signal_reader_alloc(pin, ISO15693_PARSER_SIGNAL_READER_BUFF_SIZE);
    signal_reader_set_sample_rate(
        instance->signal_reader, SignalReaderTimeUnit64Mhz, ISO15693_PARSER_BITRATE_F64MHZ);
    signal_reader_set_pull(instance->signal_reader, GpioPullDown);
    signal_reader_set_polarity(instance->signal_reader, SignalReaderPolarityNormal);
    signal_reader_set_trigger(instance->signal_reader, SignalReaderTriggerRisingFallingEdge);

    return instance;
}

void iso15693_parser_free(Iso15693Parser* instance) {
    furi_assert(instance);

    bit_buffer_free(instance->parsed_frame);
    signal_reader_free(instance->signal_reader);
    free(instance);
}

void iso15693_parser_reset(Iso15693Parser* instance) {
    furi_assert(instance);

    instance->state = Iso15693ParserStateParseSoF;
    instance->mode = Iso15693ParserMode1OutOf4;
    memset(instance->bitstream_buff, 0x00, sizeof(instance->bitstream_buff));
    instance->bitstream_idx = 0;

    instance->next_byte = 0;
    instance->next_byte_part = 0;

    instance->bit_offset = 0;
    instance->byte_idx = 0;
    instance->bytes_to_process = 0;
    instance->signal_detected = false;
    instance->bit_offset_calculated = false;

    instance->last_byte = 0x00;
    instance->zero_found = false;
    instance->eof_received = false;

    bit_buffer_reset(instance->parsed_frame);
    instance->frame_parsed = false;
}

static void signal_reader_callback(SignalReaderEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.data->data);
    furi_assert(event.data->len == ISO15693_PARSER_SIGNAL_READER_BUFF_SIZE / 2);

    Iso15693Parser* instance = context;
    furi_assert(instance->callback);

    const uint8_t sof_1_out_of_4 = 0x21;
    const uint8_t sof_1_out_of_256 = 0x81;
    const uint8_t eof_single = 0x01;
    const uint8_t eof = 0x04;

    if(instance->state == Iso15693ParserStateParseSoF) {
        if(event.data->data[0] == sof_1_out_of_4) {
            instance->mode = Iso15693ParserMode1OutOf4;
            instance->state = Iso15693ParserStateParseFrame;
        } else if(event.data->data[0] == sof_1_out_of_256) {
            instance->mode = Iso15693ParserMode1OutOf256;
            instance->state = Iso15693ParserStateParseFrame;
        } else if(event.data->data[0] == eof_single) {
            instance->eof_received = true;
            instance->callback(Iso15693ParserEventDataReceived, instance->context);
        } else {
            instance->state = Iso15693ParserStateFail;
            instance->callback(Iso15693ParserEventDataReceived, instance->context);
        }
    } else {
        if(instance->mode == Iso15693ParserMode1OutOf4) {
            if(event.data->data[0] == eof) {
                instance->eof_received = true;
                instance->callback(Iso15693ParserEventDataReceived, instance->context);
            } else {
                instance->bitstream_buff[instance->bytes_to_process] = event.data->data[0];
                instance->bytes_to_process++;
                if(instance->bytes_to_process == ISO15693_PARSER_BITSTREAM_BUFF_SIZE) {
                    instance->callback(Iso15693ParserEventDataReceived, instance->context);
                }
            }
        } else {
            instance->bitstream_buff[instance->bytes_to_process] = event.data->data[0];
            instance->bytes_to_process++;
            if(instance->bytes_to_process == ISO15693_PARSER_BITSTREAM_BUFF_SIZE) {
                instance->callback(Iso15693ParserEventDataReceived, instance->context);
            }
        }
    }
}

static void iso15693_parser_start_signal_reader(Iso15693Parser* instance) {
    iso15693_parser_reset(instance);
    signal_reader_start(instance->signal_reader, signal_reader_callback, instance);
}

void iso15693_parser_start(
    Iso15693Parser* instance,
    Iso15693ParserCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
    iso15693_parser_start_signal_reader(instance);
}

void iso15693_parser_stop(Iso15693Parser* instance) {
    furi_assert(instance);

    signal_reader_stop(instance->signal_reader);
}

static Iso15693ParserCommand iso15693_parser_parse_1_out_of_4(Iso15693Parser* instance) {
    Iso15693ParserCommand command = Iso15693ParserCommandWaitData;
    const uint8_t bit_patterns_1_out_of_4[] = {0x02, 0x08, 0x20, 0x80};

    for(size_t i = 0; i < instance->bytes_to_process; i++) {
        // Check next pattern
        size_t j = 0;
        for(j = 0; j < COUNT_OF(bit_patterns_1_out_of_4); j++) {
            if(instance->bitstream_buff[i] == bit_patterns_1_out_of_4[j]) {
                instance->next_byte |= j << (instance->next_byte_part * 2);
                instance->next_byte_part++;
                if(instance->next_byte_part == 4) {
                    instance->next_byte_part = 0;
                    bit_buffer_append_byte(instance->parsed_frame, instance->next_byte);
                    instance->next_byte = 0;
                }
                break;
            }
        }
        if(j == COUNT_OF(bit_patterns_1_out_of_4)) {
            command = Iso15693ParserCommandFail;
            break;
        }
    }

    if(command != Iso15693ParserCommandFail) {
        if(instance->eof_received) {
            command = Iso15693ParserCommandSuccess;
            instance->frame_parsed = true;
        }
    }

    instance->bytes_to_process = 0;

    return command;
}

static Iso15693ParserCommand iso15693_parser_parse_1_out_of_256(Iso15693Parser* instance) {
    Iso15693ParserCommand command = Iso15693ParserCommandWaitData;
    const uint8_t eof = 0x04;

    for(size_t i = instance->byte_idx; i < instance->bytes_to_process; i++) {
        // Check EoF
        if(instance->next_byte_part == 0) {
            if(instance->bitstream_buff[i] == eof) {
                instance->frame_parsed = true;
                command = Iso15693ParserCommandSuccess;
                break;
            }
        }

        if(instance->zero_found) {
            if(instance->bitstream_buff[i] != 0x00) {
                command = Iso15693ParserCommandFail;
                break;
            }
        } else {
            if(instance->bitstream_buff[i] != 0x00) {
                for(size_t j = 0; j < 8; j++) {
                    if(FURI_BIT(instance->bitstream_buff[i], j) == 1) {
                        bit_buffer_append_byte(
                            instance->parsed_frame, instance->next_byte_part * 4 + j / 2);
                    }
                }
            }
        }
        instance->next_byte_part = (instance->next_byte_part + 1) % 64;
    }
    instance->bytes_to_process = 0;
    instance->byte_idx = 0;

    return command;
}

static const Iso15693ParserStateHandler iso15693_parser_state_handlers[Iso15693ParserModeNum] = {
    [Iso15693ParserMode1OutOf4] = iso15693_parser_parse_1_out_of_4,
    [Iso15693ParserMode1OutOf256] = iso15693_parser_parse_1_out_of_256,
};

bool iso15693_parser_run(Iso15693Parser* instance) {
    if(instance->state == Iso15693ParserStateFail) {
        iso15693_parser_stop(instance);
        iso15693_parser_start_signal_reader(instance);
    } else if((instance->state == Iso15693ParserStateParseSoF) && (instance->eof_received)) {
        instance->frame_parsed = true;
    } else if(instance->bytes_to_process) {
        Iso15693ParserCommand command = Iso15693ParserCommandProcessed;
        while(command == Iso15693ParserCommandProcessed) {
            command = iso15693_parser_state_handlers[instance->mode](instance);
        }

        if(command == Iso15693ParserCommandFail) {
            iso15693_parser_stop(instance);
            iso15693_parser_start_signal_reader(instance);
            FURI_LOG_D(TAG, "Frame parse failed");
        }
    }

    return instance->frame_parsed;
}

size_t iso15693_parser_get_data_size_bytes(Iso15693Parser* instance) {
    furi_assert(instance);

    return bit_buffer_get_size_bytes(instance->parsed_frame);
}

void iso15693_parser_get_data(
    Iso15693Parser* instance,
    uint8_t* buff,
    size_t buff_size,
    size_t* data_bits) {
    furi_assert(instance);
    furi_assert(buff);
    furi_assert(data_bits);

    bit_buffer_write_bytes(instance->parsed_frame, buff, buff_size);
    *data_bits = bit_buffer_get_size(instance->parsed_frame);
}
