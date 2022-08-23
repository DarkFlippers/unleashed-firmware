#pragma once
#include <furi.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LFRFIDRawFile LFRFIDRawFile;

/**
 * @brief Allocate a new LFRFIDRawFile instance
 * 
 * @param storage 
 * @return LFRFIDRawFile* 
 */
LFRFIDRawFile* lfrfid_raw_file_alloc(Storage* storage);

/**
 * @brief Free a LFRFIDRawFile instance
 * 
 * @param file 
 */
void lfrfid_raw_file_free(LFRFIDRawFile* file);

/**
 * @brief Open RAW file for writing
 * 
 * @param file 
 * @param file_path 
 * @return bool 
 */
bool lfrfid_raw_file_open_write(LFRFIDRawFile* file, const char* file_path);

/**
 * @brief Open RAW file for reading
 * @param file 
 * @param file_path 
 * @return bool 
 */
bool lfrfid_raw_file_open_read(LFRFIDRawFile* file, const char* file_path);

/**
 * @brief Write RAW file header
 * 
 * @param file 
 * @param frequency 
 * @param duty_cycle 
 * @param max_buffer_size 
 * @return bool 
 */
bool lfrfid_raw_file_write_header(
    LFRFIDRawFile* file,
    float frequency,
    float duty_cycle,
    uint32_t max_buffer_size);

/**
 * @brief Write data to RAW file
 * 
 * @param file 
 * @param buffer_data 
 * @param buffer_size 
 * @return bool 
 */
bool lfrfid_raw_file_write_buffer(LFRFIDRawFile* file, uint8_t* buffer_data, size_t buffer_size);

/**
 * @brief Read RAW file header
 * 
 * @param file 
 * @param frequency 
 * @param duty_cycle 
 * @return bool 
 */
bool lfrfid_raw_file_read_header(LFRFIDRawFile* file, float* frequency, float* duty_cycle);

/**
 * @brief Read varint-encoded pair from RAW file
 * 
 * @param file 
 * @param duration 
 * @param pulse 
 * @param pass_end file was wrapped around, can be NULL
 * @return bool 
 */
bool lfrfid_raw_file_read_pair(
    LFRFIDRawFile* file,
    uint32_t* duration,
    uint32_t* pulse,
    bool* pass_end);

#ifdef __cplusplus
}
#endif
