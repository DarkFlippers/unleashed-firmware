#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STREAM_CACHE_SIZE 512u

typedef struct StreamVTable StreamVTable;

typedef void (*StreamFreeFn)(Stream* stream);
typedef bool (*StreamEOFFn)(Stream* stream);
typedef void (*StreamCleanFn)(Stream* stream);
typedef bool (*StreamSeekFn)(Stream* stream, int32_t offset, StreamOffset offset_type);
typedef size_t (*StreamTellFn)(Stream* stream);
typedef size_t (*StreamSizeFn)(Stream* stream);
typedef size_t (*StreamWriteFn)(Stream* stream, const uint8_t* data, size_t size);
typedef size_t (*StreamReadFn)(Stream* stream, uint8_t* data, size_t count);
typedef bool (*StreamDeleteAndInsertFn)(
    Stream* stream,
    size_t delete_size,
    StreamWriteCB write_cb,
    const void* ctx);

struct StreamVTable {
    const StreamFreeFn free;
    const StreamEOFFn eof;
    const StreamCleanFn clean;
    const StreamSeekFn seek;
    const StreamTellFn tell;
    const StreamSizeFn size;
    const StreamWriteFn write;
    const StreamReadFn read;
    const StreamDeleteAndInsertFn delete_and_insert;
};

struct Stream {
    const StreamVTable* vtable;
};

#ifdef __cplusplus
}
#endif
