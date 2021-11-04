#include <stdint.h>
#include <storage/storage.h>

struct FlipperFile {
    File* file;
    Storage* storage;
};

/**
 *  Value write type callback
 */
typedef bool (*flipper_file_cb)(File* file, const char* key, const void* data, uint16_t data_size);

/**
 * 
 * @param flipper_file 
 * @param key 
 * @param cb 
 * @param cb_key 
 * @param cb_data 
 * @param cb_data_size 
 * @return bool 
 */
bool flipper_file_delete_key_and_call(
    FlipperFile* flipper_file,
    const char* key,
    flipper_file_cb cb,
    const char* cb_key,
    const void* cb_data,
    const uint16_t cb_data_size);

/**
 * Value types
 */
typedef enum {
    FlipperFileValueHex,
    FlipperFileValueFloat,
    FlipperFileValueInt32,
    FlipperFileValueUint32,
} FlipperFileValueType;

/**
 * Internal write values function
 * @param file 
 * @param key 
 * @param _data 
 * @param data_size 
 * @param type 
 * @return bool 
 */
bool flipper_file_write_internal(
    File* file,
    const char* key,
    const void* _data,
    const uint16_t data_size,
    FlipperFileValueType type);

/**
 * Internal read values function
 * @param file 
 * @param key 
 * @param _data 
 * @param data_size 
 * @param type 
 * @return bool 
 */
bool flipper_file_read_internal(
    File* file,
    const char* key,
    void* _data,
    const uint16_t data_size,
    FlipperFileValueType type);