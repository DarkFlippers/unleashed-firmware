#include "md5.h"
#include "md5_calc.h"

bool md5_calc_file(File* file, const char* path, unsigned char output[16], FS_Error* file_error) {
    bool result = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);

    if(result) {
        const uint16_t size_to_read = 512;
        uint8_t* data = malloc(size_to_read);
        md5_context* md5_ctx = malloc(sizeof(md5_context));

        md5_starts(md5_ctx);
        while(true) {
            uint16_t read_size = storage_file_read(file, data, size_to_read);
            if(read_size == 0) break;
            md5_update(md5_ctx, data, read_size);
        }
        md5_finish(md5_ctx, output);
        free(md5_ctx);
        free(data);
    }

    if(file_error != NULL) {
        *file_error = storage_file_get_error(file);
    }

    storage_file_close(file);
    return result;
}

bool md5_string_calc_file(File* file, const char* path, FuriString* output, FS_Error* file_error) {
    const size_t hash_size = 16;
    unsigned char hash[hash_size];
    bool result = md5_calc_file(file, path, hash, file_error);

    if(result) {
        furi_string_set(output, "");
        for(size_t i = 0; i < hash_size; i++) {
            furi_string_cat_printf(output, "%02x", hash[i]);
        }
    }

    return result;
}