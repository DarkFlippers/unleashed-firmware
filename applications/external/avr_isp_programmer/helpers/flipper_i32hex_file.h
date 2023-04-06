#pragma once

#include <furi_hal.h>

typedef struct FlipperI32HexFile FlipperI32HexFile;

typedef enum {
    FlipperI32HexFileStatusOK = 0,
    FlipperI32HexFileStatusData = 2,
    FlipperI32HexFileStatusUdateAddr = 3,
    FlipperI32HexFileStatusEofFile = 4,
    FlipperI32HexFileStatusOpenFileWrite = 5,
    FlipperI32HexFileStatusOpenFileRead = 6,

    // Errors
    FlipperI32HexFileStatusErrorCrc = (-1),
    FlipperI32HexFileStatusErrorOverflow = (-2),
    FlipperI32HexFileStatusErrorData = (-3),
    FlipperI32HexFileStatusErrorUnsupportedCommand = (-4),
    FlipperI32HexFileStatusErrorNoOpenFile = (-5),
    FlipperI32HexFileStatusErrorFileWrite = (-6),
    FlipperI32HexFileStatusErrorFileRead = (-7),

    FlipperI32HexFileStatusReserved =
        0x7FFFFFFF, ///< Prevents enum down-size compiler optimization.
} FlipperI32HexFileStatus;

typedef struct {
    FlipperI32HexFileStatus status;
    uint32_t data_size;
} FlipperI32HexFileRet;

FlipperI32HexFile* flipper_i32hex_file_open_write(const char* name, uint32_t start_addr);

FlipperI32HexFile* flipper_i32hex_file_open_read(const char* name);

void flipper_i32hex_file_close(FlipperI32HexFile* instance);

FlipperI32HexFileRet flipper_i32hex_file_bin_to_i32hex_set_data(
    FlipperI32HexFile* instance,
    uint8_t* data,
    uint32_t data_size);

FlipperI32HexFileRet flipper_i32hex_file_bin_to_i32hex_set_end_line(FlipperI32HexFile* instance);

const char* flipper_i32hex_file_get_string(FlipperI32HexFile* instance);

void flipper_i32hex_file_bin_to_i32hex_set_addr(FlipperI32HexFile* instance, uint32_t addr);

bool flipper_i32hex_file_check(FlipperI32HexFile* instance);

FlipperI32HexFileRet flipper_i32hex_file_i32hex_to_bin_get_data(
    FlipperI32HexFile* instance,
    uint8_t* data,
    uint32_t data_size);