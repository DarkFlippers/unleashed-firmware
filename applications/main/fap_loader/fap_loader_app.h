#pragma once
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FapLoader FapLoader;

/**
 * @brief Load name and icon from FAP file.
 * 
 * @param path Path to FAP file.
 * @param storage Storage instance.
 * @param icon_ptr Icon pointer.
 * @param item_name Application name.
 * @return true if icon and name were loaded successfully.
 */
bool fap_loader_load_name_and_icon(
    FuriString* path,
    Storage* storage,
    uint8_t** icon_ptr,
    FuriString* item_name);

#ifdef __cplusplus
}
#endif