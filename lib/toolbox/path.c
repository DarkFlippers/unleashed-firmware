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

static inline void path_cleanup(string_t path) {
    string_strim(path);
    while(string_end_with_str_p(path, "/")) {
        string_left(path, string_size(path) - 1);
    }
}

void path_extract_basename(const char* path, string_t basename) {
    string_set(basename, path);
    path_cleanup(basename);
    size_t pos = string_search_rchar(basename, '/');
    if(pos != STRING_FAILURE) {
        string_right(basename, pos);
    }
}

void path_extract_dirname(const char* path, string_t dirname) {
    string_set(dirname, path);
    path_cleanup(dirname);
    size_t pos = string_search_rchar(dirname, '/');
    if(pos != STRING_FAILURE) {
        string_left(dirname, pos);
    }
}

void path_append(string_t path, const char* suffix) {
    path_cleanup(path);
    string_t suffix_str;
    string_init_set(suffix_str, suffix);
    string_strim(suffix_str);
    string_strim(suffix_str, "/");
    string_cat_printf(path, "/%s", string_get_cstr(suffix_str));
    string_clear(suffix_str);
}

void path_concat(const char* path, const char* suffix, string_t out_path) {
    string_set(out_path, path);
    path_append(out_path, suffix);
}
