#include "test_runner.h"

#include "tests/test_api.h"

#include <toolbox/cli/cli_command.h>
#include <toolbox/path.h>
#include <toolbox/pipe.h>
#include <loader/loader.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

#include <loader/firmware_api/firmware_api.h>
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
    int suites_not_run;
    // Set when the run did not complete, so "no failures" cannot be reported as "all passed"
    bool incomplete;
};

TestRunner* test_runner_alloc(PipeSide* pipe, FuriString* args) {
    TestRunner* instance = malloc(sizeof(TestRunner));

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

    // One manager per suite: PluginManager only unloads on free, so a shared one would keep
    // every suite mapped
    PluginManager* manager = plugin_manager_alloc(
        APPID, API_VERSION, composite_api_resolver_get(instance->composite_resolver));

    // plugin_manager, not flipper_application: it rejects a .fal whose appid or API version is
    // not ours, instead of calling whatever it exports as a TestApi
    PluginManagerError error = plugin_manager_load_single(manager, path);

    const TestApi* test = NULL;
    if(error == PluginManagerErrorNone) {
        test = plugin_manager_get_ep(manager, 0);
    }

    if(test && test->run && test->get_minunit_run && test->get_minunit_assert) {
        int failed = test->run();

        instance->minunit_run += test->get_minunit_run();
        instance->minunit_assert += test->get_minunit_assert();
        instance->minunit_fail += failed;

        if(failed != 0) {
            printf("Failed tests in %s: %d\r\n", name, failed);
        }
    } else {
        // printf: FURI_LOG goes to the log channel, not to this command's output
        printf("Could not load %s: error %d\r\n", name, error);
        instance->suites_not_run++;
        instance->incomplete = true;
    }

    plugin_manager_free(manager);
}

static bool test_runner_prepare(TestRunner* instance, File* directory) {
    if(!storage_simply_mkdir(instance->storage, TEST_RUNNER_TMP_DIR)) {
        printf("Cannot create dir %s\r\n", TEST_RUNNER_TMP_DIR);
        return false;
    }

    if(!storage_simply_mkdir(instance->storage, TEST_RUNNER_TMP_UNIT_TESTS_DIR)) {
        printf("Cannot create dir %s\r\n", TEST_RUNNER_TMP_UNIT_TESTS_DIR);
        return false;
    }

    if(!storage_dir_open(directory, PLUGINS_PATH)) {
        printf("Cannot open %s - are the SD card resources deployed?\r\n", PLUGINS_PATH);
        return false;
    }

    return true;
}

static void test_runner_run_internal(TestRunner* instance) {
    furi_assert(instance);

    char file_name_buffer[256];
    FuriString* file_name = furi_string_alloc();
    FuriString* file_basename = furi_string_alloc();
    File* directory = storage_file_alloc(instance->storage);

    if(!test_runner_prepare(instance, directory)) {
        instance->incomplete = true;
    } else {
        size_t suites_started = 0;

        // Carry on after a failing suite: one broken suite once cost the run the other 28
        while(true) {
            if(cli_is_pipe_broken_or_is_etx_next_char(instance->pipe)) {
                printf("Interrupted\r\n");
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

            suites_started++;
            test_runner_run_plugin(instance, file_basename_cstr, furi_string_get_cstr(file_name));
        }

        // Naming a suite that is not there is a typo, not an empty pass
        if(suites_started == 0 && furi_string_size(instance->args)) {
            printf("No suite named %s\r\n", furi_string_get_cstr(instance->args));
            instance->incomplete = true;
        }
    }

    storage_dir_close(directory);
    storage_file_free(directory);
    furi_string_free(file_name);
    furi_string_free(file_basename);
}

/** Print the whole report on every path, and return whether the run may be called a pass. */
static bool test_runner_print_report(TestRunner* instance, uint32_t heap_before, uint32_t start) {
    printf("\r\nTests run: %d, asserts: %d\r\n", instance->minunit_run, instance->minunit_assert);
    printf("Failed tests: %d\r\n", instance->minunit_fail);
    if(instance->suites_not_run) {
        printf("Suites that could not run: %d\r\n", instance->suites_not_run);
    }

    printf("Consumed: %lu ms\r\n", furi_get_tick() - start);

    // Wait for tested services and apps to deallocate memory
    furi_delay_ms(200);
    uint32_t heap_after = memmgr_get_free_heap();
    // Clamped: a run can end with more free heap than it started with, and an unsigned difference
    // renders that as four billion bytes leaked
    uint32_t leaked = (heap_after < heap_before) ? heap_before - heap_after : 0;
    printf("Leaked: %lu\r\n", leaked);

    if(instance->minunit_run == 0) {
        printf("No tests were run\r\n");
        instance->incomplete = true;
    }

    // Not-failed is not passed: nothing run, a suite that never loaded, or an aborted run FAILS
    bool passed = (instance->minunit_fail == 0) && !instance->incomplete;
    printf("Status: %s\r\n", passed ? "PASSED" : "FAILED");

    return passed;
}

void test_runner_run(TestRunner* instance) {
    furi_assert(instance);

    uint32_t heap_before = memmgr_get_free_heap();
    uint32_t start = furi_get_tick();

    // TODO FL-3491: lock device while test running
    bool locked = loader_is_locked(instance->loader);
    if(locked) {
        printf("RPC: stop all applications to run tests\r\n");
        instance->incomplete = true;
    } else {
        notification_message_block(instance->notification, &sequence_set_only_blue_255);
        test_runner_run_internal(instance);
    }

    bool passed = test_runner_print_report(instance, heap_before, start);

    // Always ends on a sequence, so the blue "running" light cannot be left on
    const NotificationSequence* result = &sequence_error;
    if(locked) {
        result = &sequence_blink_magenta_100;
    } else if(passed) {
        result = &sequence_success;
    }
    notification_message(instance->notification, result);
}
