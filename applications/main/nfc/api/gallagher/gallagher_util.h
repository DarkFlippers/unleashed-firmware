/* gallagher_util.h - Utilities for parsing Gallagher cards (New Zealand).
 * Author: Nick Mooney (nick@mooney.nz)
 * 
 * Reference: https://github.com/megabug/gallagher-research
*/

#pragma once

#include <lib/nfc/protocols/mf_classic/mf_classic.h>

#define GALLAGHER_CREDENTIAL_SECTOR 15

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t GALLAGHER_DECODE_TABLE[256];
extern const uint8_t GALLAGHER_CARDAX_ASCII[MF_CLASSIC_BLOCK_SIZE];

typedef struct GallagherCredential {
    uint8_t region;
    uint8_t issue;
    uint16_t facility;
    uint32_t card;
} GallagherCredential;

void gallagher_deobfuscate_and_parse_credential(
    GallagherCredential* credential,
    const uint8_t* cardholder_data_obfuscated);

#ifdef __cplusplus
}
#endif
