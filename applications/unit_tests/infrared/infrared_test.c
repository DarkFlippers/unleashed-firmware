#include <furi.h>
#include <flipper_format.h>
#include <infrared.h>
#include <common/infrared_common_i.h>
#include "../minunit.h"

#define IR_TEST_FILES_DIR EXT_PATH("unit_tests/infrared/")
#define IR_TEST_FILE_PREFIX "test_"
#define IR_TEST_FILE_SUFFIX ".irtest"

typedef struct {
    InfraredDecoderHandler* decoder_handler;
    InfraredEncoderHandler* encoder_handler;
    string_t file_path;
    FlipperFormat* ff;
} InfraredTest;

static InfraredTest* test;

static void infrared_test_alloc() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    test = malloc(sizeof(InfraredTest));
    test->decoder_handler = infrared_alloc_decoder();
    test->encoder_handler = infrared_alloc_encoder();
    test->ff = flipper_format_buffered_file_alloc(storage);
    string_init(test->file_path);
}

static void infrared_test_free() {
    furi_assert(test);
    infrared_free_decoder(test->decoder_handler);
    infrared_free_encoder(test->encoder_handler);
    flipper_format_free(test->ff);
    string_clear(test->file_path);
    furi_record_close(RECORD_STORAGE);
    free(test);
    test = NULL;
}

static bool infrared_test_prepare_file(const char* protocol_name) {
    string_t file_type;
    string_init(file_type);
    bool success = false;

    string_printf(
        test->file_path,
        "%s%s%s%s",
        IR_TEST_FILES_DIR,
        IR_TEST_FILE_PREFIX,
        protocol_name,
        IR_TEST_FILE_SUFFIX);

    do {
        uint32_t format_version;
        if(!flipper_format_buffered_file_open_existing(test->ff, string_get_cstr(test->file_path)))
            break;
        if(!flipper_format_read_header(test->ff, file_type, &format_version)) break;
        if(string_cmp_str(file_type, "IR tests file") || format_version != 1) break;
        success = true;
    } while(false);

    string_clear(file_type);
    return success;
}

static bool infrared_test_load_raw_signal(
    FlipperFormat* ff,
    const char* signal_name,
    uint32_t** timings,
    uint32_t* timings_count) {
    string_t buf;
    string_init(buf);
    bool success = false;

    do {
        bool is_name_found = false;
        for(; !is_name_found && flipper_format_read_string(ff, "name", buf);
            is_name_found = !string_cmp_str(buf, signal_name))
            ;

        if(!is_name_found) break;
        if(!flipper_format_read_string(ff, "type", buf) || string_cmp_str(buf, "raw")) break;
        if(!flipper_format_get_value_count(ff, "data", timings_count)) break;
        if(!*timings_count) break;

        *timings = malloc(*timings_count * sizeof(uint32_t*));
        if(!flipper_format_read_uint32(ff, "data", *timings, *timings_count)) {
            free(*timings);
            break;
        }
        success = true;
    } while(false);

    string_clear(buf);
    return success;
}

static bool infrared_test_read_message(FlipperFormat* ff, InfraredMessage* message) {
    string_t buf;
    string_init(buf);
    bool success = false;

    do {
        if(!flipper_format_read_string(ff, "protocol", buf)) break;
        message->protocol = infrared_get_protocol_by_name(string_get_cstr(buf));
        if(!infrared_is_protocol_valid(message->protocol)) break;
        if(!flipper_format_read_hex(ff, "address", (uint8_t*)&message->address, sizeof(uint32_t)))
            break;
        if(!flipper_format_read_hex(ff, "command", (uint8_t*)&message->command, sizeof(uint32_t)))
            break;
        if(!flipper_format_read_bool(ff, "repeat", &message->repeat, 1)) break;
        success = true;
    } while(false);

    string_clear(buf);
    return success;
}

static bool infrared_test_load_messages(
    FlipperFormat* ff,
    const char* signal_name,
    InfraredMessage** messages,
    uint32_t* messages_count) {
    string_t buf;
    string_init(buf);
    bool success = false;

    do {
        bool is_name_found = false;
        for(; !is_name_found && flipper_format_read_string(ff, "name", buf);
            is_name_found = !string_cmp_str(buf, signal_name))
            ;

        if(!is_name_found) break;
        if(!flipper_format_read_string(ff, "type", buf) || string_cmp_str(buf, "parsed_array"))
            break;
        if(!flipper_format_read_uint32(ff, "count", messages_count, 1)) break;
        if(!*messages_count) break;

        *messages = malloc(*messages_count * sizeof(InfraredMessage));
        uint32_t i;
        for(i = 0; i < *messages_count; ++i) {
            if(!infrared_test_read_message(ff, (*messages) + i)) {
                break;
            }
        }
        if(*messages_count != i) {
            free(*messages);
            break;
        }
        success = true;
    } while(false);

    string_clear(buf);
    return success;
}

