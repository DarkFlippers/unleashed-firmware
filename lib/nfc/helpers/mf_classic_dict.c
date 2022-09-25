#include "mf_classic_dict.h"

#include <lib/toolbox/args.h>
#include <lib/flipper_format/flipper_format.h>

#define MF_CLASSIC_DICT_FLIPPER_PATH EXT_PATH("nfc/assets/mf_classic_dict.nfc")
#define MF_CLASSIC_DICT_USER_PATH EXT_PATH("nfc/assets/mf_classic_dict_user.nfc")
#define MF_CLASSIC_DICT_UNIT_TEST_PATH EXT_PATH("unit_tests/mf_classic_dict.nfc")

#define TAG "MfClassicDict"

#define NFC_MF_CLASSIC_KEY_LEN (13)

struct MfClassicDict {
    Stream* stream;
    uint32_t total_keys;
};

bool mf_classic_dict_check_presence(MfClassicDictType dict_type) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool dict_present = false;
    if(dict_type == MfClassicDictTypeFlipper) {
        dict_present = storage_common_stat(storage, MF_CLASSIC_DICT_FLIPPER_PATH, NULL) == FSE_OK;
    } else if(dict_type == MfClassicDictTypeUser) {
        dict_present = storage_common_stat(storage, MF_CLASSIC_DICT_USER_PATH, NULL) == FSE_OK;
    } else if(dict_type == MfClassicDictTypeUnitTest) {
        dict_present = storage_common_stat(storage, MF_CLASSIC_DICT_UNIT_TEST_PATH, NULL) ==
                       FSE_OK;
    }

    furi_record_close(RECORD_STORAGE);

    return dict_present;
}

MfClassicDict* mf_classic_dict_alloc(MfClassicDictType dict_type) {
    MfClassicDict* dict = malloc(sizeof(MfClassicDict));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    dict->stream = buffered_file_stream_alloc(storage);
    furi_record_close(RECORD_STORAGE);

    bool dict_loaded = false;
    do {
        if(dict_type == MfClassicDictTypeFlipper) {
            if(!buffered_file_stream_open(
                   dict->stream, MF_CLASSIC_DICT_FLIPPER_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
                buffered_file_stream_close(dict->stream);
                break;
            }
        } else if(dict_type == MfClassicDictTypeUser) {
            if(!buffered_file_stream_open(
                   dict->stream, MF_CLASSIC_DICT_USER_PATH, FSAM_READ_WRITE, FSOM_OPEN_ALWAYS)) {
                buffered_file_stream_close(dict->stream);
                break;
            }
        } else if(dict_type == MfClassicDictTypeUnitTest) {
            if(!buffered_file_stream_open(
                   dict->stream,
                   MF_CLASSIC_DICT_UNIT_TEST_PATH,
                   FSAM_READ_WRITE,
                   FSOM_CREATE_ALWAYS)) {
                buffered_file_stream_close(dict->stream);
                break;
            }
        }

        // Read total amount of keys
        string_t next_line;
        string_init(next_line);
        while(true) {
            if(!stream_read_line(dict->stream, next_line)) break;
            if(string_get_char(next_line, 0) == '#') continue;
            if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
            dict->total_keys++;
        }
        string_clear(next_line);
        stream_rewind(dict->stream);

        dict_loaded = true;
        FURI_LOG_I(TAG, "Loaded dictionary with %d keys", dict->total_keys);
    } while(false);

    if(!dict_loaded) {
        buffered_file_stream_close(dict->stream);
        free(dict);
        dict = NULL;
    }

    return dict;
}

void mf_classic_dict_free(MfClassicDict* dict) {
    furi_assert(dict);
    furi_assert(dict->stream);

    buffered_file_stream_close(dict->stream);
    stream_free(dict->stream);
    free(dict);
}

static void mf_classic_dict_int_to_str(uint8_t* key_int, string_t key_str) {
    string_reset(key_str);
    for(size_t i = 0; i < 6; i++) {
        string_cat_printf(key_str, "%02X", key_int[i]);
    }
}

static void mf_classic_dict_str_to_int(string_t key_str, uint64_t* key_int) {
    uint8_t key_byte_tmp;

    *key_int = 0ULL;
    for(uint8_t i = 0; i < 12; i += 2) {
        args_char_to_hex(
            string_get_char(key_str, i), string_get_char(key_str, i + 1), &key_byte_tmp);
        *key_int |= (uint64_t)key_byte_tmp << 8 * (5 - i / 2);
    }
}

uint32_t mf_classic_dict_get_total_keys(MfClassicDict* dict) {
    furi_assert(dict);

    return dict->total_keys;
}

bool mf_classic_dict_rewind(MfClassicDict* dict) {
    furi_assert(dict);
    furi_assert(dict->stream);

    return stream_rewind(dict->stream);
}

bool mf_classic_dict_get_next_key_str(MfClassicDict* dict, string_t key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    bool key_read = false;
    string_reset(key);
    while(!key_read) {
        if(!stream_read_line(dict->stream, key)) break;
        if(string_get_char(key, 0) == '#') continue;
        if(string_size(key) != NFC_MF_CLASSIC_KEY_LEN) continue;
        string_left(key, 12);
        key_read = true;
    }

    return key_read;
}

bool mf_classic_dict_get_next_key(MfClassicDict* dict, uint64_t* key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t temp_key;
    string_init(temp_key);
    bool key_read = mf_classic_dict_get_next_key_str(dict, temp_key);
    if(key_read) {
        mf_classic_dict_str_to_int(temp_key, key);
    }
    string_clear(temp_key);
    return key_read;
}

bool mf_classic_dict_is_key_present_str(MfClassicDict* dict, string_t key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t next_line;
    string_init(next_line);

    bool key_found = false;
    stream_rewind(dict->stream);
    while(!key_found) {
        if(!stream_read_line(dict->stream, next_line)) break;
        if(string_get_char(next_line, 0) == '#') continue;
        if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
        string_left(next_line, 12);
        if(!string_equal_p(key, next_line)) continue;
        key_found = true;
    }

    string_clear(next_line);
    return key_found;
}

bool mf_classic_dict_is_key_present(MfClassicDict* dict, uint8_t* key) {
    string_t temp_key;

    string_init(temp_key);
    mf_classic_dict_int_to_str(key, temp_key);
    bool key_found = mf_classic_dict_is_key_present_str(dict, temp_key);
    string_clear(temp_key);
    return key_found;
}

bool mf_classic_dict_add_key_str(MfClassicDict* dict, string_t key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_cat_printf(key, "\n");

    bool key_added = false;
    do {
        if(!stream_seek(dict->stream, 0, StreamOffsetFromEnd)) break;
        if(!stream_insert_string(dict->stream, key)) break;
        dict->total_keys++;
        key_added = true;
    } while(false);

    string_left(key, 12);
    return key_added;
}

bool mf_classic_dict_add_key(MfClassicDict* dict, uint8_t* key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t temp_key;
    string_init(temp_key);
    mf_classic_dict_int_to_str(key, temp_key);
    bool key_added = mf_classic_dict_add_key_str(dict, temp_key);

    string_clear(temp_key);
    return key_added;
}

bool mf_classic_dict_get_key_at_index_str(MfClassicDict* dict, string_t key, uint32_t target) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t next_line;
    uint32_t index = 0;
    string_init(next_line);
    string_reset(key);

    bool key_found = false;
    while(!key_found) {
        if(!stream_read_line(dict->stream, next_line)) break;
        if(string_get_char(next_line, 0) == '#') continue;
        if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
        if(index++ != target) continue;
        string_set_n(key, next_line, 0, 12);
        key_found = true;
    }

    string_clear(next_line);
    return key_found;
}

