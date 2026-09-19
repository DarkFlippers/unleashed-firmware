#include "lfrfid_hid_format.h"

#include <bit_lib/bit_lib.h>

// The Generic HIDProx data is the 44 bits between the frame preambles: a size
// header, then the Wiegand frame right-aligned. 37-bit frames carry no header,
// shorter ones set bit 6 and a 1 right before the frame.
#define HID_FIELD_BIT_SIZE     (44)
#define HID_FIELD_DATA_SIZE    (6)
#define HID_FIELD_SHORT_FLAG   (6)
#define HID_FRAME_BIT_SIZE_MAX (37)
#define HID_FRAME_DATA_SIZE    (5)
#define HID_FRAME_BIT_SIZE_MIN (26)

struct LfRfidHidFormat {
    const char* name; // as shown to the user, e.g. "H10304"
    uint8_t bit_size; // Wiegand frame length
    uint8_t fc_position; // facility code bit index in the frame, msb first
    uint8_t fc_size; // facility code bits, 0 for a format without one
    uint8_t cn_position; // card number bit index in the frame, msb first
    uint8_t cn_size; // card number bits
    void (*set_parity)(uint8_t* frame); // fill in the parity bits of a frame
    bool (*check_parity)(const uint8_t* frame); // whether the parity bits of a frame hold
};

// The bit that makes a run of bits even, i.e. 1 when it holds an odd count of ones
static bool
    lfrfid_hid_format_even_parity_bit(const uint8_t* frame, uint8_t position, uint8_t size) {
    return bit_lib_test_parity_32(bit_lib_get_bits_32(frame, position, size), BitLibParityEven);
}

static bool lfrfid_hid_format_even_parity_bit_of(
    const uint8_t* frame,
    const uint8_t* positions,
    uint8_t count) {
    bool parity = false;
    for(uint8_t i = 0; i < count; i++) {
        parity ^= bit_lib_get_bit(frame, positions[i]);
    }
    return parity;
}

// H10302 and H10304: even parity over bits 1-18, odd parity over bits 18-35
static void lfrfid_hid_format_37_set_parity(uint8_t* frame) {
    bit_lib_set_bit(frame, 0, lfrfid_hid_format_even_parity_bit(frame, 1, 18));
    bit_lib_set_bit(frame, 36, !lfrfid_hid_format_even_parity_bit(frame, 18, 18));
}

static bool lfrfid_hid_format_37_check_parity(const uint8_t* frame) {
    return bit_lib_get_bit(frame, 0) == lfrfid_hid_format_even_parity_bit(frame, 1, 18) &&
           bit_lib_get_bit(frame, 36) == !lfrfid_hid_format_even_parity_bit(frame, 18, 18);
}

// AMAG S10401 (PointGuard MDI): even parity over bits 1-17, odd parity over bits 18-35.
// Proxmark3's MDI37 has the even parity over bits 1-18 and a 4-bit facility code at bit
// 3. Of 42 badges exported from one access system (card numbers up to 22 million, 29 of
// them with bit 18 set) all pass this parity and only the 13 with bit 18 clear pass
// Proxmark3's. The facility code is 6 bits since the vendor's range for it is 0-63.
static void lfrfid_hid_format_s10401_set_parity(uint8_t* frame) {
    bit_lib_set_bit(frame, 0, lfrfid_hid_format_even_parity_bit(frame, 1, 17));
    bit_lib_set_bit(frame, 36, !lfrfid_hid_format_even_parity_bit(frame, 18, 18));
}

static bool lfrfid_hid_format_s10401_check_parity(const uint8_t* frame) {
    return bit_lib_get_bit(frame, 0) == lfrfid_hid_format_even_parity_bit(frame, 1, 17) &&
           bit_lib_get_bit(frame, 36) == !lfrfid_hid_format_even_parity_bit(frame, 18, 18);
}

// H10306: even parity over bits 1-16, odd parity over bits 17-32
static void lfrfid_hid_format_h10306_set_parity(uint8_t* frame) {
    bit_lib_set_bit(frame, 0, lfrfid_hid_format_even_parity_bit(frame, 1, 16));
    bit_lib_set_bit(frame, 33, !lfrfid_hid_format_even_parity_bit(frame, 17, 16));
}

static bool lfrfid_hid_format_h10306_check_parity(const uint8_t* frame) {
    return bit_lib_get_bit(frame, 0) == lfrfid_hid_format_even_parity_bit(frame, 1, 16) &&
           bit_lib_get_bit(frame, 33) == !lfrfid_hid_format_even_parity_bit(frame, 17, 16);
}