static void infrared_test_compare_message_results(
    const InfraredMessage* message_decoded,
    const InfraredMessage* message_expected) {
    mu_check(message_decoded->protocol == message_expected->protocol);
    mu_check(message_decoded->command == message_expected->command);
    mu_check(message_decoded->address == message_expected->address);
    if((message_expected->protocol == InfraredProtocolSIRC) ||
       (message_expected->protocol == InfraredProtocolSIRC15) ||
       (message_expected->protocol == InfraredProtocolSIRC20)) {
        mu_check(message_decoded->repeat == false);
    } else {
        mu_check(message_decoded->repeat == message_expected->repeat);
    }
}

/* Encodes signal and merges same levels (high+high, low+low) */
static void infrared_test_run_encoder_fill_array(
    InfraredEncoderHandler* handler,
    uint32_t* timings,
    uint32_t* timings_len,
    bool* start_level) {
    uint32_t duration = 0;
    bool level = false;
    bool level_read;
    InfraredStatus status = InfraredStatusError;
    size_t i = 0;
    bool first = true;

    while(1) {
        status = infrared_encode(handler, &duration, &level_read);
        if(first) {
            if(start_level) *start_level = level_read;
            first = false;
            timings[0] = 0;
        } else if(level_read != level) {
            ++i;
            furi_check(i < *timings_len);
            timings[i] = 0;
        }
        level = level_read;
        timings[i] += duration;

        furi_check((status == InfraredStatusOk) || (status == InfraredStatusDone));
        if(status == InfraredStatusDone) break;
    }

    *timings_len = i + 1;
}

// messages in input array for encoder should have one protocol
static void infrared_test_run_encoder(InfraredProtocol protocol, uint32_t test_index) {
    uint32_t* timings;
    uint32_t timings_count = 200;
    uint32_t* expected_timings;
    uint32_t expected_timings_count;
    InfraredMessage* input_messages;
    uint32_t input_messages_count;

    string_t buf;
    string_init(buf);

    const char* protocol_name = infrared_get_protocol_name(protocol);
    mu_assert(infrared_test_prepare_file(protocol_name), "Failed to prepare test file");

    string_printf(buf, "encoder_input%d", test_index);
    mu_assert(
        infrared_test_load_messages(
            test->ff, string_get_cstr(buf), &input_messages, &input_messages_count),
        "Failed to load messages from file");

    string_printf(buf, "encoder_expected%d", test_index);
    mu_assert(
        infrared_test_load_raw_signal(
            test->ff, string_get_cstr(buf), &expected_timings, &expected_timings_count),
        "Failed to load raw signal from file");

    flipper_format_buffered_file_close(test->ff);
    string_clear(buf);

    uint32_t j = 0;
    timings = malloc(sizeof(uint32_t) * timings_count);

    for(uint32_t message_counter = 0; message_counter < input_messages_count; ++message_counter) {
        const InfraredMessage* message = &input_messages[message_counter];
        if(!message->repeat) {
            infrared_reset_encoder(test->encoder_handler, message);
        }

        timings_count = 200;
        infrared_test_run_encoder_fill_array(test->encoder_handler, timings, &timings_count, NULL);
        furi_check(timings_count <= 200);

        for(size_t i = 0; i < timings_count; ++i, ++j) {
            mu_check(MATCH_TIMING(timings[i], expected_timings[j], 120));
            mu_assert(j < expected_timings_count, "encoded more timings than expected");
        }
    }

    free(input_messages);
    free(expected_timings);
    free(timings);

    mu_assert(j == expected_timings_count, "encoded less timings than expected");
}

static void infrared_test_run_encoder_decoder(InfraredProtocol protocol, uint32_t test_index) {
    uint32_t* timings = 0;
    uint32_t timings_count = 200;
    InfraredMessage* input_messages;
    uint32_t input_messages_count;
    bool level = false;

    string_t buf;
    string_init(buf);

    timings = malloc(sizeof(uint32_t) * timings_count);

    const char* protocol_name = infrared_get_protocol_name(protocol);
    mu_assert(infrared_test_prepare_file(protocol_name), "Failed to prepare test file");

    string_printf(buf, "encoder_decoder_input%d", test_index);
    mu_assert(
        infrared_test_load_messages(
            test->ff, string_get_cstr(buf), &input_messages, &input_messages_count),
        "Failed to load messages from file");

    flipper_format_buffered_file_close(test->ff);
    string_clear(buf);

    for(uint32_t message_counter = 0; message_counter < input_messages_count; ++message_counter) {
        const InfraredMessage* message_encoded = &input_messages[message_counter];
        if(!message_encoded->repeat) {
            infrared_reset_encoder(test->encoder_handler, message_encoded);
        }

        timings_count = 200;
        infrared_test_run_encoder_fill_array(
            test->encoder_handler, timings, &timings_count, &level);
        furi_check(timings_count <= 200);

        const InfraredMessage* message_decoded = 0;
        for(size_t i = 0; i < timings_count; ++i) {
            message_decoded = infrared_decode(test->decoder_handler, level, timings[i]);
            if((i == timings_count - 2) && level && message_decoded) {
                /* In case we end with space timing - message can be decoded at last mark */
                break;
            } else if(i < timings_count - 1) {
                mu_check(!message_decoded);
            } else {
                if(!message_decoded) {
                    message_decoded = infrared_check_decoder_ready(test->decoder_handler);
                }
                mu_check(message_decoded);
            }
            level = !level;
        }
        if(message_decoded) {
            infrared_test_compare_message_results(message_decoded, message_encoded);
        } else {
            mu_check(0);
        }
    }
    free(input_messages);
    free(timings);
}

