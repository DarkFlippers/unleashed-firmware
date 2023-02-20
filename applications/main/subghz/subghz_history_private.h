#pragma once

#include "subghz_history.h"
#include <toolbox/stream/stream.h>

/**
 * @brief Generate filename like 000.tmp
 *
 * @param index - index of file, timestamp doesn't accepted!
 */
FuriString* subghz_history_generate_temp_filename(uint32_t index);

/**
 * @brief Check if directory for temporary files is exists
 *
 * @param instance SubGhzHistory*
 * @return true
 * @return false
 */
bool subghz_history_is_tmp_dir_exists(SubGhzHistory* instance);

/**
 * @brief Check SD card and create temporary dir if not exists,
 * Result write_tmp_files without this unstable work is GUARANTEED
 *
 * @param instance - SubGhzHistory*
 * @return - true all ok
 * @return - false we have a problems
 */
bool subghz_history_check_sdcard(SubGhzHistory* instance);

/**
 * @brief Recursive delete dir and files and create new temp dir
 *
 * @param instance - SubGhzHistory*
 * @return true - if all ok
 * @return false - if something failed
 */
void subghz_history_clear_tmp_dir(SubGhzHistory* instance);

/**
 * @brief Free item and free all resources
 *
 * @param current_item - SubGhzHistoryItem*
 */
void subghz_history_item_free(void* current_item);

/**
 * @brief free all items in array
 *
 * @param instance
 */
void subghz_history_clean_item_array(SubGhzHistory* instance);

/**
 * @brief Write temp file fully, without splitting
 * 
 * @param instance - SubGhzHistory*
 * @param current_item - SubGhzHistoryItem*
 * @param dir_path - full path to file
 */
void subghz_history_tmp_write_file_full(
    SubGhzHistory* instance,
    void* current_item,
    FuriString* dir_path);

/**
 * @brief Write temp spited to lines
 * 
 * @param instance - SubGhzHistory*
 * @param current_item - SubGhzHistoryItem*
 * @param dir_path - full path to file
 * @return true - file saved
 * @return false - error occurred
 */
bool subghz_history_tmp_write_file_split(
    SubGhzHistory* instance,
    void* current_item,
    const char* dir_path);

/**
 * @brief generate random value
 * 
 * @param min - min value
 * @param max - max value
 * @return uint32_t 
 */
uint32_t subghz_history_rand_range(uint32_t min, uint32_t max);

/**
 * @brief write random noise signals to file applying to max line value
 * 
 * @param file - Stream*
 * @param is_negative_start - first value is negative or positive?
 * @param current_position - 0 if started from beginning
 * @param empty_line - add RAW_Data to this line
 * @return true 
 * @return false 
 */
bool subghz_history_write_file_noise(
    Stream* file,
    bool is_negative_start,
    size_t current_position,
    bool empty_line);

/**
 * @brief taken from flipper_format_stream_read_value_line but takes only one int32 value
 * 
 * @param stream - Stream*
 * @param _data - int32_t* output data
 * @param data_size - size of data
 * @return true 
 * @return false 
 */
bool subghz_history_read_int32(Stream* stream, int32_t* _data, const uint16_t data_size);

/**
 * @brief write payload to file spliting by lines
 * 
 * @param src - Stream* of source
 * @param file - Stream* of file
 * @param is_negative_start - first value is negative or positive?
 * @param current_position - by default is 0 but in this value returned last position of payload
 * @return true 
 * @return false 
 */
bool subghz_history_write_file_data(
    Stream* src,
    Stream* file,
    bool* is_negative_start,
    size_t* current_position);

/**
 * @brief taken from flipper_format_stream_read_valid_key
 * 
 * @param stream - Stream*
 * @param key - FuriString* output value
 * @return true 
 * @return false 
 */
bool subghz_history_stream_read_valid_key(Stream* stream, FuriString* key);

/**
 * @brief taken from flipper_format_stream_seek_to_key
 * 
 * @param stream  - Stream*
 * @param key - key
 * @param strict_mode - false 
 * @return true 
 * @return false 
 */
bool subghz_history_stream_seek_to_key(Stream* stream, const char* key, bool strict_mode);

/**
 * @brief taken from flipper_format_stream_read_value
 * 
 * @param stream - Stream*
 * @param value - FuriString* output value
 * @param last - return position is last flag
 * @return true 
 * @return false 
 */
bool subghz_history_stream_read_value(Stream* stream, FuriString* value, bool* last);
