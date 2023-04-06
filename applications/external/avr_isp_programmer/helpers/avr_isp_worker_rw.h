#pragma once

#include <furi_hal.h>

typedef struct AvrIspWorkerRW AvrIspWorkerRW;

typedef void (*AvrIspWorkerRWCallback)(
    void* context,
    const char* name,
    bool detect_chip,
    uint32_t flash_size);

typedef enum {
    AvrIspWorkerRWStatusILDE = 0,
    AvrIspWorkerRWStatusEndReading = 1,
    AvrIspWorkerRWStatusEndVerification = 2,
    AvrIspWorkerRWStatusEndWriting = 3,
    AvrIspWorkerRWStatusEndWritingFuse = 4,

    AvrIspWorkerRWStatusErrorReading = (-1),
    AvrIspWorkerRWStatusErrorVerification = (-2),
    AvrIspWorkerRWStatusErrorWriting = (-3),
    AvrIspWorkerRWStatusErrorWritingFuse = (-4),

    AvrIspWorkerRWStatusReserved = 0x7FFFFFFF, ///< Prevents enum down-size compiler optimization.
} AvrIspWorkerRWStatus;

typedef void (*AvrIspWorkerRWStatusCallback)(void* context, AvrIspWorkerRWStatus status);

AvrIspWorkerRW* avr_isp_worker_rw_alloc(void* context);

void avr_isp_worker_rw_free(AvrIspWorkerRW* instance);

void avr_isp_worker_rw_start(AvrIspWorkerRW* instance);

void avr_isp_worker_rw_stop(AvrIspWorkerRW* instance);

bool avr_isp_worker_rw_is_running(AvrIspWorkerRW* instance);

void avr_isp_worker_rw_set_callback(
    AvrIspWorkerRW* instance,
    AvrIspWorkerRWCallback callback,
    void* context);

void avr_isp_worker_rw_set_callback_status(
    AvrIspWorkerRW* instance,
    AvrIspWorkerRWStatusCallback callback_status,
    void* context_status);

bool avr_isp_worker_rw_detect_chip(AvrIspWorkerRW* instance);

float avr_isp_worker_rw_get_progress_flash(AvrIspWorkerRW* instance);

float avr_isp_worker_rw_get_progress_eeprom(AvrIspWorkerRW* instance);

bool avr_isp_worker_rw_read_dump(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

void avr_isp_worker_rw_read_dump_start(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

bool avr_isp_worker_rw_verification(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

void avr_isp_worker_rw_verification_start(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

bool avr_isp_worker_rw_check_hex(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

bool avr_isp_worker_rw_write_dump(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

void avr_isp_worker_rw_write_dump_start(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

bool avr_isp_worker_rw_write_fuse(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);

void avr_isp_worker_rw_write_fuse_start(
    AvrIspWorkerRW* instance,
    const char* file_path,
    const char* file_name);