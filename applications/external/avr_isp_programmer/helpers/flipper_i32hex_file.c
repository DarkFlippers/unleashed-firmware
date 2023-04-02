#include "flipper_i32hex_file.h"
#include <string.h>
#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/hex.h>

//https://en.wikipedia.org/wiki/Intel_HEX

#define TAG "FlipperI32HexFile"

#define COUNT_BYTE_PAYLOAD 32 //how much payload will be used

#define I32HEX_TYPE_DATA 0x00
#define I32HEX_TYPE_END_OF_FILE 0x01
#define I32HEX_TYPE_EXT_LINEAR_ADDR 0x04
#define I32HEX_TYPE_START_LINEAR_ADDR 0x05

struct FlipperI32HexFile {
    uint32_t addr;
    uint32_t addr_last;
    Storage* storage;
    Stream* stream;
    FuriString* str_data;
    FlipperI32HexFileStatus file_open;
};

FlipperI32HexFile* flipper_i32hex_file_open_write(const char* name, uint32_t start_addr) {
    furi_assert(name);

    FlipperI32HexFile* instance = malloc(sizeof(FlipperI32HexFile));
    instance->addr = start_addr;
    instance->addr_last = 0;
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->stream = file_stream_alloc(instance->storage);

    if(file_stream_open(instance->stream, name, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        instance->file_open = FlipperI32HexFileStatusOpenFileWrite;
        FURI_LOG_D(TAG, "Open write file %s", name);
    } else {
        FURI_LOG_E(TAG, "Failed to open file %s", name);
        instance->file_open = FlipperI32HexFileStatusErrorNoOpenFile;
    }
    instance->str_data = furi_string_alloc(instance->storage);

    return instance;
}

FlipperI32HexFile* flipper_i32hex_file_open_read(const char* name) {
    furi_assert(name);

    FlipperI32HexFile* instance = malloc(sizeof(FlipperI32HexFile));
    instance->addr = 0;
    instance->addr_last = 0;
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->stream = file_stream_alloc(instance->storage);

    if(file_stream_open(instance->stream, name, FSAM_READ, FSOM_OPEN_EXISTING)) {
        instance->file_open = FlipperI32HexFileStatusOpenFileRead;
        FURI_LOG_D(TAG, "Open read file %s", name);
    } else {
        FURI_LOG_E(TAG, "Failed to open file %s", name);
        instance->file_open = FlipperI32HexFileStatusErrorNoOpenFile;
    }
    instance->str_data = furi_string_alloc(instance->storage);

    return instance;
}

void flipper_i32hex_file_close(FlipperI32HexFile* instance) {
    furi_assert(instance);

    furi_string_free(instance->str_data);
    file_stream_close(instance->stream);
    stream_free(instance->stream);
    furi_record_close(RECORD_STORAGE);
}

FlipperI32HexFileRet flipper_i32hex_file_bin_to_i32hex_set_data(
    FlipperI32HexFile* instance,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);
    furi_assert(data);

    FlipperI32HexFileRet ret = {.status = FlipperI32HexFileStatusOK, .data_size = 0};
    if(instance->file_open != FlipperI32HexFileStatusOpenFileWrite) {
        ret.status = FlipperI32HexFileStatusErrorFileWrite;
    }
    uint8_t count_byte = 0;
    uint32_t ind = 0;
    uint8_t crc = 0;

    furi_string_reset(instance->str_data);

    if((instance->addr_last & 0xFF0000) < (instance->addr & 0xFF0000)) {
        crc = 0x02 + 0x04 + ((instance->addr >> 24) & 0xFF) + ((instance->addr >> 16) & 0xFF);
        crc = 0x01 + ~crc;
        //I32HEX_TYPE_EXT_LINEAR_ADDR
        furi_string_cat_printf(
            instance->str_data, ":02000004%04lX%02X\r\n", (instance->addr >> 16), crc);
        instance->addr_last = instance->addr;
    }

    while(ind < data_size) {
        if((ind + COUNT_BYTE_PAYLOAD) > data_size) {
            count_byte = data_size - ind;
        } else {
            count_byte = COUNT_BYTE_PAYLOAD;
        }
        //I32HEX_TYPE_DATA
        furi_string_cat_printf(
            instance->str_data, ":%02X%04lX00", count_byte, (instance->addr & 0xFFFF));
        crc = count_byte + ((instance->addr >> 8) & 0xFF) + (instance->addr & 0xFF);

        for(uint32_t i = 0; i < count_byte; i++) {
            furi_string_cat_printf(instance->str_data, "%02X", *data);
            crc += *data++;
        }
        crc = 0x01 + ~crc;
        furi_string_cat_printf(instance->str_data, "%02X\r\n", crc);

        ind += count_byte;
        instance->addr += count_byte;
    }
    if(instance->file_open) stream_write_string(instance->stream, instance->str_data);
    return ret;
}

FlipperI32HexFileRet flipper_i32hex_file_bin_to_i32hex_set_end_line(FlipperI32HexFile* instance) {
    furi_assert(instance);

    FlipperI32HexFileRet ret = {.status = FlipperI32HexFileStatusOK, .data_size = 0};
    if(instance->file_open != FlipperI32HexFileStatusOpenFileWrite) {
        ret.status = FlipperI32HexFileStatusErrorFileWrite;
    }
    furi_string_reset(instance->str_data);
    //I32HEX_TYPE_END_OF_FILE
    furi_string_cat_printf(instance->str_data, ":00000001FF\r\n");
    if(instance->file_open) stream_write_string(instance->stream, instance->str_data);
    return ret;
}

void flipper_i32hex_file_bin_to_i32hex_set_addr(FlipperI32HexFile* instance, uint32_t addr) {
    furi_assert(instance);

    instance->addr = addr;
}

