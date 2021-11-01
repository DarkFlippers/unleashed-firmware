#include "file-worker.h"
#include <m-string.h>
#include <lib/toolbox/hex.h>
#include <dialogs/dialogs.h>
#include <furi.h>

struct FileWorker {
    Storage* api;
    bool silent;
    File* file;
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
    file_worker->api = furi_record_open("storage");
    file_worker->file = storage_file_alloc(file_worker->api);

    return file_worker;
}

void file_worker_free(FileWorker* file_worker) {
    storage_file_free(file_worker->file);
    furi_record_close("storage");
    free(file_worker);
}

bool file_worker_open(
    FileWorker* file_worker,
    const char* filename,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool result = storage_file_open(file_worker->file, filename, access_mode, open_mode);

    if(!result) {
        file_worker_show_error_internal(file_worker, "Cannot open\nfile");
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_close(FileWorker* file_worker) {
    if(storage_file_is_open(file_worker->file)) {
        storage_file_close(file_worker->file);
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_mkdir(FileWorker* file_worker, const char* dirname) {
    FS_Error fs_result = storage_common_mkdir(file_worker->api, dirname);

    if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
        file_worker_show_error_internal(file_worker, "Cannot create\nfolder");
        return false;
    };

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_remove(FileWorker* file_worker, const char* filename) {
    FS_Error fs_result = storage_common_remove(file_worker->api, filename);
    if(fs_result != FSE_OK && fs_result != FSE_NOT_EXIST) {
        file_worker_show_error_internal(file_worker, "Cannot remove\nold file");
        return false;
    };

    return file_worker_check_common_errors(file_worker);
}

void file_worker_get_next_filename(
    FileWorker* file_worker,
    const char* dirname,
    const char* filename,
    const char* fileextension,
    string_t nextfilename) {
    string_t temp_str;
    string_init(temp_str);
    uint16_t num = 0;

    string_printf(temp_str, "%s/%s%s", dirname, filename, fileextension);

    while(storage_common_stat(file_worker->api, string_get_cstr(temp_str), NULL) == FSE_OK) {
        num++;
        string_printf(temp_str, "%s/%s%d%s", dirname, filename, num, fileextension);
    }

    if(num) {
        string_printf(nextfilename, "%s%d", filename, num);
    } else {
        string_printf(nextfilename, "%s", filename);
    }

    string_clear(temp_str);
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
        uint16_t read_count = storage_file_read(file_worker->file, buffer, buffer_size);
        if(storage_file_get_error(file_worker->file) != FSE_OK) {
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
    DialogsApp* dialogs = furi_record_open("dialogs");

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_text(message, error_text, 88, 32, AlignCenter, AlignCenter);
    dialog_message_set_icon(message, &I_SDQuestion_35x43, 5, 6);
    dialog_message_set_buttons(message, "Back", NULL, NULL);
    dialog_message_show(dialogs, message);
    dialog_message_free(message);

    furi_record_close("dialogs");
}

bool file_worker_file_select(
    FileWorker* file_worker,
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    const char* selected_filename) {
    DialogsApp* dialogs = furi_record_open("dialogs");
    bool ret =
        dialog_file_select_show(dialogs, path, extension, result, result_size, selected_filename);
    furi_record_close("dialogs");
    return ret;
}

bool file_worker_check_common_errors(FileWorker* file_worker) {
    //TODO remove
    /* TODO: [FL-1431] Add return value to file_parser.get_sd_api().check_error() and replace get_fs_info(). */
    return true;
}

void file_worker_show_error_internal(FileWorker* file_worker, const char* error_text) {
    if(!file_worker->silent) {
        file_worker_show_error(file_worker, error_text);
    }
}

bool file_worker_read_internal(FileWorker* file_worker, void* buffer, uint16_t bytes_to_read) {
    uint16_t read_count = storage_file_read(file_worker->file, buffer, bytes_to_read);

    if(storage_file_get_error(file_worker->file) != FSE_OK || read_count != bytes_to_read) {
        file_worker_show_error_internal(file_worker, "Cannot read\nfile");
        return false;
    }

    return true;
}

bool file_worker_write_internal(
    FileWorker* file_worker,
    const void* buffer,
    uint16_t bytes_to_write) {
    uint16_t write_count = storage_file_write(file_worker->file, buffer, bytes_to_write);

    if(storage_file_get_error(file_worker->file) != FSE_OK || write_count != bytes_to_write) {
        file_worker_show_error_internal(file_worker, "Cannot write\nto file");
        return false;
    }

    return true;
}

bool file_worker_tell_internal(FileWorker* file_worker, uint64_t* position) {
    *position = storage_file_tell(file_worker->file);

    if(storage_file_get_error(file_worker->file) != FSE_OK) {
        file_worker_show_error_internal(file_worker, "Cannot tell\nfile offset");
        return false;
    }

    return true;
}

bool file_worker_seek_internal(FileWorker* file_worker, uint64_t position, bool from_start) {
    storage_file_seek(file_worker->file, position, from_start);
    if(storage_file_get_error(file_worker->file) != FSE_OK) {
        file_worker_show_error_internal(file_worker, "Cannot seek\nfile");
        return false;
    }

    return true;
}

bool file_worker_read_until_buffered(
    FileWorker* file_worker,
    string_t str_result,
    char* file_buf,
    size_t* file_buf_cnt,
    size_t file_buf_size,
    char separator) {
    furi_assert(string_capacity(str_result) > 0);

    // fs_api->file.read now supports up to 512 bytes reading at a time
    furi_assert(file_buf_size <= 512);

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

            if(max_length && (string_size(str_result) + end_index > max_length))
                max_length_exceeded = true;

            if(!max_length_exceeded) {
                for(size_t i = 0; i < end_index; ++i) {
                    string_push_back(str_result, file_buf[i]);
                }
            }

            memmove(file_buf, &file_buf[end_index], *file_buf_cnt - end_index);
            *file_buf_cnt = *file_buf_cnt - end_index;
            if(found_eol) break;
        }

        *file_buf_cnt += storage_file_read(
            file_worker->file, &file_buf[*file_buf_cnt], file_buf_size - *file_buf_cnt);
        if(storage_file_get_error(file_worker->file) != FSE_OK) {
            file_worker_show_error_internal(file_worker, "Cannot read\nfile");
            string_clean(str_result);
            *file_buf_cnt = 0;
            break;
        }
        if(*file_buf_cnt == 0) {
            break; // end of reading
        }
    }

    if(max_length_exceeded) string_clean(str_result);

    return string_size(str_result) || *file_buf_cnt;
}

bool file_worker_get_value_from_key(
    FileWorker* file_worker,
    string_t key,
    char delimiter,
    string_t value) {
    bool found = false;
    string_t next_line;
    string_t next_key;
    string_init(next_line);
    string_init(next_key);
    size_t delim_pos = 0;

    while(file_worker_read_until(file_worker, next_line, '\n')) {
        delim_pos = string_search_char(next_line, delimiter);
        if(delim_pos == STRING_FAILURE) {
            break;
        }
        string_set_n(next_key, next_line, 0, delim_pos);
        if(string_equal_p(next_key, key)) {
            string_right(next_line, delim_pos);
            string_strim(next_line);
            string_set(value, next_line);
            found = true;
            break;
        }
    }

    string_clear(next_line);
    string_clear(next_key);
    return found;
}

bool file_worker_rename(FileWorker* file_worker, const char* old_path, const char* new_path) {
    FS_Error fs_result = storage_common_rename(file_worker->api, old_path, new_path);

    if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
        file_worker_show_error_internal(file_worker, "Cannot rename\n file/directory");
        return false;
    }

    return file_worker_check_common_errors(file_worker);
}

bool file_worker_check_errors(FileWorker* file_worker) {
    return file_worker_check_common_errors(file_worker);
}

bool file_worker_is_file_exist(FileWorker* file_worker, const char* filename, bool* exist) {
    File* file = storage_file_alloc(file_worker->api);

    *exist = storage_file_open(file, filename, FSAM_READ, FSOM_OPEN_EXISTING);
    storage_file_close(file);
    storage_file_free(file);

    return file_worker_check_common_errors(file_worker);
}
