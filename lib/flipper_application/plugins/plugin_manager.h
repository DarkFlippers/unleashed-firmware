#pragma once

#include <flipper_application/flipper_application.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Object that manages plugins for an application
 * Implements mass loading of plugins and provides access to their descriptors
 */
typedef struct PluginManager PluginManager;

typedef enum {
    PluginManagerErrorNone = 0,
    PluginManagerErrorLoaderError,
    PluginManagerErrorApplicationIdMismatch,
    PluginManagerErrorAPIVersionMismatch,
} PluginManagerError;

/**
 * @brief Allocates new PluginManager
 * @param application_id Application ID filter - only plugins with matching ID will be loaded
 * @param api_version Application API version filter - only plugins with matching API version
 * @param api_interface Application API interface - used to resolve plugins' API imports
 *  If plugin uses private application's API, use CompoundApiInterface
 * @return new PluginManager instance
 */
PluginManager* plugin_manager_alloc(
    const char* application_id,
    uint32_t api_version,
    const ElfApiInterface* api_interface);

/**
 * @brief Frees PluginManager
 * @param manager PluginManager instance
 */
void plugin_manager_free(PluginManager* manager);

/**
 * @brief Loads single plugin by full path
 * @param manager PluginManager instance
 * @param path Path to plugin
 * @return Error code
 */
PluginManagerError plugin_manager_load_single(PluginManager* manager, const char* path);

/**
 * @brief Loads all plugins from specified directory
 * @param manager PluginManager instance
 * @param path Path to directory
 * @return Error code
 */
PluginManagerError plugin_manager_load_all(PluginManager* manager, const char* path);

/**
 * @brief Returns number of loaded plugins
 * @param manager PluginManager instance
 * @return Number of loaded plugins
 */
uint32_t plugin_manager_get_count(PluginManager* manager);

/**
 * @brief Returns plugin descriptor by index
 * @param manager PluginManager instance
 * @param index Plugin index
 * @return Plugin descriptor
 */
const FlipperAppPluginDescriptor* plugin_manager_get(PluginManager* manager, uint32_t index);

/**
 * @brief Returns plugin entry point by index
 * @param manager PluginManager instance
 * @param index Plugin index
 * @return Plugin entry point
 */
const void* plugin_manager_get_ep(PluginManager* manager, uint32_t index);

#ifdef __cplusplus
}
#endif
