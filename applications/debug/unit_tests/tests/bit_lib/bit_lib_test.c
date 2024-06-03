#include <furi.h>
#include "../test.h" // IWYU pragma: keep
#include <bit_lib/bit_lib.h>

MU_TEST(test_bit_lib_increment_index) {
    uint32_t index = 0;

    // test increment
    for(uint32_t i = 0; i < 31; ++i) {
        bit_lib_increment_index(index, 32);
        mu_assert_int_eq(i + 1, index);
    }

    // test wrap around
    for(uint32_t i = 0; i < 512; ++i) {
        bit_lib_increment_index(index, 32);
        mu_assert_int_less_than(32, index);
    }
}

MU_TEST(test_bit_lib_is_set) {
    uint32_t value = 0x0000FFFF;

    for(uint32_t i = 0; i < 16; ++i) {
        mu_check(bit_lib_bit_is_set(value, i));
        mu_check(!bit_lib_bit_is_not_set(value, i));
    }

    for(uint32_t i = 16; i < 32; ++i) {
        mu_check(!bit_lib_bit_is_set(value, i));
        mu_check(bit_lib_bit_is_not_set(value, i));
    }
}

MU_TEST(test_bit_lib_push) {
#define TEST_BIT_LIB_PUSH_DATA_SIZE 4
    uint8_t data[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0};
    uint8_t expected_data_1[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0x00, 0x00, 0x0F, 0xFF};
    uint8_t expected_data_2[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0x00, 0xFF, 0xF0, 0x00};
    uint8_t expected_data_3[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0xFF, 0x00, 0x00, 0xFF};
    uint8_t expected_data_4[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t expected_data_5[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0x00, 0x00, 0x00, 0x00};
    uint8_t expected_data_6[TEST_BIT_LIB_PUSH_DATA_SIZE] = {0xCC, 0xCC, 0xCC, 0xCC};

    for(uint32_t i = 0; i < 12; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, true);
    }
    mu_assert_mem_eq(expected_data_1, data, TEST_BIT_LIB_PUSH_DATA_SIZE);

    for(uint32_t i = 0; i < 12; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, false);
    }
    mu_assert_mem_eq(expected_data_2, data, TEST_BIT_LIB_PUSH_DATA_SIZE);

    for(uint32_t i = 0; i < 4; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, false);
    }
    for(uint32_t i = 0; i < 8; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, true);
    }
    mu_assert_mem_eq(expected_data_3, data, TEST_BIT_LIB_PUSH_DATA_SIZE);

    for(uint32_t i = 0; i < TEST_BIT_LIB_PUSH_DATA_SIZE * 8; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, true);
    }
    mu_assert_mem_eq(expected_data_4, data, TEST_BIT_LIB_PUSH_DATA_SIZE);

    for(uint32_t i = 0; i < TEST_BIT_LIB_PUSH_DATA_SIZE * 8; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, false);
    }
    mu_assert_mem_eq(expected_data_5, data, TEST_BIT_LIB_PUSH_DATA_SIZE);

    for(uint32_t i = 0; i < TEST_BIT_LIB_PUSH_DATA_SIZE * 2; ++i) {
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, true);
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, true);
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, false);
        bit_lib_push_bit(data, TEST_BIT_LIB_PUSH_DATA_SIZE, false);
    }
    mu_assert_mem_eq(expected_data_6, data, TEST_BIT_LIB_PUSH_DATA_SIZE);
}

MU_TEST(test_bit_lib_set_bit) {
    uint8_t value[2] = {0x00, 0xFF};
    bit_lib_set_bit(value, 15, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0xFE}), 2);
    bit_lib_set_bit(value, 14, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0xFC}), 2);
    bit_lib_set_bit(value, 13, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0xF8}), 2);
    bit_lib_set_bit(value, 12, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0xF0}), 2);
    bit_lib_set_bit(value, 11, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0xE0}), 2);
    bit_lib_set_bit(value, 10, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0xC0}), 2);
    bit_lib_set_bit(value, 9, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0x80}), 2);
    bit_lib_set_bit(value, 8, false);
    mu_assert_mem_eq(value, ((uint8_t[]){0x00, 0x00}), 2);

    bit_lib_set_bit(value, 7, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x01, 0x00}), 2);
    bit_lib_set_bit(value, 6, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x03, 0x00}), 2);
    bit_lib_set_bit(value, 5, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x07, 0x00}), 2);
    bit_lib_set_bit(value, 4, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x0F, 0x00}), 2);
    bit_lib_set_bit(value, 3, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x1F, 0x00}), 2);
    bit_lib_set_bit(value, 2, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x3F, 0x00}), 2);
    bit_lib_set_bit(value, 1, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0x7F, 0x00}), 2);
    bit_lib_set_bit(value, 0, true);
    mu_assert_mem_eq(value, ((uint8_t[]){0xFF, 0x00}), 2);
}