// Corporate 1000 35-bit: bit 1 is even parity over 22 bits, bit 34 odd parity over
// another 22 and bit 0 odd parity over all of bits 1-34, so it is set last
static const uint8_t lfrfid_hid_format_c1000_35_even_bits[] = {
    2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18, 20, 21, 23, 24, 26, 27, 29, 30, 32, 33};
static const uint8_t lfrfid_hid_format_c1000_35_odd_bits[] = {
    1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20, 22, 23, 25, 26, 28, 29, 31, 32};

static bool lfrfid_hid_format_c1000_35_even_parity(const uint8_t* frame) {
    return lfrfid_hid_format_even_parity_bit_of(
        frame,
        lfrfid_hid_format_c1000_35_even_bits,
        COUNT_OF(lfrfid_hid_format_c1000_35_even_bits));
}

static bool lfrfid_hid_format_c1000_35_odd_parity(const uint8_t* frame) {
    return !lfrfid_hid_format_even_parity_bit_of(
        frame, lfrfid_hid_format_c1000_35_odd_bits, COUNT_OF(lfrfid_hid_format_c1000_35_odd_bits));
}

static bool lfrfid_hid_format_c1000_35_frame_parity(const uint8_t* frame) {
    return !(
        lfrfid_hid_format_even_parity_bit(frame, 1, 17) ^
        lfrfid_hid_format_even_parity_bit(frame, 18, 17));
}

static void lfrfid_hid_format_c1000_35_set_parity(uint8_t* frame) {
    bit_lib_set_bit(frame, 1, lfrfid_hid_format_c1000_35_even_parity(frame));
    bit_lib_set_bit(frame, 34, lfrfid_hid_format_c1000_35_odd_parity(frame));
    bit_lib_set_bit(frame, 0, lfrfid_hid_format_c1000_35_frame_parity(frame));
}

static bool lfrfid_hid_format_c1000_35_check_parity(const uint8_t* frame) {
    return bit_lib_get_bit(frame, 1) == lfrfid_hid_format_c1000_35_even_parity(frame) &&
           bit_lib_get_bit(frame, 34) == lfrfid_hid_format_c1000_35_odd_parity(frame) &&
           bit_lib_get_bit(frame, 0) == lfrfid_hid_format_c1000_35_frame_parity(frame);
}

// Formats sharing a frame length go most common first, that is the order they are listed in
static const LfRfidHidFormat lfrfid_hid_formats[] = {
    {
        .name = "H10304",
        .bit_size = 37,
        .fc_position = 1,
        .fc_size = 16,
        .cn_position = 17,
        .cn_size = 19,
        .set_parity = lfrfid_hid_format_37_set_parity,
        .check_parity = lfrfid_hid_format_37_check_parity,
    },
    {
        .name = "H10302",
        .bit_size = 37,
        .fc_position = 0,
        .fc_size = 0,
        .cn_position = 1,
        .cn_size = 35,
        .set_parity = lfrfid_hid_format_37_set_parity,
        .check_parity = lfrfid_hid_format_37_check_parity,
    },
    {
        .name = "S10401",
        .bit_size = 37,
        .fc_position = 1,
        .fc_size = 6,
        .cn_position = 7,
        .cn_size = 29,
        .set_parity = lfrfid_hid_format_s10401_set_parity,
        .check_parity = lfrfid_hid_format_s10401_check_parity,
    },
    {
        .name = "H10306",
        .bit_size = 34,
        .fc_position = 1,
        .fc_size = 16,
        .cn_position = 17,
        .cn_size = 16,
        .set_parity = lfrfid_hid_format_h10306_set_parity,
        .check_parity = lfrfid_hid_format_h10306_check_parity,
    },
    {
        .name = "Corp1000-35",
        .bit_size = 35,
        .fc_position = 2,
        .fc_size = 12,
        .cn_position = 14,
        .cn_size = 20,
        .set_parity = lfrfid_hid_format_c1000_35_set_parity,
        .check_parity = lfrfid_hid_format_c1000_35_check_parity,
    },
};

_Static_assert(
    COUNT_OF(lfrfid_hid_formats) == LFRFID_HID_FORMAT_COUNT,
    "LFRFID_HID_FORMAT_COUNT does not match lfrfid_hid_formats");

const LfRfidHidFormat* lfrfid_hid_format_get(size_t index) {
    return index < COUNT_OF(lfrfid_hid_formats) ? &lfrfid_hid_formats[index] : NULL;
}

const char* lfrfid_hid_format_get_name(const LfRfidHidFormat* format) {
    furi_check(format);
    return format->name;
}

