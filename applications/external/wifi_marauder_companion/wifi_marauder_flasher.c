#include "wifi_marauder_flasher.h"

FuriStreamBuffer* flash_rx_stream; // TODO make safe
WifiMarauderApp* global_app; // TODO make safe
FuriTimer* timer; // TODO make

static uint32_t _remaining_time = 0;
static void _timer_callback(void* context) {
    UNUSED(context);
    if(_remaining_time > 0) {
        _remaining_time--;
    }
}

static esp_loader_error_t _flash_file(WifiMarauderApp* app, char* filepath, uint32_t addr) {
    // TODO cleanup
    esp_loader_error_t err;
    static uint8_t payload[1024];
    File* bin_file = storage_file_alloc(app->storage);

    char user_msg[256];

    // open file
    if(!storage_file_open(bin_file, filepath, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_close(bin_file);
        storage_file_free(bin_file);
        dialog_message_show_storage_error(app->dialogs, "Cannot open file");
        return ESP_LOADER_ERROR_FAIL;
    }

    uint64_t size = storage_file_size(bin_file);

    loader_port_debug_print("Erasing flash...this may take a while\n");
    err = esp_loader_flash_start(addr, size, sizeof(payload));
    if(err != ESP_LOADER_SUCCESS) {
        storage_file_close(bin_file);
        storage_file_free(bin_file);
        snprintf(user_msg, sizeof(user_msg), "Erasing flash failed with error %d\n", err);
        loader_port_debug_print(user_msg);
        return err;
    }

    loader_port_debug_print("Start programming\n");
    uint64_t last_updated = size;
    while(size > 0) {
        if((last_updated - size) > 50000) {
            // inform user every 50k bytes
            // TODO: draw a progress bar next update
            snprintf(user_msg, sizeof(user_msg), "%llu bytes left.\n", size);
            loader_port_debug_print(user_msg);
            last_updated = size;
        }
        size_t to_read = MIN(size, sizeof(payload));
        uint16_t num_bytes = storage_file_read(bin_file, payload, to_read);
        err = esp_loader_flash_write(payload, num_bytes);
        if(err != ESP_LOADER_SUCCESS) {
            snprintf(user_msg, sizeof(user_msg), "Packet could not be written! Error: %u\n", err);
            storage_file_close(bin_file);
            storage_file_free(bin_file);
            loader_port_debug_print(user_msg);
            return err;
        }

        size -= num_bytes;
    }

    loader_port_debug_print("Finished programming\n");

    // TODO verify

    storage_file_close(bin_file);
    storage_file_free(bin_file);

    return ESP_LOADER_SUCCESS;
}

typedef struct {
    SelectedFlashOptions selected;
    const char* description;
    char* path;
    uint32_t addr;
} FlashItem;

static void _flash_all_files(WifiMarauderApp* app) {
    esp_loader_error_t err;
    const int num_steps = app->num_selected_flash_options;

#define NUM_FLASH_ITEMS 6
    FlashItem items[NUM_FLASH_ITEMS] = {
        {SelectedFlashBoot,
         "bootloader",
         app->bin_file_path_boot,
         app->selected_flash_options[SelectedFlashS3Mode] ? ESP_ADDR_BOOT_S3 : ESP_ADDR_BOOT},
        {SelectedFlashPart, "partition table", app->bin_file_path_part, ESP_ADDR_PART},
        {SelectedFlashNvs, "NVS", app->bin_file_path_nvs, ESP_ADDR_NVS},
        {SelectedFlashBootApp0, "boot_app0", app->bin_file_path_boot_app0, ESP_ADDR_BOOT_APP0},
        {SelectedFlashApp, "firmware", app->bin_file_path_app, ESP_ADDR_APP},
        {SelectedFlashCustom, "custom data", app->bin_file_path_custom, 0x0},
        /* if you add more entries, update NUM_FLASH_ITEMS above! */
    };

    char user_msg[256];

    int current_step = 1;
    for(FlashItem* item = &items[0]; item < &items[NUM_FLASH_ITEMS]; ++item) {
        if(app->selected_flash_options[item->selected]) {
            snprintf(
                user_msg,
                sizeof(user_msg),
                "Flashing %s (%d/%d) to address 0x%lx\n",
                item->description,
                current_step++,
                num_steps,
                item->addr);
            loader_port_debug_print(user_msg);
            err = _flash_file(app, item->path, item->addr);
            if(err) {
                break;
            }
        }
    }
}

static int32_t wifi_marauder_flash_bin(void* context) {
    WifiMarauderApp* app = (void*)context;
    esp_loader_error_t err;

    app->flash_worker_busy = true;

    // alloc global objects
    flash_rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
    timer = furi_timer_alloc(_timer_callback, FuriTimerTypePeriodic, app);

    loader_port_debug_print("Connecting\n");
    esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();
    err = esp_loader_connect(&connect_config);
    if(err != ESP_LOADER_SUCCESS) {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "Cannot connect to target. Error: %u\n", err);
        loader_port_debug_print(err_msg);
    }

#if 0 // still getting packet drops with this
    // higher BR
    if(!err) {
        loader_port_debug_print("Increasing speed for faster flash\n");
        err = esp_loader_change_transmission_rate(230400);
        if (err != ESP_LOADER_SUCCESS) {
            char err_msg[256];
            snprintf(
                err_msg,
                sizeof(err_msg),
                "Cannot change transmission rate. Error: %u\n",
                err);
            loader_port_debug_print(err_msg);
        }
        furi_hal_uart_set_br(FuriHalUartIdUSART1, 230400);
    }
#endif

    if(!err) {
        loader_port_debug_print("Connected\n");
        _flash_all_files(app);
#if 0
        loader_port_debug_print("Restoring transmission rate\n");
        furi_hal_uart_set_br(FuriHalUartIdUSART1, 115200);
#endif
        loader_port_debug_print("Done flashing. Please reset the board manually.\n");
    }

    // done
    app->flash_worker_busy = false;

    // cleanup
    furi_stream_buffer_free(flash_rx_stream);
    flash_rx_stream = NULL;
    furi_timer_free(timer);
    return 0;
}

