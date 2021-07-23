#include "file_select.h"
#include <gui/elements.h>
#include <m-string.h>
#include <sys/param.h>
#include <storage/storage.h>

#define FILENAME_COUNT 4

struct FileSelect {
    // public
    View* view;
    Storage* fs_api;
    const char* path;
    const char* extension;

    bool init_completed;

    FileSelectCallback callback;
    void* context;

    char* buffer;
    uint8_t buffer_size;
};

typedef struct {
    string_t filename[FILENAME_COUNT];
    uint8_t position;

    uint16_t first_file_index;
    uint16_t file_count;

} FileSelectModel;

bool file_select_fill_strings(FileSelect* file_select);
bool file_select_fill_count(FileSelect* file_select);
static bool file_select_init_inner(FileSelect* file_select);

static void file_select_draw_callback(Canvas* canvas, void* _model) {
    FileSelectModel* model = _model;

    string_t string_buff;
    const uint8_t item_height = 16;
    const uint8_t item_width = 123;
    const uint8_t text_max_width = 115;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    if(model->file_count) {
        for(uint8_t i = 0; i < MIN(FILENAME_COUNT, model->file_count); i++) {
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

            string_init_set(string_buff, model->filename[i]);
            elements_string_fit_width(canvas, string_buff, text_max_width);
            canvas_draw_str(
                canvas, 6, (i * item_height) + item_height - 4, string_get_cstr(string_buff));

            string_clear(string_buff);
        }
    } else {
        canvas_draw_str(canvas, 6, item_height, "Empty folder");
    }
    elements_scrollbar(canvas, model->first_file_index + model->position, model->file_count);
}

