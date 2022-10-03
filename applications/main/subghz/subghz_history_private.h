#pragma once

#include "subghz_history.h"

/**
 * @brief Generate filename like 000.tmp
 *
 * @param filename - input parameter
 * @param index - index of file, timestamp doesn't accepted!
 */
void subghz_history_generate_temp_filename(string_t filename, uint32_t index);

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
 * @brief Write temp file fully, without spliting
 * 
 * @param instance - SubGhzHistory*
 * @param current_item - SubGhzHistoryItem*
 * @param dir_path - full path to file
 */
void subghz_history_tmp_write_file_full(
    SubGhzHistory* instance,
    void* current_item,
    string_t dir_path);

/**
 * @brief Write temp splited to lines
 * 
 * @param instance - SubGhzHistory*
 * @param current_item - SubGhzHistoryItem*
 * @param dir_path - full path to file
 * @return true - file saved
 * @return false - error occured
 */
bool subghz_history_tmp_write_file_split(
    SubGhzHistory* instance,
    void* current_item,
    string_t dir_path);