/** @file lfrfid_write_fill.h
 *
 * Shared bodies for the per-protocol write_data() callbacks.
 *
 * Nearly every protocol fills a write request the same way and differs only in its config word
 * and whether EM4305 data goes back inverted. Those stay with the protocol; the filling lives
 * here once.
 *
 * Firmware internal, not exported to apps.
 */

#pragma once

#include "lfrfid_protocols.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Fill a T5577 write request from an MSB-first bit buffer.
 *
 * How much data to write is read out of the config word's maxblock field, since that is what
 * the tag will stream back and the two can never disagree.
 *
 * @param      request  The request to fill; write_type must already be LFRFIDWriteTypeT5577
 * @param      block0   The configuration word, whose maxblock field says how many data blocks
 *                      follow it - at most LFRFID_T5577_BLOCK_COUNT - 1
 * @param      bits     Encoded data; maxblock * 32 bits are read from position 0
 */
void lfrfid_write_fill_t5577(LFRFIDWriteRequest* request, uint32_t block0, const uint8_t* bits);

/** Fill an EM4305 write request from an MSB-first bit buffer.
 *
 * How many words to write is read out of the config word's maxblock field, as for T5577. Each
 * word goes back bit-reversed because em4305_write_word() sends a word least significant bit
 * first; words ascend with the data, as the T5577 blocks do.
 *
 * @param      request  The request to fill; write_type must already be LFRFIDWriteTypeEM4305
 * @param      word4    The configuration word, whose maxblock field counts itself and the data
 *                      words after it - at most EM4x05_WORD_COUNT - 1
 * @param      bits     Encoded data; (maxblock - 4) * 32 bits are read from position 0
 * @param      invert   Invert every data bit. Only GProxII needs this, and why its two
 *                      biphase siblings do not is unrecorded (#1182) - so do not infer it
 *                      from the modulation in word4
 */
void lfrfid_write_fill_em4305(
    LFRFIDWriteRequest* request,
    uint32_t word4,
    const uint8_t* bits,
    bool invert);

#ifdef __cplusplus
}
#endif
