/**
 * @file mf_classic_parser_util.h
 * @brief Shared helpers for the Mifare Classic supported card parsers.
 */
#pragma once

#include <nfc/protocols/mf_classic/mf_classic.h>

/**
 * @brief Tell whether a block holds data worth rendering.
 *
 * Knowing a sector's keys does not mean its blocks were read - the two are tracked by separate
 * masks - and a block that was never read is left zero-filled. The read mask answers this, except
 * for dumps saved before the file format recorded one: mf_classic_load() wipes the mask for those
 * while keeping the bytes. Non-zero content is therefore data as well, which keeps such dumps
 * parsing. Only a block that was genuinely read may be reported as a zero.
 *
 * @param[in] data pointer to the card data.
 * @param[in] block_num block to test; one the card does not have never holds data.
 * @returns true if the block may be rendered, false if it must be treated as missing.
 */
static inline bool mf_classic_parser_block_has_data(const MfClassicData* data, uint8_t block_num) {
    if(block_num >= mf_classic_get_total_block_num(data->type)) return false;
    if(mf_classic_is_block_read(data, block_num)) return true;

    for(size_t i = 0; i < MF_CLASSIC_BLOCK_SIZE; i++) {
        if(data->block[block_num].data[i] != 0) return true;
    }
    return false;
}
