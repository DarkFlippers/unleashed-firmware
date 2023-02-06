#pragma once

#include <furi.h>
#include <m-array.h>

typedef struct SPIMemChip SPIMemChip;

ARRAY_DEF(found_chips, const SPIMemChip*, M_POD_OPLIST)

typedef enum {
    SPIMemChipStatusBusy,
    SPIMemChipStatusIdle,
    SPIMemChipStatusError
} SPIMemChipStatus;

typedef enum {
    SPIMemChipWriteModeUnknown = 0,
    SPIMemChipWriteModePage = (0x01 << 0),
    SPIMemChipWriteModeAAIByte = (0x01 << 1),
    SPIMemChipWriteModeAAIWord = (0x01 << 2),
} SPIMemChipWriteMode;

const char* spi_mem_chip_get_vendor_name(const SPIMemChip* chip);
const char* spi_mem_chip_get_model_name(const SPIMemChip* chip);
size_t spi_mem_chip_get_size(SPIMemChip* chip);
uint8_t spi_mem_chip_get_vendor_id(SPIMemChip* chip);
uint8_t spi_mem_chip_get_type_id(SPIMemChip* chip);
uint8_t spi_mem_chip_get_capacity_id(SPIMemChip* chip);
SPIMemChipWriteMode spi_mem_chip_get_write_mode(SPIMemChip* chip);
size_t spi_mem_chip_get_page_size(SPIMemChip* chip);
bool spi_mem_chip_find_all(SPIMemChip* chip_info, found_chips_t found_chips);
void spi_mem_chip_copy_chip_info(SPIMemChip* dest, const SPIMemChip* src);
uint32_t spi_mem_chip_get_vendor_enum(const SPIMemChip* chip);
const char* spi_mem_chip_get_vendor_name_by_enum(uint32_t vendor_enum);
