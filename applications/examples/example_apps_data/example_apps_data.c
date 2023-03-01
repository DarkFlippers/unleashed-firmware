#include <furi.h>
#include <storage/storage.h>

// Define log tag
#define TAG "example_apps_data"

// Application entry point
int32_t example_apps_data_main(void* p) {
    // Mark argument as unused
    UNUSED(p);

    // Open storage
    Storage* storage = furi_record_open(RECORD_STORAGE);

    // Allocate file
    File* file = storage_file_alloc(storage);

    // Get the path to the current application data folder
    // That is: /ext/apps_data/<app_name>
    // And it will create folders in the path if they don't exist
    // In this example it will create /ext/apps_data/example_apps_data
    // And file will be /ext/apps_data/example_apps_data/test.txt

    // Open file, write data and close it
    if(!storage_file_open(file, APP_DATA_PATH("test.txt"), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open file");
    }
    if(!storage_file_write(file, "Hello World!", strlen("Hello World!"))) {
        FURI_LOG_E(TAG, "Failed to write to file");
    }
    storage_file_close(file);

    // Deallocate file
    storage_file_free(file);

    // Close storage
    furi_record_close(RECORD_STORAGE);

    return 0;
}
