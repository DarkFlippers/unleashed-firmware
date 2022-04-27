#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>

#include <fatfs.h>
#include <flipper_format/flipper_format.h>

#include <update_util/update_manifest.h>
#include <toolbox/path.h>
#include <toolbox/crc32_calc.h>

static FATFS* pfs = NULL;

static const char FS_ROOT_PATH[] = "/";

#define CHECK_FRESULT(result)   \
    {                           \
        if((result) != FR_OK) { \
            return false;       \
        }                       \
    }

static bool flipper_update_init() {
    furi_hal_clock_init();
    furi_hal_rtc_init();
    furi_hal_interrupt_init();
    furi_hal_delay_init();

    furi_hal_spi_init();

    MX_FATFS_Init();
    if(!hal_sd_detect()) {
        return false;
    }

    if(BSP_SD_Init(true)) {
        return false;
    }

    pfs = malloc(sizeof(FATFS));
    CHECK_FRESULT(f_mount(pfs, FS_ROOT_PATH, 1));
    return true;
}

static bool flipper_update_load_stage(const string_t work_dir, UpdateManifest* manifest) {
    FIL file;
    FILINFO stat;

    string_t loader_img_path;
    string_init_set(loader_img_path, work_dir);
    path_append(loader_img_path, string_get_cstr(manifest->staged_loader_file));

    if((f_stat(string_get_cstr(loader_img_path), &stat) != FR_OK) ||
       (f_open(&file, string_get_cstr(loader_img_path), FA_OPEN_EXISTING | FA_READ) != FR_OK)) {
        string_clear(loader_img_path);
        return false;
    }
    string_clear(loader_img_path);

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

static bool flipper_update_get_work_directory(string_t out_dir) {
    const uint32_t update_index = furi_hal_rtc_get_register(FuriHalRtcRegisterUpdateFolderFSIndex);
    if(update_index == 0) {
        string_set(out_dir, UPDATE_DIR_DEFAULT_REL_PATH);
        return true;
    }

    DIR dir;
    UINT entry_idx = 0;
    FILINFO fno;
    CHECK_FRESULT(f_opendir(&dir, UPDATE_DIR_DEFAULT_REL_PATH));
    string_set(out_dir, UPDATE_DIR_DEFAULT_REL_PATH);

    while(f_readdir(&dir, &fno) == FR_OK) {
        entry_idx++;
        if(fno.fname[0] == '\0') {
            return false;
        }
        if(entry_idx == update_index) {
            path_append(out_dir, fno.fname);
            return true;
        }
    }

    string_reset(out_dir);
    return false;
}

static UpdateManifest* flipper_update_process_manifest(const string_t work_dir) {
    FIL file;
    FILINFO stat;

    string_t manifest_path;
    string_init_set(manifest_path, work_dir);
    path_append(manifest_path, UPDATE_MANIFEST_DEFAULT_NAME);

    CHECK_FRESULT(f_stat(string_get_cstr(manifest_path), &stat));
    CHECK_FRESULT(f_open(&file, string_get_cstr(manifest_path), FA_OPEN_EXISTING | FA_READ));

    uint8_t* manifest_data = malloc(stat.fsize);
    uint32_t bytes_read = 0;
    const uint16_t MAX_READ = 0xFFFF;

    do {
        uint16_t size_read = 0;
        if(f_read(&file, manifest_data + bytes_read, MAX_READ, &size_read) != FR_OK) {
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

    string_clear(manifest_path);
    free(manifest_data);
    return manifest;
}

void flipper_boot_update_exec() {
    if(!flipper_update_init()) {
        return;
    }

    string_t work_dir;
    string_init(work_dir);
    do {
        if(!flipper_update_get_work_directory(work_dir)) {
            break;
        }

        UpdateManifest* manifest = flipper_update_process_manifest(work_dir);
        if(!manifest) {
            break;
        }

        if(!flipper_update_load_stage(work_dir, manifest)) {
            update_manifest_free(manifest);
        }
    } while(false);
    string_clear(work_dir);
    free(pfs);
}
