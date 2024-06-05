#include <furi.h>
#include "../test.h" // IWYU pragma: keep
#include <update_util/resources/manifest.h>

#define TAG "Manifest"

MU_TEST(manifest_type_test) {
    mu_assert(ResourceManifestEntryTypeUnknown == 0, "ResourceManifestEntryTypeUnknown != 0\r\n");
    mu_assert(ResourceManifestEntryTypeVersion == 1, "ResourceManifestEntryTypeVersion != 1\r\n");
    mu_assert(
        ResourceManifestEntryTypeTimestamp == 2, "ResourceManifestEntryTypeTimestamp != 2\r\n");
    mu_assert(
        ResourceManifestEntryTypeDirectory == 3, "ResourceManifestEntryTypeDirectory != 3\r\n");
    mu_assert(ResourceManifestEntryTypeFile == 4, "ResourceManifestEntryTypeFile != 4\r\n");
}

MU_TEST(manifest_iteration_test) {
    bool result = true;
    size_t counters[5] = {0};

    Storage* storage = furi_record_open(RECORD_STORAGE);
    ResourceManifestReader* manifest_reader = resource_manifest_reader_alloc(storage);
    do {
        // Open manifest file
        if(!resource_manifest_reader_open(manifest_reader, EXT_PATH("unit_tests/Manifest_test"))) {
            result = false;
            break;
        }

        // Iterate forward
        ResourceManifestEntry* entry_ptr = NULL;
        while((entry_ptr = resource_manifest_reader_next(manifest_reader))) {
            FURI_LOG_D(TAG, "F:%u:%s", entry_ptr->type, furi_string_get_cstr(entry_ptr->name));
            if(entry_ptr->type > 4) {
                mu_fail("entry_ptr->type > 4\r\n");
                result = false;
                break;
            }
            counters[entry_ptr->type]++;
        }
        if(!result) break;

        // Iterate backward
        while((entry_ptr = resource_manifest_reader_previous(manifest_reader))) {
            FURI_LOG_D(TAG, "B:%u:%s", entry_ptr->type, furi_string_get_cstr(entry_ptr->name));
            if(entry_ptr->type > 4) {
                mu_fail("entry_ptr->type > 4\r\n");
                result = false;
                break;
            }
            counters[entry_ptr->type]--;
        }
    } while(false);

    resource_manifest_reader_free(manifest_reader);
    furi_record_close(RECORD_STORAGE);

    mu_assert(counters[ResourceManifestEntryTypeUnknown] == 0, "Unknown counter != 0\r\n");
    mu_assert(counters[ResourceManifestEntryTypeVersion] == 0, "Version counter != 0\r\n");
    mu_assert(counters[ResourceManifestEntryTypeTimestamp] == 0, "Timestamp counter != 0\r\n");
    mu_assert(counters[ResourceManifestEntryTypeDirectory] == 0, "Directory counter != 0\r\n");
    mu_assert(counters[ResourceManifestEntryTypeFile] == 0, "File counter != 0\r\n");

    mu_assert(result, "Manifest forward iterate failed\r\n");
}

MU_TEST_SUITE(manifest_suite) {
    MU_RUN_TEST(manifest_type_test);
    MU_RUN_TEST(manifest_iteration_test);
}

int run_minunit_test_manifest(void) {
    MU_RUN_SUITE(manifest_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_manifest)