const char* flipper_i32hex_file_get_string(FlipperI32HexFile* instance) {
    furi_assert(instance);

    return furi_string_get_cstr(instance->str_data);
}

static FlipperI32HexFileRet flipper_i32hex_file_parse_line(
    FlipperI32HexFile* instance,
    const char* str,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);
    furi_assert(data);

    char* str1;
    uint32_t data_wrire_ind = 0;
    uint32_t data_len = 0;
    FlipperI32HexFileRet ret = {.status = FlipperI32HexFileStatusErrorData, .data_size = 0};

    //Search for start of data I32HEX
    str1 = strstr(str, ":");
    do {
        if(str1 == NULL) {
            ret.status = FlipperI32HexFileStatusErrorData;
            break;
        }
        str1++;
        if(!hex_char_to_uint8(*str1, str1[1], data + data_wrire_ind)) {
            ret.status = FlipperI32HexFileStatusErrorData;
            break;
        }
        str1++;
        if(++data_wrire_ind > data_size) {
            ret.status = FlipperI32HexFileStatusErrorOverflow;
            break;
        }
        data_len = 5 + data[0]; // +5 bytes per header and crc
        while(data_len > data_wrire_ind) {
            str1++;
            if(!hex_char_to_uint8(*str1, str1[1], data + data_wrire_ind)) {
                ret.status = FlipperI32HexFileStatusErrorData;
                break;
            }
            str1++;
            if(++data_wrire_ind > data_size) {
                ret.status = FlipperI32HexFileStatusErrorOverflow;
                break;
            }
        }
        ret.status = FlipperI32HexFileStatusOK;
        ret.data_size = data_wrire_ind;

    } while(0);
    return ret;
}

static bool flipper_i32hex_file_check_data(uint8_t* data, uint32_t data_size) {
    furi_assert(data);

    uint8_t crc = 0;
    uint32_t data_read_ind = 0;
    if(data[0] > data_size) return false;
    while(data_read_ind < data_size - 1) {
        crc += data[data_read_ind++];
    }
    return data[data_size - 1] == ((1 + ~crc) & 0xFF);
}

static FlipperI32HexFileRet flipper_i32hex_file_parse(
    FlipperI32HexFile* instance,
    const char* str,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);
    furi_assert(data);

    FlipperI32HexFileRet ret = flipper_i32hex_file_parse_line(instance, str, data, data_size);

    if((ret.status == FlipperI32HexFileStatusOK) && (ret.data_size > 4)) {
        switch(data[3]) {
        case I32HEX_TYPE_DATA:
            if(flipper_i32hex_file_check_data(data, ret.data_size)) {
                ret.data_size -= 5;
                memcpy(data, data + 4, ret.data_size);
                ret.status = FlipperI32HexFileStatusData;
            } else {
                ret.status = FlipperI32HexFileStatusErrorCrc;
                ret.data_size = 0;
            }
            break;
        case I32HEX_TYPE_END_OF_FILE:
            if(flipper_i32hex_file_check_data(data, ret.data_size)) {
                ret.status = FlipperI32HexFileStatusEofFile;
                ret.data_size = 0;
            } else {
                ret.status = FlipperI32HexFileStatusErrorCrc;
                ret.data_size = 0;
            }
            break;
        case I32HEX_TYPE_EXT_LINEAR_ADDR:
            if(flipper_i32hex_file_check_data(data, ret.data_size)) {
                data[0] = data[4];
                data[1] = data[5];
                data[3] = 0;
                data[4] = 0;
                ret.status = FlipperI32HexFileStatusUdateAddr;
                ret.data_size = 4;
            } else {
                ret.status = FlipperI32HexFileStatusErrorCrc;
                ret.data_size = 0;
            }
            break;
        case I32HEX_TYPE_START_LINEAR_ADDR:
            ret.status = FlipperI32HexFileStatusErrorUnsupportedCommand;
            ret.data_size = 0;
            break;
        default:
            ret.status = FlipperI32HexFileStatusErrorUnsupportedCommand;
            ret.data_size = 0;
            break;
        }
    } else {
        ret.status = FlipperI32HexFileStatusErrorData;
        ret.data_size = 0;
    }
    return ret;
}

bool flipper_i32hex_file_check(FlipperI32HexFile* instance) {
    furi_assert(instance);

    uint32_t data_size = 280;
    uint8_t data[280] = {0};
    bool ret = true;

    if(instance->file_open != FlipperI32HexFileStatusOpenFileRead) {
        FURI_LOG_E(TAG, "File is not open");
        ret = false;
    } else {
        stream_rewind(instance->stream);

        while(stream_read_line(instance->stream, instance->str_data)) {
            FlipperI32HexFileRet parse_ret = flipper_i32hex_file_parse(
                instance, furi_string_get_cstr(instance->str_data), data, data_size);

            if(parse_ret.status < 0) {
                ret = false;
            }
        }
        stream_rewind(instance->stream);
    }
    return ret;
}

FlipperI32HexFileRet flipper_i32hex_file_i32hex_to_bin_get_data(
    FlipperI32HexFile* instance,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);
    furi_assert(data);

    FlipperI32HexFileRet ret = {.status = FlipperI32HexFileStatusOK, .data_size = 0};
    if(instance->file_open != FlipperI32HexFileStatusOpenFileRead) {
        ret.status = FlipperI32HexFileStatusErrorFileRead;
    } else {
        stream_read_line(instance->stream, instance->str_data);
        ret = flipper_i32hex_file_parse(
            instance, furi_string_get_cstr(instance->str_data), data, data_size);
    }

    return ret;
}