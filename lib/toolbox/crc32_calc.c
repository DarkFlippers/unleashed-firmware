#include "crc32_calc.h"

#define CRC_DATA_BUFFER_MAX_LEN 512

uint32_t crc32_calc_buffer(uint32_t crc, const void* buffer, size_t size) {
    crc = ~crc;

    static const uint32_t rtable[16] = {
        0x00000000,
        0x1db71064,
        0x3b6e20c8,
        0x26d930ac,
        0x76dc4190,
        0x6b6b51f4,
        0x4db26158,
        0x5005713c,
        0xedb88320,
        0xf00f9344,
        0xd6d6a3e8,
        0xcb61b38c,
        0x9b64c2b0,
        0x86d3d2d4,
        0xa00ae278,
        0xbdbdf21c,
    };

    const uint8_t* data = buffer;

    for(size_t i = 0; i < size; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
    }

    return ~crc;
}

uint32_t crc32_calc_file(File* file, const FileCrcProgressCb progress_cb, void* context) {
    furi_check(storage_file_is_open(file) && storage_file_seek(file, 0, true));

    uint32_t file_crc = 0;

    uint8_t* data_buffer = malloc(CRC_DATA_BUFFER_MAX_LEN);
    size_t data_buffer_valid_len;

    uint32_t file_size = storage_file_size(file);

    /* Feed file contents per sector into CRC calc */
    for(uint32_t fptr = 0; fptr < file_size;) {
        data_buffer_valid_len = storage_file_read(file, data_buffer, CRC_DATA_BUFFER_MAX_LEN);
        if(data_buffer_valid_len == 0) {
            break;
        }
        fptr += data_buffer_valid_len;

        if(progress_cb && (fptr % CRC_DATA_BUFFER_MAX_LEN == 0)) {
            progress_cb(fptr * 100 / file_size, context);
        }

        file_crc = crc32_calc_buffer(file_crc, data_buffer, data_buffer_valid_len);
    }
    free(data_buffer);

    return file_crc;
}
