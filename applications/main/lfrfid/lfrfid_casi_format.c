#include "lfrfid_casi_format.h"

#include <bit_lib/bit_lib.h>

// The 40 EM4100 data bits: two zero bits, the 19-bit credential, then a 19-bit card
// field. The card number the access system prints is that field as is when its top
// bit is clear and the field less LFRFID_CASI_CARD_OFFSET when it is set; fields
// between the two ranges belong to neither. Credentials begin with 15, as the vendor
// documents for the twelve-digit badge id. There is no public layout: this one fits
// eleven badges from four credential series at two sites, three of them public
// (https://redd.it/12v4oi4, raw bits next to printed ids, all in the offset range),
// and the EM4100 Casi-Rusco sample in the Proxmark3 traces takes the other branch.
// The frame carries no parity, so an EM4100 card whose bits happen to fit reads as a
// badge too; the credential range keeps that to well under 2% of cards.
#define CASI_DATA_SIZE           (5)
#define CASI_CREDENTIAL_POSITION (2)
#define CASI_CARD_POSITION       (21)
#define CASI_FIELD_SIZE          (19)
#define CASI_CARD_FIELD_HIGH     (1UL << (CASI_FIELD_SIZE - 1))

static bool lfrfid_casi_format_decode(const uint8_t* data, uint32_t* credential, uint32_t* card) {
    if(bit_lib_get_bits(data, 0, CASI_CREDENTIAL_POSITION) != 0) {
        return false;
    }

    *credential = bit_lib_get_bits_32(data, CASI_CREDENTIAL_POSITION, CASI_FIELD_SIZE);
    if(*credential < LFRFID_CASI_CREDENTIAL_MIN || *credential > LFRFID_CASI_CREDENTIAL_MAX) {
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
        (uint64_t)credential << CASI_FIELD_SIZE | card_field, CASI_DATA_SIZE, data);
}

void lfrfid_casi_format_render(const uint8_t* data, FuriString* result) {
    furi_check(data);
    furi_check(result);

    uint32_t credential;
    uint32_t card;
    if(!lfrfid_casi_format_decode(data, &credential, &card)) {
        return;
    }

    furi_string_cat_printf(result, "\nC10106: %06lu %06lu", credential, card);
}