bool mf_classic_dict_get_key_at_index(MfClassicDict* dict, uint64_t* key, uint32_t target) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t temp_key;
    string_init(temp_key);
    bool key_found = mf_classic_dict_get_key_at_index_str(dict, temp_key, target);
    if(key_found) {
        mf_classic_dict_str_to_int(temp_key, key);
    }
    string_clear(temp_key);
    return key_found;
}

bool mf_classic_dict_find_index_str(MfClassicDict* dict, string_t key, uint32_t* target) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t next_line;
    string_init(next_line);

    bool key_found = false;
    uint32_t index = 0;
    stream_rewind(dict->stream);
    while(!key_found) {
        if(!stream_read_line(dict->stream, next_line)) break;
        if(string_get_char(next_line, 0) == '#') continue;
        if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
        string_left(next_line, 12);
        if(!string_equal_p(key, next_line)) continue;
        key_found = true;
        *target = index;
    }

    string_clear(next_line);
    return key_found;
}

bool mf_classic_dict_find_index(MfClassicDict* dict, uint8_t* key, uint32_t* target) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t temp_key;
    string_init(temp_key);
    mf_classic_dict_int_to_str(key, temp_key);
    bool key_found = mf_classic_dict_find_index_str(dict, temp_key, target);

    string_clear(temp_key);
    return key_found;
}

bool mf_classic_dict_delete_index(MfClassicDict* dict, uint32_t target) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t next_line;
    string_init(next_line);
    uint32_t index = 0;

    bool key_removed = false;
    while(!key_removed) {
        if(!stream_read_line(dict->stream, next_line)) break;
        if(string_get_char(next_line, 0) == '#') continue;
        if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
        if(index++ != target) continue;
        stream_seek(dict->stream, -NFC_MF_CLASSIC_KEY_LEN, StreamOffsetFromCurrent);
        if(!stream_delete(dict->stream, NFC_MF_CLASSIC_KEY_LEN)) break;
        dict->total_keys--;
        key_removed = true;
    }

    string_clear(next_line);
    return key_removed;
}
