#include "file_select.h"
#include <gui/elements.h>
#include <m-string.h>

#define FILENAME_COUNT 4

struct FileSelect {
    // public
    View* view;
    FS_Api* fs_api;
    char* path;
    char* extension;

    bool init_completed;

    FileSelectCallback callback;
    void* context;
};

typedef struct {
    string_t filename[FILENAME_COUNT];
    uint8_t position;

    uint16_t first_file_index;
    uint16_t file_count;

} FileSelectModel;

bool file_select_fill_strings(FileSelect* file_select);
bool file_select_fill_count(FileSelect* file_select);
static bool file_select_init(FileSelect* file_select);

static void file_select_draw_callback(Canvas* canvas, void* _model) {
    FileSelectModel* model = _model;

    const uint8_t item_height = 16;
    const uint8_t item_width = 123;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    for(uint8_t i = 0; i < FILENAME_COUNT; i++) {
        if(i == model->position) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, (i * item_height) + 1, item_width, item_height - 2);

            canvas_set_color(canvas, ColorWhite);
            canvas_draw_dot(canvas, 0, (i * item_height) + 1);
            canvas_draw_dot(canvas, 0, (i * item_height) + item_height - 2);
            canvas_draw_dot(canvas, item_width - 1, (i * item_height) + 1);
            canvas_draw_dot(canvas, item_width - 1, (i * item_height) + item_height - 2);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_draw_str(
            canvas, 6, (i * item_height) + item_height - 4, string_get_cstr(model->filename[i]));
    }

    elements_scrollbar(canvas, model->first_file_index + model->position, model->file_count);
}

static bool file_select_input_callback(InputEvent* event, void* context) {
    FileSelect* file_select = (FileSelect*)context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(!file_select->init_completed) {
            if(!file_select_init(file_select)) {
                file_select->callback(NULL, file_select->context);
            }
        } else if(event->key == InputKeyUp) {
            with_view_model(
                file_select->view, (FileSelectModel * model) {
                    if(model->position == 0) {
                        if(model->first_file_index == 0) {
                            // wrap
                            uint16_t max_first_file_index = model->file_count - FILENAME_COUNT;
                            model->position = FILENAME_COUNT - 1;
                            model->first_file_index = max_first_file_index;
                        } else {
                            model->first_file_index--;
                        }
                    } else if(model->position == 1) {
                        if(model->first_file_index == 0) {
                            model->position--;
                        } else {
                            model->first_file_index--;
                        }
                    } else {
                        model->position--;
                    }
                    return true;
                });
            consumed = true;
        } else if(event->key == InputKeyDown) {
            with_view_model(
                file_select->view, (FileSelectModel * model) {
                    uint16_t max_first_file_index = model->file_count - FILENAME_COUNT;

                    if(model->position >= (FILENAME_COUNT - 1)) {
                        if(model->first_file_index >= max_first_file_index) {
                            // wrap
                            model->position = 0;
                            model->first_file_index = 0;
                        } else {
                            model->first_file_index++;
                        }
                    } else if(model->position >= (FILENAME_COUNT - 2)) {
                        if(model->first_file_index >= max_first_file_index) {
                            model->position++;
                        } else {
                            model->first_file_index++;
                        }
                    } else {
                        model->position++;
                    }
                    return true;
                });
            consumed = true;
        } else if(event->key == InputKeyOk) {
            if(file_select->callback != NULL) {
                const char* result;
                with_view_model(
                    file_select->view, (FileSelectModel * model) {
                        result = string_get_cstr(model->filename[model->position]);
                        return false;
                    });

                file_select->callback(result, file_select->context);
            }
            consumed = true;
        }

        if(!file_select_fill_strings(file_select)) {
            file_select->callback(NULL, file_select->context);
        }
    }

    return consumed;
}

static bool file_select_init(FileSelect* file_select) {
    bool result = false;
    if(file_select->path && file_select->extension && file_select->fs_api) {
        if(file_select_fill_count(file_select)) {
            if(file_select_fill_strings(file_select)) {
                file_select->init_completed = true;
                result = true;
            }
        }
    }

    return result;
}

FileSelect* file_select_alloc() {
    FileSelect* file_select = furi_alloc(sizeof(FileSelect));
    file_select->view = view_alloc();
    view_set_context(file_select->view, file_select);
    view_allocate_model(file_select->view, ViewModelTypeLockFree, sizeof(FileSelectModel));
    view_set_draw_callback(file_select->view, file_select_draw_callback);
    view_set_input_callback(file_select->view, file_select_input_callback);

    file_select->fs_api = NULL;
    file_select->path = NULL;
    file_select->extension = NULL;
    file_select->init_completed = false;
    file_select->callback = NULL;
    file_select->context = NULL;

    with_view_model(
        file_select->view, (FileSelectModel * model) {
            for(uint8_t i = 0; i < FILENAME_COUNT; i++) {
                string_init(model->filename[i]);
            }

            model->first_file_index = 0;
            model->file_count = 0;
            return false;
        });

    return file_select;
}

