#include "flipper_application.h"
#include "elf/elf_file.h"
#include <notification/notification_messages.h>
#include "application_assets.h"
#include <loader/firmware_api/firmware_api.h>

#include <m-list.h>

#define TAG "Fap"

struct FlipperApplication {
    ELFDebugInfo state;
    FlipperApplicationManifest manifest;
    ELFFile* elf;
    FuriThread* thread;
    void* ep_thread_args;
};

/********************** Debugger access to loader state **********************/

LIST_DEF(FlipperApplicationList, const FlipperApplication*, M_POD_OPLIST);

FlipperApplicationList_t flipper_application_loaded_app_list = {0};
static bool flipper_application_loaded_app_list_initialized = false;

static void flipper_application_list_add_app(const FlipperApplication* app) {
    furi_assert(app);

    if(!flipper_application_loaded_app_list_initialized) {
        FlipperApplicationList_init(flipper_application_loaded_app_list);
        flipper_application_loaded_app_list_initialized = true;
    }
    FlipperApplicationList_push_back(flipper_application_loaded_app_list, app);
}

static void flipper_application_list_remove_app(const FlipperApplication* app) {
    furi_assert(flipper_application_loaded_app_list_initialized);
    furi_assert(app);

    FlipperApplicationList_it_t it;
    for(FlipperApplicationList_it(it, flipper_application_loaded_app_list);
        !FlipperApplicationList_end_p(it);
        FlipperApplicationList_next(it)) {
        if(*FlipperApplicationList_ref(it) == app) {
            FlipperApplicationList_remove(flipper_application_loaded_app_list, it);
            break;
        }
    }
}

/*****************************************************************************/

FlipperApplication*
    flipper_application_alloc(Storage* storage, const ElfApiInterface* api_interface) {
    FlipperApplication* app = malloc(sizeof(FlipperApplication));
    app->elf = elf_file_alloc(storage, api_interface);
    app->thread = NULL;
    app->ep_thread_args = NULL;
    return app;
}

bool flipper_application_is_plugin(FlipperApplication* app) {
    return app->manifest.stack_size == 0;
}

void flipper_application_free(FlipperApplication* app) {
    furi_assert(app);

    if(app->thread) {
        furi_thread_join(app->thread);
        furi_thread_free(app->thread);
    }

    if(app->state.entry) {
        flipper_application_list_remove_app(app);
    }

    elf_file_clear_debug_info(&app->state);

    if(elf_file_is_init_complete(app->elf)) {
        elf_file_call_fini(app->elf);
    }

    elf_file_free(app->elf);

    if(app->ep_thread_args) {
        free(app->ep_thread_args);
        app->ep_thread_args = NULL;
    }

    free(app);
}

static FlipperApplicationPreloadStatus
    flipper_application_validate_manifest(FlipperApplication* app) {
    if(!flipper_application_manifest_is_valid(&app->manifest)) {
        return FlipperApplicationPreloadStatusInvalidManifest;
    }

    if(!flipper_application_manifest_is_target_compatible(&app->manifest)) {
        return FlipperApplicationPreloadStatusTargetMismatch;
    }

    if(!flipper_application_manifest_is_compatible(
           &app->manifest, elf_file_get_api_interface(app->elf))) {
        return FlipperApplicationPreloadStatusApiMismatch;
    }

    return FlipperApplicationPreloadStatusSuccess;
}

static bool flipper_application_process_manifest_section(
    File* file,
    size_t offset,
    size_t size,
    void* context) {
    FlipperApplicationManifest* manifest = context;

    if(size < sizeof(FlipperApplicationManifest)) {
        return false;
    }

    if(manifest == NULL) {
        return true;
    }

    return storage_file_seek(file, offset, true) &&
           storage_file_read(file, manifest, size) == size;
}

// we can't use const char* as context because we will lose the const qualifier
typedef struct {
    const char* path;
} FlipperApplicationPreloadAssetsContext;

static bool flipper_application_process_assets_section(
    File* file,
    size_t offset,
    size_t size,
    void* context) {
    FlipperApplicationPreloadAssetsContext* preload_context = context;
    return flipper_application_assets_load(file, preload_context->path, offset, size);
}

static FlipperApplicationPreloadStatus
    flipper_application_load(FlipperApplication* app, const char* path, bool load_full) {
    if(!elf_file_open(app->elf, path)) {
        return FlipperApplicationPreloadStatusInvalidFile;
    }

    // if we are loading full file
    if(load_full) {
        // load section table
        if(!elf_file_load_section_table(app->elf)) {
            return FlipperApplicationPreloadStatusInvalidFile;
        }

        // load assets section
        FlipperApplicationPreloadAssetsContext preload_context = {.path = path};
        if(elf_process_section(
               app->elf,
               ".fapassets",
               flipper_application_process_assets_section,
               &preload_context) == ElfProcessSectionResultCannotProcess) {
            return FlipperApplicationPreloadStatusInvalidFile;
        }
    }

    // load manifest section
    if(elf_process_section(
           app->elf, ".fapmeta", flipper_application_process_manifest_section, &app->manifest) !=
       ElfProcessSectionResultSuccess) {
        return FlipperApplicationPreloadStatusInvalidFile;
    }

    return flipper_application_validate_manifest(app);
}

/* Parse headers, load manifest */
FlipperApplicationPreloadStatus
    flipper_application_preload_manifest(FlipperApplication* app, const char* path) {
    return flipper_application_load(app, path, false);
}

