#pragma once
#include <stdlib.h>
#include <mlib/m-string.h>
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate string stream
 * @return Stream* 
 */
Stream* string_stream_alloc();

#ifdef __cplusplus
}
#endif
