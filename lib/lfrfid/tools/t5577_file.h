/**
 * @file t5577_file.h
 * @brief T5577 dump files.
 *
 * The chip's contents as a file holds them, which is a different thing from a `.rfid` file: that
 * one says what a card is, this one says what is in the silicon - the configuration that decides
 * how the chip talks, and the password that protects it.
 *
 * documentation/file_formats/T5577FileFormat.md is the format itself.
 */
#pragma once

#include "t5577.h"

#ifdef __cplusplus
extern "C" {
#endif

#define T5577_FILE_EXTENSION ".t5577"

/** Blocks a page of a T5577 has. Page 1 is shorter than page 0, and indexing both from the same
 * array means every caller has to ask.
 */
static inline uint8_t t5577_page_blocks(uint8_t page) {
    return (page == 0) ? LFRFID_T5577_BLOCKS_IN_PAGE_0 : LFRFID_T5577_BLOCKS_IN_PAGE_1;
}

/** A T5577's contents.
 *
 * Both pages are stored at page 0's length, so a block is always block[page][index]; the four
 * words page 1 does not have are never touched.
 *
 * `read` is what separates a block holding zero from a block that was never read - a read is
 * partial more often than not, since a protected chip answers nothing without its password and a
 * chip that drifts out of the field leaves a gap. A tool must not write back a block it does not
 * have a bit for here.
 */
typedef struct {
    uint32_t block[2][LFRFID_T5577_BLOCKS_IN_PAGE_0];
    uint8_t read[2]; // one bit per block: came from the chip, rather than padding
    uint32_t password; // a password known to work on the chip
    bool has_password;
} T5577Dump;

/** True when this block's word came from the chip. */
static inline bool t5577_dump_was_read(const T5577Dump* dump, uint8_t page, uint8_t block) {
    return (dump->read[page] & (1u << block)) != 0;
}

/** Record a block's word as having come from the chip. */
static inline void t5577_dump_set(T5577Dump* dump, uint8_t page, uint8_t block, uint32_t value) {
    dump->block[page][block] = value;
    dump->read[page] |= (uint8_t)(1u << block);
}

/** Write a dump to a file, replacing whatever was there.
 *
 * @param      dump  contents to write
 * @param      path  full path, conventionally ending in T5577_FILE_EXTENSION
 * @return     true when the whole file was written
 */
bool t5577_file_save(const T5577Dump* dump, const char* path);

/** Read a dump from a file.
 *
 * @param[out] dump  contents, zeroed first, so a key the file leaves out reads as not read
 * @param      path  full path
 * @return     true when the file was a T5577 dump this version understands
 */
bool t5577_file_load(T5577Dump* dump, const char* path);

#ifdef __cplusplus
}
#endif
