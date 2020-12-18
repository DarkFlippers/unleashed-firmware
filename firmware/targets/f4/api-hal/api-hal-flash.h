#pragma once

#include <stdint.h>
#include <stddef.h>

/*
 * Write double word (64 bits)
 * Locking operation, uses HSEM to manage shared access.
 * @param address - destination address, must be double word aligned.
 * @param data - data to write
 */
void api_hal_flash_write_dword(size_t address, uint64_t data);

/*
 * Write page (4096 bytes or 64 rows of double words).
 * Locking operation, uses HSEM to manage shared access.
 * @param address - destination address, must be page aligned
 * @param source_address - source address
 */
void api_hal_flash_write_page(size_t address, size_t source_address);
