#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoaderApplications LoaderApplications;

LoaderApplications* loader_applications_alloc(void (*closed_cb)(void*), void* context);

// Private launcher entry point; does not change the public FAP API.
LoaderApplications* loader_applications_alloc_at_path(
    void (*closed_cb)(void*), void* context, const char* path);

void loader_applications_free(LoaderApplications* loader_applications);

#ifdef __cplusplus
}
#endif
