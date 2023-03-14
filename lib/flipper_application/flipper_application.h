/**
 * @file flipper_application.h
 * Flipper application
 */
#pragma once

#include "application_manifest.h"
#include "elf/elf_api_interface.h"

#include <furi.h>
#include <storage/storage.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FlipperApplicationPreloadStatusSuccess = 0,
    FlipperApplicationPreloadStatusUnspecifiedError,
    FlipperApplicationPreloadStatusInvalidFile,
    FlipperApplicationPreloadStatusInvalidManifest,
    FlipperApplicationPreloadStatusApiMismatch,
    FlipperApplicationPreloadStatusTargetMismatch,
} FlipperApplicationPreloadStatus;

typedef enum {
    FlipperApplicationLoadStatusSuccess = 0,
    FlipperApplicationLoadStatusUnspecifiedError,
    FlipperApplicationLoadStatusNoFreeMemory,
    FlipperApplicationLoadStatusMissingImports,
} FlipperApplicationLoadStatus;

/**
 * @brief Get text description of preload status
 * @param status Status code
 * @return String pointer to description
 */
const char* flipper_application_preload_status_to_string(FlipperApplicationPreloadStatus status);

/**
 * @brief Get text description of load status
 * @param status Status code
 * @return String pointer to description
 */
const char* flipper_application_load_status_to_string(FlipperApplicationLoadStatus status);

typedef struct FlipperApplication FlipperApplication;

typedef struct {
    const char* name;
    uint32_t address;
} FlipperApplicationMemoryMapEntry;

typedef struct {
    uint32_t mmap_entry_count;
    FlipperApplicationMemoryMapEntry* mmap_entries;
    uint32_t debug_link_size;
    uint8_t* debug_link;
} FlipperApplicationState;

/**
 * @brief Initialize FlipperApplication object
 * @param storage Storage instance
 * @param api_interface ELF API interface to use for pre-loading and symbol resolving
 * @return Application instance
 */
FlipperApplication*
    flipper_application_alloc(Storage* storage, const ElfApiInterface* api_interface);

/**
 * @brief Destroy FlipperApplication object
 * @param app Application pointer
 */
void flipper_application_free(FlipperApplication* app);

/**
 * @brief Validate elf file and load application metadata 
 * @param app Application pointer
 * @return Preload result code
 */
FlipperApplicationPreloadStatus
    flipper_application_preload(FlipperApplication* app, const char* path);

/**
 * @brief Validate elf file and load application manifest 
 * @param app Application pointer
 * @return Preload result code
 */
FlipperApplicationPreloadStatus
    flipper_application_preload_manifest(FlipperApplication* app, const char* path);

/**
 * @brief Get pointer to application manifest for preloaded application
 * @param app Application pointer
 * @return Pointer to application manifest
 */
const FlipperApplicationManifest* flipper_application_get_manifest(FlipperApplication* app);

/**
 * @brief Load sections and process relocations for already pre-loaded application
 * @param app Application pointer
 * @return Load result code
 */
FlipperApplicationLoadStatus flipper_application_map_to_memory(FlipperApplication* app);

/**
 * @brief Create application thread at entry point address, using app name and
 * stack size from metadata. Returned thread isn't started yet. 
 * Can be only called once for application instance.
 * @param app Applicaiton pointer
 * @param args Object to pass to app's entry point
 * @return Created thread
 */
FuriThread* flipper_application_spawn(FlipperApplication* app, void* args);

/**
 * @brief Check if application is a plugin (not a runnable standalone app)
 * @param app Application pointer
 * @return true if application is a plugin, false otherwise
 */
bool flipper_application_is_plugin(FlipperApplication* app);

/**
 * @brief Entry point prototype for standalone applications
 */
typedef int32_t (*FlipperApplicationEntryPoint)(void*);

/**
 * @brief An object that describes a plugin - must be returned by plugin's entry point 
 */
typedef struct {
    const char* appid;
    const uint32_t ep_api_version;
    const void* entry_point;
} FlipperAppPluginDescriptor;

/**
 * @brief Entry point prototype for plugins
 */
typedef const FlipperAppPluginDescriptor* (*FlipperApplicationPluginEntryPoint)(void);

/**
 * @brief Get plugin descriptor for preloaded plugin
 * @param app Application pointer
 * @return Pointer to plugin descriptor
 */
const FlipperAppPluginDescriptor*
    flipper_application_plugin_get_descriptor(FlipperApplication* app);

#ifdef __cplusplus
}
#endif
