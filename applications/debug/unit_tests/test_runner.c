#include "test_runner.h"

#include "tests/test_api.h"

#include <toolbox/cli/cli_command.h>
#include <toolbox/path.h>
#include <toolbox/pipe.h>
#include <loader/loader.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

#include <loader/firmware_api/firmware_api.h>
#include <flipper_application/flipper_application.h>
#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/plugins/composite_resolver.h>
#include <flipper_application/plugins/plugin_manager.h>

extern const ElfApiInterface* const unit_tests_api_interface;

#define TAG "TestRunner"

#define PLUGINS_PATH "/ext/apps_data/unit_tests/plugins"

struct TestRunner {
    Storage* storage;
    Loader* loader;
    NotificationApp* notification;

    // Temporary used things
    PipeSide* pipe;
    FuriString* args;

    // ELF related stuff
    CompositeApiResolver* composite_resolver;

    // Report data
    int minunit_run;
    int minunit_assert;
    int minunit_fail;
    // Suites that never got as far as running a test, which no minunit counter can express
    uint32_t suites_not_run;
    // Set when the run stops early, so the report cannot pass off "no failures" as "all passed"
    bool incomplete;
};

TestRunner* test_runner_alloc(PipeSide* pipe, FuriString* args) {
    TestRunner* instance = malloc(sizeof(TestRunner));
    *instance = (TestRunner){0};

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->loader = furi_record_open(RECORD_LOADER);
    instance->notification = furi_record_open(RECORD_NOTIFICATION);

    instance->pipe = pipe;
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

static void test_runner_run_plugin(TestRunner* instance, const char* name, const char* path) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Loading %s", path);

    // One manager per suite: PluginManager unloads only when freed, and the suites do not all fit
    // in RAM at once
    PluginManager* manager = plugin_manager_alloc(
        APPID, API_VERSION, composite_api_resolver_get(instance->composite_resolver));

    // Checks the appid and the API version as well, so a .fal that is not a test suite is turned
    // away rather than having whatever it does export called as one
    PluginManagerError error = plugin_manager_load_single(manager, path);

    const TestApi* test = NULL;
    if(error == PluginManagerErrorNone) {
        test = plugin_manager_get_ep(manager, 0);
    }

    if(!test || !test->run) {
        // printf, not FURI_LOG: the log level is a user setting, and a suite that stopped loading
        // is the failure that went unnoticed for three months (#1185)
        printf("Could not load %s: error %d\r\n", name, error);
        instance->suites_not_run++;
        plugin_manager_free(manager);
        return;
    }

    int failed = test->run();

    instance->minunit_run += test->get_minunit_run();
    instance->minunit_assert += test->get_minunit_assert();
    // Sum: one suite's count is not the run's
    instance->minunit_fail += failed;

    plugin_manager_free(manager);

    if(failed != 0) {
        printf("Failed tests in %s: %d\r\n", name, failed);
    }
}

static void test_runner_run_internal(TestRunner* instance) {
    furi_assert(instance);

    char file_name_buffer[256];
    FuriString* file_name = furi_string_alloc();
    FuriString* file_basename = furi_string_alloc();
    File* directory = storage_file_alloc(instance->storage);

    do {
        if(!storage_simply_mkdir(instance->storage, TEST_RUNNER_TMP_DIR)) {
            printf("Cannot create dir %s\r\n", TEST_RUNNER_TMP_DIR);
            instance->incomplete = true;
            break;
        }

        if(!storage_simply_mkdir(instance->storage, TEST_RUNNER_TMP_UNIT_TESTS_DIR)) {
            printf("Cannot create dir %s\r\n", TEST_RUNNER_TMP_UNIT_TESTS_DIR);
            instance->incomplete = true;
            break;
        }

        if(!storage_dir_open(directory, PLUGINS_PATH)) {
            printf("Cannot open %s - are the SD card resources deployed?\r\n", PLUGINS_PATH);
            instance->incomplete = true;
            break;
        }

        // Every suite runs, including the ones after a failing one: a broken suite must not hide
        // the rest, which is how 13 of them went untested before #1199
        while(true) {
            if(cli_is_pipe_broken_or_is_etx_next_char(instance->pipe)) {
                instance->incomplete = true;
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

            if(furi_string_size(instance->args) &&
               furi_string_cmp_str(instance->args, file_basename_cstr) != 0) {
                printf("Skipping %s\r\n", file_basename_cstr);
                continue;
            }

            test_runner_run_plugin(instance, file_basename_cstr, furi_string_get_cstr(file_name));
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
        // A status line even here, so nothing reading this output has to treat silence as a result
        printf("Status: NOT RUN\r\n");
        notification_message(instance->notification, &sequence_blink_magenta_100);
        return;
    }

    notification_message_block(instance->notification, &sequence_set_only_blue_255);

    uint32_t heap_before = memmgr_get_free_heap();
    uint32_t cycle_counter = furi_get_tick();

    test_runner_run_internal(instance);

    printf("\r\nTests run: %d, asserts: %d\r\n", instance->minunit_run, instance->minunit_assert);
    printf("Failed tests: %d\r\n", instance->minunit_fail);
    if(instance->suites_not_run) {
        printf("Suites that could not run: %lu\r\n", instance->suites_not_run);
    }

    // Time report
    cycle_counter = (furi_get_tick() - cycle_counter);
    printf("Consumed: %lu ms\r\n", cycle_counter);

    // Wait for tested services and apps to deallocate memory
    furi_delay_ms(200);
    uint32_t heap_after = memmgr_get_free_heap();
    printf("Leaked: %ld\r\n", heap_before - heap_after);

    if(instance->minunit_run == 0) {
        printf("No tests were run\r\n");
    }

    // Final Report. A run that tested nothing, or stopped early, has established nothing: the
    // absence of a failure is not a pass, and reporting it as one is the bug this guard exists for
    if(instance->minunit_fail == 0 && instance->suites_not_run == 0 &&
       instance->minunit_run != 0 && !instance->incomplete) {
        notification_message(instance->notification, &sequence_success);
        printf("Status: PASSED\r\n");
    } else {
        notification_message(instance->notification, &sequence_error);
        printf("Status: FAILED\r\n");
    }
}
