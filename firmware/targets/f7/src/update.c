#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>

#include <fatfs.h>
#include <flipper_format/flipper_format.h>

#include <update_util/update_manifest.h>
#include <update_util/update_operation.h>
#include <toolbox/path.h>
#include <toolbox/crc32_calc.h>

#define UPDATE_POINTER_FILE_PATH "/" UPDATE_MANIFEST_POINTER_FILE_NAME

static FATFS* pfs = NULL;

#define CHECK_FRESULT(result)   \
    {                           \
        if((result) != FR_OK) { \
            return false;       \
        }                       \
    }

static bool flipper_update_mount_sd() {
    for(int i = 0; i < BSP_SD_MaxMountRetryCount(); ++i) {
        if(BSP_SD_Init((i % 2) == 0) != MSD_OK) {
            /* Next attempt will be without card reset, let it settle */
            furi_delay_ms(1000);
            continue;
        }

        if(f_mount(pfs, "/", 1) == FR_OK) {
            return true;
        }
    }
    return false;
}

static bool flipper_update_init() {
    furi_hal_clock_init();
    furi_hal_rtc_init();
    furi_hal_interrupt_init();

    furi_hal_spi_config_init();

    MX_FATFS_Init();
    if(!hal_sd_detect()) {
        return false;
    }

    pfs = malloc(sizeof(FATFS));

    return flipper_update_mount_sd();
}

static bool flipper_update_load_stage(const FuriString* work_dir, UpdateManifest* manifest) {
    FIL file;
    FILINFO stat;

    FuriString* loader_img_path;
    loader_img_path = furi_string_alloc_set(work_dir);
    path_append(loader_img_path, furi_string_get_cstr(manifest->staged_loader_file));

    if((f_stat(furi_string_get_cstr(loader_img_path), &stat) != FR_OK) ||
       (f_open(&file, furi_string_get_cstr(loader_img_path), FA_OPEN_EXISTING | FA_READ) !=
        FR_OK)) {
        furi_string_free(loader_img_path);
        return false;
    }
    furi_string_free(loader_img_path);

    void* img = malloc(stat.fsize);
    uint32_t bytes_read = 0;
    const uint16_t MAX_READ = 0xFFFF;

    uint32_t crc = 0;
    do {
        uint16_t size_read = 0;
        if(f_read(&file, img + bytes_read, MAX_READ, &size_read) != FR_OK) {
            break;
        }
        crc = crc32_calc_buffer(crc, img + bytes_read, size_read);
        bytes_read += size_read;
    } while(bytes_read == MAX_READ);

    do {
        if((bytes_read != stat.fsize) || (crc != manifest->staged_loader_crc)) {
            break;
        }

        /* Point of no return. Literally
         *
         * NB: we MUST disable IRQ, otherwise handlers from flash
         * will change global variables (like tick count) 
         * that are located in .data. And we move staged loader 
         * to the same memory region. So, IRQ handlers will mess up 
         * memmove'd .text section and ruin your day. 
         * We don't want that to happen.
         */
        __disable_irq();

        memmove((void*)(SRAM1_BASE), img, stat.fsize);
        LL_SYSCFG_SetRemapMemory(LL_SYSCFG_REMAP_SRAM);
        furi_hal_switch((void*)SRAM1_BASE);
        return true;

    } while(false);

    free(img);
    return false;
}

static bool flipper_update_get_manifest_path(FuriString* out_path) {
    FIL file;
    FILINFO stat;
    uint16_t size_read = 0;
    char manifest_name_buf[UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN] = {0};

    furi_string_reset(out_path);
    CHECK_FRESULT(f_stat(UPDATE_POINTER_FILE_PATH, &stat));
    CHECK_FRESULT(f_open(&file, UPDATE_POINTER_FILE_PATH, FA_OPEN_EXISTING | FA_READ));
    do {
        if(f_read(&file, manifest_name_buf, UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN, &size_read) !=
           FR_OK) {
            break;
        }

        if((size_read == 0) || (size_read == UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN)) {
            break;
        }
        furi_string_set(out_path, manifest_name_buf);
        furi_string_right(out_path, strlen(STORAGE_EXT_PATH_PREFIX));
    } while(0);
    f_close(&file);
    return !furi_string_empty(out_path);
}

static UpdateManifest* flipper_update_process_manifest(const FuriString* manifest_path) {
    FIL file;
    FILINFO stat;

    CHECK_FRESULT(f_stat(furi_string_get_cstr(manifest_path), &stat));
    CHECK_FRESULT(f_open(&file, furi_string_get_cstr(manifest_path), FA_OPEN_EXISTING | FA_READ));

    uint8_t* manifest_data = malloc(stat.fsize);
    uint32_t bytes_read = 0;
    const uint16_t MAX_READ = 0xFFFF;

    do {
        uint16_t size_read = 0;
        if(f_read(&file, manifest_data + bytes_read, MAX_READ, &size_read) != FR_OK) { //-V769
            break;
        }
        bytes_read += size_read;
    } while(bytes_read == MAX_READ);

    UpdateManifest* manifest = NULL;
    do {
        if(bytes_read != stat.fsize) {
            break;
        }

        manifest = update_manifest_alloc();
        if(!update_manifest_init_mem(manifest, manifest_data, bytes_read)) {
            update_manifest_free(manifest);
            manifest = NULL;
        }
    } while(false);

    f_close(&file);
    free(manifest_data);
    return manifest;
}

void flipper_boot_update_exec() {
    if(!flipper_update_init()) {
        return;
    }

    FuriString* work_dir = furi_string_alloc();
    FuriString* manifest_path = furi_string_alloc();

    do {
        if(!flipper_update_get_manifest_path(manifest_path)) {
            break;
        }

        UpdateManifest* manifest = flipper_update_process_manifest(manifest_path);
        if(!manifest) {
            break;
        }

        path_extract_dirname(furi_string_get_cstr(manifest_path), work_dir);
        if(!flipper_update_load_stage(work_dir, manifest)) {
            update_manifest_free(manifest);
        }
    } while(false);
    furi_string_free(manifest_path);
    furi_string_free(work_dir);
    free(pfs);
}
