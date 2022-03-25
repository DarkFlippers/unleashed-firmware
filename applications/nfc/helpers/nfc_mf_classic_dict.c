#include "nfc_mf_classic_dict.h"

#include <flipper_format/flipper_format.h>
#include <lib/toolbox/args.h>

#define NFC_MF_CLASSIC_DICT_PATH "/ext/nfc/assets/mf_classic_dict.nfc"

#define NFC_MF_CLASSIC_KEY_LEN (13)

bool nfc_mf_classic_dict_check_presence(Storage* storage) {
    furi_assert(storage);
    return storage_common_stat(storage, NFC_MF_CLASSIC_DICT_PATH, NULL) == FSE_OK;
}

bool nfc_mf_classic_dict_open_file(Stream* stream) {
    furi_assert(stream);
    return file_stream_open(stream, NFC_MF_CLASSIC_DICT_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
}

void nfc_mf_classic_dict_close_file(Stream* stream) {
    furi_assert(stream);
    file_stream_close(stream);
}

bool nfc_mf_classic_dict_get_next_key(Stream* stream, uint64_t* key) {
    furi_assert(stream);
    furi_assert(key);
    uint8_t key_byte_tmp = 0;
    string_t next_line;
    string_init(next_line);
    *key = 0;

    bool next_key_read = false;
    while(!next_key_read) {
        if(!stream_read_line(stream, next_line)) break;
        if(string_get_char(next_line, 0) == '#') continue;
        if(string_size(next_line) != NFC_MF_CLASSIC_KEY_LEN) continue;
        for(uint8_t i = 0; i < 12; i += 2) {
            args_char_to_hex(
                string_get_char(next_line, i), string_get_char(next_line, i + 1), &key_byte_tmp);
            *key |= (uint64_t)key_byte_tmp << 8 * (5 - i / 2);
        }
        next_key_read = true;
    }

    string_clear(next_line);
    return next_key_read;
}

void nfc_mf_classic_dict_reset(Stream* stream) {
    furi_assert(stream);
    stream_rewind(stream);
}
