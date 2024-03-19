#include "buffered_file_stream.h"

#include "stream_i.h"
#include "file_stream.h"
#include "stream_cache.h"

typedef struct {
    Stream stream_base;
    Stream* file_stream;
    StreamCache* cache;
    bool sync_pending;
} BufferedFileStream;

static void buffered_file_stream_free(BufferedFileStream* stream);
static bool buffered_file_stream_eof(BufferedFileStream* stream);
static void buffered_file_stream_clean(BufferedFileStream* stream);
static bool
    buffered_file_stream_seek(BufferedFileStream* stream, int32_t offset, StreamOffset offset_type);
static size_t buffered_file_stream_tell(BufferedFileStream* stream);
static size_t buffered_file_stream_size(BufferedFileStream* stream);
static size_t
    buffered_file_stream_write(BufferedFileStream* stream, const uint8_t* data, size_t size);
static size_t buffered_file_stream_read(BufferedFileStream* stream, uint8_t* data, size_t size);
static bool buffered_file_stream_delete_and_insert(
    BufferedFileStream* stream,
    size_t delete_size,
    StreamWriteCB write_callback,
    const void* ctx);

static bool buffered_file_stream_flush(BufferedFileStream* stream);
static bool buffered_file_stream_unread(BufferedFileStream* stream);

const StreamVTable buffered_file_stream_vtable = {
    .free = (StreamFreeFn)buffered_file_stream_free,
    .eof = (StreamEOFFn)buffered_file_stream_eof,
    .clean = (StreamCleanFn)buffered_file_stream_clean,
    .seek = (StreamSeekFn)buffered_file_stream_seek,
    .tell = (StreamTellFn)buffered_file_stream_tell,
    .size = (StreamSizeFn)buffered_file_stream_size,
    .write = (StreamWriteFn)buffered_file_stream_write,
    .read = (StreamReadFn)buffered_file_stream_read,
    .delete_and_insert = (StreamDeleteAndInsertFn)buffered_file_stream_delete_and_insert,
};

Stream* buffered_file_stream_alloc(Storage* storage) {
    BufferedFileStream* stream = malloc(sizeof(BufferedFileStream));

    stream->file_stream = file_stream_alloc(storage);
    stream->cache = stream_cache_alloc();
    stream->sync_pending = false;

    stream->stream_base.vtable = &buffered_file_stream_vtable;
    return (Stream*)stream;
}

bool buffered_file_stream_open(
    Stream* _stream,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    furi_check(_stream);
    BufferedFileStream* stream = (BufferedFileStream*)_stream;
    furi_check(stream->stream_base.vtable == &buffered_file_stream_vtable);
    return file_stream_open(stream->file_stream, path, access_mode, open_mode);
}

bool buffered_file_stream_close(Stream* _stream) {
    furi_check(_stream);
    BufferedFileStream* stream = (BufferedFileStream*)_stream;
    furi_check(stream->stream_base.vtable == &buffered_file_stream_vtable);
    bool success = false;
    do {
        if(!(stream->sync_pending ? buffered_file_stream_flush(stream) :
                                    buffered_file_stream_unread(stream)))
            break;
        if(!file_stream_close(stream->file_stream)) break;
        success = true;
    } while(false);
    return success;
}

bool buffered_file_stream_sync(Stream* _stream) {
    furi_check(_stream);
    BufferedFileStream* stream = (BufferedFileStream*)_stream;
    furi_check(stream->stream_base.vtable == &buffered_file_stream_vtable);
    return stream->sync_pending ? buffered_file_stream_flush(stream) : true;
}

FS_Error buffered_file_stream_get_error(Stream* _stream) {
    furi_check(_stream);
    BufferedFileStream* stream = (BufferedFileStream*)_stream;
    furi_check(stream->stream_base.vtable == &buffered_file_stream_vtable);
    return file_stream_get_error(stream->file_stream);
}

static void buffered_file_stream_free(BufferedFileStream* stream) {
    furi_check(stream);
    buffered_file_stream_sync((Stream*)stream);
    stream_free(stream->file_stream);
    stream_cache_free(stream->cache);
    free(stream);
}

static bool buffered_file_stream_eof(BufferedFileStream* stream) {
    bool ret;
    const bool file_stream_eof = stream_eof(stream->file_stream);
    const bool cache_at_end = stream_cache_at_end(stream->cache);
    if(!stream->sync_pending) {
        ret = file_stream_eof && cache_at_end;
    } else {
        const size_t remaining_size =
            stream_size(stream->file_stream) - stream_tell(stream->file_stream);
        ret = stream_cache_size(stream->cache) >=
              (remaining_size ? cache_at_end : file_stream_eof);
    }
    return ret;
}

