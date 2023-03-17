#include "iclass_elite_dict.h"

#include <lib/toolbox/args.h>
#include <lib/flipper_format/flipper_format.h>

#define ICLASS_ELITE_DICT_FLIPPER_NAME APP_DATA_PATH("assets/iclass_elite_dict.txt")
#define ICLASS_ELITE_DICT_USER_NAME APP_DATA_PATH("assets/iclass_elite_dict_user.txt")
#define ICLASS_STANDARD_DICT_FLIPPER_NAME APP_DATA_PATH("assets/iclass_standard_dict.txt")

#define TAG "IclassEliteDict"

#define ICLASS_ELITE_KEY_LINE_LEN (17)
#define ICLASS_ELITE_KEY_LEN (8)

struct IclassEliteDict {
    Stream* stream;
    uint32_t total_keys;
};

bool iclass_elite_dict_check_presence(IclassEliteDictType dict_type) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool dict_present = false;
    if(dict_type == IclassEliteDictTypeFlipper) {
        dict_present =
            (storage_common_stat(storage, ICLASS_ELITE_DICT_FLIPPER_NAME, NULL) == FSE_OK);
    } else if(dict_type == IclassEliteDictTypeUser) {
        dict_present = (storage_common_stat(storage, ICLASS_ELITE_DICT_USER_NAME, NULL) == FSE_OK);
    } else if(dict_type == IclassStandardDictTypeFlipper) {
        dict_present =
            (storage_common_stat(storage, ICLASS_STANDARD_DICT_FLIPPER_NAME, NULL) == FSE_OK);
    }

    furi_record_close(RECORD_STORAGE);

    return dict_present;
}

IclassEliteDict* iclass_elite_dict_alloc(IclassEliteDictType dict_type) {
    IclassEliteDict* dict = malloc(sizeof(IclassEliteDict));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    dict->stream = buffered_file_stream_alloc(storage);
    FuriString* next_line = furi_string_alloc();

    bool dict_loaded = false;
    do {
        if(dict_type == IclassEliteDictTypeFlipper) {
            if(!buffered_file_stream_open(
                   dict->stream, ICLASS_ELITE_DICT_FLIPPER_NAME, FSAM_READ, FSOM_OPEN_EXISTING)) {
                buffered_file_stream_close(dict->stream);
                break;
            }
        } else if(dict_type == IclassEliteDictTypeUser) {
            if(!buffered_file_stream_open(
                   dict->stream, ICLASS_ELITE_DICT_USER_NAME, FSAM_READ_WRITE, FSOM_OPEN_ALWAYS)) {
                buffered_file_stream_close(dict->stream);
                break;
            }
        } else if(dict_type == IclassStandardDictTypeFlipper) {
            if(!buffered_file_stream_open(
                   dict->stream,
                   ICLASS_STANDARD_DICT_FLIPPER_NAME,
                   FSAM_READ,
                   FSOM_OPEN_EXISTING)) {
                buffered_file_stream_close(dict->stream);
                break;
            }
        }

        // Read total amount of keys
        while(true) { //-V547
            if(!stream_read_line(dict->stream, next_line)) break;
            if(furi_string_get_char(next_line, 0) == '#') continue;
            if(furi_string_size(next_line) != ICLASS_ELITE_KEY_LINE_LEN) continue;
            dict->total_keys++;
        }
        furi_string_reset(next_line);
        stream_rewind(dict->stream);

        dict_loaded = true;
        FURI_LOG_I(TAG, "Loaded dictionary with %lu keys", dict->total_keys);
    } while(false);

    if(!dict_loaded) { //-V547
        buffered_file_stream_close(dict->stream);
        free(dict);
        dict = NULL;
    }

    furi_record_close(RECORD_STORAGE);
    furi_string_free(next_line);

    return dict;
}

void iclass_elite_dict_free(IclassEliteDict* dict) {
    furi_assert(dict);
    furi_assert(dict->stream);

    buffered_file_stream_close(dict->stream);
    stream_free(dict->stream);
    free(dict);
}

uint32_t iclass_elite_dict_get_total_keys(IclassEliteDict* dict) {
    furi_assert(dict);

    return dict->total_keys;
}

bool iclass_elite_dict_get_next_key(IclassEliteDict* dict, uint8_t* key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    uint8_t key_byte_tmp = 0;
    FuriString* next_line = furi_string_alloc();

    bool key_read = false;
    *key = 0ULL;
    while(!key_read) {
        if(!stream_read_line(dict->stream, next_line)) break;
        if(furi_string_get_char(next_line, 0) == '#') continue;
        if(furi_string_size(next_line) != ICLASS_ELITE_KEY_LINE_LEN) continue;
        for(uint8_t i = 0; i < ICLASS_ELITE_KEY_LEN * 2; i += 2) {
            args_char_to_hex(
                furi_string_get_char(next_line, i),
                furi_string_get_char(next_line, i + 1),
                &key_byte_tmp);
            key[i / 2] = key_byte_tmp;
        }
        key_read = true;
    }

    furi_string_free(next_line);
    return key_read;
}

bool iclass_elite_dict_rewind(IclassEliteDict* dict) {
    furi_assert(dict);
    furi_assert(dict->stream);

    return stream_rewind(dict->stream);
}

bool iclass_elite_dict_add_key(IclassEliteDict* dict, uint8_t* key) {
    furi_assert(dict);
    furi_assert(dict->stream);

    FuriString* key_str = furi_string_alloc();
    for(size_t i = 0; i < 6; i++) {
        furi_string_cat_printf(key_str, "%02X", key[i]);
    }
    furi_string_cat_printf(key_str, "\n");

    bool key_added = false;
    do {
        if(!stream_seek(dict->stream, 0, StreamOffsetFromEnd)) break;
        if(!stream_insert_string(dict->stream, key_str)) break;
        key_added = true;
    } while(false);

    furi_string_free(key_str);
    return key_added;
}
