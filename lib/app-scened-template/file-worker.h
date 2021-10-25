#pragma once
#include <m-string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief File operations helper class.
 * Automatically opens API records, shows error message on error.
 */
typedef struct FileWorker FileWorker;

/**
 * @brief Allocate FileWorker
 * 
 * @param silent do not show errors except from file_worker_show_error fn
 * @return FileWorker* 
 */
FileWorker* file_worker_alloc(bool silent);

/**
 * @brief free FileWorker
 * 
 * @param file_worker 
 */
void file_worker_free(FileWorker* file_worker);

/**
 * @brief Open file
 * 
 * @param file_worker FileWorker instance 
 * @param filename 
 * @param access_mode 
 * @param open_mode 
 * @return true on success
 */
bool file_worker_open(
    FileWorker* file_worker,
    const char* filename,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode);

/**
 * @brief Close file
 * 
 * @param file_worker FileWorker instance 
 * @return true on success
 */
bool file_worker_close(FileWorker* file_worker);

/**
 * @brief Creates a directory. Doesn't show error if directory exist. 
 * 
 * @param file_worker FileWorker instance 
 * @param dirname 
 * @return true on success
 */
bool file_worker_mkdir(FileWorker* file_worker, const char* dirname);

/**
 * @brief Removes the file. Doesn't show error if file doesn't exist.
 * 
 * @param file_worker FileWorker instance 
 * @param filename 
 * @return true on success  
 */
bool file_worker_remove(FileWorker* file_worker, const char* filename);

/**
 * @brief Get next free filename.
 * 
 * @param file_worker FileWorker instance 
 * @param dirname 
 * @param filename 
 * @param fileextension 
 * @param nextfilename return name
 */
void file_worker_get_next_filename(
    FileWorker* file_worker,
    const char* dirname,
    const char* filename,
    const char* fileextension,
    string_t nextfilename);

/**
 * @brief Reads data from a file.
 * 
 * @param file_worker FileWorker instance 
 * @param buffer 
 * @param bytes_to_read 
 * @return true on success  
 */
bool file_worker_read(FileWorker* file_worker, void* buffer, uint16_t bytes_to_read);

/**
 * @brief Reads data from a file until separator or EOF is found. 
 * Moves seek pointer to the next symbol after the separator. The separator is not included in the result.
 * 
 * @param file_worker FileWorker instance 
 * @param result 
 * @param separator 
 * @return true on success  
 */
bool file_worker_read_until(FileWorker* file_worker, string_t result, char separator);

/**
 * @brief Reads data in hexadecimal space-delimited format. For example "AF FF" in a file - [175, 255] in a read buffer.
 * 
 * @param file_worker FileWorker instance 
 * @param buffer 
 * @param bytes_to_read 
 * @return true on success  
 */
bool file_worker_read_hex(FileWorker* file_worker, uint8_t* buffer, uint16_t bytes_to_read);

/**
 * @brief Read seek pointer value
 * 
 * @param file_worker FileWorker instance 
 * @param position 
 * @return true on success  
 */
bool file_worker_tell(FileWorker* file_worker, uint64_t* position);

/**
 * @brief Set seek pointer value
 * 
 * @param file_worker FileWorker instance 
 * @param position 
 * @param from_start 
 * @return true on success  
 */
bool file_worker_seek(FileWorker* file_worker, uint64_t position, bool from_start);

/**
 * @brief Write data to file.
 * 
 * @param file_worker FileWorker instance 
 * @param buffer 
 * @param bytes_to_write 
 * @return true on success  
 */
bool file_worker_write(FileWorker* file_worker, const void* buffer, uint16_t bytes_to_write);

/**
 * @brief Write data to file in hexadecimal space-delimited format. For example [175, 255] in a write buffer - "AF FF" in a file.
 * 
 * @param file_worker FileWorker instance 
 * @param buffer 
 * @param bytes_to_write 
 * @return true on success  
 */
bool file_worker_write_hex(FileWorker* file_worker, const uint8_t* buffer, uint16_t bytes_to_write);

/**
 * @brief Show system file error message
 * 
 * @param file_worker FileWorker instance 
 * @param error_text 
 */
void file_worker_show_error(FileWorker* file_worker, const char* error_text);

/**
 * @brief Show system file select widget
 * 
 * @param file_worker FileWorker instance 
 * @param path path to directory
 * @param extension file extension to be offered for selection
 * @param selected_filename buffer where the selected filename will be saved
 * @param selected_filename_size and the size of this buffer
 * @param preselected_filename filename to be preselected
 * @return bool whether a file was selected
 */
bool file_worker_file_select(
    FileWorker* file_worker,
    const char* path,
    const char* extension,
    char* selected_filename,
    uint8_t selected_filename_size,
    const char* preselected_filename);

/**
 * @brief Reads data from a file until separator or EOF is found.
 * The separator is included in the result.
 *
 * @param file_worker FileWorker instance
 * @param str_result
 * @param file_buf
 * @param file_buf_cnt
 * @param max_length
 * @param separator
 * @return true on success
 */
bool file_worker_read_until_buffered(
    FileWorker* file_worker,
    string_t str_result,
    char* file_buf,
    size_t* file_buf_cnt,
    size_t max_length,
    char separator);

/**
 * @brief Gets value from key
 *
 * @param file_worker FileWorker instance
 * @param key key
 * @param delimeter key-value delimeter
 * @param value value for given key
 * @return true on success
 */
bool file_worker_get_value_from_key(
    FileWorker* file_worker,
    string_t key,
    char delimiter,
    string_t value);

/**
 * @brief Check whether file exist or not
 *
 * @param file_worker FileWorker instance
 * @param filename
 * @param exist - flag to show file exist
 * @return true on success
 */
bool file_worker_is_file_exist(FileWorker* file_worker, const char* filename, bool* exist);

/**
 * @brief Rename file or directory
 *
 * @param file_worker FileWorker instance
 * @param old_filename
 * @param new_filename
 * @return true on success
 */
bool file_worker_rename(FileWorker* file_worker, const char* old_path, const char* new_path);

/**
 * @brief Check errors
 *
 * @param file_worker FileWorker instance
 * @return true on success
 */
bool file_worker_check_errors(FileWorker* file_worker);

#ifdef __cplusplus
}
#endif
