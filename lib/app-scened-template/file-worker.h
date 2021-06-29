#pragma once
#include "record-controller.hpp"
#include <sd-card-api.h>
#include <filesystem-api.h>
#include <m-string.h>

/**
 * @brief File operations helper class.
 * Automatically opens API records, shows error message on error.
 */
class FileWorker {
public:
    FileWorker(bool silent = false);
    ~FileWorker();

    RecordController<FS_Api> fs_api;
    RecordController<SdCard_Api> sd_ex_api;

    /**
     * @brief Open file
     * 
     * @param filename 
     * @param access_mode 
     * @param open_mode 
     * @return true on success
     */
    bool open(const char* filename, FS_AccessMode access_mode, FS_OpenMode open_mode);

    /**
     * @brief Close file
     * 
     * @return true on success
     */
    bool close();

    /**
     * @brief Creates a directory. Doesn't show error if directory exist. 
     * 
     * @param dirname 
     * @return true on success
     */
    bool mkdir(const char* dirname);

    /**
     * @brief Removes the file. Doesn't show error if file doesn't exist.
     * 
     * @param filename 
     * @return true on success  
     */
    bool remove(const char* filename);

    /**
     * @brief Reads data from a file.
     * 
     * @param buffer 
     * @param bytes_to_read 
     * @return true on success  
     */
    bool read(void* buffer, uint16_t bytes_to_read = 1);

    /**
     * @brief Reads data from a file until separator or EOF is found. 
     * Moves seek pointer to the next symbol after the separator. The separator is not included in the result.
     * 
     * @param result 
     * @param separator 
     * @return true on success  
     */
    bool read_until(string_t result, char separator = '\n');

    /**
     * @brief Reads data in hexadecimal space-delimited format. For example "AF FF" in a file - [175, 255] in a read buffer.
     * 
     * @param buffer 
     * @param bytes_to_read 
     * @return true on success  
     */
    bool read_hex(uint8_t* buffer, uint16_t bytes_to_read = 1);

    /**
     * @brief Read seek pointer value
     * 
     * @param position 
     * @return true on success  
     */
    bool tell(uint64_t* position);

    /**
     * @brief Set seek pointer value
     * 
     * @param position 
     * @param from_start 
     * @return true on success  
     */
    bool seek(uint64_t position, bool from_start);

    /**
     * @brief Write data to file.
     * 
     * @param buffer 
     * @param bytes_to_write 
     * @return true on success  
     */
    bool write(const void* buffer, uint16_t bytes_to_write = 1);

    /**
     * @brief Write data to file in hexadecimal space-delimited format. For example [175, 255] in a write buffer - "AF FF" in a file.
     * 
     * @param buffer 
     * @param bytes_to_write 
     * @return true on success  
     */
    bool write_hex(const uint8_t* buffer, uint16_t bytes_to_write = 1);

    /**
     * @brief Show system file error message
     * 
     * @param error_text 
     */
    void show_error(const char* error_text);

    /**
     * @brief Show system file select widget
     * 
     * @param path 
     * @param extension 
     * @param result 
     * @param result_size 
     * @param selected_filename 
     * @return true if file was selected
     */
    bool file_select(
        const char* path,
        const char* extension,
        char* result,
        uint8_t result_size,
        char* selected_filename);

private:
    File file;
    bool silent;

    bool check_common_errors();

    void show_error_internal(const char* error_text);

    bool read_internal(void* buffer, uint16_t bytes_to_read = 1);
    bool write_internal(const void* buffer, uint16_t bytes_to_write = 1);
    bool tell_internal(uint64_t* position);
    bool seek_internal(uint64_t position, bool from_start);
};
