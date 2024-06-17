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
    FlipperApplicationPreloadStatusInvalidFile,
    FlipperApplicationPreloadStatusNotEnoughMemory,
    FlipperApplicationPreloadStatusInvalidManifest,
    FlipperApplicationPreloadStatusApiTooOld,
    FlipperApplicationPreloadStatusApiTooNew,
    FlipperApplicationPreloadStatusTargetMismatch,
} FlipperApplicationPreloadStatus;

typedef enum {
    FlipperApplicationLoadStatusSuccess = 0,
    FlipperApplicationLoadStatusUnspecifiedError,
    FlipperApplicationLoadStatusMissingImports,
} FlipperApplicationLoadStatus;

/** Get text description of preload status
 * @param status Status code
 * @return String pointer to description
 */
const char* flipper_application_preload_status_to_string(FlipperApplicationPreloadStatus status);

/** Get text description of load status
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

/** Initialize FlipperApplication object
 * @param storage Storage instance
 * @param api_interface ELF API interface to use for pre-loading and symbol resolving
 * @return Application instance
 */
FlipperApplication*
    flipper_application_alloc(Storage* storage, const ElfApiInterface* api_interface);

/** Destroy FlipperApplication object
 * @param app Application pointer
 */
void flipper_application_free(FlipperApplication* app);

/** Validate elf file and load application metadata
 *
 * @param      app   Application pointer
 * @param[in]  path  The path to fap file
 *
 * @return     Preload result code
 */
FlipperApplicationPreloadStatus
    flipper_application_preload(FlipperApplication* app, const char* path);

/** Validate elf file and load application manifest
 *
 * @param      app   Application pointer
 * @param[in]  path  The path to fap file
 *
 * @return     Preload result code
 */
FlipperApplicationPreloadStatus
    flipper_application_preload_manifest(FlipperApplication* app, const char* path);

/** Get pointer to application manifest for preloaded application
 * @param app Application pointer
 * @return Pointer to application manifest
 */
const FlipperApplicationManifest* flipper_application_get_manifest(FlipperApplication* app);

/** Load sections and process relocations for already pre-loaded application
 * @param app Application pointer
 * @return Load result code
 */
FlipperApplicationLoadStatus flipper_application_map_to_memory(FlipperApplication* app);

/** Allocate application thread at entry point address, using app name and
 * stack size from metadata. Returned thread isn't started yet. 
 * Can be only called once for application instance.
 * @param app Applicaiton pointer
 * @param args Args to pass to app's entry point
 * @return Created thread
 */
FuriThread* flipper_application_alloc_thread(FlipperApplication* app, const char* args);

/** Check if application is a plugin (not a runnable standalone app)
 * @param app Application pointer
 * @return true if application is a plugin, false otherwise
 */
bool flipper_application_is_plugin(FlipperApplication* app);

/** Entry point prototype for standalone applications
 */
typedef int32_t (*FlipperApplicationEntryPoint)(void*);

/** An object that describes a plugin - must be returned by plugin's entry point 
 */
typedef struct {
    const char* appid;
    const uint32_t ep_api_version;
    const void* entry_point;
} FlipperAppPluginDescriptor;

/** Entry point prototype for plugins
 */
typedef const FlipperAppPluginDescriptor* (*FlipperApplicationPluginEntryPoint)(void);

/** Get plugin descriptor for preloaded plugin
 * @param app Application pointer
 * @return Pointer to plugin descriptor
 */
const FlipperAppPluginDescriptor*
    flipper_application_plugin_get_descriptor(FlipperApplication* app);

/** Load name and icon from FAP file.
 * 
 * @param path Path to FAP file.
 * @param storage Storage instance.
 * @param icon_ptr Icon pointer.
 * @param item_name Application name.
 * @return true if icon and name were loaded successfully.
 */
bool flipper_application_load_name_and_icon(
    FuriString* path,
    Storage* storage,
    uint8_t** icon_ptr,
    FuriString* item_name);

#ifdef __cplusplus
}
#endif
