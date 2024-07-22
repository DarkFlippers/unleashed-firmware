#include "lfrfid_dict_file.h"
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <bit_lib/bit_lib.h>

#define LFRFID_DICT_FILETYPE "Flipper RFID key"

bool lfrfid_dict_file_save(ProtocolDict* dict, ProtocolId protocol, const char* filename) {
    furi_check(dict);
    furi_check(protocol != PROTOCOL_NO);
    furi_check(filename);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    size_t data_size = protocol_dict_get_data_size(dict, protocol);
    uint8_t* data = malloc(data_size);
    bool result = false;

    do {
        if(!flipper_format_file_open_always(file, filename)) break;
        if(!flipper_format_write_header_cstr(file, LFRFID_DICT_FILETYPE, 1)) break;

        // TODO FL-3517: write comment about protocol types into file

        if(!flipper_format_write_string_cstr(
               file, "Key type", protocol_dict_get_name(dict, protocol)))
            break;

        // TODO FL-3517: write comment about protocol sizes into file

        protocol_dict_get_data(dict, protocol, data, data_size);

        if(!flipper_format_write_hex(file, "Data", data, data_size)) break;
        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    free(data);

    return result;
}

static void lfrfid_dict_protocol_indala_data(
    uint8_t* data,
    size_t data_size,
    uint8_t* protocol_data,
    size_t protocol_data_size) {
    UNUSED(data_size);
    memset(protocol_data, 0, protocol_data_size);

    // fc
    bit_lib_set_bit(protocol_data, 24, bit_lib_get_bit(data, 0));
    bit_lib_set_bit(protocol_data, 16, bit_lib_get_bit(data, 1));
    bit_lib_set_bit(protocol_data, 11, bit_lib_get_bit(data, 2));
    bit_lib_set_bit(protocol_data, 14, bit_lib_get_bit(data, 3));
    bit_lib_set_bit(protocol_data, 15, bit_lib_get_bit(data, 4));
    bit_lib_set_bit(protocol_data, 20, bit_lib_get_bit(data, 5));
    bit_lib_set_bit(protocol_data, 6, bit_lib_get_bit(data, 6));
    bit_lib_set_bit(protocol_data, 25, bit_lib_get_bit(data, 7));

    // cn
    bit_lib_set_bit(protocol_data, 9, bit_lib_get_bit(data, 8 + 0));
    bit_lib_set_bit(protocol_data, 12, bit_lib_get_bit(data, 8 + 1));
    bit_lib_set_bit(protocol_data, 10, bit_lib_get_bit(data, 8 + 2));
    bit_lib_set_bit(protocol_data, 7, bit_lib_get_bit(data, 8 + 3));
    bit_lib_set_bit(protocol_data, 19, bit_lib_get_bit(data, 8 + 4));
    bit_lib_set_bit(protocol_data, 3, bit_lib_get_bit(data, 8 + 5));
    bit_lib_set_bit(protocol_data, 2, bit_lib_get_bit(data, 8 + 6));
    bit_lib_set_bit(protocol_data, 18, bit_lib_get_bit(data, 8 + 7));
    bit_lib_set_bit(protocol_data, 13, bit_lib_get_bit(data, 8 + 8));
    bit_lib_set_bit(protocol_data, 0, bit_lib_get_bit(data, 8 + 9));
    bit_lib_set_bit(protocol_data, 4, bit_lib_get_bit(data, 8 + 10));
    bit_lib_set_bit(protocol_data, 21, bit_lib_get_bit(data, 8 + 11));
    bit_lib_set_bit(protocol_data, 23, bit_lib_get_bit(data, 8 + 12));
    bit_lib_set_bit(protocol_data, 26, bit_lib_get_bit(data, 8 + 13));
    bit_lib_set_bit(protocol_data, 17, bit_lib_get_bit(data, 8 + 14));
    bit_lib_set_bit(protocol_data, 8, bit_lib_get_bit(data, 8 + 15));

    const uint32_t fc_and_card = data[0] << 16 | data[1] << 8 | data[2];

    // indala checksum
    uint8_t checksum_sum = 0;
    checksum_sum += ((fc_and_card >> 14) & 1);
    checksum_sum += ((fc_and_card >> 12) & 1);
    checksum_sum += ((fc_and_card >> 9) & 1);
    checksum_sum += ((fc_and_card >> 8) & 1);
    checksum_sum += ((fc_and_card >> 6) & 1);
    checksum_sum += ((fc_and_card >> 5) & 1);
    checksum_sum += ((fc_and_card >> 2) & 1);
    checksum_sum += ((fc_and_card >> 0) & 1);
    checksum_sum = checksum_sum & 0b1;

    if(checksum_sum) {
        bit_lib_set_bit(protocol_data, 27, 0);
        bit_lib_set_bit(protocol_data, 28, 1);
    } else {
        bit_lib_set_bit(protocol_data, 27, 1);
        bit_lib_set_bit(protocol_data, 28, 0);
    }

    // wiegand parity
    uint8_t even_parity_sum = 0;
    for(int8_t i = 12; i < 24; i++) {
        if(((fc_and_card >> i) & 1) == 1) {
            even_parity_sum++;
        }
    }
    bit_lib_set_bit(protocol_data, 1, even_parity_sum % 2);

    uint8_t odd_parity_sum = 1;
    for(int8_t i = 0; i < 12; i++) {
        if(((fc_and_card >> i) & 1) == 1) {
            odd_parity_sum++;
        }
    }
    bit_lib_set_bit(protocol_data, 5, odd_parity_sum % 2);
}

static ProtocolId lfrfid_dict_protocol_fallback(
    ProtocolDict* dict,
    const char* protocol_name,
    FlipperFormat* file) {
    ProtocolId result = PROTOCOL_NO;
    if(strcmp(protocol_name, "I40134") == 0) {
        ProtocolId protocol = LFRFIDProtocolIndala26;

        size_t data_size = 3;
        size_t protocol_data_size = protocol_dict_get_data_size(dict, protocol);
        uint8_t* data = malloc(data_size);
        uint8_t* protocol_data = malloc(protocol_data_size);
        if(flipper_format_read_hex(file, "Data", data, data_size)) {
            lfrfid_dict_protocol_indala_data(data, data_size, protocol_data, protocol_data_size);
            protocol_dict_set_data(dict, protocol, protocol_data, protocol_data_size);
            result = protocol;
        }
        free(protocol_data);
        free(data);
    }

    return result;
}

ProtocolId lfrfid_dict_file_load(ProtocolDict* dict, const char* filename) {
    furi_check(dict);
    furi_check(filename);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    ProtocolId result = PROTOCOL_NO;
    uint8_t* data = malloc(protocol_dict_get_max_data_size(dict));
    FuriString* str_result;
    str_result = furi_string_alloc();

    do {
        if(!flipper_format_file_open_existing(file, filename)) break;

        // header
        uint32_t version;
        if(!flipper_format_read_header(file, str_result, &version)) break;
        if(furi_string_cmp_str(str_result, LFRFID_DICT_FILETYPE) != 0) break;
        if(version != 1) break;

        // type
        if(!flipper_format_read_string(file, "Key type", str_result)) break;
        ProtocolId protocol;
        protocol = protocol_dict_get_protocol_by_name(dict, furi_string_get_cstr(str_result));

        if(protocol == PROTOCOL_NO) {
            protocol = lfrfid_dict_protocol_fallback(dict, furi_string_get_cstr(str_result), file);
            if(protocol == PROTOCOL_NO) break;
        } else {
            // data
            size_t data_size = protocol_dict_get_data_size(dict, protocol);
            if(!flipper_format_read_hex(file, "Data", data, data_size)) break;
            protocol_dict_set_data(dict, protocol, data, data_size);
        }

        result = protocol;
    } while(false);

    free(data);
    furi_string_free(str_result);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}