MU_TEST(test_bit_lib_set_bits) {
    uint8_t value[2] = {0b00000000, 0b11111111};
    // set 4 bits to 0b0100 from 12 index
    bit_lib_set_bits(value, 12, 0b0100, 4);
    //                                                    [0100]
    mu_assert_mem_eq(value, ((uint8_t[]){0b00000000, 0b11110100}), 2);

    // set 2 bits to 0b11 from 11 index
    bit_lib_set_bits(value, 11, 0b11, 2);
    //                                                    [11]
    mu_assert_mem_eq(value, ((uint8_t[]){0b00000000, 0b11111100}), 2);

    // set 3 bits to 0b111 from 0 index
    bit_lib_set_bits(value, 0, 0b111, 3);
    //                                    [111]
    mu_assert_mem_eq(value, ((uint8_t[]){0b11100000, 0b11111100}), 2);

    // set 8 bits to 0b11111000 from 3 index
    bit_lib_set_bits(value, 3, 0b11111000, 8);
    //                                       [11111    000]
    mu_assert_mem_eq(value, ((uint8_t[]){0b11111111, 0b00011100}), 2);
}

MU_TEST(test_bit_lib_get_bit) {
    uint8_t value[2] = {0b00000000, 0b11111111};
    for(uint32_t i = 0; i < 8; ++i) {
        mu_check(bit_lib_get_bit(value, i) == false);
    }
    for(uint32_t i = 8; i < 16; ++i) {
        mu_check(bit_lib_get_bit(value, i) == true);
    }
}

MU_TEST(test_bit_lib_get_bits) {
    uint8_t value[2] = {0b00000000, 0b11111111};
    mu_assert_int_eq(0b00000000, bit_lib_get_bits(value, 0, 8));
    mu_assert_int_eq(0b00000001, bit_lib_get_bits(value, 1, 8));
    mu_assert_int_eq(0b00000011, bit_lib_get_bits(value, 2, 8));
    mu_assert_int_eq(0b00000111, bit_lib_get_bits(value, 3, 8));
    mu_assert_int_eq(0b00001111, bit_lib_get_bits(value, 4, 8));
    mu_assert_int_eq(0b00011111, bit_lib_get_bits(value, 5, 8));
    mu_assert_int_eq(0b00111111, bit_lib_get_bits(value, 6, 8));
    mu_assert_int_eq(0b01111111, bit_lib_get_bits(value, 7, 8));
    mu_assert_int_eq(0b11111111, bit_lib_get_bits(value, 8, 8));
}

MU_TEST(test_bit_lib_get_bits_16) {
    uint8_t value[2] = {0b00001001, 0b10110001};
    mu_assert_int_eq(0b0, bit_lib_get_bits_16(value, 0, 1));
    mu_assert_int_eq(0b00, bit_lib_get_bits_16(value, 0, 2));
    mu_assert_int_eq(0b000, bit_lib_get_bits_16(value, 0, 3));
    mu_assert_int_eq(0b0000, bit_lib_get_bits_16(value, 0, 4));
    mu_assert_int_eq(0b00001, bit_lib_get_bits_16(value, 0, 5));
    mu_assert_int_eq(0b000010, bit_lib_get_bits_16(value, 0, 6));
    mu_assert_int_eq(0b0000100, bit_lib_get_bits_16(value, 0, 7));
    mu_assert_int_eq(0b00001001, bit_lib_get_bits_16(value, 0, 8));
    mu_assert_int_eq(0b000010011, bit_lib_get_bits_16(value, 0, 9));
    mu_assert_int_eq(0b0000100110, bit_lib_get_bits_16(value, 0, 10));
    mu_assert_int_eq(0b00001001101, bit_lib_get_bits_16(value, 0, 11));
    mu_assert_int_eq(0b000010011011, bit_lib_get_bits_16(value, 0, 12));
    mu_assert_int_eq(0b0000100110110, bit_lib_get_bits_16(value, 0, 13));
    mu_assert_int_eq(0b00001001101100, bit_lib_get_bits_16(value, 0, 14));
    mu_assert_int_eq(0b000010011011000, bit_lib_get_bits_16(value, 0, 15));
    mu_assert_int_eq(0b0000100110110001, bit_lib_get_bits_16(value, 0, 16));
}

