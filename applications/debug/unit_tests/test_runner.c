#include "test_runner.h"

#include "tests/test_api.h"

#include <cli/cli.h>
#include <toolbox/path.h>
#include <loader/loader.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

#include <loader/firmware_api/firmware_api.h>
#include <flipper_application/flipper_application.h>
#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/plugins/composite_resolver.h>

extern const ElfApiInterface* const unit_tests_api_interface;

#define TAG "TestRunner"

#define PLUGINS_PATH "/ext/apps_data/unit_tests/plugins"

struct TestRunner {
    Storage* storage;
    Loader* loader;
    NotificationApp* notification;

    // Temporary used things
    Cli* cli;
    FuriString* args;

    // ELF related stuff
    CompositeApiResolver* composite_resolver;

    // Report data
    int minunit_run;
    int minunit_assert;
    int minunit_fail;
    int minunit_status;
};

TestRunner* test_runner_alloc(Cli* cli, FuriString* args) {
    TestRunner* instance = malloc(sizeof(TestRunner));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->loader = furi_record_open(RECORD_LOADER);
    instance->notification = furi_record_open(RECORD_NOTIFICATION);

    instance->cli = cli;
    instance->args = args;

    instance->composite_resolver = composite_api_resolver_alloc();
    composite_api_resolver_add(instance->composite_resolver, firmware_api_interface);
    composite_api_resolver_add(instance->composite_resolver, unit_tests_api_interface);

    return instance;
}

void test_runner_free(TestRunner* instance) {
    furi_assert(instance);

    composite_api_resolver_free(instance->composite_resolver);

    furi_record_close(RECORD_NOTIFICATION);
    instance->notification = NULL;

    furi_record_close(RECORD_LOADER);
    instance->loader = NULL;

    furi_record_close(RECORD_STORAGE);
    instance->storage = NULL;

    free(instance);
}

#define TEST_RUNNER_TMP_DIR            EXT_PATH(".tmp")
#define TEST_RUNNER_TMP_UNIT_TESTS_DIR TEST_RUNNER_TMP_DIR "/unit_tests"

static bool test_runner_run_plugin(TestRunner* instance, const char* path) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Loading %s", path);
    FlipperApplication* lib = flipper_application_alloc(
        instance->storage, composite_api_resolver_get(instance->composite_resolver));

    bool result = false;
    instance->minunit_fail = -1;
    do {
        FlipperApplicationPreloadStatus preload_res = flipper_application_preload(lib, path);

        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to preload %s, %d", path, preload_res);
            break;
        }

        if(!flipper_application_is_plugin(lib)) {
            FURI_LOG_E(TAG, "Not a plugin %s", path);
            break;
        }

        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(lib);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to load %s", path);
            break;
        }

        const FlipperAppPluginDescriptor* app_descriptor =
            flipper_application_plugin_get_descriptor(lib);

        const TestApi* test = app_descriptor->entry_point;

        instance->minunit_fail = test->run();

        instance->minunit_run += test->get_minunit_run();
        instance->minunit_assert += test->get_minunit_assert();
        instance->minunit_status += test->get_minunit_status();

        result = (instance->minunit_fail == 0);
    } while(false);

    flipper_application_free(lib);

    return result;
}

static void test_runner_run_internal(TestRunner* instance) {
    furi_assert(instance);

    char file_name_buffer[256];
    FuriString* file_name = furi_string_alloc();
    FuriString* file_basename = furi_string_alloc();
    File* directory = storage_file_alloc(instance->storage);

    do {
        if(!storage_simply_mkdir(instance->storage, TEST_RUNNER_TMP_DIR)) {
            FURI_LOG_E(TAG, "Cannot create dir %s", TEST_RUNNER_TMP_DIR);
            break;
        }

        if(!storage_simply_mkdir(instance->storage, TEST_RUNNER_TMP_UNIT_TESTS_DIR)) {
            FURI_LOG_E(TAG, "Cannot create dir %s", TEST_RUNNER_TMP_UNIT_TESTS_DIR);
            break;
        }

        if(!storage_dir_open(directory, PLUGINS_PATH)) {
            FURI_LOG_E(TAG, "Failed to open directory %s", PLUGINS_PATH);
            break;
        }

        while(true) {
            if(cli_cmd_interrupt_received(instance->cli)) {
                break;
            }

            if(!storage_dir_read(directory, NULL, file_name_buffer, sizeof(file_name_buffer))) {
                break;
            }

            furi_string_set(file_name, file_name_buffer);
            if(!furi_string_end_with_str(file_name, ".fal")) {
                continue;
            }

            path_concat(PLUGINS_PATH, file_name_buffer, file_name);

            path_extract_filename(file_name, file_basename, true);
            const char* file_basename_cstr = furi_string_get_cstr(file_basename);

            bool result = true;
            if(furi_string_size(instance->args)) {
                if(furi_string_cmp_str(instance->args, file_basename_cstr) == 0) {
                    result = test_runner_run_plugin(instance, furi_string_get_cstr(file_name));
                } else {
                    printf("Skipping %s\r\n", file_basename_cstr);
                }
            } else {
                result = test_runner_run_plugin(instance, furi_string_get_cstr(file_name));
            }

            if(!result) {
                printf("Failed to execute test: %s\r\n", file_basename_cstr);
                break;
            }
        }
    } while(false);

    storage_dir_close(directory);
    storage_file_free(directory);
    furi_string_free(file_name);
    furi_string_free(file_basename);
}

void test_runner_run(TestRunner* instance) {
    furi_assert(instance);

    // TODO FL-3491: lock device while test running
    if(loader_is_locked(instance->loader)) {
        printf("RPC: stop all applications to run tests\r\n");
        notification_message(instance->notification, &sequence_blink_magenta_100);
    } else {
        notification_message_block(instance->notification, &sequence_set_only_blue_255);

        uint32_t heap_before = memmgr_get_free_heap();
        uint32_t cycle_counter = furi_get_tick();

        test_runner_run_internal(instance);

        if(instance->minunit_run != 0) {
            printf("\r\nFailed tests: %d\r\n", instance->minunit_fail);

            // Time report
            cycle_counter = (furi_get_tick() - cycle_counter);
            printf("Consumed: %lu ms\r\n", cycle_counter);

            // Wait for tested services and apps to deallocate memory
            furi_delay_ms(200);
            uint32_t heap_after = memmgr_get_free_heap();
            printf("Leaked: %ld\r\n", heap_before - heap_after);

            // Final Report
            if(instance->minunit_fail == 0) {
                notification_message(instance->notification, &sequence_success);
                printf("Status: PASSED\r\n");
            } else {
                notification_message(instance->notification, &sequence_error);
                printf("Status: FAILED\r\n");
            }
        }
    }
}
