#pragma once
#include "flipper-file.h"

class FlipperFileCpp {
private:
    FlipperFile* file;

public:
    FlipperFileCpp(Storage* storage);
    ~FlipperFileCpp();

    bool open_read(const char* filename);

    bool new_write(const char* filename);

    bool close();

    bool read_header(string_t filetype, uint32_t* version);

    bool write_header(string_t filetype, const uint32_t version);

    bool write_header_cstr(const char* filetype, const uint32_t version);

    bool read_string(const char* key, string_t data);

    bool write_string(const char* key, string_t data);

    bool write_string_cstr(const char* key, const char* data);

    bool read_uint32(const char* key, uint32_t* data);

    bool write_uint32(const char* key, const uint32_t data);

    bool write_comment(string_t data);

    bool write_comment_cstr(const char* data);

    bool write_hex_array(const char* key, const uint8_t* data, const uint16_t data_size);

    bool read_hex_array(const char* key, uint8_t* data, const uint16_t data_size);
};
