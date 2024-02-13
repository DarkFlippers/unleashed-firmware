#include <furi.h>
#include <toolbox/stream/file_stream.h>
#include "../cs_dbg.h"
#include "../frozen/frozen.h"

char* cs_read_file(const char* path, size_t* size) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(storage);
    char* data = NULL;
    if(!file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
    } else {
        *size = stream_size(stream);
        data = (char*)malloc(*size + 1);
        if(data != NULL) {
            stream_rewind(stream);
            if(stream_read(stream, (uint8_t*)data, *size) != *size) {
                file_stream_close(stream);
                furi_record_close(RECORD_STORAGE);
                stream_free(stream);
                free(data);
                return NULL;
            }
            data[*size] = '\0';
        }
    }
    file_stream_close(stream);
    furi_record_close(RECORD_STORAGE);
    stream_free(stream);
    return data;
}

char* json_fread(const char* path) {
    UNUSED(path);
    return NULL;
}

int json_vfprintf(const char* file_name, const char* fmt, va_list ap) {
    UNUSED(file_name);
    UNUSED(fmt);
    UNUSED(ap);
    return 0;
}

int json_prettify_file(const char* file_name) {
    UNUSED(file_name);
    return 0;
}

int json_printer_file(struct json_out* out, const char* buf, size_t len) {
    UNUSED(out);
    UNUSED(buf);
    UNUSED(len);
    return 0;
}

int cs_log_print_prefix(enum cs_log_level level, const char* file, int ln) {
    (void)level;
    (void)file;
    (void)ln;
    return 0;
}

void cs_log_printf(const char* fmt, ...) {
    (void)fmt;
}
