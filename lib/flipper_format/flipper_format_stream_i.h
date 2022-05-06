#pragma once
#include "flipper_format_stream.h"

static const char flipper_format_delimiter = ':';
static const char flipper_format_comment = '#';
static const char flipper_format_eoln = '\n';
static const char flipper_format_eolr = '\r';

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Write Flipper Format EOL to the stream
 * @param stream 
 * @return true 
 * @return false 
 */
bool flipper_format_stream_write_eol(Stream* stream);

/**
 * Seek to the key from the current position of the stream.
 * Position will be at the beginning of the value corresponding to the key, if the key is found,, or at the end of the stream.
 * @param stream 
 * @param key 
 * @param strict_mode 
 * @return true key is found
 * @return false key is not found
 */
bool flipper_format_stream_seek_to_key(Stream* stream, const char* key, bool strict_mode);

#ifdef __cplusplus
}
#endif