MU_TEST(test_bit_lib_get_bits_32) {
    uint8_t value[4] = {0b00001001, 0b10110001, 0b10001100, 0b01100010};
    mu_assert_int_eq(0b0, bit_lib_get_bits_32(value, 0, 1));
    mu_assert_int_eq(0b00, bit_lib_get_bits_32(value, 0, 2));
    mu_assert_int_eq(0b000, bit_lib_get_bits_32(value, 0, 3));
    mu_assert_int_eq(0b0000, bit_lib_get_bits_32(value, 0, 4));
    mu_assert_int_eq(0b00001, bit_lib_get_bits_32(value, 0, 5));
    mu_assert_int_eq(0b000010, bit_lib_get_bits_32(value, 0, 6));
    mu_assert_int_eq(0b0000100, bit_lib_get_bits_32(value, 0, 7));
    mu_assert_int_eq(0b00001001, bit_lib_get_bits_32(value, 0, 8));
    mu_assert_int_eq(0b000010011, bit_lib_get_bits_32(value, 0, 9));
    mu_assert_int_eq(0b0000100110, bit_lib_get_bits_32(value, 0, 10));
    mu_assert_int_eq(0b00001001101, bit_lib_get_bits_32(value, 0, 11));
    mu_assert_int_eq(0b000010011011, bit_lib_get_bits_32(value, 0, 12));
    mu_assert_int_eq(0b0000100110110, bit_lib_get_bits_32(value, 0, 13));
    mu_assert_int_eq(0b00001001101100, bit_lib_get_bits_32(value, 0, 14));
    mu_assert_int_eq(0b000010011011000, bit_lib_get_bits_32(value, 0, 15));
    mu_assert_int_eq(0b0000100110110001, bit_lib_get_bits_32(value, 0, 16));
    mu_assert_int_eq(0b00001001101100011, bit_lib_get_bits_32(value, 0, 17));
    mu_assert_int_eq(0b000010011011000110, bit_lib_get_bits_32(value, 0, 18));
    mu_assert_int_eq(0b0000100110110001100, bit_lib_get_bits_32(value, 0, 19));
    mu_assert_int_eq(0b00001001101100011000, bit_lib_get_bits_32(value, 0, 20));
    mu_assert_int_eq(0b000010011011000110001, bit_lib_get_bits_32(value, 0, 21));
    mu_assert_int_eq(0b0000100110110001100011, bit_lib_get_bits_32(value, 0, 22));
    mu_assert_int_eq(0b00001001101100011000110, bit_lib_get_bits_32(value, 0, 23));
    mu_assert_int_eq(0b000010011011000110001100, bit_lib_get_bits_32(value, 0, 24));
    mu_assert_int_eq(0b0000100110110001100011000, bit_lib_get_bits_32(value, 0, 25));
    mu_assert_int_eq(0b00001001101100011000110001, bit_lib_get_bits_32(value, 0, 26));
    mu_assert_int_eq(0b000010011011000110001100011, bit_lib_get_bits_32(value, 0, 27));
    mu_assert_int_eq(0b0000100110110001100011000110, bit_lib_get_bits_32(value, 0, 28));
    mu_assert_int_eq(0b00001001101100011000110001100, bit_lib_get_bits_32(value, 0, 29));
    mu_assert_int_eq(0b000010011011000110001100011000, bit_lib_get_bits_32(value, 0, 30));
    mu_assert_int_eq(0b0000100110110001100011000110001, bit_lib_get_bits_32(value, 0, 31));
    mu_assert_int_eq(0b00001001101100011000110001100010, bit_lib_get_bits_32(value, 0, 32));
}

