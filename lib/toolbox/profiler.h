#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Profiler Profiler;

Profiler* profiler_alloc(void);

void profiler_free(Profiler* profiler);

void profiler_prealloc(Profiler* profiler, const char* key);

void profiler_start(Profiler* profiler, const char* key);

void profiler_stop(Profiler* profiler, const char* key);

void profiler_dump(Profiler* profiler);

#ifdef __cplusplus
}
#endif