void file_select_free(FileSelect* file_select) {
    furi_assert(file_select);
    with_view_model(
        file_select->view, (FileSelectModel * model) {
            for(uint8_t i = 0; i < FILENAME_COUNT; i++) {
                string_clear(model->filename[i]);
            }
            return false;
        });
    view_free(file_select->view);
    free(file_select);
}

View* file_select_get_view(FileSelect* file_select) {
    furi_assert(file_select);
    return file_select->view;
}

void file_select_set_api(FileSelect* file_select, FS_Api* fs_api) {
    furi_assert(file_select);
    file_select->fs_api = fs_api;
}

void file_select_set_callback(FileSelect* file_select, FileSelectCallback callback, void* context) {
}

void file_select_set_filter(FileSelect* file_select, char* path, char* extension) {
    furi_assert(file_select);
    file_select->path = path;
    file_select->extension = extension;
}

static bool filter_file(FileSelect* file_select, FileInfo* file_info, char* name) {
    bool result = false;

    if(!(file_info->flags & FSF_DIRECTORY)) {
        if(strcmp(file_select->extension, "*") == 0) {
            result = true;
        } else if(strstr(name, file_select->extension) != NULL) {
            result = true;
        }
    }

    return result;
}

bool file_select_fill_strings(FileSelect* file_select) {
    furi_assert(file_select);
    furi_assert(file_select->fs_api);
    furi_assert(file_select->path);
    furi_assert(file_select->extension);

    FileInfo file_info;
    File directory;
    bool result;
    FS_Dir_Api* dir_api = &file_select->fs_api->dir;
    uint8_t string_counter = 0;
    uint16_t file_counter = 0;
    const uint8_t name_length = 100;
    char* name = calloc(name_length, sizeof(char));
    uint16_t first_file_index = 0;

    with_view_model(
        file_select->view, (FileSelectModel * model) {
            first_file_index = model->first_file_index;
            return false;
        });

    if(name == NULL) {
        return false;
    }

    result = dir_api->open(&directory, file_select->path);

    if(!result) {
        dir_api->close(&directory);
        free(name);
        return false;
    }

    while(1) {
        result = dir_api->read(&directory, &file_info, name, name_length);

        if(directory.error_id == FSE_NOT_EXIST || name[0] == 0) {
            break;
        }

        if(result) {
            if(directory.error_id == FSE_OK) {
                if(filter_file(file_select, &file_info, name)) {
                    if(file_counter >= first_file_index) {
                        with_view_model(
                            file_select->view, (FileSelectModel * model) {
                                string_set_str(model->filename[string_counter], name);
                                return true;
                            });
                        string_counter++;

                        if(string_counter >= FILENAME_COUNT) {
                            break;
                        }
                    }
                    file_counter++;
                }
            } else {
                dir_api->close(&directory);
                free(name);
                return false;
            }
        }
    }

    dir_api->close(&directory);
    free(name);
    return true;
}

bool file_select_fill_count(FileSelect* file_select) {
    furi_assert(file_select);
    furi_assert(file_select->fs_api);
    furi_assert(file_select->path);
    furi_assert(file_select->extension);

    FileInfo file_info;
    File directory;
    bool result;
    FS_Dir_Api* dir_api = &file_select->fs_api->dir;
    uint16_t file_counter = 0;
    const uint8_t name_length = 100;
    char* name = calloc(name_length, sizeof(char));

    if(name == NULL) {
        return false;
    }

    result = dir_api->open(&directory, file_select->path);

    if(!result) {
        dir_api->close(&directory);
        free(name);
        return false;
    }

    while(1) {
        result = dir_api->read(&directory, &file_info, name, name_length);

        if(directory.error_id == FSE_NOT_EXIST || name[0] == 0) {
            break;
        }

        if(result) {
            if(directory.error_id == FSE_OK) {
                if(filter_file(file_select, &file_info, name)) {
                    file_counter++;
                }
            } else {
                dir_api->close(&directory);
                free(name);
                return false;
            }
        }
    }

    with_view_model(
        file_select->view, (FileSelectModel * model) {
            model->file_count = file_counter;
            return false;
        });

    dir_api->close(&directory);
    free(name);
    return true;
}