static void buffered_file_stream_clean(BufferedFileStream* stream) {
    // Not syncing because data will be deleted anyway
    stream->sync_pending = false;
    stream_cache_drop(stream->cache);
    stream_clean(stream->file_stream);
}

static bool buffered_file_stream_seek(
    BufferedFileStream* stream,
    int32_t offset,
    StreamOffset offset_type) {
    bool success = true;
    int32_t new_offset = offset;

    if(offset_type == StreamOffsetFromCurrent) {
        new_offset -= stream_cache_seek(stream->cache, offset);
        if(new_offset < 0) {
            new_offset -= (int32_t)stream_cache_size(stream->cache);
        }
    }

    if((new_offset != 0) || (offset_type != StreamOffsetFromCurrent)) {
        if(stream->sync_pending) {
            success = buffered_file_stream_sync((Stream*)stream);
        } else {
            stream_cache_drop(stream->cache);
        }
        if(success) {
            success = stream_seek(stream->file_stream, new_offset, offset_type);
        }
    }

    return success;
}

static size_t buffered_file_stream_tell(BufferedFileStream* stream) {
    size_t pos = stream_tell(stream->file_stream) + stream_cache_pos(stream->cache);
    if(!stream->sync_pending) {
        pos -= stream_cache_size(stream->cache);
    }
    return pos;
}

static size_t buffered_file_stream_size(BufferedFileStream* stream) {
    size_t size = stream_size(stream->file_stream);
    if(stream->sync_pending) {
        const size_t remaining_size = size - stream_tell(stream->file_stream);
        const size_t cache_size = stream_cache_size(stream->cache);
        if(cache_size > remaining_size) {
            size += (cache_size - remaining_size);
        }
    }
    return size;
}

static size_t
    buffered_file_stream_write(BufferedFileStream* stream, const uint8_t* data, size_t size) {
    size_t need_to_write = size;
    do {
        if(!stream->sync_pending) {
            if(!buffered_file_stream_unread(stream)) break;
        }
        while(need_to_write) {
            stream->sync_pending = true;
            need_to_write -=
                stream_cache_write(stream->cache, data + (size - need_to_write), need_to_write);
            if(need_to_write) {
                stream->sync_pending = false;
                if(!stream_cache_flush(stream->cache, stream->file_stream)) break;
            }
        }
    } while(false);
    return size - need_to_write;
}

static size_t buffered_file_stream_read(BufferedFileStream* stream, uint8_t* data, size_t size) {
    size_t need_to_read = size;
    while(need_to_read) {
        need_to_read -=
            stream_cache_read(stream->cache, data + (size - need_to_read), need_to_read);
        if(need_to_read) {
            if(stream->sync_pending) {
                if(!buffered_file_stream_flush(stream)) break;
            }
            if(!stream_cache_fill(stream->cache, stream->file_stream)) break;
        }
    }
    return size - need_to_read;
}

static bool buffered_file_stream_delete_and_insert(
    BufferedFileStream* stream,
    size_t delete_size,
    StreamWriteCB write_callback,
    const void* ctx) {
    bool success = false;
    do {
        if(!(stream->sync_pending ? buffered_file_stream_flush(stream) :
                                    buffered_file_stream_unread(stream)))
            break;
        if(!stream_delete_and_insert(stream->file_stream, delete_size, write_callback, ctx)) break;
        success = true;
    } while(false);
    return success;
}

// Write the cache into the underlying stream and adjust seek position
static bool buffered_file_stream_flush(BufferedFileStream* stream) {
    bool success = false;
    do {
        const int32_t offset = stream_cache_size(stream->cache) - stream_cache_pos(stream->cache);
        if(!stream_cache_flush(stream->cache, stream->file_stream)) break;
        if(offset > 0) {
            if(!stream_seek(stream->file_stream, -offset, StreamOffsetFromCurrent)) break;
        }
        success = true;
    } while(false);
    stream->sync_pending = false;
    return success;
}

// Drop read cache and adjust the underlying stream seek position
static bool buffered_file_stream_unread(BufferedFileStream* stream) {
    bool success = true;
    const size_t cache_size = stream_cache_size(stream->cache);
    if(cache_size > 0) {
        const size_t cache_pos = stream_cache_pos(stream->cache);
        if(cache_pos < cache_size) {
            const int32_t offset = cache_size - cache_pos;
            success = stream_seek(stream->file_stream, -offset, StreamOffsetFromCurrent);
        }
        stream_cache_drop(stream->cache);
    }
    return success;
}
