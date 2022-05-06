#include <furi.h>
#include "../minunit.h"
#include "infrared.h"
#include "common/infrared_common_i.h"
#include "test_data/infrared_nec_test_data.srcdata"
#include "test_data/infrared_necext_test_data.srcdata"
#include "test_data/infrared_samsung_test_data.srcdata"
#include "test_data/infrared_rc6_test_data.srcdata"
#include "test_data/infrared_rc5_test_data.srcdata"
#include "test_data/infrared_sirc_test_data.srcdata"

#define RUN_ENCODER(data, expected) \
    run_encoder((data), COUNT_OF(data), (expected), COUNT_OF(expected))

#define RUN_DECODER(data, expected) \
    run_decoder((data), COUNT_OF(data), (expected), COUNT_OF(expected))

#define RUN_ENCODER_DECODER(data) run_encoder_decoder((data), COUNT_OF(data))

static InfraredDecoderHandler* decoder_handler;
static InfraredEncoderHandler* encoder_handler;

static void test_setup(void) {
    decoder_handler = infrared_alloc_decoder();
    encoder_handler = infrared_alloc_encoder();
}

static void test_teardown(void) {
    infrared_free_decoder(decoder_handler);
    infrared_free_encoder(encoder_handler);
}

static void compare_message_results(
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
static void run_encoder_fill_array(
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
static void run_encoder(
    const InfraredMessage input_messages[],
    uint32_t input_messages_len,
    const uint32_t expected_timings[],
    uint32_t expected_timings_len) {
    uint32_t* timings = 0;
    uint32_t timings_len = 200;
    uint32_t j = 0;
    timings = malloc(sizeof(uint32_t) * timings_len);

    for(uint32_t message_counter = 0; message_counter < input_messages_len; ++message_counter) {
        const InfraredMessage* message = &input_messages[message_counter];
        if(!message->repeat) {
            infrared_reset_encoder(encoder_handler, message);
        }

        timings_len = 200;
        run_encoder_fill_array(encoder_handler, timings, &timings_len, NULL);
        furi_check(timings_len <= 200);

        for(size_t i = 0; i < timings_len; ++i, ++j) {
            mu_check(MATCH_TIMING(timings[i], expected_timings[j], 120));
            mu_assert(j < expected_timings_len, "encoded more timings than expected");
        }
    }
    free(timings);
    mu_assert(j == expected_timings_len, "encoded less timings than expected");
}

static void
    run_encoder_decoder(const InfraredMessage input_messages[], uint32_t input_messages_len) {
    uint32_t* timings = 0;
    uint32_t timings_len = 200;
    bool level = false;
    timings = malloc(sizeof(uint32_t) * timings_len);

    for(uint32_t message_counter = 0; message_counter < input_messages_len; ++message_counter) {
        const InfraredMessage* message_encoded = &input_messages[message_counter];
        if(!message_encoded->repeat) {
            infrared_reset_encoder(encoder_handler, message_encoded);
        }

        timings_len = 200;
        run_encoder_fill_array(encoder_handler, timings, &timings_len, &level);
        furi_check(timings_len <= 200);

        const InfraredMessage* message_decoded = 0;
        for(size_t i = 0; i < timings_len; ++i) {
            message_decoded = infrared_decode(decoder_handler, level, timings[i]);
            if((i == timings_len - 2) && level && message_decoded) {
                /* In case we end with space timing - message can be decoded at last mark */
                break;
            } else if(i < timings_len - 1) {
                mu_check(!message_decoded);
            } else {
                if(!message_decoded) {
                    message_decoded = infrared_check_decoder_ready(decoder_handler);
                }
                mu_check(message_decoded);
            }
            level = !level;
        }
        if(message_decoded) {
            compare_message_results(message_decoded, message_encoded);
        } else {
            mu_check(0);
        }
    }
    free(timings);
}

static void run_decoder(
    const uint32_t* input_delays,
    uint32_t input_delays_len,
    const InfraredMessage* message_expected,
    uint32_t message_expected_len) {
    InfraredMessage message_decoded_check_local;
    bool level = 0;
    uint32_t message_counter = 0;
    const InfraredMessage* message_decoded = 0;

    for(uint32_t i = 0; i < input_delays_len; ++i) {
        const InfraredMessage* message_decoded_check = 0;

        if(input_delays[i] > INFRARED_RAW_RX_TIMING_DELAY_US) {
            message_decoded_check = infrared_check_decoder_ready(decoder_handler);
            if(message_decoded_check) {
                /* infrared_decode() can reset message, but we have to call infrared_decode() to perform real
                 * simulation: infrared_check() by timeout, then infrared_decode() when meet edge */
                message_decoded_check_local = *message_decoded_check;
                message_decoded_check = &message_decoded_check_local;
            }
        }

        message_decoded = infrared_decode(decoder_handler, level, input_delays[i]);

        if(message_decoded_check || message_decoded) {
            mu_assert(
                !(message_decoded_check && message_decoded),
                "both messages decoded: check_ready() and infrared_decode()");

            if(message_decoded_check) {
                message_decoded = message_decoded_check;
            }

            mu_assert(message_counter < message_expected_len, "decoded more than expected");
            compare_message_results(message_decoded, &message_expected[message_counter]);

            ++message_counter;
        }
        level = !level;
    }

    message_decoded = infrared_check_decoder_ready(decoder_handler);
    if(message_decoded) {
        compare_message_results(message_decoded, &message_expected[message_counter]);
        ++message_counter;
    }

    mu_assert(message_counter == message_expected_len, "decoded less than expected");
}

MU_TEST(test_decoder_samsung32) {
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
}

MU_TEST(test_mix) {
    RUN_DECODER(test_decoder_rc5_input2, test_decoder_rc5_expected2);
    RUN_DECODER(test_decoder_sirc_input1, test_decoder_sirc_expected1);
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    // can use encoder data for decoding, but can't do opposite
    RUN_DECODER(test_encoder_rc6_expected1, test_encoder_rc6_input1);
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
    RUN_DECODER(test_decoder_rc6_input1, test_decoder_rc6_expected1);
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
    RUN_DECODER(test_decoder_rc5_input1, test_decoder_rc5_expected1);
    RUN_DECODER(test_decoder_sirc_input2, test_decoder_sirc_expected2);
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    RUN_DECODER(test_decoder_sirc_input4, test_decoder_sirc_expected4);
    RUN_DECODER(test_decoder_nec_input2, test_decoder_nec_expected2);
    RUN_DECODER(test_decoder_rc6_input1, test_decoder_rc6_expected1);
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    RUN_DECODER(test_decoder_sirc_input5, test_decoder_sirc_expected5);
    RUN_DECODER(test_decoder_nec_input3, test_decoder_nec_expected3);
    RUN_DECODER(test_decoder_rc5_input5, test_decoder_rc5_expected5);
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
    RUN_DECODER(test_decoder_sirc_input3, test_decoder_sirc_expected3);
}

MU_TEST(test_decoder_nec) {
    RUN_DECODER(test_decoder_nec_input1, test_decoder_nec_expected1);
    RUN_DECODER(test_decoder_nec_input2, test_decoder_nec_expected2);
    RUN_DECODER(test_decoder_nec_input3, test_decoder_nec_expected3);
}

MU_TEST(test_decoder_unexpected_end_in_sequence) {
    // test_decoder_nec_input1 and test_decoder_nec_input2 shuts unexpected
    RUN_DECODER(test_decoder_nec_input1, test_decoder_nec_expected1);
    RUN_DECODER(test_decoder_nec_input1, test_decoder_nec_expected1);
    RUN_DECODER(test_decoder_nec_input2, test_decoder_nec_expected2);
    RUN_DECODER(test_decoder_nec_input2, test_decoder_nec_expected2);
}

MU_TEST(test_decoder_necext1) {
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
}

MU_TEST(test_decoder_long_packets_with_nec_start) {
    RUN_DECODER(test_decoder_nec42ext_input1, test_decoder_nec42ext_expected1);
    RUN_DECODER(test_decoder_nec42ext_input2, test_decoder_nec42ext_expected2);
}

MU_TEST(test_encoder_sirc) {
    RUN_ENCODER(test_encoder_sirc_input1, test_encoder_sirc_expected1);
    RUN_ENCODER(test_encoder_sirc_input2, test_encoder_sirc_expected2);
}

MU_TEST(test_decoder_sirc) {
    RUN_DECODER(test_decoder_sirc_input3, test_decoder_sirc_expected3);
    RUN_DECODER(test_decoder_sirc_input1, test_decoder_sirc_expected1);
    RUN_DECODER(test_decoder_sirc_input2, test_decoder_sirc_expected2);
    RUN_DECODER(test_decoder_sirc_input4, test_decoder_sirc_expected4);
    RUN_DECODER(test_decoder_sirc_input5, test_decoder_sirc_expected5);
    RUN_ENCODER_DECODER(test_sirc);
}

MU_TEST(test_decoder_rc5) {
    RUN_DECODER(test_decoder_rc5x_input1, test_decoder_rc5x_expected1);
    RUN_DECODER(test_decoder_rc5_input1, test_decoder_rc5_expected1);
    RUN_DECODER(test_decoder_rc5_input2, test_decoder_rc5_expected2);
    RUN_DECODER(test_decoder_rc5_input3, test_decoder_rc5_expected3);
    RUN_DECODER(test_decoder_rc5_input4, test_decoder_rc5_expected4);
    RUN_DECODER(test_decoder_rc5_input5, test_decoder_rc5_expected5);
    RUN_DECODER(test_decoder_rc5_input6, test_decoder_rc5_expected6);
    RUN_DECODER(test_decoder_rc5_input_all_repeats, test_decoder_rc5_expected_all_repeats);
}

MU_TEST(test_encoder_rc5x) {
    RUN_ENCODER(test_decoder_rc5x_expected1, test_decoder_rc5x_input1);
}

MU_TEST(test_encoder_rc5) {
    RUN_ENCODER(test_decoder_rc5_expected_all_repeats, test_decoder_rc5_input_all_repeats);
}

MU_TEST(test_decoder_rc6) {
    RUN_DECODER(test_decoder_rc6_input1, test_decoder_rc6_expected1);
}

MU_TEST(test_encoder_rc6) {
    RUN_ENCODER(test_encoder_rc6_input1, test_encoder_rc6_expected1);
}

MU_TEST(test_encoder_decoder_all) {
    RUN_ENCODER_DECODER(test_nec);
    RUN_ENCODER_DECODER(test_necext);
    RUN_ENCODER_DECODER(test_nec42);
    RUN_ENCODER_DECODER(test_nec42ext);
    RUN_ENCODER_DECODER(test_samsung32);
    RUN_ENCODER_DECODER(test_rc6);
    RUN_ENCODER_DECODER(test_rc5);
    RUN_ENCODER_DECODER(test_sirc);
}

MU_TEST_SUITE(test_infrared_decoder_encoder) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_encoder_sirc);
    MU_RUN_TEST(test_decoder_sirc);
    MU_RUN_TEST(test_encoder_rc5x);
    MU_RUN_TEST(test_encoder_rc5);
    MU_RUN_TEST(test_decoder_rc5);
    MU_RUN_TEST(test_decoder_rc6);
    MU_RUN_TEST(test_encoder_rc6);
    MU_RUN_TEST(test_decoder_unexpected_end_in_sequence);
    MU_RUN_TEST(test_decoder_long_packets_with_nec_start);
    MU_RUN_TEST(test_decoder_nec);
    MU_RUN_TEST(test_decoder_samsung32);
    MU_RUN_TEST(test_decoder_necext1);
    MU_RUN_TEST(test_mix);
    MU_RUN_TEST(test_encoder_decoder_all);
}

int run_minunit_test_infrared_decoder_encoder() {
    MU_RUN_SUITE(test_infrared_decoder_encoder);

    return MU_EXIT_CODE;
}
