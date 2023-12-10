/**
 * @file simple_array.h
 *
 * @brief This file provides a simple (non-type safe) array for elements with
 * (optional) in-place construction support.
 *
 * No append, remove or take operations are supported.
 * Instead, the user must call simple_array_init() in order to reserve the memory for n elements.
 * In case if init() is specified in the configuration, then the elements are constructed in-place.
 *
 * To clear all elements from the array, call simple_array_reset(), which will also call reset()
 * for each element in case if it was specified in the configuration. This is useful if a custom
 * destructor is required for a particular type. Calling simple_array_free() will also result in a
 * simple_array_reset() call automatically.
 *
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SimpleArray SimpleArray;

typedef void SimpleArrayData;
typedef void SimpleArrayElement;

typedef void (*SimpleArrayInit)(SimpleArrayElement* elem);
typedef void (*SimpleArrayReset)(SimpleArrayElement* elem);
typedef void (*SimpleArrayCopy)(SimpleArrayElement* elem, const SimpleArrayElement* other);

/** Simple Array configuration structure. Defined per type. */
typedef struct {
    SimpleArrayInit init; /**< Initialisation (in-place constructor) method. */
    SimpleArrayReset reset; /**< Reset (custom destructor) method. */
    SimpleArrayCopy copy; /**< Copy (custom copy-constructor) method. */
    const size_t type_size; /** Type size, in bytes. */
} SimpleArrayConfig;

/**
 * Allocate a SimpleArray instance with the given configuration.
 *
 * @param [in] config Pointer to the type-specific configuration
 * @return Pointer to the allocated SimpleArray instance
 */
SimpleArray* simple_array_alloc(const SimpleArrayConfig* config);

/**
 * Free a SimpleArray instance and release its contents.
 *
 * @param [in] instance Pointer to the SimpleArray instance to be freed
 */
void simple_array_free(SimpleArray* instance);

/**
 * Initialise a SimpleArray instance by allocating additional space to contain
 * the requested number of elements.
 * If init() is specified in the config, then it is called for each element,
 * otherwise the data is filled with zeroes.
 *
 * @param [in] instance Pointer to the SimpleArray instance to be init'd
 * @param [in] count Number of elements to be allocated and init'd
 */
void simple_array_init(SimpleArray* instance, uint32_t count);

/**
 * Reset a SimpleArray instance and delete all of its elements.
 * If reset() is specified in the config, then it is called for each element,
 * otherwise the data is simply free()'d.
 *
 * @param [in] instance Pointer to the SimpleArray instance to be reset
 */
void simple_array_reset(SimpleArray* instance);

/**
 * Copy (duplicate) another SimpleArray instance to this one.
 * If copy() is specified in the config, then it is called for each element,
 * otherwise the data is simply memcpy()'d.
 *
 * @param [in] instance Pointer to the SimpleArray instance to copy to
 * @param [in] other Pointer to the SimpleArray instance to copy from
 */
void simple_array_copy(SimpleArray* instance, const SimpleArray* other);

/**
 * Check if another SimpleArray instance is equal (the same object or holds the
 * same data) to this one.
 *
 * @param [in] instance Pointer to the SimpleArray instance to be compared
 * @param [in] other Pointer to the SimpleArray instance to be compared
 * @return True if instances are considered equal, false otherwise
 */
bool simple_array_is_equal(const SimpleArray* instance, const SimpleArray* other);

/**
 * Get the count of elements currently contained in a SimpleArray instance.
 *
 * @param [in] instance Pointer to the SimpleArray instance to query the count from
 * @return Count of elements contained in the instance
 */
uint32_t simple_array_get_count(const SimpleArray* instance);

/**
 * Get a pointer to an element contained in a SimpleArray instance.
 *
 * @param [in] instance Pointer to the SimpleArray instance to get an element from
 * @param [in] index Index of the element in question. MUST be less than total element count
 * @return Pointer to the element specified by index
 */
SimpleArrayElement* simple_array_get(SimpleArray* instance, uint32_t index);

/**
 * Get a const pointer to an element contained in a SimpleArray instance.
 *
 * @param [in] instance Pointer to the SimpleArray instance to get an element from
 * @param [in] index Index of the element in question. MUST be less than total element count
 * @return Const pointer to the element specified by index
 */
const SimpleArrayElement* simple_array_cget(const SimpleArray* instance, uint32_t index);

/**
 * Get a pointer to the internal data of a SimpleArray instance.
 *
 * @param [in] instance Pointer to the SimpleArray instance to get the data of
 * @return Pointer to the instance's internal data
 */
SimpleArrayData* simple_array_get_data(SimpleArray* instance);

/**
 * Get a constant pointer to the internal data of a SimpleArray instance.
 *
 * @param [in] instance Pointer to the SimpleArray instance to get the data of
 * @return Constant pointer to the instance's internal data
 */
const SimpleArrayData* simple_array_cget_data(const SimpleArray* instance);

// Standard preset configurations

// Preset configuration for a byte(uint8_t) array.
extern const SimpleArrayConfig simple_array_config_uint8_t;

#ifdef __cplusplus
}
#endif
