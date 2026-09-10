/**
 * @file application_assets.h
 * Flipper application assets
 */
#pragma once

#include <storage/storage.h>

//for FlipperApplicationAssetsProgress, which callers set on the application
#include "flipper_application.h"

#ifdef __cplusplus
extern "C" {
#endif

bool flipper_application_assets_load(
    File* file,
    const char* elf_path,
    size_t offset,
    size_t size,
    FlipperApplicationAssetsProgress progress,
    void* progress_context);

#ifdef __cplusplus
}
#endif
