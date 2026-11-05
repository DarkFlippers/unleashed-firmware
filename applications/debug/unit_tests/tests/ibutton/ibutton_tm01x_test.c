#include <furi.h>
#include <ibutton/protocols/blanks/tm01x.h>

#include "../test.h" // IWYU pragma: keep

#define TEST_ROM_SIZE                 8U
#define TEST_PROTOCOL_MISC_BIT_COUNT  36U
#define TEST_PROTOCOL_MISC_DATA_BYTES 5U

/** Build the image the same way the protocols do, then compare byte-wise. */
static bool tm01x_test_image_matches(
    uint8_t prefix,
    uint32_t wire_bits,
    const uint8_t expected[TEST_ROM_SIZE]) {
    uint8_t rom[TEST_ROM_SIZE];
    tm01x_protocol_misc_make_image(rom, prefix, wire_bits);
    return memcmp(rom, expected, TEST_ROM_SIZE) == 0;
}

MU_TEST(test_tm01x_bit_reverse32) {
    mu_check(tm01x_bit_reverse32(0x00000001) == 0x80000000);
    mu_check(tm01x_bit_reverse32(0x80000000) == 0x00000001);
    mu_check(tm01x_bit_reverse32(0x00000000) == 0x00000000);
    mu_check(tm01x_bit_reverse32(0xFFFFFFFF) == 0xFFFFFFFF);
    mu_check(tm01x_bit_reverse32(0x12345678) == 0x1E6A2C48);

    // Reversing twice is the identity
    mu_check(tm01x_bit_reverse32(tm01x_bit_reverse32(0xDEADBEEF)) == 0xDEADBEEF);
}

MU_TEST(test_tm01x_image_layout) {
    // Cyfral: prefix 0b0001, then 32 data bits, most significant first
    const uint8_t cyfral[TEST_ROM_SIZE] = {0x17, 0xB7, 0xD7, 0xEB, 0x70, 0, 0, 0};
    mu_check(tm01x_test_image_matches(0x01, 0x7B7D7EB7, cyfral));

    // Metakom: prefix 0b0010
    const uint8_t metakom[TEST_ROM_SIZE] = {0x2C, 0xA4, 0x2C, 0xC5, 0x50, 0, 0, 0};
    mu_check(tm01x_test_image_matches(0x02, 0xCA42CC55, metakom));

    // All-zero and all-one payloads
    const uint8_t zeros[TEST_ROM_SIZE] = {0x20, 0x00, 0x00, 0x00, 0x00, 0, 0, 0};
    mu_check(tm01x_test_image_matches(0x02, 0x00000000, zeros));

    const uint8_t ones[TEST_ROM_SIZE] = {0x2F, 0xFF, 0xFF, 0xFF, 0xF0, 0, 0, 0};
    mu_check(tm01x_test_image_matches(0x02, 0xFFFFFFFF, ones));

    // Bits above the 36th are always left clear, whatever was passed in
    uint8_t rom[TEST_ROM_SIZE];
    memset(rom, 0xFF, sizeof(rom));
    tm01x_protocol_misc_make_image(rom, 0x0F, 0xFFFFFFFF);
    mu_check(rom[5] == 0 && rom[6] == 0 && rom[7] == 0);
    mu_check((rom[4] & 0x0F) == 0);
}

MU_TEST(test_tm01x_ibutton_write_bit_order) {
    uint8_t rom[TEST_ROM_SIZE];
    tm01x_protocol_misc_make_image(rom, 0x01, 0x7B7D7EB7);

    // The Cyfral prefix 0b0001 is sent inverted (0b1110), MSB first
    mu_check(tm01x_protocol_misc_get_bit(rom, 0) == true);
    mu_check(tm01x_protocol_misc_get_bit(rom, 1) == true);
    mu_check(tm01x_protocol_misc_get_bit(rom, 2) == true);
    mu_check(tm01x_protocol_misc_get_bit(rom, 3) == false);

    // The whole 36-bit stream is the inverted image, MSB first
    const uint8_t expected_inverted[TEST_PROTOCOL_MISC_DATA_BYTES] = {
        0xE8, 0x48, 0x28, 0x14, 0x80};
    uint8_t reconstructed[TEST_PROTOCOL_MISC_DATA_BYTES] = {0};

    for(size_t bit = 0; bit < TEST_PROTOCOL_MISC_BIT_COUNT; ++bit) {
        if(tm01x_protocol_misc_get_bit(rom, bit)) {
            reconstructed[bit / 8] |= (uint8_t)(1U << (7 - (bit % 8)));
        }
    }

    mu_assert_mem_eq(expected_inverted, reconstructed, sizeof(reconstructed));

    // The last programmed bit comes from the lowest nibble
    mu_check(tm01x_protocol_misc_get_bit(rom, 35) == false);
}

MU_TEST(test_tm01x_protocol_misc_normalize_read) {
    // Raw 1-Wire read of the image above: the blank stores it inverted and in
    // reverse bit order, bits above the 36th read as ones
    const uint8_t read_data[TEST_ROM_SIZE] = {0x17, 0x12, 0x14, 0x28, 0xF1, 0xFF, 0xFF, 0xFF};
    const uint8_t expected[TEST_ROM_SIZE] = {0x17, 0xB7, 0xD7, 0xEB, 0x70, 0x00, 0x00, 0x00};

    uint8_t rom[TEST_ROM_SIZE];
    memcpy(rom, read_data, sizeof(rom));
    tm01x_protocol_misc_normalize_read(rom);
    mu_assert_mem_eq(expected, rom, sizeof(rom));
}

MU_TEST(test_tm01x_ibutton_read_roundtrip) {
    const uint8_t images[][TEST_ROM_SIZE] = {
        {0x17, 0xB7, 0xD7, 0xEB, 0x70, 0x00, 0x00, 0x00},
        {0x17, 0x77, 0x77, 0x77, 0x70, 0x00, 0x00, 0x00},
        {0x1E, 0xEE, 0xEE, 0xEE, 0xE0, 0x00, 0x00, 0x00},
        {0x2C, 0xA4, 0x2C, 0xC5, 0x50, 0x00, 0x00, 0x00},
        {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x2F, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00},
    };

    for(size_t r = 0; r < COUNT_OF(images); ++r) {
        // Simulate a blank read: unwritten bits read as ones
        uint8_t read_data[TEST_ROM_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        for(size_t bit = 0; bit < TEST_PROTOCOL_MISC_BIT_COUNT; ++bit) {
            const uint8_t mask = (uint8_t)(1U << (bit % 8));
            if(tm01x_protocol_misc_get_bit(images[r], bit)) {
                read_data[bit / 8] |= mask;
            } else {
                read_data[bit / 8] &= (uint8_t)~mask;
            }
        }

        tm01x_protocol_misc_normalize_read(read_data);
        mu_assert_mem_eq(images[r], read_data, TEST_ROM_SIZE);
    }
}

MU_TEST_SUITE(test_tm01x_suite) {
    MU_RUN_TEST(test_tm01x_bit_reverse32);
    MU_RUN_TEST(test_tm01x_image_layout);
    MU_RUN_TEST(test_tm01x_ibutton_write_bit_order);
    MU_RUN_TEST(test_tm01x_protocol_misc_normalize_read);
    MU_RUN_TEST(test_tm01x_ibutton_read_roundtrip);
}

int run_minunit_test_tm01x(void) {
    MU_RUN_SUITE(test_tm01x_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_tm01x)
