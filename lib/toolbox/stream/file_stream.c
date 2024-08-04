#include "stream.h"
#include "stream_i.h"
#include "file_stream.h"

typedef struct {
    Stream stream_base;
    Storage* storage;
    File* file;
} FileStream;

static void file_stream_free(FileStream* stream);
static bool file_stream_eof(FileStream* stream);
static void file_stream_clean(FileStream* stream);
static bool file_stream_seek(FileStream* stream, int32_t offset, StreamOffset offset_type);
static size_t file_stream_tell(FileStream* stream);
static size_t file_stream_size(FileStream* stream);
static size_t file_stream_write(FileStream* stream, const uint8_t* data, size_t size);
static size_t file_stream_read(FileStream* stream, uint8_t* data, size_t size);
static bool file_stream_delete_and_insert(
    FileStream* stream,
    size_t delete_size,
    StreamWriteCB write_callback,
    const void* ctx);

const StreamVTable file_stream_vtable = {
    .free = (StreamFreeFn)file_stream_free,
    .eof = (StreamEOFFn)file_stream_eof,
    .clean = (StreamCleanFn)file_stream_clean,
    .seek = (StreamSeekFn)file_stream_seek,
    .tell = (StreamTellFn)file_stream_tell,
    .size = (StreamSizeFn)file_stream_size,
    .write = (StreamWriteFn)file_stream_write,
    .read = (StreamReadFn)file_stream_read,
    .delete_and_insert = (StreamDeleteAndInsertFn)file_stream_delete_and_insert,
};

Stream* file_stream_alloc(Storage* storage) {
    furi_check(storage);

    FileStream* stream = malloc(sizeof(FileStream));
    stream->file = storage_file_alloc(storage);
    stream->storage = storage;

    stream->stream_base.vtable = &file_stream_vtable;
    return (Stream*)stream;
}

bool file_stream_open(
    Stream* _stream,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    furi_check(_stream);
    FileStream* stream = (FileStream*)_stream;
    furi_check(stream->stream_base.vtable == &file_stream_vtable);
    return storage_file_open(stream->file, path, access_mode, open_mode);
}

bool file_stream_close(Stream* _stream) {
    furi_check(_stream);
    FileStream* stream = (FileStream*)_stream;
    furi_check(stream->stream_base.vtable == &file_stream_vtable);
    return storage_file_close(stream->file);
}

FS_Error file_stream_get_error(Stream* _stream) {
    furi_check(_stream);
    FileStream* stream = (FileStream*)_stream;
    furi_check(stream->stream_base.vtable == &file_stream_vtable);
    return storage_file_get_error(stream->file);
}

static void file_stream_free(FileStream* stream) {
    storage_file_free(stream->file);
    free(stream);
}

static bool file_stream_eof(FileStream* stream) {
    return storage_file_eof(stream->file);
}

static void file_stream_clean(FileStream* stream) {
    storage_file_seek(stream->file, 0, true);
    storage_file_truncate(stream->file);
}

static bool file_stream_seek(FileStream* stream, int32_t offset, StreamOffset offset_type) {
    bool result = false;
    size_t seek_position = 0;
    size_t current_position = file_stream_tell(stream);
    size_t size = file_stream_size(stream);

    // calc offset and limit to bottom
    switch(offset_type) {
    case StreamOffsetFromCurrent: {
        if((int32_t)(current_position + offset) >= 0) {
            seek_position = current_position + offset;
            result = true;
        }
    } break;
    case StreamOffsetFromStart: {
        if(offset >= 0) {
            seek_position = offset;
            result = true;
        }
    } break;
    case StreamOffsetFromEnd: {
        if((int32_t)(size + offset) >= 0) {
            seek_position = size + offset;
            result = true;
        }
    } break;
    }

    if(result) {
        // limit to top
        if((int32_t)(seek_position - size) > 0) {
            storage_file_seek(stream->file, size, true);
            result = false;
        } else {
            result = storage_file_seek(stream->file, seek_position, true);
        }
    } else {
        storage_file_seek(stream->file, 0, true);
    }

    return result;
}

static size_t file_stream_tell(FileStream* stream) {
    return storage_file_tell(stream->file);
}

static size_t file_stream_size(FileStream* stream) {
    return storage_file_size(stream->file);
}

static size_t file_stream_write(FileStream* stream, const uint8_t* data, size_t size) {
    return storage_file_write(stream->file, data, size);
}

static size_t file_stream_read(FileStream* stream, uint8_t* data, size_t size) {
    return storage_file_read(stream->file, data, size);
}

static bool file_stream_delete_and_insert(
    FileStream* _stream,
    size_t delete_size,
    StreamWriteCB write_callback,
    const void* ctx) {
    bool result = false;
    Stream* stream = (Stream*)_stream;

    // open scratchpad
    Stream* scratch_stream = file_stream_alloc(_stream->storage);

    // TODO FL-3546: we need something like "storage_open_tmpfile and storage_close_tmpfile"
    FuriString* scratch_name;
    FuriString* tmp_name;
    tmp_name = furi_string_alloc();
    storage_get_next_filename(
        _stream->storage, STORAGE_EXT_PATH_PREFIX, ".scratch", ".pad", tmp_name, 255);
    scratch_name = furi_string_alloc_printf(EXT_PATH("%s.pad"), furi_string_get_cstr(tmp_name));
    furi_string_free(tmp_name);

    do {
        if(!file_stream_open(
               scratch_stream,
               furi_string_get_cstr(scratch_name),
               FSAM_READ_WRITE,
               FSOM_CREATE_NEW))
            break;

        size_t current_position = stream_tell(stream);
        size_t file_size = stream_size(stream);

        size_t size_to_delete = file_size - current_position;
        size_to_delete = MIN(delete_size, size_to_delete);

        size_t size_to_copy_before = current_position;
        size_t size_to_copy_after = file_size - current_position - size_to_delete;

        // copy file from 0 to insert position to scratchpad
        if(!stream_rewind(stream)) break;
        if(stream_copy(stream, scratch_stream, size_to_copy_before) != size_to_copy_before) break;

        if(write_callback) {
            if(!write_callback(scratch_stream, ctx)) break;
        }
        size_t new_position = stream_tell(scratch_stream);

        // copy key file after insert position + size_to_delete to scratchpad
        if(!stream_seek(stream, size_to_delete, StreamOffsetFromCurrent)) break;
        if(stream_copy(stream, scratch_stream, size_to_copy_after) != size_to_copy_after) break;

        size_t new_file_size = stream_size(scratch_stream);

        // copy whole scratchpad file to the original file
        if(!stream_rewind(stream)) break;
        if(!stream_rewind(scratch_stream)) break;
        if(stream_copy(scratch_stream, stream, new_file_size) != new_file_size) break;

        // and truncate original file
        if(!storage_file_truncate(_stream->file)) break;

        // move seek pointer at insert end
        if(!stream_seek(stream, new_position, StreamOffsetFromStart)) break;

        result = true;
    } while(false);

    stream_free(scratch_stream);
    storage_common_remove(_stream->storage, furi_string_get_cstr(scratch_name));
    furi_string_free(scratch_name);

    return result;
}