/* Parse headers, load full file */
FlipperApplicationPreloadStatus
    flipper_application_preload(FlipperApplication* app, const char* path) {
    return flipper_application_load(app, path, true);
}

const FlipperApplicationManifest* flipper_application_get_manifest(FlipperApplication* app) {
    return &app->manifest;
}

FlipperApplicationLoadStatus flipper_application_map_to_memory(FlipperApplication* app) {
    ELFFileLoadStatus status = elf_file_load_sections(app->elf);

    switch(status) {
    case ELFFileLoadStatusSuccess:
        elf_file_init_debug_info(app->elf, &app->state);
        flipper_application_list_add_app(app);
        return FlipperApplicationLoadStatusSuccess;
    case ELFFileLoadStatusNoFreeMemory:
        return FlipperApplicationLoadStatusNoFreeMemory;
    case ELFFileLoadStatusMissingImports:
        return FlipperApplicationLoadStatusMissingImports;
    default:
        return FlipperApplicationLoadStatusUnspecifiedError;
    }
}

static int32_t flipper_application_thread(void* context) {
    furi_assert(context);
    FlipperApplication* app = (FlipperApplication*)context;

    elf_file_call_init(app->elf);

    FlipperApplicationEntryPoint entry_point = elf_file_get_entry_point(app->elf);
    int32_t ret_code = entry_point(app->ep_thread_args);

    elf_file_call_fini(app->elf);

    // wait until all notifications from RAM are completed
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    const NotificationSequence sequence_empty = {
        NULL,
    };
    notification_message_block(notifications, &sequence_empty);
    furi_record_close(RECORD_NOTIFICATION);

    return ret_code;
}

FuriThread* flipper_application_alloc_thread(FlipperApplication* app, const char* args) {
    furi_check(app->thread == NULL);
    furi_check(!flipper_application_is_plugin(app));

    if(app->ep_thread_args) {
        free(app->ep_thread_args);
    }

    if(args) {
        app->ep_thread_args = strdup(args);
    } else {
        app->ep_thread_args = NULL;
    }

    const FlipperApplicationManifest* manifest = flipper_application_get_manifest(app);
    app->thread = furi_thread_alloc_ex(
        manifest->name, manifest->stack_size, flipper_application_thread, app);

    return app->thread;
}

static const char* preload_status_strings[] = {
    [FlipperApplicationPreloadStatusSuccess] = "Success",
    [FlipperApplicationPreloadStatusUnspecifiedError] = "Unknown error",
    [FlipperApplicationPreloadStatusInvalidFile] = "Invalid file",
    [FlipperApplicationPreloadStatusInvalidManifest] = "Invalid file manifest",
    [FlipperApplicationPreloadStatusApiMismatch] = "API version mismatch",
    [FlipperApplicationPreloadStatusTargetMismatch] = "Hardware target mismatch",
};

static const char* load_status_strings[] = {
    [FlipperApplicationLoadStatusSuccess] = "Success",
    [FlipperApplicationLoadStatusUnspecifiedError] = "Unknown error",
    [FlipperApplicationLoadStatusNoFreeMemory] = "Out of memory",
    [FlipperApplicationLoadStatusMissingImports] = "Found unsatisfied imports",
};

const char* flipper_application_preload_status_to_string(FlipperApplicationPreloadStatus status) {
    if(status >= COUNT_OF(preload_status_strings) || preload_status_strings[status] == NULL) {
        return "Unknown error";
    }
    return preload_status_strings[status];
}

const char* flipper_application_load_status_to_string(FlipperApplicationLoadStatus status) {
    if(status >= COUNT_OF(load_status_strings) || load_status_strings[status] == NULL) {
        return "Unknown error";
    }
    return load_status_strings[status];
}

const FlipperAppPluginDescriptor*
    flipper_application_plugin_get_descriptor(FlipperApplication* app) {
    if(!flipper_application_is_plugin(app)) {
        return NULL;
    }

    if(!elf_file_is_init_complete(app->elf)) {
        elf_file_call_init(app->elf);
    }

    typedef const FlipperAppPluginDescriptor* (*get_lib_descriptor_t)(void);
    get_lib_descriptor_t lib_ep = elf_file_get_entry_point(app->elf);
    furi_check(lib_ep);

    const FlipperAppPluginDescriptor* lib_descriptor = lib_ep();

    FURI_LOG_D(
        TAG,
        "Library for %s, API v. %lu loaded",
        lib_descriptor->appid,
        lib_descriptor->ep_api_version);

    return lib_descriptor;
}

bool flipper_application_load_name_and_icon(
    FuriString* path,
    Storage* storage,
    uint8_t** icon_ptr,
    FuriString* item_name) {
    FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);

    FlipperApplicationPreloadStatus preload_res =
        flipper_application_preload_manifest(app, furi_string_get_cstr(path));

    bool load_success = false;

    if(preload_res == FlipperApplicationPreloadStatusSuccess) {
        const FlipperApplicationManifest* manifest = flipper_application_get_manifest(app);
        if(manifest->has_icon) {
            memcpy(*icon_ptr, manifest->icon, FAP_MANIFEST_MAX_ICON_SIZE);
        }
        furi_string_set(item_name, manifest->name);
        load_success = true;
    } else {
        FURI_LOG_E(TAG, "Failed to preload %s", furi_string_get_cstr(path));
        load_success = false;
    }

    flipper_application_free(app);
    return load_success;
}