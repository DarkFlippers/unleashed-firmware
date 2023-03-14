#pragma once

#include <flipper_application/elf/elf_api_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Composite API resolver 
 * Resolves API interface by calling all resolvers in order
 * Uses API version from first resolver
 * Note: when using hashtable resolvers, collisions between tables are not detected
 * Can be cast to ElfApiInterface*
 */
typedef struct CompositeApiResolver CompositeApiResolver;

/**
 * @brief Allocate composite API resolver
 * @return CompositeApiResolver* instance
 */
CompositeApiResolver* composite_api_resolver_alloc();

/**
 * @brief Free composite API resolver
 * @param resolver Instance
 */
void composite_api_resolver_free(CompositeApiResolver* resolver);

/**
 * @brief Add API resolver to composite resolver
 * @param resolver Instance
 * @param interface API resolver
 */
void composite_api_resolver_add(CompositeApiResolver* resolver, const ElfApiInterface* interface);

/**
 * @brief Get API interface from composite resolver
 * @param resolver Instance
 * @return API interface
 */
const ElfApiInterface* composite_api_resolver_get(CompositeApiResolver* resolver);

#ifdef __cplusplus
}
#endif