void wifi_marauder_flash_start_thread(WifiMarauderApp* app) {
    global_app = app;

    app->flash_worker = furi_thread_alloc();
    furi_thread_set_name(app->flash_worker, "WifiMarauderFlashWorker");
    furi_thread_set_stack_size(app->flash_worker, 2048);
    furi_thread_set_context(app->flash_worker, app);
    furi_thread_set_callback(app->flash_worker, wifi_marauder_flash_bin);
    furi_thread_start(app->flash_worker);
}

void wifi_marauder_flash_stop_thread(WifiMarauderApp* app) {
    furi_thread_join(app->flash_worker);
    furi_thread_free(app->flash_worker);
}

esp_loader_error_t loader_port_read(uint8_t* data, uint16_t size, uint32_t timeout) {
    size_t read = furi_stream_buffer_receive(flash_rx_stream, data, size, pdMS_TO_TICKS(timeout));
    if(read < size) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_SUCCESS;
    }
}

esp_loader_error_t loader_port_write(const uint8_t* data, uint16_t size, uint32_t timeout) {
    UNUSED(timeout);
    wifi_marauder_uart_tx((uint8_t*)data, size);
    return ESP_LOADER_SUCCESS;
}

void loader_port_enter_bootloader(void) {
    // unimplemented
}

void loader_port_delay_ms(uint32_t ms) {
    furi_delay_ms(ms);
}

void loader_port_start_timer(uint32_t ms) {
    _remaining_time = ms;
    furi_timer_start(timer, pdMS_TO_TICKS(1));
}

uint32_t loader_port_remaining_time(void) {
    return _remaining_time;
}

extern void wifi_marauder_console_output_handle_rx_data_cb(
    uint8_t* buf,
    size_t len,
    void* context); // TODO cleanup
void loader_port_debug_print(const char* str) {
    if(global_app)
        wifi_marauder_console_output_handle_rx_data_cb((uint8_t*)str, strlen(str), global_app);
}

void loader_port_spi_set_cs(uint32_t level) {
    UNUSED(level);
    // unimplemented
}

void wifi_marauder_flash_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    UNUSED(context);
    if(flash_rx_stream) {
        furi_stream_buffer_send(flash_rx_stream, buf, len, 0);
    } else {
        // done flashing
        if(global_app) wifi_marauder_console_output_handle_rx_data_cb(buf, len, global_app);
    }
}