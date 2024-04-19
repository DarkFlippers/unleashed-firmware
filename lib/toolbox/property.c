#include "property.h"

#include <core/check.h>

void property_value_out(PropertyValueContext* ctx, const char* fmt, unsigned int nparts, ...) {
    furi_check(ctx);
    furi_string_reset(ctx->key);

    va_list args;
    va_start(args, nparts);

    for(size_t i = 0; i < nparts; ++i) {
        const char* keypart = va_arg(args, const char*);
        furi_string_cat(ctx->key, keypart);
        if(i < nparts - 1) {
            furi_string_push_back(ctx->key, ctx->sep);
        }
    }

    const char* value_str;

    if(fmt) {
        furi_string_vprintf(ctx->value, fmt, args);
        value_str = furi_string_get_cstr(ctx->value);
    } else {
        // C string passthrough (no formatting)
        value_str = va_arg(args, const char*);
    }

    va_end(args);

    ctx->out(furi_string_get_cstr(ctx->key), value_str, ctx->last, ctx->context);
}
