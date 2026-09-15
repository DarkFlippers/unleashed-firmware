#include "lfrfid_casi_format.h"

#include <bit_lib/bit_lib.h>

// The 40 EM4100 data bits: two zero bits, the 19-bit credential, then a 19-bit card
// field. The card number the access system knows is that field as is when its top
// bit is clear and the field less LFRFID_CASI_CARD_OFFSET when it is set (empirical,
// there is no public layout). Fields between the two ranges belong to neither, so
// they do not read as a badge. There is no parity, so any other EM4100 card whose
// bits fit reads as one too: this is a possible reading.
#define CASI_DATA_SIZE           (5)
#define CASI_CREDENTIAL_POSITION (2)
#define CASI_CARD_POSITION       (21)
#define CASI_FIELD_SIZE          (19)
#define CASI_CARD_FIELD_HIGH     (1UL << (CASI_FIELD_SIZE - 1))

static bool lfrfid_casi_format_decode(const uint8_t* data, uint32_t* credential, uint32_t* card) {
    if(bit_lib_get_bits(data, 0, CASI_CREDENTIAL_POSITION) != 0) {
        return false;
    }

    const uint32_t card_field = bit_lib_get_bits_32(data, CASI_CARD_POSITION, CASI_FIELD_SIZE);
    if(card_field >= CASI_CARD_FIELD_HIGH) {
        *card = card_field - LFRFID_CASI_CARD_OFFSET;
    } else if(card_field < CASI_CARD_FIELD_HIGH - LFRFID_CASI_CARD_OFFSET) {
        *card = card_field;
    } else {
        return false;
    }

    *credential = bit_lib_get_bits_32(data, CASI_CREDENTIAL_POSITION, CASI_FIELD_SIZE);
    return true;
}

static void lfrfid_casi_format_set_field(uint8_t* data, uint8_t position, uint32_t value) {
    uint8_t bytes[sizeof(uint32_t)];
    bit_lib_num_to_bytes_be(value, sizeof(bytes), bytes);
    bit_lib_copy_bits(data, position, CASI_FIELD_SIZE, bytes, sizeof(bytes) * 8 - CASI_FIELD_SIZE);
}

void lfrfid_casi_format_encode(uint32_t credential, uint32_t card, uint8_t* data) {
    furi_check(data);

    uint32_t card_field = card;
    if(card >= CASI_CARD_FIELD_HIGH - LFRFID_CASI_CARD_OFFSET) {
        card_field += LFRFID_CASI_CARD_OFFSET;
    }

    memset(data, 0, CASI_DATA_SIZE);
    lfrfid_casi_format_set_field(data, CASI_CREDENTIAL_POSITION, credential);
    lfrfid_casi_format_set_field(data, CASI_CARD_POSITION, card_field);
}

void lfrfid_casi_format_render(const uint8_t* data, FuriString* result) {
    furi_check(data);
    furi_check(result);

    uint32_t credential;
    uint32_t card;
    if(!lfrfid_casi_format_decode(data, &credential, &card)) {
        return;
    }

    furi_string_cat_printf(result, "\nC10106: %lu %06lu", credential, card);
}
