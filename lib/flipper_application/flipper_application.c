#include "flipper_application.h"
#include "elf/elf_file.h"
#include <notification/notification_messages.h>

#define TAG "fapp"

struct FlipperApplication {
    ELFDebugInfo state;
    FlipperApplicationManifest manifest;
    ELFFile* elf;
    FuriThread* thread;
};

/* For debugger access to app state */
FlipperApplication* last_loaded_app = NULL;

FlipperApplication*
    flipper_application_alloc(Storage* storage, const ElfApiInterface* api_interface) {
    FlipperApplication* app = malloc(sizeof(FlipperApplication));
    app->elf = elf_file_alloc(storage, api_interface);
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

    elf_file_clear_debug_info(&app->state);
    elf_file_free(app->elf);
    free(app);
}

static FlipperApplicationPreloadStatus
    flipper_application_validate_manifest(FlipperApplication* app) {
    if(!flipper_application_manifest_is_valid(&app->manifest)) {
        return FlipperApplicationPreloadStatusInvalidManifest;
    }

    if(!flipper_application_manifest_is_compatible(
           &app->manifest, elf_file_get_api_interface(app->elf))) {
        return FlipperApplicationPreloadStatusApiMismatch;
    }

    return FlipperApplicationPreloadStatusSuccess;
}

/* Parse headers, load manifest */
FlipperApplicationPreloadStatus
    flipper_application_preload_manifest(FlipperApplication* app, const char* path) {
    if(!elf_file_open(app->elf, path) || !elf_file_load_manifest(app->elf, &app->manifest)) {
        return FlipperApplicationPreloadStatusInvalidFile;
    }

    return flipper_application_validate_manifest(app);
}

/* Parse headers, load full file */
FlipperApplicationPreloadStatus
    flipper_application_preload(FlipperApplication* app, const char* path) {
    if(!elf_file_open(app->elf, path) || !elf_file_load_section_table(app->elf, &app->manifest)) {
        return FlipperApplicationPreloadStatusInvalidFile;
    }

    return flipper_application_validate_manifest(app);
}

const FlipperApplicationManifest* flipper_application_get_manifest(FlipperApplication* app) {
    return &app->manifest;
}

FlipperApplicationLoadStatus flipper_application_map_to_memory(FlipperApplication* app) {
    last_loaded_app = app;
    ELFFileLoadStatus status = elf_file_load_sections(app->elf);

    switch(status) {
    case ELFFileLoadStatusSuccess:
        elf_file_init_debug_info(app->elf, &app->state);
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
    elf_file_pre_run(last_loaded_app->elf);
    int32_t result = elf_file_run(last_loaded_app->elf, context);
    elf_file_post_run(last_loaded_app->elf);

    // wait until all notifications from RAM are completed
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    const NotificationSequence sequence_empty = {
        NULL,
    };
    notification_message_block(notifications, &sequence_empty);
    furi_record_close(RECORD_NOTIFICATION);

    return result;
}

FuriThread* flipper_application_spawn(FlipperApplication* app, void* args) {
    furi_check(app->thread == NULL);

    const FlipperApplicationManifest* manifest = flipper_application_get_manifest(app);
    furi_check(manifest->stack_size > 0);

    app->thread = furi_thread_alloc_ex(
        manifest->name, manifest->stack_size, flipper_application_thread, args);

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
