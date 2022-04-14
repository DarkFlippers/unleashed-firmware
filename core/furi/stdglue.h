/**
 * @file stdglue.h
 * Furi: stdlibc glue
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Write callback
 * @param      _cookie  pointer to cookie (see stdio gnu extension)
 * @param      data     pointer to data
 * @param      size     data size @warnign your handler must consume everything
 */
typedef void (*FuriStdglueWriteCallback)(void* _cookie, const char* data, size_t size);

/** Initialized std library glue code */
void furi_stdglue_init();

/** Set STDOUT callback for your thread
 *
 * @param      callback  callback or NULL to clear
 *
 * @return     true on success, otherwise fail
 * @warning    function is thread aware, use this API from the same thread
 */
bool furi_stdglue_set_thread_stdout_callback(FuriStdglueWriteCallback callback);

#ifdef __cplusplus
}
#endif
