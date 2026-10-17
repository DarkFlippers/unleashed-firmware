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
#include <furi.h>

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

// One bit per block of a page in T5577Dump::read, which is what fixes its width.
static_assert(
    LFRFID_T5577_BLOCKS_IN_PAGE_0 <= 8,
    "T5577Dump::read holds one bit per block of a page");

/** True when this block's word came from the chip.
 *
 * Page and block are checked, because both are the same type and a caller that has them the
 * wrong way round is otherwise answered as if it did not.
 */
static inline bool t5577_dump_was_read(const T5577Dump* dump, uint8_t page, uint8_t block) {
    furi_check(page <= 1);
    furi_check(block < t5577_page_blocks(page));
    return (dump->read[page] & (1u << block)) != 0;
}

/** Record a block's word as having come from the chip. */
static inline void t5577_dump_set(T5577Dump* dump, uint8_t page, uint8_t block, uint32_t value) {
    furi_check(page <= 1);
    furi_check(block < t5577_page_blocks(page));
    dump->block[page][block] = value;
    dump->read[page] |= (uint8_t)(1u << block);
}

/** Record a password as one the chip answered to. */
static inline void t5577_dump_set_password(T5577Dump* dump, uint32_t password) {
    dump->password = password;
    dump->has_password = true;
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
 * @param[out] dump  contents, zeroed first, so a key the file leaves out reads as not read.
 *                   Left zeroed when the file is rejected
 * @param      path  full path
 * @return     true when the file was a T5577 dump this version understands and it held at
 *             least one block that came from a chip
 */
bool t5577_file_load(T5577Dump* dump, const char* path);

#ifdef __cplusplus
}
#endif
