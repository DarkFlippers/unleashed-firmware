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

/** The app that opens one, which is not part of the firmware.
 *
 * Here rather than in either app: the lfrfid saved list and the archive both hand a dump to it,
 * and a path written twice is a path that moves once and breaks the other caller silently. The
 * app is installed from the catalogue, so the path is where its category puts it.
 */
#define T5577_APP_PATH EXT_PATH("apps/RFID/t5577.fap")

/** A T5577's contents.
 *
 * Both pages are stored at page 0's length, so a block is always block[page][index]; the four
 * words page 1 does not have are never touched.
 *
 * `read` is what separates a block holding zero from a block this dump has nothing for - a read
 * comes back short whenever the chip drifts out of the field mid-dump, and a chip that wants a
 * password refuses the detect's own block 0 read, so its dump is empty rather than partial. A
 * tool must not write back a block it does not have a bit for here.
 *
 * It does not say where the word came from: a block typed into an editor carries the same bit as
 * a block the chip answered with, because what a write puts back is what the dump holds.
 */
typedef struct {
    uint32_t block[2][LFRFID_T5577_BLOCKS_IN_PAGE_0];
    uint8_t read[2]; // one bit per block: this dump holds a word for it, rather than padding
    uint32_t password; // a password known to work on the chip
    bool has_password;
} T5577Dump;

// One bit per block of a page in T5577Dump::read, which is what fixes its width.
static_assert(
    LFRFID_T5577_BLOCKS_IN_PAGE_0 <= 8,
    "T5577Dump::read holds one bit per block of a page");

// Both pages are stored at page 0's length, which only holds while page 0 is the longer one.
static_assert(
    LFRFID_T5577_BLOCKS_IN_PAGE_1 <= LFRFID_T5577_BLOCKS_IN_PAGE_0,
    "a page has to fit in page 0's worth of blocks");

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

/** Blocks a T5577 holds across both pages. */
#define T5577_BLOCKS (LFRFID_T5577_BLOCKS_IN_PAGE_0 + LFRFID_T5577_BLOCKS_IN_PAGE_1)

/** Blocks a write puts back: page 0 entire, page 1 without block 0. */
#define T5577_RESTORE_BLOCKS (T5577_BLOCKS - 1)

/** Blocks a dump holds.
 *
 * T5577_BLOCKS of them is the only thing that says a read got the whole chip rather than as
 * much of it as it was allowed.
 */
static inline uint8_t t5577_dump_blocks_read(const T5577Dump* dump) {
    uint8_t count = 0;

    for(uint8_t page = 0; page <= 1; page++) {
        for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
            if(t5577_dump_was_read(dump, page, block)) count++;
        }
    }

    return count;
}

/** One block's address, as a value.
 *
 * Both fields are the same type, so this does not stop a swap at the brace. What it stops is a
 * page and a block being split apart and re-paired wrongly on the way through a call chain,
 * which is where the restore's two walks would otherwise do it.
 */
typedef struct {
    uint8_t page;
    uint8_t block;
} T5577Target;

/** The block at one place in the order a write puts them back in.
 *
 * Data blocks first and block 0 last, because block 0 decides how the chip talks: write it
 * first and the rest of the sequence is verified against a configuration the chip is no
 * longer using. Block 7 is the password store, and it has to be in place before block 0
 * turns protection on - and from the write of block 7 onwards, on a chip that is already
 * protected, frames carry the password block 7 now holds rather than the one the chip was
 * opened with. Page 1 carries the traceability data the chip was made with and goes
 * back as well, since a copy that leaves it behind is not the same tag; its block 0 repeats
 * the configuration and is left alone. This is the order Proxmark3 restores in.
 */
static inline T5577Target t5577_restore_target(uint8_t index) {
    static const T5577Target order[] = {
        {0, 1},
        {0, 2},
        {0, 3},
        {0, 4},
        {0, 5},
        {0, 6},
        {0, 7},
        {1, 1},
        {1, 2},
        {1, 3},
        {0, 0},
    };

    // Unbounded above, and asserted here: a deleted row would otherwise zero-fill, which reads
    // as page 0 block 0 and writes the dropped block nowhere, with the count still agreeing.
    static_assert(
        sizeof(order) / sizeof(order[0]) == T5577_RESTORE_BLOCKS,
        "every block a write puts back needs a place in the order");

    furi_check(index < T5577_RESTORE_BLOCKS);

    return order[index];
}

/** Whether a write would put back the block at this place in that order. */
static inline bool t5577_restore_wanted(const T5577Dump* dump, uint8_t index) {
    const T5577Target target = t5577_restore_target(index);

    if(!t5577_dump_was_read(dump, target.page, target.block)) return false;

    // Zero is not a configuration: a chip holding it answers nothing at all, and it is what a
    // dump from a failed read holds, so a write skips it as a Proxmark3 restore does. Anything
    // else goes back, including a configuration this reader cannot follow afterwards - the
    // editor composes those deliberately, and the caller is the one that says so.
    if(target.page == 0 && target.block == 0 && dump->block[0][0] == 0) return false;

    return true;
}

/** Whether two dumps hold the same thing.
 *
 * Field by field rather than memcmp: the struct has padding after `read` and after
 * `has_password`, and what is in it is nobody's business.
 */
static inline bool t5577_dump_equal(const T5577Dump* a, const T5577Dump* b) {
    for(uint8_t page = 0; page <= 1; page++) {
        if(a->read[page] != b->read[page]) return false;

        for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
            if(a->block[page][block] != b->block[page][block]) return false;
        }
    }

    return a->has_password == b->has_password && a->password == b->password;
}

/** Blocks a write would put back at all, which is what makes one worth offering. */
static inline uint8_t t5577_restore_total(const T5577Dump* dump) {
    uint8_t total = 0;

    for(uint8_t index = 0; index < T5577_RESTORE_BLOCKS; index++) {
        if(t5577_restore_wanted(dump, index)) total++;
    }

    return total;
}

/** The password a write at this place in the order has to carry.
 *
 * Writing block 7 replaces the password the chip is asking for, so every frame after it carries
 * block 7's new word rather than the one the chip was opened with. The switch is derived from
 * the order rather than written down as an index, so the two cannot drift apart.
 *
 * `with_pass` answers a different question from "is there a password": a frame carrying one is
 * read off the wrong bit positions by a chip that wants none, which on a write is a corrupted
 * block 0. So it is sent only for a dump that came back with a password, which is a dump the
 * chip refused to be read without.
 *
 * @param      dump       contents being written back
 * @param      index      place in the write order, or T5577_RESTORE_BLOCKS for the read back
 *                        that follows the whole order
 * @param[out] with_pass  whether the frame should carry a password at all
 * @return     the password to address the frame with, when it carries one
 */
static inline uint32_t
    t5577_restore_password(const T5577Dump* dump, uint8_t index, bool* with_pass) {
    furi_check(index <= T5577_RESTORE_BLOCKS);
    furi_check(with_pass);

    *with_pass = dump->has_password;

    for(uint8_t i = 0; i < index; i++) {
        const T5577Target target = t5577_restore_target(i);
        if(target.page == 0 && target.block == LFRFID_T5577_PASSWORD_BLOCK &&
           t5577_restore_wanted(dump, i)) {
            return dump->block[0][7];
        }
    }

    return dump->has_password ? dump->password : 0;
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