static void infrared_test_run_decoder(InfraredProtocol protocol, uint32_t test_index) {
    uint32_t* timings;
    uint32_t timings_count;
    InfraredMessage* messages;
    uint32_t messages_count;

    string_t buf;
    string_init(buf);

    mu_assert(
        infrared_test_prepare_file(infrared_get_protocol_name(protocol)),
        "Failed to prepare test file");

    string_printf(buf, "decoder_input%d", test_index);
    mu_assert(
        infrared_test_load_raw_signal(test->ff, string_get_cstr(buf), &timings, &timings_count),
        "Failed to load raw signal from file");

    string_printf(buf, "decoder_expected%d", test_index);
    mu_assert(
        infrared_test_load_messages(test->ff, string_get_cstr(buf), &messages, &messages_count),
        "Failed to load messages from file");

    flipper_format_buffered_file_close(test->ff);
    string_clear(buf);

    InfraredMessage message_decoded_check_local;
    bool level = 0;
    uint32_t message_counter = 0;
    const InfraredMessage* message_decoded = 0;

    for(uint32_t i = 0; i < timings_count; ++i) {
        const InfraredMessage* message_decoded_check = 0;

        if(timings[i] > INFRARED_RAW_RX_TIMING_DELAY_US) {
            message_decoded_check = infrared_check_decoder_ready(test->decoder_handler);
            if(message_decoded_check) {
                /* infrared_decode() can reset message, but we have to call infrared_decode() to perform real
                 * simulation: infrared_check() by timeout, then infrared_decode() when meet edge */
                message_decoded_check_local = *message_decoded_check;
                message_decoded_check = &message_decoded_check_local;
            }
        }

        message_decoded = infrared_decode(test->decoder_handler, level, timings[i]);

        if(message_decoded_check || message_decoded) {
            mu_assert(
                !(message_decoded_check && message_decoded),
                "both messages decoded: check_ready() and infrared_decode()");

            if(message_decoded_check) {
                message_decoded = message_decoded_check;
            }

            mu_assert(message_counter < messages_count, "decoded more than expected");
            infrared_test_compare_message_results(message_decoded, &messages[message_counter]);

            ++message_counter;
        }
        level = !level;
    }

    message_decoded = infrared_check_decoder_ready(test->decoder_handler);
    if(message_decoded) {
        infrared_test_compare_message_results(message_decoded, &messages[message_counter]);
        ++message_counter;
    }

    free(timings);
    free(messages);

    mu_assert(message_counter == messages_count, "decoded less than expected");
}

MU_TEST(infrared_test_decoder_samsung32) {
    infrared_test_run_decoder(InfraredProtocolSamsung32, 1);
}

MU_TEST(infrared_test_decoder_mixed) {
    infrared_test_run_decoder(InfraredProtocolRC5, 2);
    infrared_test_run_decoder(InfraredProtocolSIRC, 1);
    infrared_test_run_decoder(InfraredProtocolNECext, 1);
    infrared_test_run_decoder(InfraredProtocolRC6, 2);
    infrared_test_run_decoder(InfraredProtocolSamsung32, 1);
    infrared_test_run_decoder(InfraredProtocolRC6, 1);
    infrared_test_run_decoder(InfraredProtocolSamsung32, 1);
    infrared_test_run_decoder(InfraredProtocolRC5, 1);
    infrared_test_run_decoder(InfraredProtocolSIRC, 2);
    infrared_test_run_decoder(InfraredProtocolNECext, 1);
    infrared_test_run_decoder(InfraredProtocolSIRC, 4);
    infrared_test_run_decoder(InfraredProtocolNEC, 2);
    infrared_test_run_decoder(InfraredProtocolRC6, 1);
    infrared_test_run_decoder(InfraredProtocolNECext, 1);
    infrared_test_run_decoder(InfraredProtocolSIRC, 5);
    infrared_test_run_decoder(InfraredProtocolNEC, 3);
    infrared_test_run_decoder(InfraredProtocolRC5, 5);
    infrared_test_run_decoder(InfraredProtocolSamsung32, 1);
    infrared_test_run_decoder(InfraredProtocolSIRC, 3);
}

