#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/** Init flash, applying necessary workarounds
 */
void furi_hal_flash_init();

/** Get flash base address
 *
 * @return     pointer to flash base
 */
size_t furi_hal_flash_get_base();

/** Get flash read block size
 *
 * @return     size in bytes
 */
size_t furi_hal_flash_get_read_block_size();

/** Get flash write block size
 *
 * @return     size in bytes
 */
size_t furi_hal_flash_get_write_block_size();

/** Get flash page size
 *
 * @return     size in bytes
 */
size_t furi_hal_flash_get_page_size();

/** Get expected flash cycles count
 *
 * @return     count of erase-write operations
 */
size_t furi_hal_flash_get_cycles_count();

/** Get free flash start address
 *
 * @return     pointer to free region start
 */
const void* furi_hal_flash_get_free_start_address();

/** Get free flash end address
 *
 * @return     pointer to free region end
 */
const void* furi_hal_flash_get_free_end_address();

/** Get first free page start address
 *
 * @return     first free page memory address
 */
size_t furi_hal_flash_get_free_page_start_address();

/** Get free page count
 *
 * @return     free page count
 */
size_t furi_hal_flash_get_free_page_count();

/** Erase Flash
 *
 * @warning    locking operation with critical section, stales execution
 *
 * @param      page  The page to erase
 *
 * @return     true on success
 */
bool furi_hal_flash_erase(uint8_t page);

/** Write double word (64 bits)
 *
 * @warning locking operation with critical section, stales execution
 *
 * @param      address  destination address, must be double word aligned.
 * @param      data     data to write
 *
 * @return     true on success
 */
bool furi_hal_flash_write_dword(size_t address, uint64_t data);

/** Write aligned page data (up to page size)
 *
 * @warning locking operation with critical section, stales execution
 *
 * @param      address  destination address, must be page aligned.
 * @param      data     data to write
 * @param      length   data length
 *
 * @return     true on success
 */
bool furi_hal_flash_program_page(const uint8_t page, const uint8_t* data, uint16_t length);

/** Get flash page number for address
 *
 * @return     page number, -1 for invalid address
 */

int16_t furi_hal_flash_get_page_number(size_t address);