MU_TEST(test_bit_lib_get_bits_64) {
    uint8_t value[8] = {
        0b00001001,
        0b10110001,
        0b10001100,
        0b01100010,
        0b00001001,
        0b10110001,
        0b10001100,
        0b01100010};
    mu_assert_int_eq(0b0, bit_lib_get_bits_64(value, 0, 1));
    mu_assert_int_eq(0b00, bit_lib_get_bits_64(value, 0, 2));
    mu_assert_int_eq(0b000, bit_lib_get_bits_64(value, 0, 3));
    mu_assert_int_eq(0b0000, bit_lib_get_bits_64(value, 0, 4));
    mu_assert_int_eq(0b00001, bit_lib_get_bits_64(value, 0, 5));
    mu_assert_int_eq(0b000010, bit_lib_get_bits_64(value, 0, 6));
    mu_assert_int_eq(0b0000100, bit_lib_get_bits_64(value, 0, 7));
    mu_assert_int_eq(0b00001001, bit_lib_get_bits_64(value, 0, 8));
    mu_assert_int_eq(0b000010011, bit_lib_get_bits_64(value, 0, 9));
    mu_assert_int_eq(0b0000100110, bit_lib_get_bits_64(value, 0, 10));
    mu_assert_int_eq(0b00001001101, bit_lib_get_bits_64(value, 0, 11));
    mu_assert_int_eq(0b000010011011, bit_lib_get_bits_64(value, 0, 12));
    mu_assert_int_eq(0b0000100110110, bit_lib_get_bits_64(value, 0, 13));
    mu_assert_int_eq(0b00001001101100, bit_lib_get_bits_64(value, 0, 14));
    mu_assert_int_eq(0b000010011011000, bit_lib_get_bits_64(value, 0, 15));
    mu_assert_int_eq(0b0000100110110001, bit_lib_get_bits_64(value, 0, 16));
    mu_assert_int_eq(0b00001001101100011, bit_lib_get_bits_64(value, 0, 17));
    mu_assert_int_eq(0b000010011011000110, bit_lib_get_bits_64(value, 0, 18));
    mu_assert_int_eq(0b0000100110110001100, bit_lib_get_bits_64(value, 0, 19));
    mu_assert_int_eq(0b00001001101100011000, bit_lib_get_bits_64(value, 0, 20));
    mu_assert_int_eq(0b000010011011000110001, bit_lib_get_bits_64(value, 0, 21));
    mu_assert_int_eq(0b0000100110110001100011, bit_lib_get_bits_64(value, 0, 22));
    mu_assert_int_eq(0b00001001101100011000110, bit_lib_get_bits_64(value, 0, 23));
    mu_assert_int_eq(0b000010011011000110001100, bit_lib_get_bits_64(value, 0, 24));
    mu_assert_int_eq(0b0000100110110001100011000, bit_lib_get_bits_64(value, 0, 25));
    mu_assert_int_eq(0b00001001101100011000110001, bit_lib_get_bits_64(value, 0, 26));
    mu_assert_int_eq(0b000010011011000110001100011, bit_lib_get_bits_64(value, 0, 27));
    mu_assert_int_eq(0b0000100110110001100011000110, bit_lib_get_bits_64(value, 0, 28));
    mu_assert_int_eq(0b00001001101100011000110001100, bit_lib_get_bits_64(value, 0, 29));
    mu_assert_int_eq(0b000010011011000110001100011000, bit_lib_get_bits_64(value, 0, 30));
    mu_assert_int_eq(0b0000100110110001100011000110001, bit_lib_get_bits_64(value, 0, 31));
    mu_assert_int_eq(0b00001001101100011000110001100010, bit_lib_get_bits_64(value, 0, 32));

    uint64_t res = bit_lib_get_bits_64(value, 0, 33);
    uint64_t expected = 0b000010011011000110001100011000100;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 34);
    expected = 0b0000100110110001100011000110001000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 35);
    expected = 0b00001001101100011000110001100010000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 36);
    expected = 0b000010011011000110001100011000100000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 37);
    expected = 0b0000100110110001100011000110001000001;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 38);
    expected = 0b00001001101100011000110001100010000010;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 39);
    expected = 0b000010011011000110001100011000100000100;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 40);
    expected = 0b0000100110110001100011000110001000001001;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 41);
    expected = 0b00001001101100011000110001100010000010011;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 42);
    expected = 0b000010011011000110001100011000100000100110;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 43);
    expected = 0b0000100110110001100011000110001000001001101;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 44);
    expected = 0b00001001101100011000110001100010000010011011;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 45);
    expected = 0b000010011011000110001100011000100000100110110;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 46);
    expected = 0b0000100110110001100011000110001000001001101100;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 47);
    expected = 0b00001001101100011000110001100010000010011011000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 48);
    expected = 0b000010011011000110001100011000100000100110110001;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 49);
    expected = 0b0000100110110001100011000110001000001001101100011;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 50);
    expected = 0b00001001101100011000110001100010000010011011000110;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 51);
    expected = 0b000010011011000110001100011000100000100110110001100;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 52);
    expected = 0b0000100110110001100011000110001000001001101100011000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 53);
    expected = 0b00001001101100011000110001100010000010011011000110001;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 54);
    expected = 0b000010011011000110001100011000100000100110110001100011;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 55);
    expected = 0b0000100110110001100011000110001000001001101100011000110;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 56);
    expected = 0b00001001101100011000110001100010000010011011000110001100;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 57);
    expected = 0b000010011011000110001100011000100000100110110001100011000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 58);
    expected = 0b0000100110110001100011000110001000001001101100011000110001;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 59);
    expected = 0b00001001101100011000110001100010000010011011000110001100011;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 60);
    expected = 0b000010011011000110001100011000100000100110110001100011000110;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 61);
    expected = 0b0000100110110001100011000110001000001001101100011000110001100;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 62);
    expected = 0b00001001101100011000110001100010000010011011000110001100011000;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 63);
    expected = 0b000010011011000110001100011000100000100110110001100011000110001;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));

    res = bit_lib_get_bits_64(value, 0, 64);
    expected = 0b0000100110110001100011000110001000001001101100011000110001100010;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));
}

MU_TEST(test_bit_lib_test_parity_u32) {
    // test even parity
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000000, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000001, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000010, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000011, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000100, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000101, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000110, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000111, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001000, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001001, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001010, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001011, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001100, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001101, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001110, BitLibParityEven), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001111, BitLibParityEven), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00010000, BitLibParityEven), 1);

    // test odd parity
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000000, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000001, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000010, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000011, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000100, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000101, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000110, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00000111, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001000, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001001, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001010, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001011, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001100, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001101, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001110, BitLibParityOdd), 0);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00001111, BitLibParityOdd), 1);
    mu_assert_int_eq(bit_lib_test_parity_32(0b00010000, BitLibParityOdd), 0);
}

