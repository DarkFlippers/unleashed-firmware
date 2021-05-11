#pragma once

#include "internal-storage.h"
#include <furi.h>
#include <api-hal.h>
#include <lfs.h>

#define INTERNAL_STORAGE_THREAD_FLAG_CALL_COMPLETE (1)

struct InternalStorage {
    osMessageQueueId_t queue;
    InternalStorageState state;
    const size_t start_address;
    const size_t start_page;
    struct lfs_config config;
    lfs_t lfs;
};

typedef struct {
    const char* key;
    uint8_t* buffer;
    size_t size;
    int ret;
} InternalStorageCommandKey;

typedef void (*InternalStorageCommandFunction)(InternalStorage* internal_storage, void* data);

typedef struct {
    osThreadId thread;
    InternalStorageCommandFunction function;
    void* data;
} InternalStorageCommand;

int internal_storage_device_read(
    const struct lfs_config* c,
    lfs_block_t block,
    lfs_off_t off,
    void* buffer,
    lfs_size_t size);

int internal_storage_device_prog(
    const struct lfs_config* c,
    lfs_block_t block,
    lfs_off_t off,
    const void* buffer,
    lfs_size_t size);

int internal_storage_device_erase(const struct lfs_config* c, lfs_block_t block);

int internal_storage_device_sync(const struct lfs_config* c);

InternalStorage* internal_storage_alloc();

void internal_storage_free(InternalStorage* internal_storage);

int32_t internal_storage_task(void* p);

void _internal_storage_read_key(InternalStorage* internal_storage, InternalStorageCommandKey* data);

void _internal_storage_write_key(
    InternalStorage* internal_storage,
    InternalStorageCommandKey* data);
