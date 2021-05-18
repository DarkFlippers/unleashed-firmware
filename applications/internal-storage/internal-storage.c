#include "internal-storage-i.h"

int internal_storage_device_read(
    const struct lfs_config* c,
    lfs_block_t block,
    lfs_off_t off,
    void* buffer,
    lfs_size_t size) {
    InternalStorage* internal_storage = c->context;
    size_t address = internal_storage->start_address + block * c->block_size + off;

    FURI_LOG_D(
        "internal-storage",
        "Device read: block %d, off %d, buffer: %p, size %d, translated address: %p",
        block,
        off,
        buffer,
        size,
        address);

    memcpy(buffer, (void*)address, size);

    return 0;
}

int internal_storage_device_prog(
    const struct lfs_config* c,
    lfs_block_t block,
    lfs_off_t off,
    const void* buffer,
    lfs_size_t size) {
    InternalStorage* internal_storage = c->context;
    size_t address = internal_storage->start_address + block * c->block_size + off;

    FURI_LOG_D(
        "internal-storage",
        "Device prog: block %d, off %d, buffer: %p, size %d, translated address: %p",
        block,
        off,
        buffer,
        size,
        address);

    int ret = 0;
    while(size > 0) {
        if(!api_hal_flash_write_dword(address, *(uint64_t*)buffer)) {
            ret = -1;
            break;
        }
        address += c->prog_size;
        buffer += c->prog_size;
        size -= c->prog_size;
    }

    return ret;
}

int internal_storage_device_erase(const struct lfs_config* c, lfs_block_t block) {
    InternalStorage* internal_storage = c->context;
    size_t page = internal_storage->start_page + block;

    FURI_LOG_D("internal-storage", "Device erase: page %d, translated page: %d", block, page);

    if(api_hal_flash_erase(page, 1)) {
        return 0;
    } else {
        return -1;
    }
}

int internal_storage_device_sync(const struct lfs_config* c) {
    FURI_LOG_D("internal-storage", "Device sync: skipping, cause ");
    return 0;
}

InternalStorage* internal_storage_alloc() {
    InternalStorage* internal_storage = furi_alloc(sizeof(InternalStorage));

    internal_storage->queue = osMessageQueueNew(8, sizeof(InternalStorageCommand), NULL);

    // Internal storage start address
    internal_storage->state = InternalStorageStateInitializing;

    // Internal storage start address
    *(size_t*)(&internal_storage->start_address) = api_hal_flash_get_free_page_start_address();
    *(size_t*)(&internal_storage->start_page) =
        (internal_storage->start_address - api_hal_flash_get_base()) /
        api_hal_flash_get_page_size();

    // LFS configuration
    // Glue and context
    internal_storage->config.context = internal_storage;
    internal_storage->config.read = internal_storage_device_read;
    internal_storage->config.prog = internal_storage_device_prog;
    internal_storage->config.erase = internal_storage_device_erase;
    internal_storage->config.sync = internal_storage_device_sync;
    // Block device description
    internal_storage->config.read_size = api_hal_flash_get_read_block_size();
    internal_storage->config.prog_size = api_hal_flash_get_write_block_size();
    internal_storage->config.block_size = api_hal_flash_get_page_size();
    internal_storage->config.block_count = api_hal_flash_get_free_page_count();
    internal_storage->config.block_cycles = api_hal_flash_get_cycles_count();
    internal_storage->config.cache_size = 16;
    internal_storage->config.lookahead_size = 16;

    return internal_storage;
}

void internal_storage_free(InternalStorage* internal_storage) {
    furi_assert(internal_storage);
    free(internal_storage);
}