MU_TEST(test_bit_lib_test_parity) {
    // next data contains valid parity for 1-3 nibble and invalid for 4 nibble
    uint8_t data_always_0_parity[2] = {0b11101110, 0b11101111};
    uint8_t data_always_1_parity[2] = {0b00010001, 0b00010000};
    uint8_t data_always_odd_parity[2] = {0b00000011, 0b11110111};
    uint8_t data_always_even_parity[2] = {0b00010111, 0b10110011};

    // test alawys 0 parity
    mu_check(bit_lib_test_parity(data_always_0_parity, 0, 12, BitLibParityAlways0, 4));
    mu_check(bit_lib_test_parity(data_always_0_parity, 4, 8, BitLibParityAlways0, 4));
    mu_check(bit_lib_test_parity(data_always_0_parity, 8, 4, BitLibParityAlways0, 4));
    mu_check(bit_lib_test_parity(data_always_1_parity, 12, 4, BitLibParityAlways0, 4));

    mu_check(!bit_lib_test_parity(data_always_0_parity, 0, 16, BitLibParityAlways0, 4));
    mu_check(!bit_lib_test_parity(data_always_0_parity, 4, 12, BitLibParityAlways0, 4));
    mu_check(!bit_lib_test_parity(data_always_0_parity, 8, 8, BitLibParityAlways0, 4));
    mu_check(!bit_lib_test_parity(data_always_0_parity, 12, 4, BitLibParityAlways0, 4));

    // test alawys 1 parity
    mu_check(bit_lib_test_parity(data_always_1_parity, 0, 12, BitLibParityAlways1, 4));
    mu_check(bit_lib_test_parity(data_always_1_parity, 4, 8, BitLibParityAlways1, 4));
    mu_check(bit_lib_test_parity(data_always_1_parity, 8, 4, BitLibParityAlways1, 4));
    mu_check(bit_lib_test_parity(data_always_0_parity, 12, 4, BitLibParityAlways1, 4));

    mu_check(!bit_lib_test_parity(data_always_1_parity, 0, 16, BitLibParityAlways1, 4));
    mu_check(!bit_lib_test_parity(data_always_1_parity, 4, 12, BitLibParityAlways1, 4));
    mu_check(!bit_lib_test_parity(data_always_1_parity, 8, 8, BitLibParityAlways1, 4));
    mu_check(!bit_lib_test_parity(data_always_1_parity, 12, 4, BitLibParityAlways1, 4));

    // test odd parity
    mu_check(bit_lib_test_parity(data_always_odd_parity, 0, 12, BitLibParityOdd, 4));
    mu_check(bit_lib_test_parity(data_always_odd_parity, 4, 8, BitLibParityOdd, 4));
    mu_check(bit_lib_test_parity(data_always_odd_parity, 8, 4, BitLibParityOdd, 4));
    mu_check(bit_lib_test_parity(data_always_even_parity, 12, 4, BitLibParityOdd, 4));

    mu_check(!bit_lib_test_parity(data_always_odd_parity, 0, 16, BitLibParityOdd, 4));
    mu_check(!bit_lib_test_parity(data_always_odd_parity, 4, 12, BitLibParityOdd, 4));
    mu_check(!bit_lib_test_parity(data_always_odd_parity, 8, 8, BitLibParityOdd, 4));
    mu_check(!bit_lib_test_parity(data_always_odd_parity, 12, 4, BitLibParityOdd, 4));

    // test even parity
    mu_check(bit_lib_test_parity(data_always_even_parity, 0, 12, BitLibParityEven, 4));
    mu_check(bit_lib_test_parity(data_always_even_parity, 4, 8, BitLibParityEven, 4));
    mu_check(bit_lib_test_parity(data_always_even_parity, 8, 4, BitLibParityEven, 4));
    mu_check(bit_lib_test_parity(data_always_odd_parity, 12, 4, BitLibParityEven, 4));

    mu_check(!bit_lib_test_parity(data_always_even_parity, 0, 16, BitLibParityEven, 4));
    mu_check(!bit_lib_test_parity(data_always_even_parity, 4, 12, BitLibParityEven, 4));
    mu_check(!bit_lib_test_parity(data_always_even_parity, 8, 8, BitLibParityEven, 4));
    mu_check(!bit_lib_test_parity(data_always_even_parity, 12, 4, BitLibParityEven, 4));
}

MU_TEST(test_bit_lib_remove_bit_every_nth) {
    // TODO FL-3494: more tests
    uint8_t data_i[1] = {0b00001111};
    uint8_t data_o[1] = {0b00011111};
    size_t length;

    length = bit_lib_remove_bit_every_nth(data_i, 0, 8, 3);
    mu_assert_int_eq(6, length);
    mu_assert_mem_eq(data_o, data_i, 1);
}

