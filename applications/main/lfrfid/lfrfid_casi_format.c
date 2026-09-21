#include "lfrfid_casi_format.h"

#include <bit_lib/bit_lib.h>

// The 40 EM4100 data bits: two zero bits, the 19-bit credential, then a 19-bit card
// field. The card number the access system prints is that field as is when its top bit
// is clear and the field less LFRFID_CASI_CARD_OFFSET when it is set. No public layout:
// this one fits eleven badges from two sites, three of them public (https://redd.it/12v4oi4,
// raw bits next to printed ids), and the Proxmark3 Casi-Rusco trace. Where the offset
// starts is the one part not observed - no sample falls between cards 173xxx and 286xxx,
// so the split is put where the card numbers stay continuous. The frame carries no parity,
// so only the credential range keeps other EM4100 cards from reading as a badge; it holds
// them to 0.4% of all ids, though a card whose id starts 0x12 or 0x13 matches about half
// the time.
#define CASI_DATA_SIZE           (5)
#define CASI_CREDENTIAL_POSITION (2)
#define CASI_CARD_POSITION       (21)
#define CASI_CARD_FIELD_HIGH     (1UL << (LFRFID_CASI_FIELD_SIZE - 1))

static bool lfrfid_casi_format_decode(const uint8_t* data, uint32_t* credential, uint32_t* card) {
    if(bit_lib_get_bits(data, 0, CASI_CREDENTIAL_POSITION) != 0) {
        return false;
    }

    *credential = bit_lib_get_bits_32(data, CASI_CREDENTIAL_POSITION, LFRFID_CASI_FIELD_SIZE);
    if(*credential < LFRFID_CASI_CREDENTIAL_MIN || *credential > LFRFID_CASI_CREDENTIAL_MAX) {
        return false;
    }

    const uint32_t card_field =
        bit_lib_get_bits_32(data, CASI_CARD_POSITION, LFRFID_CASI_FIELD_SIZE);
    if(card_field >= CASI_CARD_FIELD_HIGH) {
        *card = card_field - LFRFID_CASI_CARD_OFFSET;
    } else if(card_field < CASI_CARD_FIELD_HIGH - LFRFID_CASI_CARD_OFFSET) {
        *card = card_field;
    } else {
        return false;
    }

    return true;
}

void lfrfid_casi_format_encode(uint32_t credential, uint32_t card, uint8_t* data) {
    furi_check(data);
    furi_check(credential >= LFRFID_CASI_CREDENTIAL_MIN);
    furi_check(credential <= LFRFID_CASI_CREDENTIAL_MAX);
    furi_check(card <= LFRFID_CASI_CARD_MAX);

    uint32_t card_field = card;
    if(card >= CASI_CARD_FIELD_HIGH - LFRFID_CASI_CARD_OFFSET) {
        card_field += LFRFID_CASI_CARD_OFFSET;
    }

    // the two zero bits, the credential and the card field are the whole frame
    bit_lib_num_to_bytes_be(
        (uint64_t)credential << LFRFID_CASI_FIELD_SIZE | card_field, CASI_DATA_SIZE, data);
}

void lfrfid_casi_format_render(const uint8_t* data, FuriString* result) {
    furi_check(data);
    furi_check(result);

    uint32_t credential;
    uint32_t card;
    if(!lfrfid_casi_format_decode(data, &credential, &card)) {
        return;
    }

    // it follows the EM4100 reading, so it is marked as the alternative like the HID lines
    furi_string_cat_printf(result, "\nor C10106: %06lu %06lu", credential, card);
}
