#include "plugin_manager.h"

#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>
#include <toolbox/path.h>

#include <m-array.h>
#include <m-algo.h>

#include <furi.h>

#define TAG "PluginManager"

ARRAY_DEF(FlipperApplicationList, FlipperApplication*, M_PTR_OPLIST) // NOLINT
#define M_OPL_FlipperApplicationList_t() ARRAY_OPLIST(FlipperApplicationList, M_PTR_OPLIST)

struct PluginManager {
    const char* application_id;
    uint32_t api_version;
    Storage* storage;
    FlipperApplicationList_t libs;
    const ElfApiInterface* api_interface;
};

PluginManager* plugin_manager_alloc(
    const char* application_id,
    uint32_t api_version,
    const ElfApiInterface* api_interface) {
    PluginManager* manager = malloc(sizeof(PluginManager));
    manager->application_id = application_id;
    manager->api_version = api_version;
    manager->api_interface = api_interface ? api_interface : firmware_api_interface;
    manager->storage = furi_record_open(RECORD_STORAGE);
    FlipperApplicationList_init(manager->libs);
    return manager;
}

void plugin_manager_free(PluginManager* manager) {
    furi_check(manager);

    for
        M_EACH(loaded_lib, manager->libs, FlipperApplicationList_t) {
            flipper_application_free(*loaded_lib);
        }
    FlipperApplicationList_clear(manager->libs);
    furi_record_close(RECORD_STORAGE);
    free(manager);
}

static PluginManagerError
    plugin_manager_load_file(PluginManager* manager, const char* path, bool scanning) {
    FlipperApplication* lib = flipper_application_alloc(manager->storage, manager->api_interface);

    PluginManagerError error = PluginManagerErrorNone;
    do {
        FlipperApplicationPreloadStatus preload_res = flipper_application_preload(lib, path);

        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to preload %s", path);
            error = PluginManagerErrorLoaderError;
            break;
        }

        if(!flipper_application_is_plugin(lib)) {
            FURI_LOG_E(TAG, "Not a plugin %s", path);
            error = PluginManagerErrorLoaderError;
            break;
        }

        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(lib);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to load %s", path);
            error = PluginManagerErrorLoaderError;
            break;
        }

        const FlipperAppPluginDescriptor* app_descriptor =
            flipper_application_plugin_get_descriptor(lib);

        if(!app_descriptor) {
            FURI_LOG_E(TAG, "Failed to get descriptor %s", path);
            error = PluginManagerErrorLoaderError;
            break;
        }

        if(strcmp(app_descriptor->appid, manager->application_id) != 0) {
            // Somebody else's plugin. Naming one file and getting this is a mistake; meeting it
            // while scanning a directory is just how a shared directory looks.
            if(scanning) {
                FURI_LOG_D(TAG, "Not ours, skipping %s", path);
            } else {
                FURI_LOG_E(TAG, "Application id mismatch %s", path);
            }
            error = PluginManagerErrorApplicationIdMismatch;
            break;
        }

        if(app_descriptor->ep_api_version != manager->api_version) {
            FURI_LOG_E(TAG, "API version mismatch %s", path);
            error = PluginManagerErrorAPIVersionMismatch;
            break;
        }

        FlipperApplicationList_push_back(manager->libs, lib);
    } while(false);

    if(error != PluginManagerErrorNone) {
        flipper_application_free(lib);
    }

    return error;
}

PluginManagerError plugin_manager_load_single(PluginManager* manager, const char* path) {
    furi_check(manager);
    return plugin_manager_load_file(manager, path, false);
}

PluginManagerError plugin_manager_load_all(PluginManager* manager, const char* path) {
    furi_check(manager);
    File* directory = storage_file_alloc(manager->storage);
    char file_name_buffer[256];
    FuriString* file_name = furi_string_alloc();
    PluginManagerError result = PluginManagerErrorNone;
    do {
        if(!storage_dir_open(directory, path)) {
            FURI_LOG_E(TAG, "Failed to open directory %s", path);
            break;
        }
        while(storage_dir_read(directory, NULL, file_name_buffer, sizeof(file_name_buffer))) {
            furi_string_set(file_name, file_name_buffer);
            if(!furi_string_end_with_str(file_name, ".fal")) {
                continue;
            }

            path_concat(path, file_name_buffer, file_name);
            FURI_LOG_D(TAG, "Loading %s", furi_string_get_cstr(file_name));
            PluginManagerError error =
                plugin_manager_load_file(manager, furi_string_get_cstr(file_name), true);

            // Keep the first real error and keep scanning, so one unusable file does not cost us
            // the plugins listed after it. Somebody else's plugin is not one of those errors.
            if(error != PluginManagerErrorApplicationIdMismatch &&
               result == PluginManagerErrorNone) {
                result = error;
            }
        }

        // storage_dir_read() ends the loop the same way whether it ran out of entries or failed.
        if(storage_file_get_error(directory) != FSE_NOT_EXIST) {
            FURI_LOG_E(
                TAG,
                "Failed to read directory %s: %s",
                path,
                storage_file_get_error_desc(directory));
            if(result == PluginManagerErrorNone) {
                result = PluginManagerErrorLoaderError;
            }
        }
    } while(false);
    storage_dir_close(directory);
    storage_file_free(directory);
    furi_string_free(file_name);
    return result;
}

uint32_t plugin_manager_get_count(PluginManager* manager) {
    furi_check(manager);

    return FlipperApplicationList_size(manager->libs);
}

const FlipperAppPluginDescriptor* plugin_manager_get(PluginManager* manager, uint32_t index) {
    furi_check(manager);

    FlipperApplication* app = *FlipperApplicationList_get(manager->libs, index);
    return flipper_application_plugin_get_descriptor(app);
}

const void* plugin_manager_get_ep(PluginManager* manager, uint32_t index) {
    furi_check(manager);

    const FlipperAppPluginDescriptor* lib_descr = plugin_manager_get(manager, index);
    furi_check(lib_descr);
    return lib_descr->entry_point;
}
