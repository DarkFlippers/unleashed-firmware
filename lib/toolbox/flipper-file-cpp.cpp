#include "flipper-file-cpp.h"

FlipperFileCpp::FlipperFileCpp(Storage* storage) {
    file = flipper_file_alloc(storage);
}

FlipperFileCpp::~FlipperFileCpp() {
    flipper_file_free(file);
}

bool FlipperFileCpp::open_read(const char* filename) {
    return flipper_file_open_read(file, filename);
}

bool FlipperFileCpp::new_write(const char* filename) {
    return flipper_file_new_write(file, filename);
}

bool FlipperFileCpp::close() {
    return flipper_file_close(file);
}

bool FlipperFileCpp::read_header(string_t filetype, uint32_t* version) {
    return flipper_file_read_header(file, filetype, version);
}

bool FlipperFileCpp::write_header(string_t filetype, const uint32_t version) {
    return flipper_file_write_header(file, filetype, version);
}

bool FlipperFileCpp::write_header_cstr(const char* filetype, const uint32_t version) {
    return flipper_file_write_header_cstr(file, filetype, version);
}

bool FlipperFileCpp::read_string(const char* key, string_t data) {
    return flipper_file_read_string(file, key, data);
}

bool FlipperFileCpp::write_string(const char* key, string_t data) {
    return flipper_file_write_string(file, key, data);
}

bool FlipperFileCpp::write_string_cstr(const char* key, const char* data) {
    return flipper_file_write_string_cstr(file, key, data);
}

bool FlipperFileCpp::read_uint32(const char* key, uint32_t* data) {
    return flipper_file_read_uint32(file, key, data);
}

bool FlipperFileCpp::write_uint32(const char* key, const uint32_t data) {
    return flipper_file_write_uint32(file, key, data);
}

bool FlipperFileCpp::write_comment(string_t data) {
    return flipper_file_write_comment(file, data);
}

bool FlipperFileCpp::write_comment_cstr(const char* data) {
    return flipper_file_write_comment_cstr(file, data);
}

bool FlipperFileCpp::write_hex_array(
    const char* key,
    const uint8_t* data,
    const uint16_t data_size) {
    return flipper_file_write_hex_array(file, key, data, data_size);
}

bool FlipperFileCpp::read_hex_array(const char* key, uint8_t* data, const uint16_t data_size) {
    return flipper_file_read_hex_array(file, key, data, data_size);
}
