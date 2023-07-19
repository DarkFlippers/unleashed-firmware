#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_spi.h>
#include <stdio.h>
#include <string.h>
#include <nrf24.h>
#include <furi.h>
#include <furi_hal.h>
#include <toolbox/stream/file_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* name;
    uint8_t hid;
    uint8_t mod;
} MJDuckyKey;

typedef struct {
    FuriMutex* mutex;
    bool ducky_err;
    bool addr_err;
    bool is_thread_running;
    bool is_ducky_running;
    bool is_nrf24_connected;
    bool close_thread_please;
    Storage* storage;
    FuriThread* mjthread;
    Stream* file_stream;
} PluginState;

void mj_process_ducky_script(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    char* script,
    PluginState* plugin_state);

#ifdef __cplusplus
}
#endif