#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <core/string.h>

/** Callback type called every time another key-value pair of device information is ready
 *
 * @param      key[in]      device information type identifier
 * @param      value[in]    device information value
 * @param      last[in]     whether the passed key-value pair is the last one
 * @param      context[in]  to pass to callback
 */
typedef void (*PropertyValueCallback)(const char* key, const char* value, bool last, void* context);

typedef struct {
    FuriString* key; /**< key string buffer, must be initialised before use */
    FuriString* value; /**< value string buffer, must be initialised before use */
    PropertyValueCallback out; /**< output callback function */
    char sep; /**< separator character between key parts */
    bool last; /**< flag to indicate last element */
    void* context; /**< user-defined context, passed through to out callback */
} PropertyValueContext;

/** Builds key and value strings and outputs them via a callback function
 *
 * @param       ctx[in]     local property context
 * @param       fmt[in]     value format, set to NULL to bypass formatting
 * @param       nparts[in]  number of key parts (separated by character)
 * @param       ...[in]     list of key parts followed by value
 */
void property_value_out(PropertyValueContext* ctx, const char* fmt, unsigned int nparts, ...);

#ifdef __cplusplus
}
#endif
