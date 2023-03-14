#pragma once
#include <gui/view.h>

typedef struct SPIMemDetectView SPIMemDetectView;
typedef void (*SPIMemDetectViewCallback)(void* context);

View* spi_mem_view_detect_get_view(SPIMemDetectView* app);
SPIMemDetectView* spi_mem_view_detect_alloc();
void spi_mem_view_detect_free(SPIMemDetectView* app);
