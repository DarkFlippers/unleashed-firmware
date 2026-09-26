#include "lfrfid_casi_format.h"

#include <bit_lib/bit_lib.h>

// The 40 EM4100 data bits: two zero bits, the 19-bit credential, then a 19-bit card
// field. The card number is the field while its top bit is clear and the field less
// LFRFID_CASI_CARD_OFFSET once it is set, so the top bit tells the two field encodings
// apart. Cards are plain fields up to at least 195543 and offset fields from at least
// 286571; the encoder switches at 2^18, the first value a plain field cannot hold, and
// the decoder is right for any vendor switch at or below that (refs #1158). The vendor's
// card range, 0-457681, is 2^19 - 1 - 66606. The frame carries no parity, so only the
// credential range (six digits beginning with 15, the vendor's rule) keeps other EM4100
// cards from reading as a badge: about 0.5% of all ids, though a card whose id starts
// 0x12 or 0x13 matches more often than not.
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
    *card = card_field >= CASI_CARD_FIELD_HIGH ? card_field - LFRFID_CASI_CARD_OFFSET : card_field;

    return true;
}

void lfrfid_casi_format_encode(uint32_t credential, uint32_t card, uint8_t* data) {
    furi_check(data);
    furi_check(credential >= LFRFID_CASI_CREDENTIAL_MIN);
    furi_check(credential <= LFRFID_CASI_CREDENTIAL_MAX);
    furi_check(card <= LFRFID_CASI_CARD_MAX);

    // a plain field up to the top bit, offset from there on
    uint32_t card_field = card;
    if(card >= CASI_CARD_FIELD_HIGH) {
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