int32_t internal_storage_task(void* p) {
    FURI_LOG_I("internal-storage", "Starting");
    InternalStorage* internal_storage = internal_storage_alloc();
    FURI_LOG_I(
        "internal-storage",
        "Config: start %p, read %d, write %d, page size: %d, page count: %d, cycles: %d",
        internal_storage->start_address,
        internal_storage->config.read_size,
        internal_storage->config.prog_size,
        internal_storage->config.block_size,
        internal_storage->config.block_count,
        internal_storage->config.block_cycles);

    int err;
    ApiHalBootFlag boot_flags = api_hal_boot_get_flags();
    if(boot_flags & ApiHalBootFlagFactoryReset) {
        // Factory reset
        err = lfs_format(&internal_storage->lfs, &internal_storage->config);
        if(err == 0) {
            FURI_LOG_I("internal-storage", "Factory reset: Format successful, trying to mount");
            api_hal_boot_set_flags(boot_flags & ~ApiHalBootFlagFactoryReset);
            err = lfs_mount(&internal_storage->lfs, &internal_storage->config);
            if(err == 0) {
                FURI_LOG_I("internal-storage", "Factory reset: Mounted");
                internal_storage->state = InternalStorageStateReady;
            } else {
                FURI_LOG_E("internal-storage", "Factory reset: Mount after format failed");
                internal_storage->state = InternalStorageStateBroken;
            }
        } else {
            FURI_LOG_E("internal-storage", "Factory reset: Format failed");
            internal_storage->state = InternalStorageStateBroken;
        }
    } else {
        // Normal
        err = lfs_mount(&internal_storage->lfs, &internal_storage->config);
        if(err == 0) {
            FURI_LOG_I("internal-storage", "Mounted");
            internal_storage->state = InternalStorageStateReady;
        } else {
            FURI_LOG_E("internal-storage", "Mount failed, formatting");
            err = lfs_format(&internal_storage->lfs, &internal_storage->config);
            if(err == 0) {
                FURI_LOG_I("internal-storage", "Format successful, trying to mount");
                err = lfs_mount(&internal_storage->lfs, &internal_storage->config);
                if(err == 0) {
                    FURI_LOG_I("internal-storage", "Mounted");
                    internal_storage->state = InternalStorageStateReady;
                } else {
                    FURI_LOG_E("internal-storage", "Mount after format failed");
                    internal_storage->state = InternalStorageStateBroken;
                }
            } else {
                FURI_LOG_E("internal-storage", "Format failed");
                internal_storage->state = InternalStorageStateBroken;
            }
        }
    }

    furi_record_create("internal-storage", internal_storage);

    InternalStorageCommand command;
    while(1) {
        furi_check(
            osMessageQueueGet(internal_storage->queue, &command, NULL, osWaitForever) == osOK);
        command.function(internal_storage, command.data);
        osThreadFlagsSet(command.thread, INTERNAL_STORAGE_THREAD_FLAG_CALL_COMPLETE);
    }

    lfs_unmount(&internal_storage->lfs);
    internal_storage_free(internal_storage);

    return 0;
}

void _internal_storage_read_key(InternalStorage* internal_storage, InternalStorageCommandKey* data) {
    lfs_file_t file;
    int ret = lfs_file_open(&internal_storage->lfs, &file, data->key, LFS_O_RDONLY);
    if(ret == 0) {
        ret = lfs_file_read(&internal_storage->lfs, &file, data->buffer, data->size);
        lfs_file_close(&internal_storage->lfs, &file);
    }
    data->ret = ret;
}

int internal_storage_read_key(
    InternalStorage* internal_storage,
    const char* key,
    uint8_t* buffer,
    size_t size) {
    osThreadId_t caller_thread = osThreadGetId();
    if(caller_thread == 0) {
        return -1;
    }

    InternalStorageCommandKey data = {.key = key, .buffer = buffer, .size = size, .ret = 0};
    InternalStorageCommand command = {
        .thread = caller_thread,
        .function = (InternalStorageCommandFunction)_internal_storage_read_key,
        .data = &data,
    };
    furi_check(osMessageQueuePut(internal_storage->queue, &command, 0, osWaitForever) == osOK);
    osThreadFlagsWait(INTERNAL_STORAGE_THREAD_FLAG_CALL_COMPLETE, osFlagsWaitAny, osWaitForever);
    return data.ret;
}

void _internal_storage_write_key(
    InternalStorage* internal_storage,
    InternalStorageCommandKey* data) {
    lfs_file_t file;
    int ret = lfs_file_open(
        &internal_storage->lfs, &file, data->key, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if(ret == 0) {
        ret = lfs_file_write(&internal_storage->lfs, &file, data->buffer, data->size);
        lfs_file_close(&internal_storage->lfs, &file);
    }
    data->ret = ret;
}

int internal_storage_write_key(
    InternalStorage* internal_storage,
    const char* key,
    uint8_t* buffer,
    size_t size) {
    osThreadId_t caller_thread = osThreadGetId();
    if(caller_thread == 0) {
        return -1;
    }

    InternalStorageCommandKey data = {.key = key, .buffer = buffer, .size = size, .ret = 0};
    InternalStorageCommand command = {
        .thread = caller_thread,
        .function = (InternalStorageCommandFunction)_internal_storage_write_key,
        .data = &data,
    };
    furi_check(osMessageQueuePut(internal_storage->queue, &command, 0, osWaitForever) == osOK);
    osThreadFlagsWait(INTERNAL_STORAGE_THREAD_FLAG_CALL_COMPLETE, osFlagsWaitAny, osWaitForever);
    return data.ret;
}