MU_TEST(infrared_test_decoder_nec) {
    infrared_test_run_decoder(InfraredProtocolNEC, 1);
    infrared_test_run_decoder(InfraredProtocolNEC, 2);
    infrared_test_run_decoder(InfraredProtocolNEC, 3);
}

MU_TEST(infrared_test_decoder_unexpected_end_in_sequence) {
    infrared_test_run_decoder(InfraredProtocolNEC, 1);
    infrared_test_run_decoder(InfraredProtocolNEC, 1);
    infrared_test_run_decoder(InfraredProtocolNEC, 2);
    infrared_test_run_decoder(InfraredProtocolNEC, 2);
}

MU_TEST(infrared_test_decoder_necext1) {
    infrared_test_run_decoder(InfraredProtocolNECext, 1);
    infrared_test_run_decoder(InfraredProtocolNECext, 1);
}

MU_TEST(infrared_test_decoder_long_packets_with_nec_start) {
    infrared_test_run_decoder(InfraredProtocolNEC42ext, 1);
    infrared_test_run_decoder(InfraredProtocolNEC42ext, 2);
}

MU_TEST(infrared_test_encoder_sirc) {
    infrared_test_run_encoder(InfraredProtocolSIRC, 1);
    infrared_test_run_encoder(InfraredProtocolSIRC, 2);
}

MU_TEST(infrared_test_decoder_sirc) {
    infrared_test_run_decoder(InfraredProtocolSIRC, 3);
    infrared_test_run_decoder(InfraredProtocolSIRC, 1);
    infrared_test_run_decoder(InfraredProtocolSIRC, 2);
    infrared_test_run_decoder(InfraredProtocolSIRC, 4);
    infrared_test_run_decoder(InfraredProtocolSIRC, 5);
}

MU_TEST(infrared_test_decoder_rc5) {
    infrared_test_run_decoder(InfraredProtocolRC5X, 1);
    infrared_test_run_decoder(InfraredProtocolRC5, 1);
    infrared_test_run_decoder(InfraredProtocolRC5, 2);
    infrared_test_run_decoder(InfraredProtocolRC5, 3);
    infrared_test_run_decoder(InfraredProtocolRC5, 4);
    infrared_test_run_decoder(InfraredProtocolRC5, 5);
    infrared_test_run_decoder(InfraredProtocolRC5, 6);
    infrared_test_run_decoder(InfraredProtocolRC5, 7);
}

MU_TEST(infrared_test_encoder_rc5x) {
    infrared_test_run_encoder(InfraredProtocolRC5X, 1);
}

MU_TEST(infrared_test_encoder_rc5) {
    infrared_test_run_encoder(InfraredProtocolRC5, 1);
}

MU_TEST(infrared_test_decoder_rc6) {
    infrared_test_run_decoder(InfraredProtocolRC6, 1);
}

MU_TEST(infrared_test_encoder_rc6) {
    infrared_test_run_encoder(InfraredProtocolRC6, 1);
}

MU_TEST(infrared_test_encoder_decoder_all) {
    infrared_test_run_encoder_decoder(InfraredProtocolNEC, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolNECext, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolNEC42, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolNEC42ext, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolSamsung32, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolRC6, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolRC5, 1);
    infrared_test_run_encoder_decoder(InfraredProtocolSIRC, 1);
}

MU_TEST_SUITE(infrared_test) {
    MU_SUITE_CONFIGURE(&infrared_test_alloc, &infrared_test_free);

    MU_RUN_TEST(infrared_test_encoder_sirc);
    MU_RUN_TEST(infrared_test_decoder_sirc);
    MU_RUN_TEST(infrared_test_encoder_rc5x);
    MU_RUN_TEST(infrared_test_encoder_rc5);
    MU_RUN_TEST(infrared_test_decoder_rc5);
    MU_RUN_TEST(infrared_test_decoder_rc6);
    MU_RUN_TEST(infrared_test_encoder_rc6);
    MU_RUN_TEST(infrared_test_decoder_unexpected_end_in_sequence);
    MU_RUN_TEST(infrared_test_decoder_long_packets_with_nec_start);
    MU_RUN_TEST(infrared_test_decoder_nec);
    MU_RUN_TEST(infrared_test_decoder_samsung32);
    MU_RUN_TEST(infrared_test_decoder_necext1);
    MU_RUN_TEST(infrared_test_decoder_mixed);
    MU_RUN_TEST(infrared_test_encoder_decoder_all);
}

int run_minunit_test_infrared() {
    MU_RUN_SUITE(infrared_test);
    return MU_EXIT_CODE;
}
