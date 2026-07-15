#include "str_buffer.h"

const char* str_buffer_make_owned_clone(StrBuffer* buffer, const char* str) {
    char* owned = strdup(str);
    buffer->n_owned_strings++;
    buffer->owned_strings =
        realloc(buffer->owned_strings, buffer->n_owned_strings * sizeof(const char*)); // -V701
    buffer->owned_strings[buffer->n_owned_strings - 1] = owned;
    return owned;
}

void str_buffer_clear_all_clones(StrBuffer* buffer) {
    for(size_t i = 0; i < buffer->n_owned_strings; i++) {
        free(buffer->owned_strings[i]);
    }
    free(buffer->owned_strings);
    buffer->owned_strings = NULL;
}
