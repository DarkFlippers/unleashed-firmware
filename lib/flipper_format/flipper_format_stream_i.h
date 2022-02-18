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

#ifdef __cplusplus
}
#endif