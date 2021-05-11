#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/** Get flash base address
 * @return pointer to flash base
 */
size_t api_hal_flash_get_base();

/** Get flash read block size
 * @return size in bytes
 */
size_t api_hal_flash_get_read_block_size();

/** Get flash write block size
 * @return size in bytes
 */
size_t api_hal_flash_get_write_block_size();

/** Get flash page size
 * @return size in bytes
 */
size_t api_hal_flash_get_page_size();

/** Get expected flash cycles count
 * @return count of erase-write operations 
 */
size_t api_hal_flash_get_cycles_count();

/** Get free flash start address
 * @return pointer to free region start
 */
const void* api_hal_flash_get_free_start_address();

/** Get free flash end address
 * @return pointer to free region end
 */
const void* api_hal_flash_get_free_end_address();

/** Get first free page start address
 * @return first free page memory address
 */
size_t api_hal_flash_get_free_page_start_address();

/** Get free page count
 * @return free page count
 */
size_t api_hal_flash_get_free_page_count();

/*
 * Erase Flash
 * Locking operation, uses HSEM to manage shared access.
 * @param page, page number
 * @param count, page count to erase
 */
bool api_hal_flash_erase(uint8_t page, uint8_t count);

/*
 * Write double word (64 bits)
 * Locking operation, uses HSEM to manage shared access.
 * @param address - destination address, must be double word aligned.
 * @param data - data to write
 */
bool api_hal_flash_write_dword(size_t address, uint64_t data);

/*
 * Write double word (64 bits) from address
 * Locking operation, uses HSEM to manage shared access.
 * @param address - destination address, must be block aligned
 * @param source_address - source address
 */
bool api_hal_flash_write_dword_from(size_t address, size_t source_address);
