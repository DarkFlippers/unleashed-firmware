#include "file-worker-cpp.h"
#include <hex.h>

FileWorkerCpp::FileWorkerCpp(bool _silent) {
    file_worker = file_worker_alloc(_silent);
}

FileWorkerCpp::~FileWorkerCpp() {
    file_worker_free(file_worker);
}

bool FileWorkerCpp::open(const char* filename, FS_AccessMode access_mode, FS_OpenMode open_mode) {
    return file_worker_open(file_worker, filename, access_mode, open_mode);
}

bool FileWorkerCpp::close() {
    return file_worker_close(file_worker);
}

bool FileWorkerCpp::mkdir(const char* dirname) {
    return file_worker_mkdir(file_worker, dirname);
}

bool FileWorkerCpp::remove(const char* filename) {
    return file_worker_remove(file_worker, filename);
}

bool FileWorkerCpp::read(void* buffer, uint16_t bytes_to_read) {
    return file_worker_read(file_worker, buffer, bytes_to_read);
}

bool FileWorkerCpp::read_until(string_t str_result, char separator) {
    return file_worker_read_until(file_worker, str_result, separator);
}

bool FileWorkerCpp::read_hex(uint8_t* buffer, uint16_t bytes_to_read) {
    return file_worker_read_hex(file_worker, buffer, bytes_to_read);
}

bool FileWorkerCpp::tell(uint64_t* position) {
    return file_worker_tell(file_worker, position);
}

bool FileWorkerCpp::seek(uint64_t position, bool from_start) {
    return file_worker_seek(file_worker, position, from_start);
}

bool FileWorkerCpp::write(const void* buffer, uint16_t bytes_to_write) {
    return file_worker_write(file_worker, buffer, bytes_to_write);
}

bool FileWorkerCpp::write_hex(const uint8_t* buffer, uint16_t bytes_to_write) {
    return file_worker_write_hex(file_worker, buffer, bytes_to_write);
}

void FileWorkerCpp::show_error(const char* error_text) {
    file_worker_show_error(file_worker, error_text);
}

bool FileWorkerCpp::file_select(
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    char* selected_filename) {
    return file_worker_file_select(
        file_worker, path, extension, result, result_size, selected_filename);
}