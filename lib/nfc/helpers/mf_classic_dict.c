#include "mf_classic_dict.h"

#include <lib/toolbox/args.h>
#include <lib/flipper_format/flipper_format.h>

#define MF_CLASSIC_DICT_FLIPPER_PATH EXT_PATH("nfc/assets/mf_classic_dict.nfc")
#define MF_CLASSIC_DICT_USER_PATH EXT_PATH("nfc/assets/mf_classic_dict_user.nfc")

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

uint32_t mf_classic_dict_get_total_keys(MfClassicDict* dict) {
    furi_assert(dict);

    return dict->total_keys;
}

bool mf_classic_dict_get_next_key(MfClassicDict* dict, uint64_t* key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    uint8_t key_byte_tmp = 0;
    string_t next_line;
    string_init(next_line);

    bool key_read = false;
    *key = 0ULL;
    while(!key_read) {
        if(!stream_read_line(dict->stream, next_line)) break;
        if(string_get_char(next_line, 0) == '#') continue;
        if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
        for(uint8_t i = 0; i < 12; i += 2) {
            args_char_to_hex(
                string_get_char(next_line, i), string_get_char(next_line, i + 1), &key_byte_tmp);
            *key |= (uint64_t)key_byte_tmp << 8 * (5 - i / 2);
        }
        key_read = true;
    }

    string_clear(next_line);
    return key_read;
}

bool mf_classic_dict_rewind(MfClassicDict* dict) {
    furi_assert(dict);
    furi_assert(dict->stream);

    return stream_rewind(dict->stream);
}

bool mf_classic_dict_add_key(MfClassicDict* dict, uint8_t* key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    string_t key_str;
    string_init(key_str);
    for(size_t i = 0; i < 6; i++) {
        string_cat_printf(key_str, "%02X", key[i]);
    }
    string_cat_printf(key_str, "\n");

    bool key_added = false;
    do {
        if(!stream_seek(dict->stream, 0, StreamOffsetFromEnd)) break;
        if(!stream_insert_string(dict->stream, key_str)) break;
        key_added = true;
    } while(false);

    string_clear(key_str);
    return key_added;
}
