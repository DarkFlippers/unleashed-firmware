#include "path.h"

void path_extract_filename_no_ext(const char* path, string_t filename) {
    string_set(filename, path);

    size_t start_position = string_search_rchar(filename, '/');
    size_t end_position = string_search_rchar(filename, '.');

    if(start_position == STRING_FAILURE) {
        start_position = 0;
    } else {
        start_position += 1;
    }

    if(end_position == STRING_FAILURE) {
        end_position = string_size(filename);
    }

    string_mid(filename, start_position, end_position - start_position);
}
