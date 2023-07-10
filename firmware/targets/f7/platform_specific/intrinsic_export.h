#pragma once
#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void __clear_cache(void*, void*);
void* __aeabi_uldivmod(uint64_t, uint64_t);

#ifdef __cplusplus
}
#endif