MU_TEST(test_bit_lib_reverse_bits) {
    uint8_t data_1_i[2] = {0b11001010, 0b00011111};
    uint8_t data_1_o[2] = {0b11111000, 0b01010011};

    // reverse bits [0..15]
    bit_lib_reverse_bits(data_1_i, 0, 16);
    mu_assert_mem_eq(data_1_o, data_1_i, 2);

    uint8_t data_2_i[2] = {0b11001010, 0b00011111};
    uint8_t data_2_o[2] = {0b11001000, 0b01011111};

    // reverse bits [4..11]
    bit_lib_reverse_bits(data_2_i, 4, 8);
    mu_assert_mem_eq(data_2_o, data_2_i, 2);
}

MU_TEST(test_bit_lib_copy_bits) {
    uint8_t data_1_i[2] = {0b11001010, 0b00011111};
    uint8_t data_1_o[2] = {0};

    // data_1_o[0..15] = data_1_i[0..15]
    bit_lib_copy_bits(data_1_o, 0, 16, data_1_i, 0);
    mu_assert_mem_eq(data_1_i, data_1_o, 2);

    memset(data_1_o, 0, 2);
    // data_1_o[4..11] = data_1_i[0..7]
    bit_lib_copy_bits(data_1_o, 4, 8, data_1_i, 0);
    mu_assert_mem_eq(((uint8_t[]){0b00001100, 0b10100000}), data_1_o, 2);
}

MU_TEST(test_bit_lib_get_bit_count) {
    mu_assert_int_eq(0, bit_lib_get_bit_count(0));
    mu_assert_int_eq(1, bit_lib_get_bit_count(0b1));
    mu_assert_int_eq(1, bit_lib_get_bit_count(0b10));
    mu_assert_int_eq(2, bit_lib_get_bit_count(0b11));
    mu_assert_int_eq(4, bit_lib_get_bit_count(0b11000011));
    mu_assert_int_eq(6, bit_lib_get_bit_count(0b11000011000011));
    mu_assert_int_eq(8, bit_lib_get_bit_count(0b11111111));
    mu_assert_int_eq(16, bit_lib_get_bit_count(0b11111110000000000000000111111111));
    mu_assert_int_eq(32, bit_lib_get_bit_count(0b11111111111111111111111111111111));
}

MU_TEST(test_bit_lib_reverse_16_fast) {
    mu_assert_int_eq(0b0000000000000000, bit_lib_reverse_16_fast(0b0000000000000000));
    mu_assert_int_eq(0b1000000000000000, bit_lib_reverse_16_fast(0b0000000000000001));
    mu_assert_int_eq(0b1100000000000000, bit_lib_reverse_16_fast(0b0000000000000011));
    mu_assert_int_eq(0b0000100000001001, bit_lib_reverse_16_fast(0b1001000000010000));
}

