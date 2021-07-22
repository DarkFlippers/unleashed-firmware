#include "file-worker.h"
#include "m-string.h"
#include <hex.h>
#include <sd-card-api.h>
#include <furi.h>

struct FileWorker {
    FS_Api* fs_api;
    SdCard_Api* sd_ex_api;
    bool silent;
    File file;
    char file_buf[48];
    size_t file_buf_cnt;
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
    file_worker->file_buf_cnt = 0;

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
    const char* selected_filename) {
    return file_worker->sd_ex_api->file_select(
        file_worker->sd_ex_api->context, path, extension, result, result_size, selected_filename);
}

bool file_worker_check_common_errors(FileWorker* file_worker) {
    /* TODO: [FL-1431] Add return value to file_parser.get_sd_api().check_error() and replace get_fs_info(). */
    FS_Error fs_err = file_worker->fs_api->common.get_fs_info(NULL, NULL);
    if(fs_err != FSE_OK)
        file_worker->sd_ex_api->show_error(file_worker->sd_ex_api->context, "SD card not found");
    return fs_err == FSE_OK;
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

bool file_worker_read_until_buffered(FileWorker* file_worker, string_t str_result, char* file_buf, size_t* file_buf_cnt, size_t file_buf_size, char separator) {
    furi_assert(string_capacity(str_result) > 0);
    furi_assert(file_buf_size <= 512);  /* fs_api->file.read now supports up to 512 bytes reading at a time */

    string_clean(str_result);
    size_t newline_index = 0;
    bool found_eol = false;
    bool max_length_exceeded = false;
    size_t max_length = string_capacity(str_result) - 1;

    while(1) {
        if(*file_buf_cnt > 0) {
            size_t end_index = 0;
            char* endline_ptr = (char*)memchr(file_buf, separator, *file_buf_cnt);
            newline_index = endline_ptr - file_buf;

            if(endline_ptr == 0) {
                end_index = *file_buf_cnt;
            } else if(newline_index < *file_buf_cnt) {
                end_index = newline_index + 1;
                found_eol = true;
            } else {
                furi_assert(0);
            }

            if (max_length && (string_size(str_result) + end_index > max_length))
                max_length_exceeded = true;

            if (!max_length_exceeded) {
                for (size_t i = 0; i < end_index; ++i) {
                    string_push_back(str_result, file_buf[i]);
                }
            }

            memmove(file_buf, &file_buf[end_index], *file_buf_cnt - end_index);
            *file_buf_cnt = *file_buf_cnt - end_index;
            if(found_eol) break;
        }

        *file_buf_cnt +=
            file_worker->fs_api->file.read(&file_worker->file, &file_buf[*file_buf_cnt], file_buf_size - *file_buf_cnt);
        if(file_worker->file.error_id != FSE_OK) {
            file_worker_show_error_internal(file_worker, "Cannot read\nfile");
            string_clear(str_result);
            *file_buf_cnt = 0;
            break;
        }
        if(*file_buf_cnt == 0) {
            break; // end of reading
        }
    }

    if (max_length_exceeded)
        string_clear(str_result);

    return string_size(str_result) || *file_buf_cnt;
}

bool file_worker_rename(FileWorker* file_worker, const char* old_path, const char* new_path) {
    FS_Error fs_result = file_worker->fs_api->common.rename(old_path, new_path);

    if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
        file_worker_show_error_internal(file_worker, "Cannot rename\n file/directory");
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_check_errors(FileWorker* file_worker) {
    return file_worker_check_common_errors(file_worker);
}

bool file_worker_is_file_exist(
    FileWorker* file_worker,
    const char* filename,
    bool* exist) {

    File file;
    *exist = file_worker->fs_api->file.open(&file, filename, FSAM_READ, FSOM_OPEN_EXISTING);
    if (*exist)
        file_worker->fs_api->file.close(&file);

    return file_worker_check_common_errors(file_worker);
}

