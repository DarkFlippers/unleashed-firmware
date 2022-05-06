/**
 * @file file_select.h
 * GUI: FileSelect view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FileSelect FileSelect;

typedef void (*FileSelectCallback)(bool result, void* context);

FileSelect* file_select_alloc();

void file_select_free(FileSelect* file_select);
View* file_select_get_view(FileSelect* file_select);

void file_select_set_callback(FileSelect* file_select, FileSelectCallback callback, void* context);
void file_select_set_filter(FileSelect* file_select, const char* path, const char* extension);
void file_select_set_result_buffer(FileSelect* file_select, char* buffer, uint8_t buffer_size);
bool file_select_init(FileSelect* file_select);
void file_select_set_selected_file(FileSelect* file_select, const char* filename);

#ifdef __cplusplus
}
#endif
