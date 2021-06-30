#include "file-worker.h"
#include <hex.h>
#include <sd-card-api.h>
#include <furi.h>

struct FileWorker {
    FS_Api* fs_api;
    SdCard_Api* sd_ex_api;
    bool silent;
    File file;
};

bool file_worker_check_common_errors(FileWorker* file_worker);
void file_worker_show_error_internal(FileWorker* file_worker, const char* error_text);
bool file_worker_read_internal(FileWorker* file_worker, void* buffer, uint16_t bytes_to_read);
bool file_worker_write_internal(
    FileWorker* file_worker,
    const void* buffer,
    uint16_t bytes_to_write);
bool file_worker_tell_internal(FileWorker* file_worker, uint64_t* position);
bool file_worker_seek_internal(FileWorker* file_worker, uint64_t position, bool from_start);

FileWorker* file_worker_alloc(bool _silent) {
    FileWorker* file_worker = malloc(sizeof(FileWorker));
    file_worker->silent = _silent;
    file_worker->fs_api = furi_record_open("sdcard");
    file_worker->sd_ex_api = furi_record_open("sdcard-ex");

    return file_worker;
}

void file_worker_free(FileWorker* file_worker) {
    furi_record_close("sdcard");
    furi_record_close("sdcard-ex");
    free(file_worker);
}

bool file_worker_open(
    FileWorker* file_worker,
    const char* filename,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool result =
        file_worker->fs_api->file.open(&file_worker->file, filename, access_mode, open_mode);

    if(!result) {
        file_worker_show_error_internal(file_worker, "Cannot open\nfile");
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_close(FileWorker* file_worker) {
    file_worker->fs_api->file.close(&file_worker->file);

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_mkdir(FileWorker* file_worker, const char* dirname) {
    FS_Error fs_result = file_worker->fs_api->common.mkdir(dirname);

    if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
        file_worker_show_error_internal(file_worker, "Cannot create\nfolder");
        return false;
    };

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_remove(FileWorker* file_worker, const char* filename) {
    FS_Error fs_result = file_worker->fs_api->common.remove(filename);
    if(fs_result != FSE_OK && fs_result != FSE_NOT_EXIST) {
        file_worker_show_error_internal(file_worker, "Cannot remove\nold file");
        return false;
    };

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_read(FileWorker* file_worker, void* buffer, uint16_t bytes_to_read) {
    if(!file_worker_read_internal(file_worker, buffer, bytes_to_read)) {
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_read_until(FileWorker* file_worker, string_t str_result, char separator) {
    string_clean(str_result);
    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];

    do {
        uint16_t read_count =
            file_worker->fs_api->file.read(&file_worker->file, buffer, buffer_size);
        if(file_worker->file.error_id != FSE_OK) {
            file_worker_show_error_internal(file_worker, "Cannot read\nfile");
            return false;
        }

        bool result = false;
        for(uint16_t i = 0; i < read_count; i++) {
            if(buffer[i] == separator) {
                uint64_t position;
                if(!file_worker_tell_internal(file_worker, &position)) {
                    return false;
                }

                position = position - read_count + i + 1;

                if(!file_worker_seek_internal(file_worker, position, true)) {
                    return false;
                }

                result = true;
                break;
            } else {
                string_push_back(str_result, buffer[i]);
            }
        }

        if(result || read_count == 0) {
            break;
        }
    } while(true);

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_read_hex(FileWorker* file_worker, uint8_t* buffer, uint16_t bytes_to_read) {
    uint8_t hi_nibble_value, low_nibble_value;
    uint8_t text[2];

    for(uint8_t i = 0; i < bytes_to_read; i++) {
        if(i != 0) {
            // space
            if(!file_worker_read_internal(file_worker, text, 1)) {
                return false;
            }
        }

        // actual data
        if(!file_worker_read_internal(file_worker, text, 2)) {
            return false;
        }

        // convert hex value to byte
        if(hex_char_to_hex_nibble(text[0], &hi_nibble_value) &&
           hex_char_to_hex_nibble(text[1], &low_nibble_value)) {
            buffer[i] = (hi_nibble_value << 4) | low_nibble_value;
        } else {
            file_worker_show_error_internal(file_worker, "Cannot parse\nfile");
            return false;
        }
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_tell(FileWorker* file_worker, uint64_t* position) {
    if(!file_worker_tell_internal(file_worker, position)) {
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_seek(FileWorker* file_worker, uint64_t position, bool from_start) {
    if(!file_worker_seek_internal(file_worker, position, from_start)) {
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_write(FileWorker* file_worker, const void* buffer, uint16_t bytes_to_write) {
    if(!file_worker_write_internal(file_worker, buffer, bytes_to_write)) {
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_write_hex(FileWorker* file_worker, const uint8_t* buffer, uint16_t bytes_to_write) {
    const uint8_t byte_text_size = 3;
    char byte_text[byte_text_size];

    for(uint8_t i = 0; i < bytes_to_write; i++) {
        sniprintf(byte_text, byte_text_size, "%02X", buffer[i]);

        if(i != 0) {
            // space
            const char* space = " ";
            if(!file_worker_write_internal(file_worker, space, 1)) {
                return false;
            }
        }

        if(!file_worker_write_internal(file_worker, byte_text, 2)) {
            return false;
        }
    }

    return file_worker_check_common_errors(file_worker);
}

void file_worker_show_error(FileWorker* file_worker, const char* error_text) {
    file_worker->sd_ex_api->show_error(file_worker->sd_ex_api->context, error_text);
}

bool file_worker_file_select(
    FileWorker* file_worker,
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    char* selected_filename) {
    return file_worker->sd_ex_api->file_select(
        file_worker->sd_ex_api->context, path, extension, result, result_size, selected_filename);
}

bool file_worker_check_common_errors(FileWorker* file_worker) {
    file_worker->sd_ex_api->check_error(file_worker->sd_ex_api->context);
    return true;
}

void file_worker_show_error_internal(FileWorker* file_worker, const char* error_text) {
    if(!file_worker->silent) {
        file_worker_show_error(file_worker, error_text);
    }
}

bool file_worker_read_internal(FileWorker* file_worker, void* buffer, uint16_t bytes_to_read) {
    uint16_t read_count =
        file_worker->fs_api->file.read(&file_worker->file, buffer, bytes_to_read);

    if(file_worker->file.error_id != FSE_OK || read_count != bytes_to_read) {
        file_worker_show_error_internal(file_worker, "Cannot read\nfile");
        return false;
    }

    return true;
}

bool file_worker_write_internal(
    FileWorker* file_worker,
    const void* buffer,
    uint16_t bytes_to_write) {
    uint16_t write_count =
        file_worker->fs_api->file.write(&file_worker->file, buffer, bytes_to_write);

    if(file_worker->file.error_id != FSE_OK || write_count != bytes_to_write) {
        file_worker_show_error_internal(file_worker, "Cannot write\nto file");
        return false;
    }

    return true;
}

bool file_worker_tell_internal(FileWorker* file_worker, uint64_t* position) {
    *position = file_worker->fs_api->file.tell(&file_worker->file);

    if(file_worker->file.error_id != FSE_OK) {
        file_worker_show_error_internal(file_worker, "Cannot tell\nfile offset");
        return false;
    }

    return true;
}

bool file_worker_seek_internal(FileWorker* file_worker, uint64_t position, bool from_start) {
    file_worker->fs_api->file.seek(&file_worker->file, position, from_start);
    if(file_worker->file.error_id != FSE_OK) {
        file_worker_show_error_internal(file_worker, "Cannot seek\nfile");
        return false;
    }

    return true;
}