static bool file_select_input_callback(InputEvent* event, void* context) {
    FileSelect* file_select = (FileSelect*)context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(!file_select->init_completed) {
            if(!file_select_init_inner(file_select)) {
                file_select->callback(false, file_select->context);
            }
        } else if(event->key == InputKeyUp) {
            with_view_model(
                file_select->view, (FileSelectModel * model) {
                    if(model->position == 0) {
                        if(model->first_file_index == 0) {
                            // wrap
                            int16_t max_first_file_index = model->file_count - FILENAME_COUNT;
                            model->position = MIN(FILENAME_COUNT - 1, model->file_count - 1);
                            model->first_file_index =
                                max_first_file_index < 0 ? 0 : max_first_file_index;
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
            if(!file_select_fill_strings(file_select)) {
                file_select->callback(false, file_select->context);
            }
        } else if(event->key == InputKeyDown) {
            with_view_model(
                file_select->view, (FileSelectModel * model) {
                    uint16_t max_first_file_index = model->file_count > FILENAME_COUNT ?
                                                        model->file_count - FILENAME_COUNT :
                                                        0;

                    if(model->position >= MIN(FILENAME_COUNT - 1, model->file_count - 1)) {
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
            if(!file_select_fill_strings(file_select)) {
                file_select->callback(false, file_select->context);
            }
        } else if(event->key == InputKeyOk) {
            if(file_select->callback != NULL) {
                if(file_select->buffer) {
                    with_view_model(
                        file_select->view, (FileSelectModel * model) {
                            strlcpy(
                                file_select->buffer,
                                string_get_cstr(model->filename[model->position]),
                                file_select->buffer_size);

                            return false;
                        });
                };

                file_select->callback(true, file_select->context);
            }
            consumed = true;
        }
    }

    return consumed;
}

static bool file_select_init_inner(FileSelect* file_select) {
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
    file_select->fs_api = furi_record_open("storage");

    view_set_context(file_select->view, file_select);
    view_allocate_model(file_select->view, ViewModelTypeLockFree, sizeof(FileSelectModel));
    view_set_draw_callback(file_select->view, file_select_draw_callback);
    view_set_input_callback(file_select->view, file_select_input_callback);

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
    furi_record_close("storage");
}

View* file_select_get_view(FileSelect* file_select) {
    furi_assert(file_select);
    return file_select->view;
}

void file_select_set_callback(FileSelect* file_select, FileSelectCallback callback, void* context) {
    file_select->context = context;
    file_select->callback = callback;
}

void file_select_set_filter(FileSelect* file_select, const char* path, const char* extension) {
    furi_assert(file_select);
    file_select->path = path;
    file_select->extension = extension;
}

void file_select_set_result_buffer(FileSelect* file_select, char* buffer, uint8_t buffer_size) {
    file_select->buffer = buffer;
    file_select->buffer_size = buffer_size;

    if(file_select->buffer) {
        strlcpy(file_select->buffer, "", file_select->buffer_size);
    }
}

bool file_select_init(FileSelect* file_select) {
    if(!file_select_init_inner(file_select)) {
        file_select->callback(false, file_select->context);
        return false;
    } else {
        return true;
    }
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
    File* directory = storage_file_alloc(file_select->fs_api);

    uint8_t string_counter = 0;
    uint16_t file_counter = 0;
    const uint8_t name_length = 100;
    char* name = furi_alloc(name_length);
    uint16_t first_file_index = 0;

    with_view_model(
        file_select->view, (FileSelectModel * model) {
            first_file_index = model->first_file_index;
            return false;
        });

    if(!storage_dir_open(directory, file_select->path)) {
        storage_dir_close(directory);
        storage_file_free(directory);
        free(name);
        return false;
    }

    while(1) {
        if(!storage_dir_read(directory, &file_info, name, name_length)) {
            break;
        }

        if(storage_file_get_error(directory) == FSE_OK) {
            if(filter_file(file_select, &file_info, name)) {
                if(file_counter >= first_file_index) {
                    with_view_model(
                        file_select->view, (FileSelectModel * model) {
                            string_set_str(model->filename[string_counter], name);

                            if(strcmp(file_select->extension, "*") != 0) {
                                string_replace_all_str(
                                    model->filename[string_counter], file_select->extension, "");
                            }

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
            storage_dir_close(directory);
            storage_file_free(directory);
            free(name);
            return false;
        }
    }

    storage_dir_close(directory);
    storage_file_free(directory);
    free(name);
    return true;
}

bool file_select_fill_count(FileSelect* file_select) {
    furi_assert(file_select);
    furi_assert(file_select->fs_api);
    furi_assert(file_select->path);
    furi_assert(file_select->extension);

    FileInfo file_info;
    File* directory = storage_file_alloc(file_select->fs_api);

    uint16_t file_counter = 0;
    const uint8_t name_length = 100;
    char* name = furi_alloc(name_length);

    if(!storage_dir_open(directory, file_select->path)) {
        storage_dir_close(directory);
        storage_file_free(directory);
        free(name);
        return false;
    }

    while(1) {
        if(!storage_dir_read(directory, &file_info, name, name_length)) {
            break;
        }

        if(storage_file_get_error(directory) == FSE_OK) {
            if(filter_file(file_select, &file_info, name)) {
                file_counter++;
            }
        } else {
            storage_dir_close(directory);
            storage_file_free(directory);
            free(name);
            return false;
        }
    }

    with_view_model(
        file_select->view, (FileSelectModel * model) {
            model->file_count = file_counter;
            return false;
        });

    storage_dir_close(directory);
    storage_file_free(directory);
    free(name);
    return true;
}

void file_select_set_selected_file_internal(FileSelect* file_select, const char* filename) {
    furi_assert(file_select);
    furi_assert(filename);
    furi_assert(file_select->fs_api);
    furi_assert(file_select->path);
    furi_assert(file_select->extension);

    if(strlen(filename) == 0) return;

    FileInfo file_info;
    File* directory = storage_file_alloc(file_select->fs_api);

    const uint8_t name_length = 100;
    char* name = furi_alloc(name_length);
    uint16_t file_position = 0;
    bool file_found = false;

    string_t filename_str;
    string_init_set_str(filename_str, filename);
    if(strcmp(file_select->extension, "*") != 0) {
        string_cat_str(filename_str, file_select->extension);
    }

    if(!storage_dir_open(directory, file_select->path)) {
        string_clear(filename_str);
        storage_dir_close(directory);
        storage_file_free(directory);
        free(name);
        return;
    }

    while(1) {
        if(!storage_dir_read(directory, &file_info, name, name_length)) {
            break;
        }

        if(storage_file_get_error(directory) == FSE_OK) {
            if(filter_file(file_select, &file_info, name)) {
                if(strcmp(string_get_cstr(filename_str), name) == 0) {
                    file_found = true;
                    break;
                }

                file_position++;
            }
        } else {
            string_clear(filename_str);
            storage_dir_close(directory);
            storage_file_free(directory);
            free(name);
            return;
        }
    }

    if(file_found) {
        with_view_model(
            file_select->view, (FileSelectModel * model) {
                uint16_t max_first_file_index =
                    model->file_count > FILENAME_COUNT ? model->file_count - FILENAME_COUNT : 0;

                model->first_file_index = file_position;

                if(model->first_file_index > 0) {
                    model->first_file_index -= 1;
                }

                if(model->first_file_index >= max_first_file_index) {
                    model->first_file_index = max_first_file_index;
                }

                model->position = file_position - model->first_file_index;

                return true;
            });
    }

    string_clear(filename_str);
    storage_dir_close(directory);
    storage_file_free(directory);
    free(name);
}

void file_select_set_selected_file(FileSelect* file_select, const char* filename) {
    file_select_set_selected_file_internal(file_select, filename);

    if(!file_select_fill_strings(file_select)) {
        file_select->callback(false, file_select->context);
    }
}