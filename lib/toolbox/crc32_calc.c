#include "crc32_calc.h"
#include <littlefs/lfs_util.h>

#define CRC_DATA_BUFFER_MAX_LEN 512

uint32_t crc32_calc_buffer(uint32_t crc, const void* buffer, size_t size) {
    // TODO FL-3547: consider removing dependency on LFS
    return ~lfs_crc(~crc, buffer, size);
}

uint32_t crc32_calc_file(File* file, const FileCrcProgressCb progress_cb, void* context) {
    furi_check(storage_file_is_open(file) && storage_file_seek(file, 0, true));

    uint32_t file_crc = 0;

    uint8_t* data_buffer = malloc(CRC_DATA_BUFFER_MAX_LEN);
    uint16_t data_buffer_valid_len;

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
