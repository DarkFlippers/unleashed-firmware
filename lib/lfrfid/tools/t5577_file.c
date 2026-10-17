#include "t5577_file.h"
#include <furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <bit_lib/bit_lib.h>

#define TAG "T5577File"

#define T5577_FILE_TYPE    "Flipper T5577 dump"
#define T5577_FILE_VERSION 1

static void t5577_file_block_key(FuriString* key, uint8_t page, uint8_t block) {
    furi_string_printf(key, "Page %u Block %u", page, block);
}

static const char* t5577_file_read_key(uint8_t page) {
    return (page == 0) ? "Page 0 blocks read" : "Page 1 blocks read";
}

// The read list as the format writes it: the block numbers, space separated. Written by hand
// rather than as a value array because the list is empty whenever a page did not answer, and an
// empty value is the one case a value array cannot express.
static void t5577_file_read_list(const T5577Dump* dump, uint8_t page, FuriString* list) {
    furi_string_reset(list);

    for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
        if(!t5577_dump_was_read(dump, page, block)) continue;
        if(!furi_string_empty(list)) furi_string_cat_str(list, " ");
        furi_string_cat_printf(list, "%u", block);
    }
}

static void t5577_file_parse_read_list(T5577Dump* dump, uint8_t page, const FuriString* list) {
    const char* cursor = furi_string_get_cstr(list);

    while(*cursor != '\0') {
        if(*cursor < '0' || *cursor > '9') {
            cursor++;
            continue;
        }

        uint32_t block = 0;
        while(*cursor >= '0' && *cursor <= '9') {
            block = block * 10 + (uint32_t)(*cursor - '0');
            cursor++;
        }

        // A number this page has no block for is a file from something else, or from a
        // version that numbers blocks differently. Either way there is nothing to mark.
        if(block >= t5577_page_blocks(page)) {
            FURI_LOG_W(TAG, "Page %u has no block %lu", page, block);
            continue;
        }

        dump->read[page] |= (uint8_t)(1u << block);
    }
}

// What block 0 says, for whoever opens the file in a text editor. It carries nothing block 0 does
// not, which is why it is a comment.
static void t5577_file_describe(const T5577Dump* dump, FuriString* text) {
    const uint32_t block0 = dump->block[0][0];
    const T5577ReadClock clock = t5577_read_clock_from_config(block0);
    const uint8_t blocks = (block0 >> LFRFID_T5577_MAXBLOCK_SHIFT) & LFRFID_T5577_MAXBLOCK_MASK;

    furi_string_printf(text, "Block 0: ");

    if(clock == T5577ReadClockUnsupported) {
        furi_string_cat_printf(text, "modulation %08lX", block0 & LFRFID_T5577_MODULATION_MASK);
    } else {
        furi_string_cat_printf(text, "ASK/Manchester, RF/%u", clock);
    }

    furi_string_cat_printf(text, ", %u data blocks", blocks);
    if(block0 & LFRFID_T5577_PWD) furi_string_cat_str(text, ", password enabled");
    if(block0 & LFRFID_T5577_ST_TERMINATOR) furi_string_cat_str(text, ", sequence terminator");
}

bool t5577_file_save(const T5577Dump* dump, const char* path) {
    furi_check(dump);
    furi_check(path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* key = furi_string_alloc();
    FuriString* text = furi_string_alloc();
    bool saved = false;

    do {
        if(!flipper_format_file_open_always(file, path)) break;
        if(!flipper_format_write_header_cstr(file, T5577_FILE_TYPE, T5577_FILE_VERSION)) break;

        t5577_file_describe(dump, text);
        if(!flipper_format_write_comment(file, text)) break;

        bool failed = false;
        for(uint8_t page = 0; page <= 1 && !failed; page++) {
            t5577_file_read_list(dump, page, text);
            if(!flipper_format_write_string(file, t5577_file_read_key(page), text)) {
                failed = true;
                break;
            }

            for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
                uint8_t word[4];
                bit_lib_num_to_bytes_be(dump->block[page][block], sizeof(word), word);

                t5577_file_block_key(key, page, block);
                if(!flipper_format_write_hex(file, furi_string_get_cstr(key), word, sizeof(word))) {
                    failed = true;
                    break;
                }
            }
        }
        if(failed) break;

        if(dump->has_password) {
            uint8_t password[4];
            bit_lib_num_to_bytes_be(dump->password, sizeof(password), password);
            if(!flipper_format_write_hex(file, "Password", password, sizeof(password))) break;
        }

        saved = true;
    } while(false);

    // Opening truncated the file, so what is on the card now is a header and whatever got
    // written before the card gave up. That loads as a dump, so it cannot be left there.
    if(!saved) {
        FURI_LOG_E(TAG, "Failed to save %s, removing what was written", path);
        flipper_format_file_close(file);
        storage_simply_remove(storage, path);
    }

    furi_string_free(text);
    furi_string_free(key);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return saved;
}

bool t5577_file_load(T5577Dump* dump, const char* path) {
    furi_check(dump);
    furi_check(path);

    memset(dump, 0, sizeof(T5577Dump));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* key = furi_string_alloc();
    FuriString* type = furi_string_alloc();
    bool loaded = false;

    do {
        if(!flipper_format_file_open_existing(file, path)) break;

        uint32_t version = 0;
        if(!flipper_format_read_header(file, type, &version)) break;
        if(furi_string_cmp_str(type, T5577_FILE_TYPE) != 0) {
            FURI_LOG_E(TAG, "%s is not a T5577 dump", path);
            break;
        }
        if(version != T5577_FILE_VERSION) {
            FURI_LOG_E(TAG, "%s is version %lu, not %u", path, version, T5577_FILE_VERSION);
            break;
        }

        // Every key is optional and looked for from the top, so a hand-edited file with keys
        // left out or moved around still reads.
        for(uint8_t page = 0; page <= 1; page++) {
            flipper_format_rewind(file);
            if(flipper_format_read_string(file, t5577_file_read_key(page), type)) {
                t5577_file_parse_read_list(dump, page, type);
            }

            for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
                uint8_t word[4] = {0};
                t5577_file_block_key(key, page, block);

                flipper_format_rewind(file);
                if(!flipper_format_read_hex(file, furi_string_get_cstr(key), word, sizeof(word))) {
                    // A block the file leaves out holds nothing and was not read, whatever the
                    // read list claims.
                    dump->read[page] &= (uint8_t) ~(1u << block);
                    continue;
                }

                dump->block[page][block] = bit_lib_bytes_to_num_be(word, sizeof(word));
            }
        }

        uint8_t password[4] = {0};
        flipper_format_rewind(file);
        if(flipper_format_read_hex(file, "Password", password, sizeof(password))) {
            dump->password = bit_lib_bytes_to_num_be(password, sizeof(password));
            dump->has_password = true;
        }

        // A file that names no block at all is not a dump of a tag - and it reaches the write
        // path as a restore with nothing to write, which reports success for touching nothing.
        if(dump->read[0] == 0 && dump->read[1] == 0) {
            FURI_LOG_E(TAG, "%s holds no blocks that were read", path);
            break;
        }

        loaded = true;
    } while(false);

    // Half of a dump is worse than none: every block it did not reach reads as not read,
    // which a restore would skip, but the password and the blocks it did reach would be
    // taken for the tag's.
    if(!loaded) memset(dump, 0, sizeof(T5577Dump));

    furi_string_free(type);
    furi_string_free(key);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return loaded;
}
