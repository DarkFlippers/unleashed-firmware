#pragma once

#include <furi.h>
#include <mjs_core_public.h>

#ifdef __cplusplus
extern "C" {
#endif

bool js_delay_with_flags(struct mjs* mjs, uint32_t time);

void js_flags_set(struct mjs* mjs, uint32_t flags);

uint32_t js_flags_wait(struct mjs* mjs, uint32_t flags, uint32_t timeout);

#ifdef __cplusplus
}
#endif