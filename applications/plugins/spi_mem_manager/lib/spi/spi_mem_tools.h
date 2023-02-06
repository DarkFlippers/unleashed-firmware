#pragma once

#include "spi_mem_chip.h"

#define SPI_MEM_SPI_TIMEOUT 1000
#define SPI_MEM_MAX_BLOCK_SIZE 256
#define SPI_MEM_FILE_BUFFER_SIZE 4096

bool spi_mem_tools_read_chip_info(SPIMemChip* chip);
bool spi_mem_tools_read_block(SPIMemChip* chip, size_t offset, uint8_t* data, size_t block_size);
size_t spi_mem_tools_get_file_max_block_size(SPIMemChip* chip);
SPIMemChipStatus spi_mem_tools_get_chip_status(SPIMemChip* chip);
bool spi_mem_tools_erase_chip(SPIMemChip* chip);
bool spi_mem_tools_write_bytes(SPIMemChip* chip, size_t offset, uint8_t* data, size_t block_size);
