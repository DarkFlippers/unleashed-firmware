#include "flipper_application.h"
#include "flipper_application_i.h"

#define TAG "fapp"

/* For debugger access to app state */
FlipperApplication* last_loaded_app = NULL;

FlipperApplication*
    flipper_application_alloc(Storage* storage, const ElfApiInterface* api_interface) {
    FlipperApplication* app = malloc(sizeof(FlipperApplication));
    app->api_interface = api_interface;
    app->fd = storage_file_alloc(storage);
    app->thread = NULL;
    return app;
}

void flipper_application_free(FlipperApplication* app) {
    furi_assert(app);

    if(app->thread) {
        furi_thread_join(app->thread);
        furi_thread_free(app->thread);
    }

    last_loaded_app = NULL;

    if(app->state.debug_link_size) {
        free(app->state.debug_link);
    }

    if(app->state.mmap_entries) {
        free(app->state.mmap_entries);
    }

    ELFSection_t* sections[] = {&app->text, &app->rodata, &app->data, &app->bss};
    for(size_t i = 0; i < COUNT_OF(sections); i++) {
        flipper_application_free_section(sections[i]);
    }

    storage_file_free(app->fd);

    free(app);
}

/* Parse headers, load manifest */
FlipperApplicationPreloadStatus
    flipper_application_preload(FlipperApplication* app, const char* path) {
    if(!flipper_application_load_elf_headers(app, path) ||
       !flipper_application_load_section_table(app)) {
        return FlipperApplicationPreloadStatusInvalidFile;
    }

    if((app->manifest.base.manifest_magic != FAP_MANIFEST_MAGIC) &&
       (app->manifest.base.manifest_version == FAP_MANIFEST_SUPPORTED_VERSION)) {
        return FlipperApplicationPreloadStatusInvalidManifest;
    }

    if(app->manifest.base.api_version.major != app->api_interface->api_version_major /* ||
       app->manifest.base.api_version.minor > app->api_interface->api_version_minor */) {
        return FlipperApplicationPreloadStatusApiMismatch;
    }

    return FlipperApplicationPreloadStatusSuccess;
}

const FlipperApplicationManifest* flipper_application_get_manifest(FlipperApplication* app) {
    return &app->manifest;
}

FlipperApplicationLoadStatus flipper_application_map_to_memory(FlipperApplication* app) {
    last_loaded_app = app;
    return flipper_application_load_sections(app);
}

const FlipperApplicationState* flipper_application_get_state(FlipperApplication* app) {
    return &app->state;
}

FuriThread* flipper_application_spawn(FlipperApplication* app, void* args) {
    furi_check(app->thread == NULL);

    const FlipperApplicationManifest* manifest = flipper_application_get_manifest(app);
    furi_check(manifest->stack_size > 0);

    app->thread = furi_thread_alloc();
    furi_thread_set_stack_size(app->thread, manifest->stack_size);
    furi_thread_set_name(app->thread, manifest->name);
    furi_thread_set_callback(app->thread, (entry_t*)app->entry);
    furi_thread_set_context(app->thread, args);

    return app->thread;
}

FuriThread* flipper_application_get_thread(FlipperApplication* app) {
    return app->thread;
}

void const* flipper_application_get_entry_address(FlipperApplication* app) {
    return (void*)app->entry;
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