MU_TEST(test_bit_lib_crc16) {
    uint8_t data[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint8_t data_size = 9;

    // Algorithm
    // Check	Poly	Init	RefIn	RefOut	XorOut
    // CRC-16/CCITT-FALSE
    // 0x29B1	0x1021	0xFFFF	false	false	0x0000
    mu_assert_int_eq(0x29B1, bit_lib_crc16(data, data_size, 0x1021, 0xFFFF, false, false, 0x0000));
    // CRC-16/ARC
    // 0xBB3D	0x8005	0x0000	true	true	0x0000
    mu_assert_int_eq(0xBB3D, bit_lib_crc16(data, data_size, 0x8005, 0x0000, true, true, 0x0000));
    // CRC-16/AUG-CCITT
    // 0xE5CC	0x1021	0x1D0F	false	false	0x0000
    mu_assert_int_eq(0xE5CC, bit_lib_crc16(data, data_size, 0x1021, 0x1D0F, false, false, 0x0000));
    // CRC-16/BUYPASS
    // 0xFEE8	0x8005	0x0000	false	false	0x0000
    mu_assert_int_eq(0xFEE8, bit_lib_crc16(data, data_size, 0x8005, 0x0000, false, false, 0x0000));
    // CRC-16/CDMA2000
    // 0x4C06	0xC867	0xFFFF	false	false	0x0000
    mu_assert_int_eq(0x4C06, bit_lib_crc16(data, data_size, 0xC867, 0xFFFF, false, false, 0x0000));
    // CRC-16/DDS-110
    // 0x9ECF	0x8005	0x800D	false	false	0x0000
    mu_assert_int_eq(0x9ECF, bit_lib_crc16(data, data_size, 0x8005, 0x800D, false, false, 0x0000));
    // CRC-16/DECT-R
    // 0x007E	0x0589	0x0000	false	false	0x0001
    mu_assert_int_eq(0x007E, bit_lib_crc16(data, data_size, 0x0589, 0x0000, false, false, 0x0001));
    // CRC-16/DECT-X
    // 0x007F	0x0589	0x0000	false	false	0x0000
    mu_assert_int_eq(0x007F, bit_lib_crc16(data, data_size, 0x0589, 0x0000, false, false, 0x0000));
    // CRC-16/DNP
    // 0xEA82	0x3D65	0x0000	true	true	0xFFFF
    mu_assert_int_eq(0xEA82, bit_lib_crc16(data, data_size, 0x3D65, 0x0000, true, true, 0xFFFF));
    // CRC-16/EN-13757
    // 0xC2B7	0x3D65	0x0000	false	false	0xFFFF
    mu_assert_int_eq(0xC2B7, bit_lib_crc16(data, data_size, 0x3D65, 0x0000, false, false, 0xFFFF));
    // CRC-16/GENIBUS
    // 0xD64E	0x1021	0xFFFF	false	false	0xFFFF
    mu_assert_int_eq(0xD64E, bit_lib_crc16(data, data_size, 0x1021, 0xFFFF, false, false, 0xFFFF));
    // CRC-16/MAXIM
    // 0x44C2	0x8005	0x0000	true	true	0xFFFF
    mu_assert_int_eq(0x44C2, bit_lib_crc16(data, data_size, 0x8005, 0x0000, true, true, 0xFFFF));
    // CRC-16/MCRF4XX
    // 0x6F91	0x1021	0xFFFF	true	true	0x0000
    mu_assert_int_eq(0x6F91, bit_lib_crc16(data, data_size, 0x1021, 0xFFFF, true, true, 0x0000));
    // CRC-16/RIELLO
    // 0x63D0	0x1021	0xB2AA	true	true	0x0000
    mu_assert_int_eq(0x63D0, bit_lib_crc16(data, data_size, 0x1021, 0xB2AA, true, true, 0x0000));
    // CRC-16/T10-DIF
    // 0xD0DB	0x8BB7	0x0000	false	false	0x0000
    mu_assert_int_eq(0xD0DB, bit_lib_crc16(data, data_size, 0x8BB7, 0x0000, false, false, 0x0000));
    // CRC-16/TELEDISK
    // 0x0FB3	0xA097	0x0000	false	false	0x0000
    mu_assert_int_eq(0x0FB3, bit_lib_crc16(data, data_size, 0xA097, 0x0000, false, false, 0x0000));
    // CRC-16/TMS37157
    // 0x26B1	0x1021	0x89EC	true	true	0x0000
    mu_assert_int_eq(0x26B1, bit_lib_crc16(data, data_size, 0x1021, 0x89EC, true, true, 0x0000));
    // CRC-16/USB
    // 0xB4C8	0x8005	0xFFFF	true	true	0xFFFF
    mu_assert_int_eq(0xB4C8, bit_lib_crc16(data, data_size, 0x8005, 0xFFFF, true, true, 0xFFFF));
    // CRC-A
    // 0xBF05	0x1021	0xC6C6	true	true	0x0000
    mu_assert_int_eq(0xBF05, bit_lib_crc16(data, data_size, 0x1021, 0xC6C6, true, true, 0x0000));
    // CRC-16/KERMIT
    // 0x2189	0x1021	0x0000	true	true	0x0000
    mu_assert_int_eq(0x2189, bit_lib_crc16(data, data_size, 0x1021, 0x0000, true, true, 0x0000));
    // CRC-16/MODBUS
    // 0x4B37	0x8005	0xFFFF	true	true	0x0000
    mu_assert_int_eq(0x4B37, bit_lib_crc16(data, data_size, 0x8005, 0xFFFF, true, true, 0x0000));
    // CRC-16/X-25
    // 0x906E	0x1021	0xFFFF	true	true	0xFFFF
    mu_assert_int_eq(0x906E, bit_lib_crc16(data, data_size, 0x1021, 0xFFFF, true, true, 0xFFFF));
    // CRC-16/XMODEM
    // 0x31C3	0x1021	0x0000	false	false	0x0000
    mu_assert_int_eq(0x31C3, bit_lib_crc16(data, data_size, 0x1021, 0x0000, false, false, 0x0000));
}

MU_TEST(test_bit_lib_num_to_bytes_be) {
    uint8_t src[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint8_t dest[8];

    bit_lib_num_to_bytes_be(0x01, 1, dest);
    mu_assert_mem_eq(src, dest, sizeof(src[0]));

    bit_lib_num_to_bytes_be(0x0123456789ABCDEF, 4, dest);
    mu_assert_mem_eq(src + 4, dest, 4 * sizeof(src[0]));

    bit_lib_num_to_bytes_be(0x0123456789ABCDEF, 8, dest);
    mu_assert_mem_eq(src, dest, 8 * sizeof(src[0]));

    bit_lib_num_to_bytes_be(bit_lib_bytes_to_num_be(src, 8), 8, dest);
    mu_assert_mem_eq(src, dest, 8 * sizeof(src[0]));
}

MU_TEST(test_bit_lib_num_to_bytes_le) {
    uint8_t dest[8];

    uint8_t n2b_le_expected_1[] = {0x01};
    bit_lib_num_to_bytes_le(0x01, 1, dest);
    mu_assert_mem_eq(n2b_le_expected_1, dest, sizeof(n2b_le_expected_1[0]));

    uint8_t n2b_le_expected_2[] = {0xEF, 0xCD, 0xAB, 0x89};
    bit_lib_num_to_bytes_le(0x0123456789ABCDEF, 4, dest);
    mu_assert_mem_eq(n2b_le_expected_2, dest, 4 * sizeof(n2b_le_expected_2[0]));

    uint8_t n2b_le_expected_3[] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
    bit_lib_num_to_bytes_le(0x0123456789ABCDEF, 8, dest);
    mu_assert_mem_eq(n2b_le_expected_3, dest, 8 * sizeof(n2b_le_expected_3[0]));

    bit_lib_num_to_bytes_le(bit_lib_bytes_to_num_le(n2b_le_expected_3, 8), 8, dest);
    mu_assert_mem_eq(n2b_le_expected_3, dest, 8 * sizeof(n2b_le_expected_3[0]));
}

MU_TEST(test_bit_lib_bytes_to_num_be) {
    uint8_t src[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint64_t res;

    res = bit_lib_bytes_to_num_be(src, 1);
    mu_assert_int_eq(0x01, res);

    res = bit_lib_bytes_to_num_be(src, 4);
    mu_assert_int_eq(0x01234567, res);

    res = bit_lib_bytes_to_num_be(src, 8);
    uint64_t expected = 0x0123456789ABCDEF;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));
}

MU_TEST(test_bit_lib_bytes_to_num_le) {
    uint8_t src[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint64_t res;

    res = bit_lib_bytes_to_num_le(src, 1);
    mu_assert_int_eq(0x01, res);

    res = bit_lib_bytes_to_num_le(src, 4);
    mu_assert_int_eq(0x67452301, res);

    res = bit_lib_bytes_to_num_le(src, 8);
    uint64_t expected = 0xEFCDAB8967452301;
    mu_assert_mem_eq(&expected, &res, sizeof(expected));
}

MU_TEST(test_bit_lib_bytes_to_num_bcd) {
    uint8_t src[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint64_t res;
    bool is_bcd_res;

    res = bit_lib_bytes_to_num_bcd(src, 1, &is_bcd_res);
    mu_assert_int_eq(01, res);
    mu_assert_int_eq(true, is_bcd_res);

    res = bit_lib_bytes_to_num_bcd(src, 4, &is_bcd_res);
    mu_assert_int_eq(1234567, res);
    mu_assert_int_eq(true, is_bcd_res);

    uint8_t digits[5] = {0x98, 0x76, 0x54, 0x32, 0x10};
    uint64_t expected = 9876543210;
    res = bit_lib_bytes_to_num_bcd(digits, 5, &is_bcd_res);
    mu_assert_mem_eq(&expected, &res, sizeof(expected));
    mu_assert_int_eq(true, is_bcd_res);

    res = bit_lib_bytes_to_num_bcd(src, 8, &is_bcd_res);
    mu_assert_int_eq(false, is_bcd_res);
}

MU_TEST_SUITE(test_bit_lib) {
    MU_RUN_TEST(test_bit_lib_increment_index);
    MU_RUN_TEST(test_bit_lib_is_set);
    MU_RUN_TEST(test_bit_lib_push);
    MU_RUN_TEST(test_bit_lib_set_bit);
    MU_RUN_TEST(test_bit_lib_set_bits);
    MU_RUN_TEST(test_bit_lib_get_bit);
    MU_RUN_TEST(test_bit_lib_get_bits);
    MU_RUN_TEST(test_bit_lib_get_bits_16);
    MU_RUN_TEST(test_bit_lib_get_bits_32);
    MU_RUN_TEST(test_bit_lib_get_bits_64);
    MU_RUN_TEST(test_bit_lib_test_parity_u32);
    MU_RUN_TEST(test_bit_lib_test_parity);
    MU_RUN_TEST(test_bit_lib_remove_bit_every_nth);
    MU_RUN_TEST(test_bit_lib_copy_bits);
    MU_RUN_TEST(test_bit_lib_reverse_bits);
    MU_RUN_TEST(test_bit_lib_get_bit_count);
    MU_RUN_TEST(test_bit_lib_reverse_16_fast);
    MU_RUN_TEST(test_bit_lib_crc16);
    MU_RUN_TEST(test_bit_lib_num_to_bytes_be);
    MU_RUN_TEST(test_bit_lib_num_to_bytes_le);
    MU_RUN_TEST(test_bit_lib_bytes_to_num_be);
    MU_RUN_TEST(test_bit_lib_bytes_to_num_le);
    MU_RUN_TEST(test_bit_lib_bytes_to_num_bcd);
}

int run_minunit_test_bit_lib(void) {
    MU_RUN_SUITE(test_bit_lib);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_bit_lib)
