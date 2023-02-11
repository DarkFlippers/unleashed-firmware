#pragma once
#include "spi_mem_app.h"

void spi_mem_file_create_folder(SPIMemApp* app);
bool spi_mem_file_select(SPIMemApp* app);
bool spi_mem_file_create(SPIMemApp* app, const char* file_name);
bool spi_mem_file_delete(SPIMemApp* app);
bool spi_mem_file_create_open(SPIMemApp* app);
bool spi_mem_file_open(SPIMemApp* app);
bool spi_mem_file_write_block(SPIMemApp* app, uint8_t* data, size_t size);
bool spi_mem_file_read_block(SPIMemApp* app, uint8_t* data, size_t size);
void spi_mem_file_close(SPIMemApp* app);
void spi_mem_file_show_storage_error(SPIMemApp* app, const char* error_text);
size_t spi_mem_file_get_size(SPIMemApp* app);
