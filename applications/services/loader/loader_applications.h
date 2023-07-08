#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoaderApplications LoaderApplications;

LoaderApplications* loader_applications_alloc(void (*closed_cb)(void*), void* context);

void loader_applications_free(LoaderApplications* loader_applications);

#ifdef __cplusplus
}
#endif