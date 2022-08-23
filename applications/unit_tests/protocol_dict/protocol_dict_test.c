#include <furi.h>
#include "../minunit.h"
#include <toolbox/protocols/protocol_dict.h>

typedef enum {
    TestDictProtocol0,
    TestDictProtocol1,

    TestDictProtocolMax,
} TestDictProtocols;

/*********************** PROTOCOL 0 START ***********************/

typedef struct {
    uint32_t data;
    size_t encoder_counter;
} Protocol0Data;

static const uint32_t protocol_0_decoder_result = 0xDEADBEEF;

static void* protocol_0_alloc() {
    void* data = malloc(sizeof(Protocol0Data));
    return data;
}

static void protocol_0_free(Protocol0Data* data) {
    free(data);
}

static uint8_t* protocol_0_get_data(Protocol0Data* data) {
    return (uint8_t*)&data->data;
}

static void protocol_0_decoder_start(Protocol0Data* data) {
    data->data = 0;
}

static bool protocol_0_decoder_feed(Protocol0Data* data, bool level, uint32_t duration) {
    if(level && duration == 666) {
        data->data = protocol_0_decoder_result;
        return true;
    } else {
        return false;
    }
}

static bool protocol_0_encoder_start(Protocol0Data* data) {
    data->encoder_counter = 0;
    return true;
}

static LevelDuration protocol_0_encoder_yield(Protocol0Data* data) {
    data->encoder_counter++;
    return level_duration_make(data->encoder_counter % 2, data->data);
}

/*********************** PROTOCOL 1 START ***********************/

typedef struct {
    uint64_t data;
    size_t encoder_counter;
} Protocol1Data;

static const uint64_t protocol_1_decoder_result = 0x1234567890ABCDEF;

static void* protocol_1_alloc() {
    void* data = malloc(sizeof(Protocol1Data));
    return data;
}

static void protocol_1_free(Protocol1Data* data) {
    free(data);
}

static uint8_t* protocol_1_get_data(Protocol1Data* data) {
    return (uint8_t*)&data->data;
}

static void protocol_1_decoder_start(Protocol1Data* data) {
    data->data = 0;
}

static bool protocol_1_decoder_feed(Protocol1Data* data, bool level, uint32_t duration) {
    if(level && duration == 543) {
        data->data = 0x1234567890ABCDEF;
        return true;
    } else {
        return false;
    }
}

static bool protocol_1_encoder_start(Protocol1Data* data) {
    data->encoder_counter = 0;
    return true;
}

static LevelDuration protocol_1_encoder_yield(Protocol1Data* data) {
    data->encoder_counter++;
    return level_duration_make(!(data->encoder_counter % 2), 100);
}

/*********************** PROTOCOLS DESCRIPTION ***********************/
static const ProtocolBase protocol_0 = {
    .name = "Protocol 0",
    .manufacturer = "Manufacturer 0",
    .data_size = 4,
    .alloc = (ProtocolAlloc)protocol_0_alloc,
    .free = (ProtocolFree)protocol_0_free,
    .get_data = (ProtocolGetData)protocol_0_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_0_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_0_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_0_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_0_encoder_yield,
        },
};

static const ProtocolBase protocol_1 = {
    .name = "Protocol 1",
    .manufacturer = "Manufacturer 1",
    .data_size = 8,
    .alloc = (ProtocolAlloc)protocol_1_alloc,
    .free = (ProtocolFree)protocol_1_free,
    .get_data = (ProtocolGetData)protocol_1_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_1_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_1_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_1_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_1_encoder_yield,
        },
};

static const ProtocolBase* test_protocols_base[] = {
    [TestDictProtocol0] = &protocol_0,
    [TestDictProtocol1] = &protocol_1,
};

MU_TEST(test_protocol_dict) {
    ProtocolDict* dict = protocol_dict_alloc(test_protocols_base, TestDictProtocolMax);
    size_t max_data_size = protocol_dict_get_max_data_size(dict);
    mu_assert_int_eq(8, max_data_size);
    uint8_t* data = malloc(max_data_size);

    protocol_dict_decoders_start(dict);
    ProtocolId protocol_id = PROTOCOL_NO;

    for(size_t i = 0; i < 100; i++) {
        protocol_id = protocol_dict_decoders_feed(dict, i % 2, 100);
        mu_assert_int_eq(PROTOCOL_NO, protocol_id);
    }

    // trigger protocol 1
    protocol_id = protocol_dict_decoders_feed(dict, true, 543);
    mu_assert_int_eq(TestDictProtocol1, protocol_id);

    mu_assert_string_eq("Protocol 1", protocol_dict_get_name(dict, protocol_id));
    mu_assert_string_eq("Manufacturer 1", protocol_dict_get_manufacturer(dict, protocol_id));

    size_t data_size = protocol_dict_get_data_size(dict, protocol_id);
    mu_assert_int_eq(8, data_size);

    protocol_dict_get_data(dict, protocol_id, data, data_size);
    mu_assert_mem_eq(&protocol_1_decoder_result, data, data_size);

    // trigger protocol 0
    protocol_id = protocol_dict_decoders_feed(dict, true, 666);
    mu_assert_int_eq(TestDictProtocol0, protocol_id);

    mu_assert_string_eq("Protocol 0", protocol_dict_get_name(dict, protocol_id));
    mu_assert_string_eq("Manufacturer 0", protocol_dict_get_manufacturer(dict, protocol_id));

    data_size = protocol_dict_get_data_size(dict, protocol_id);
    mu_assert_int_eq(4, data_size);

    protocol_dict_get_data(dict, protocol_id, data, data_size);
    mu_assert_mem_eq(&protocol_0_decoder_result, data, data_size);

    protocol_dict_decoders_start(dict);

    protocol_id = TestDictProtocol0;

    const uint8_t protocol_0_test_data[4] = {100, 0, 0, 0};
    protocol_dict_set_data(dict, protocol_id, protocol_0_test_data, 4);

    mu_check(protocol_dict_encoder_start(dict, protocol_id));

    LevelDuration level;
    level = protocol_dict_encoder_yield(dict, protocol_id);
    mu_assert_int_eq(true, level_duration_get_level(level));
    mu_assert_int_eq(100, level_duration_get_duration(level));
    level = protocol_dict_encoder_yield(dict, protocol_id);
    mu_assert_int_eq(false, level_duration_get_level(level));
    mu_assert_int_eq(100, level_duration_get_duration(level));
    level = protocol_dict_encoder_yield(dict, protocol_id);
    mu_assert_int_eq(true, level_duration_get_level(level));
    mu_assert_int_eq(100, level_duration_get_duration(level));

    mu_check(protocol_dict_encoder_start(dict, protocol_id));
    level = protocol_dict_encoder_yield(dict, protocol_id);
    mu_assert_int_eq(true, level_duration_get_level(level));
    mu_assert_int_eq(100, level_duration_get_duration(level));

    protocol_dict_free(dict);
    free(data);
}

MU_TEST_SUITE(test_protocol_dict_suite) {
    MU_RUN_TEST(test_protocol_dict);
}

int run_minunit_test_protocol_dict() {
    MU_RUN_SUITE(test_protocol_dict_suite);
    return MU_EXIT_CODE;
}