bool lfrfid_hid_format_has_facility_code(const LfRfidHidFormat* format) {
    furi_check(format);
    return format->fc_size != 0;
}

uint64_t lfrfid_hid_format_get_facility_code_max(const LfRfidHidFormat* format) {
    furi_check(format);
    return (1ULL << format->fc_size) - 1;
}

uint64_t lfrfid_hid_format_get_card_number_max(const LfRfidHidFormat* format) {
    furi_check(format);
    return (1ULL << format->cn_size) - 1;
}

// Frame length from the size header, a copy of the static
// protocol_hid_generic_decode_protocol_size() that has to track it. A 1 in the first
// six bits is a frame of 38 to 43 bits, longer than any format here, so 0.
static uint8_t lfrfid_hid_format_frame_size(const uint8_t* data) {
    for(size_t bit_index = 0; bit_index < HID_FIELD_SHORT_FLAG; bit_index++) {
        if(bit_lib_get_bit(data, bit_index)) {
            return 0;
        }
    }

    if(!bit_lib_get_bit(data, HID_FIELD_SHORT_FLAG)) {
        return HID_FRAME_BIT_SIZE_MAX;
    }

    // the 1 right before the frame, the bit lfrfid_hid_format_encode() sets
    for(uint8_t size = HID_FRAME_BIT_SIZE_MAX - 1; size >= HID_FRAME_BIT_SIZE_MIN; size--) {
        if(bit_lib_get_bit(data, HID_FIELD_BIT_SIZE - size - 1)) {
            return size;
        }
    }
    return 0;
}

static void
    lfrfid_hid_format_set_bits(uint8_t* frame, uint8_t position, uint64_t value, uint8_t size) {
    uint8_t bytes[sizeof(uint64_t)];
    bit_lib_num_to_bytes_be(value, sizeof(bytes), bytes);
    bit_lib_copy_bits(frame, position, size, bytes, sizeof(bytes) * 8 - size);
}

static bool lfrfid_hid_format_decode(
    const LfRfidHidFormat* format,
    const uint8_t* data,
    uint64_t* fc,
    uint64_t* cn) {
    if(lfrfid_hid_format_frame_size(data) != format->bit_size) {
        return false;
    }

    uint8_t frame[HID_FRAME_DATA_SIZE] = {0};
    bit_lib_copy_bits(frame, 0, format->bit_size, data, HID_FIELD_BIT_SIZE - format->bit_size);

    if(!format->check_parity(frame)) {
        return false;
    }

    *fc = format->fc_size ? bit_lib_get_bits_64(frame, format->fc_position, format->fc_size) : 0;
    *cn = bit_lib_get_bits_64(frame, format->cn_position, format->cn_size);
    return true;
}

void lfrfid_hid_format_encode(
    const LfRfidHidFormat* format,
    uint64_t fc,
    uint64_t cn,
    uint8_t* data) {
    furi_check(format);
    furi_check(data);

    uint8_t frame[HID_FRAME_DATA_SIZE] = {0};
    lfrfid_hid_format_set_bits(frame, format->fc_position, fc, format->fc_size);
    lfrfid_hid_format_set_bits(frame, format->cn_position, cn, format->cn_size);
    format->set_parity(frame);

    memset(data, 0, HID_FIELD_DATA_SIZE);
    if(format->bit_size < HID_FRAME_BIT_SIZE_MAX) {
        bit_lib_set_bit(data, HID_FIELD_SHORT_FLAG, 1);
        bit_lib_set_bit(data, HID_FIELD_BIT_SIZE - format->bit_size - 1, 1);
    }
    bit_lib_copy_bits(data, HID_FIELD_BIT_SIZE - format->bit_size, format->bit_size, frame, 0);
}

void lfrfid_hid_format_render(const uint8_t* data, FuriString* result) {
    furi_check(data);
    furi_check(result);

    // A 37-bit frame fits H10304 and H10302 alike, and S10401 whenever bit 18 is clear,
    // so every match is listed and the ones after the first are marked as alternatives
    size_t matched = 0;
    for(size_t i = 0; i < COUNT_OF(lfrfid_hid_formats); i++) {
        const LfRfidHidFormat* format = &lfrfid_hid_formats[i];
        uint64_t fc;
        uint64_t cn;
        if(!lfrfid_hid_format_decode(format, data, &fc, &cn)) {
            continue;
        }

        furi_string_cat_printf(result, "\n%s%s: ", matched ? "or " : "", format->name);
        if(format->fc_size) {
            furi_string_cat_printf(result, "FC %lu ", (uint32_t)fc);
        }
        furi_string_cat_printf(result, "Card %llu", cn);
        matched++;
    }
}
