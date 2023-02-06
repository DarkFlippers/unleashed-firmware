#pragma once
#include <gui/view.h>

typedef struct SPIMemProgressView SPIMemProgressView;
typedef void (*SPIMemProgressViewCallback)(void* context);

View* spi_mem_view_progress_get_view(SPIMemProgressView* app);
SPIMemProgressView* spi_mem_view_progress_alloc();
void spi_mem_view_progress_free(SPIMemProgressView* app);
void spi_mem_view_progress_set_read_callback(
    SPIMemProgressView* app,
    SPIMemProgressViewCallback callback,
    void* cb_ctx);
void spi_mem_view_progress_set_verify_callback(
    SPIMemProgressView* app,
    SPIMemProgressViewCallback callback,
    void* cb_ctx);
void spi_mem_view_progress_set_write_callback(
    SPIMemProgressView* app,
    SPIMemProgressViewCallback callback,
    void* cb_ctx);
void spi_mem_view_progress_set_chip_size(SPIMemProgressView* app, size_t chip_size);
void spi_mem_view_progress_set_file_size(SPIMemProgressView* app, size_t file_size);
void spi_mem_view_progress_set_block_size(SPIMemProgressView* app, size_t block_size);
void spi_mem_view_progress_inc_progress(SPIMemProgressView* app);
void spi_mem_view_progress_reset(SPIMemProgressView* app);
