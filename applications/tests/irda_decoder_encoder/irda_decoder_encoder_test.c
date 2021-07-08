#include <furi.h>
#include "../minunit.h"
#include "irda.h"
#include "irda_common_i.h"
#include "test_data/irda_nec_test_data.srcdata"
#include "test_data/irda_necext_test_data.srcdata"
#include "test_data/irda_samsung_test_data.srcdata"
#include "test_data/irda_rc6_test_data.srcdata"

#define RUN_ENCODER(data, expected) \
    run_encoder((data), COUNT_OF(data), (expected), COUNT_OF(expected))

#define RUN_DECODER(data, expected) \
    run_decoder((data), COUNT_OF(data), (expected), COUNT_OF(expected))

static IrdaDecoderHandler* decoder_handler;
static IrdaEncoderHandler* encoder_handler;

static void test_setup(void) {
    decoder_handler = irda_alloc_decoder();
    encoder_handler = irda_alloc_encoder();
}

static void test_teardown(void) {
    irda_free_decoder(decoder_handler);
    irda_free_encoder(encoder_handler);
}

static void compare_message_results(
    const IrdaMessage* message_decoded,
    const IrdaMessage* message_expected) {
    mu_check(message_decoded->protocol == message_expected->protocol);
    mu_check(message_decoded->command == message_expected->command);
    mu_check(message_decoded->address == message_expected->address);
    mu_check(message_decoded->repeat == message_expected->repeat);
}

static void
    run_encoder_fill_array(IrdaEncoderHandler* handler, uint32_t* timings, uint32_t* timings_len) {
    uint32_t duration = 0;
    bool level = false; // start from space
    bool level_read;
    IrdaStatus status = IrdaStatusError;
    int i = 0;

    while(1) {
        status = irda_encode(handler, &duration, &level_read);
        if(level_read != level) {
            level = level_read;
            ++i;
        }
        timings[i] += duration;
        furi_assert((status == IrdaStatusOk) || (status == IrdaStatusDone));
        if(status == IrdaStatusDone) break;
        furi_assert(i < *timings_len);
    }

    *timings_len = i + 1;
}

// messages in input array for encoder should have one protocol
static void run_encoder(
    const IrdaMessage input_messages[],
    uint32_t input_messages_len,
    const uint32_t expected_timings[],
    uint32_t expected_timings_len) {
    uint32_t* timings = 0;
    uint32_t timings_len = 0;
    uint32_t j = 0;

    for(uint32_t message_counter = 0; message_counter < input_messages_len; ++message_counter) {
        const IrdaMessage* message = &input_messages[message_counter];
        if(!message->repeat) {
            irda_reset_encoder(encoder_handler, message);
        }

        timings_len = 200;
        timings = furi_alloc(sizeof(uint32_t) * timings_len);
        run_encoder_fill_array(encoder_handler, timings, &timings_len);
        furi_assert(timings_len <= 200);

        for(int i = 0; i < timings_len; ++i, ++j) {
            mu_check(MATCH_BIT_TIMING(timings[i], expected_timings[j], 120));
            mu_assert(j < expected_timings_len, "encoded more timings than expected");
        }

        free(timings);
    }
    mu_assert(j == expected_timings_len, "encoded less timings than expected");
}

static void run_encoder_decoder(const IrdaMessage input_messages[], uint32_t input_messages_len) {
    uint32_t* timings = 0;
    uint32_t timings_len = 0;
    bool level = false;

    for(uint32_t message_counter = 0; message_counter < input_messages_len; ++message_counter) {
        const IrdaMessage* message_encoded = &input_messages[message_counter];
        if(!message_encoded->repeat) {
            irda_reset_encoder(encoder_handler, message_encoded);
            level = false;
        }

        timings_len = 200;
        timings = furi_alloc(sizeof(uint32_t) * timings_len);
        run_encoder_fill_array(encoder_handler, timings, &timings_len);
        furi_assert(timings_len <= 200);

        const IrdaMessage* message_decoded = 0;
        for(int i = 0; i < timings_len; ++i) {
            message_decoded = irda_decode(decoder_handler, level, timings[i]);
            if(i < timings_len - 1)
                mu_check(!message_decoded);
            else
                mu_check(message_decoded);
            level = !level;
        }
        if(message_decoded) {
            compare_message_results(message_decoded, message_encoded);
        } else {
            mu_check(0);
        }

        free(timings);
    }
}

static void run_decoder(
    const uint32_t* input_delays,
    uint32_t input_delays_len,
    const IrdaMessage* message_expected,
    uint32_t message_expected_len) {
    const IrdaMessage* message_decoded = 0;
    bool level = 0;
    uint32_t message_counter = 0;

    for(uint32_t i = 0; i < input_delays_len; ++i) {
        message_decoded = irda_decode(decoder_handler, level, input_delays[i]);
        if(message_decoded) {
            mu_assert(message_counter < message_expected_len, "decoded more than expected");
            if(message_counter >= message_expected_len) break;
            compare_message_results(message_decoded, &message_expected[message_counter]);
            ++message_counter;
        }
        level = !level;
    }

    mu_assert(message_counter == message_expected_len, "decoded less than expected");
}

MU_TEST(test_decoder_samsung32) {
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
}

MU_TEST(test_mix) {
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    // can use encoder data for decoding, but can't do opposite
    RUN_DECODER(test_encoder_rc6_expected1, test_encoder_rc6_input1);
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
    RUN_DECODER(test_decoder_rc6_input1, test_decoder_rc6_expected1);
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    RUN_DECODER(test_decoder_nec_input2, test_decoder_nec_expected2);
    RUN_DECODER(test_decoder_rc6_input1, test_decoder_rc6_expected1);
    RUN_DECODER(test_decoder_necext_input1, test_decoder_necext_expected1);
    RUN_DECODER(test_decoder_samsung32_input1, test_decoder_samsung32_expected1);
}

MU_TEST(test_decoder_nec1) {
    RUN_DECODER(test_decoder_nec_input1, test_decoder_nec_expected1);
}

MU_TEST(test_decoder_nec2) {
    RUN_DECODER(test_decoder_nec_input2, test_decoder_nec_expected2);
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

MU_TEST(test_decoder_rc6) {
    RUN_DECODER(test_decoder_rc6_input1, test_decoder_rc6_expected1);
}

MU_TEST(test_encoder_rc6) {
    RUN_ENCODER(test_encoder_rc6_input1, test_encoder_rc6_expected1);
}

MU_TEST(test_encoder_decoder_all) {
    run_encoder_decoder(test_nec_all, COUNT_OF(test_nec_all));
    run_encoder_decoder(test_necext_all, COUNT_OF(test_necext_all));
    run_encoder_decoder(test_samsung32_all, COUNT_OF(test_samsung32_all));
    run_encoder_decoder(test_rc6_all, COUNT_OF(test_rc6_all));
}

MU_TEST_SUITE(test_irda_decoder_encoder) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_encoder_decoder_all);
    MU_RUN_TEST(test_decoder_unexpected_end_in_sequence);
    MU_RUN_TEST(test_decoder_nec1);
    MU_RUN_TEST(test_decoder_nec2);
    MU_RUN_TEST(test_decoder_samsung32);
    MU_RUN_TEST(test_decoder_necext1);
    MU_RUN_TEST(test_mix);
    MU_RUN_TEST(test_decoder_rc6);
    MU_RUN_TEST(test_encoder_rc6);
}

int run_minunit_test_irda_decoder_encoder() {
    MU_RUN_SUITE(test_irda_decoder_encoder);
    MU_REPORT();

    return MU_EXIT_CODE;
}
