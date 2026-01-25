#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/text_input.h>
#include <gui/modules/dialog_ex.h>
#include <dialogs/dialogs.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "iso15693_writer_worker.h"

#define MAX_TAG_BLOCKS 256
#define MAX_TAG_BLOCK_SIZE 32
#define TAG_BLOCK_SIZE 4
#define ISO_APP_DATA_PATH "/ext/apps_data/ISO15693-3_Writer"

typedef enum {
    Iso15693WriterViewSubmenu,
    Iso15693WriterViewConfigAddress,
    Iso15693WriterViewWriter,
    Iso15693ViewTextInput,
    Iso15693ViewFileBrowser,
} Iso15693WriterView;

typedef enum {
    Iso15693AppEventWriteSuccess = 100,
    Iso15693AppEventWriteFail,
    Iso15693AppEventReadSuccess,
    Iso15693AppEventRetry,
    Iso15693AppEventExitToMenu,
    Iso15693AppEventFileNameEntered,
    Iso15693AppEventSavePreview,
} Iso15693AppEvent;

typedef enum {
    Iso15693MenuWriteSingle,
    Iso15693MenuFormatAll,
    Iso15693MenuWriteAll,
    Iso15693MenuReadSingle,
    Iso15693MenuReadDump,
    Iso15693MenuLock = 6,
    Iso15693MenuLockAll,
    Iso15693MenuSetAFI,
    Iso15693MenuWriteAFI,
    Iso15693MenuLockAFI,
    Iso15693MenuSetDSFID,
    Iso15693MenuWriteDSFID,
    Iso15693MenuLockDSFID,
    Iso15693MenuSetData,
    Iso15693MenuSetAddress,
} Iso15693MenuSelection;

typedef enum {
    Iso15693StateScanning,
    Iso15693StateSuccess,
    Iso15693StateFail,
} Iso15693State;

typedef struct {
    Iso15693State state;
    Iso15693WriterWorkerMode mode;
    uint8_t progress;
    uint8_t current_data[MAX_TAG_BLOCK_SIZE];
    uint16_t current_address;
    uint8_t* full_dump_ptr;
    uint8_t* lock_status_ptr;
    uint16_t actual_block_count;
    uint8_t actual_block_size;
    uint16_t scroll_pos;
} Iso15693Model;

static void iso15693_text_input_callback(void* context);

typedef struct {
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    Submenu* submenu;
    ByteInput* byte_input;
    TextInput* text_input;
    View* writer_view;
    Iso15693WriterWorker* worker;
    uint8_t target_block;
    uint8_t target_data[MAX_TAG_BLOCK_SIZE];
    uint8_t full_tag_data[MAX_TAG_BLOCKS * MAX_TAG_BLOCK_SIZE];
    uint8_t lock_status_data[MAX_TAG_BLOCKS];
    uint16_t actual_block_count;
    uint8_t actual_block_size;
    uint8_t afi_value;
    uint8_t dsfid_value;
    char file_path[256];
    bool save_to_file;
    bool save_preview_mode;
    Iso15693WriterView current_view;
    Iso15693WriterWorkerMode current_mode;
} Iso15693App;

static void iso15693_draw_callback(Canvas* canvas, void* model) {
    Iso15693Model* m = model;
    canvas_set_font(canvas, FontPrimary);
    
    if(m->state == Iso15693StateScanning) {
        const char* action = "Working";
        if(m->mode == Iso15693WriterWorkerModeWrite) action = "Writing";
        else if(m->mode == Iso15693WriterWorkerModeFormat) action = "Formatting";
        else if(m->mode == Iso15693WriterWorkerModeWriteFull) action = "Writing All";
        else if(m->mode == Iso15693WriterWorkerModeRead) action = "Reading";
        else if(m->mode == Iso15693WriterWorkerModeReadAll) action = "Dumping";
        else if(m->mode == Iso15693WriterWorkerModeLock) action = "Locking";
        else if(m->mode == Iso15693WriterWorkerModeLockAll) action = "Locking All";
        else if(m->mode == Iso15693WriterWorkerModeWriteAFI) action = "Writing AFI";
        else if(m->mode == Iso15693WriterWorkerModeLockAFI) action = "Locking AFI";
        else if(m->mode == Iso15693WriterWorkerModeWriteDSFID) action = "Writing DSFID";
        else if(m->mode == Iso15693WriterWorkerModeLockDSFID) action = "Locking DSFID";

        char buf[64];
        if(m->mode == Iso15693WriterWorkerModeWrite) {
            snprintf(buf, sizeof(buf), "Write %02X: %02X%02X%02X%02X...", m->current_address, m->current_data[0], m->current_data[1], m->current_data[2], m->current_data[3]);
        } else if(m->mode == Iso15693WriterWorkerModeRead) {
            snprintf(buf, sizeof(buf), "Read Block %02X...", m->current_address);
        } else if(m->mode == Iso15693WriterWorkerModeFormat || m->mode == Iso15693WriterWorkerModeReadAll || m->mode == Iso15693WriterWorkerModeWriteFull || m->mode == Iso15693WriterWorkerModeLockAll) {
            if(m->actual_block_count > 0) {
                snprintf(buf, sizeof(buf), "%s %02X/%02X", action, m->progress, m->actual_block_count - 1);
            } else {
                snprintf(buf, sizeof(buf), "%s...", action);
            }
        } else if(m->mode == Iso15693WriterWorkerModeWriteAFI) {
            snprintf(buf, sizeof(buf), "Writing AFI: %02X", m->current_data[0]);
        } else if(m->mode == Iso15693WriterWorkerModeWriteDSFID) {
            snprintf(buf, sizeof(buf), "Writing DSFID: %02X", m->current_data[0]);
        } else {
            snprintf(buf, sizeof(buf), "%s Block %02X...", action, m->current_address);
        }
        canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, buf);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Hold tag near Flipper");
    } else if(m->state == Iso15693StateSuccess) {
        if(m->mode == Iso15693WriterWorkerModeReadAll) {
            char title_buf[32];
            uint16_t display_count = m->actual_block_count > 0 ? m->actual_block_count : 28;
            uint8_t display_size = m->actual_block_size > 0 ? m->actual_block_size : 4;
            snprintf(title_buf, sizeof(title_buf), "%d Blocks %d Bytes", display_count, display_size);
            canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, title_buf);
            
            canvas_set_font(canvas, FontSecondary);
            
            for(uint16_t i = 0; i < 5; i++) {
                uint16_t block_idx = m->scroll_pos + i;
                if(block_idx < display_count && block_idx < MAX_TAG_BLOCKS && m->full_dump_ptr && m->lock_status_ptr) {
                    char buf[32];
                    bool is_locked = (m->lock_status_ptr[block_idx] & 0x01);
                    
                    uint16_t offset = block_idx * m->actual_block_size;
                    snprintf(buf, sizeof(buf), "%02X: %02X%02X%02X%02X%s %s", block_idx,
                        m->full_dump_ptr[offset+0], m->full_dump_ptr[offset+1],
                        m->full_dump_ptr[offset+2], m->full_dump_ptr[offset+3],
                        m->actual_block_size > 4 ? "..." : "",
                        is_locked ? "LOCK" : "");
                    canvas_draw_str(canvas, 10, 18 + (i * 9), buf);
                }
            }
            char footer[32];
            snprintf(footer, sizeof(footer), "OK:Back | >:Save");
            canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, footer);
        } else {
            canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "SUCCESS!");
            canvas_set_font(canvas, FontSecondary);
            char buf[64];
            if(m->mode == Iso15693WriterWorkerModeRead) {
                snprintf(buf, sizeof(buf), "Read %02X: %02X %02X %02X %02X", m->current_address, m->current_data[0], m->current_data[1], m->current_data[2], m->current_data[3]);
                canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, buf);
            } else if(m->mode == Iso15693WriterWorkerModeWrite) {
                snprintf(buf, sizeof(buf), "Wrote %02X: %02X %02X %02X %02X", m->current_address, m->current_data[0], m->current_data[1], m->current_data[2], m->current_data[3]);
                canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, buf);
            } else if(m->mode == Iso15693WriterWorkerModeWriteAFI) {
                snprintf(buf, sizeof(buf), "Wrote AFI: %02X", m->current_data[0]);
                canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, buf);
            } else if(m->mode == Iso15693WriterWorkerModeWriteDSFID) {
                snprintf(buf, sizeof(buf), "Wrote DSFID: %02X", m->current_data[0]);
                canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, buf);
            } else {
                canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, "Operation finished");
            }
            canvas_draw_str_aligned(canvas, 64, 55, AlignCenter, AlignBottom, "OK: Repeat | Back: Exit");
        }
    } else if(m->state == Iso15693StateFail) {
        canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "FAILED");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, "Check tag position");
        canvas_draw_str_aligned(canvas, 64, 55, AlignCenter, AlignBottom, "OK: Retry | Back: Exit");
    }
}

static bool iso15693_save_to_file(Iso15693App* app, const char* filepath) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, ISO_APP_DATA_PATH);
    File* file = storage_file_alloc(storage);
    bool ok = false;
    if(storage_file_open(file, filepath, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char buf[128];
        uint16_t count = app->actual_block_count > 0 ? app->actual_block_count : 28;
        uint8_t size = app->actual_block_size > 0 ? app->actual_block_size : 4;
        
        
        snprintf(buf, sizeof(buf), "# Blocks: %d, Size: %d\n", count, size);
        storage_file_write(file, buf, strlen(buf));
        
        for(int b = 0; b < count; b++) {
            snprintf(buf, sizeof(buf), "%02X: ", b);
            storage_file_write(file, buf, strlen(buf));
            
            uint16_t offset = b * size;
            for(int i = 0; i < size; i++) {
                snprintf(buf, sizeof(buf), "%02X", app->full_tag_data[offset+i]);
                storage_file_write(file, buf, strlen(buf));
            }
            storage_file_write(file, "\n", 1);
        }
        ok = true;
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

static bool iso15693_load_from_file(Iso15693App* app, const char* filepath) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool ok = false;
    if(storage_file_open(file, filepath, FSAM_READ, FSOM_OPEN_EXISTING)) {
        app->actual_block_size = 4;
        app->actual_block_count = 0;
        
        FuriString* line = furi_string_alloc();
        char buffer[1];
        int b = 0;

        while(true) {
            furi_string_reset(line);
            bool line_end = false;
            while(!line_end) {
                 size_t read = storage_file_read(file, buffer, 1);
                 if(read == 0) break;
                 if(buffer[0] == '\n') line_end = true;
                 else furi_string_push_back(line, buffer[0]);
            }
            
            if(furi_string_empty(line) && storage_file_eof(file)) break;
            
            const char* s = furi_string_get_cstr(line);
            
            
            if(s[0] == '#') {
                int count, size;
                if(sscanf(s, "# Blocks: %d, Size: %d", &count, &size) == 2) {
                     app->actual_block_count = (uint16_t)count;
                     if(size > MAX_TAG_BLOCK_SIZE) size = MAX_TAG_BLOCK_SIZE;
                     app->actual_block_size = (uint8_t)size;
                }
                continue;
            }
            
            
            const char* sep = strchr(s, ':');
            if(sep) {
                const char* data_ptr = sep + 1;
                while(*data_ptr == ' ') data_ptr++;
                
                size_t len = strlen(data_ptr);
                
                for(int i = 0; i < app->actual_block_size && i < MAX_TAG_BLOCK_SIZE; i++) {
                    if((i * 2 + 1) < (int)len) {
                        char hex[3] = {data_ptr[i*2], data_ptr[i*2+1], 0};
                        if(b < MAX_TAG_BLOCKS) {
                             app->full_tag_data[b * app->actual_block_size + i] = (uint8_t)strtol(hex, NULL, 16);
                        }
                    }
                }
                b++;
            }
        }
        
        if(app->actual_block_count == 0) app->actual_block_count = b;
        
        furi_string_free(line);
        ok = true;
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

static bool iso15693_input_callback(InputEvent* event, void* context) {
    Iso15693App* app = context;
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        if(event->key == InputKeyOk) { 
            view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventRetry); 
            return true; 
        }
        if(event->key == InputKeyBack) { 
            view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventExitToMenu); 
            return true; 
        }
        if(event->key == InputKeyUp) {
            with_view_model(app->writer_view, Iso15693Model* m, { if(m->scroll_pos > 0) m->scroll_pos--; }, true);
            return true;
        }
        if(event->key == InputKeyDown) {
            with_view_model(app->writer_view, Iso15693Model* m, { 
                uint16_t display_count = m->actual_block_count > 0 ? m->actual_block_count : 28;
                if(m->scroll_pos < display_count - 5) m->scroll_pos++; 
            }, true);
            return true;
        }
        if(event->key == InputKeyRight) {
            
            bool can_save = false;
            with_view_model(app->writer_view, Iso15693Model* m, {
                if(m->state == Iso15693StateSuccess && m->mode == Iso15693WriterWorkerModeReadAll) {
                    can_save = true;
                }
            }, true);
            
            if(can_save) {
                view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventSavePreview);
                return true;
            }
        }
    }
    return false;
}

static void iso15693_worker_callback(Iso15693WriterWorkerEvent event, void* context) {
    Iso15693App* app = context;
    if(event == Iso15693WriterWorkerEventWriteSuccess) view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventWriteSuccess);
    else if(event == Iso15693WriterWorkerEventReadSuccess) view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventReadSuccess);
    else if(event == Iso15693WriterWorkerEventWriteFail) view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventWriteFail);
    else if(event == Iso15693WriterWorkerEventProgress) {
        uint8_t p = iso15693_writer_worker_get_progress(app->worker);
        uint16_t total = iso15693_writer_worker_get_block_count(app->worker);
        with_view_model(app->writer_view, Iso15693Model* m, { 
            m->progress = p; 
            m->actual_block_count = total;
        }, true);
    }
}

static void iso15693_start_scan(Iso15693App* app) {
    with_view_model(app->writer_view, Iso15693Model* m, {
        m->state = Iso15693StateScanning;
        m->mode = app->current_mode;
        m->progress = 0;
        m->actual_block_count = 0;
        m->scroll_pos = 0;
        m->current_address = app->target_block;
        m->full_dump_ptr = NULL;
        m->lock_status_ptr = NULL;
        if(app->current_mode == Iso15693WriterWorkerModeWriteAFI) {
            m->current_data[0] = app->afi_value;
        } else if(app->current_mode == Iso15693WriterWorkerModeWriteDSFID) {
            m->current_data[0] = app->dsfid_value;
        } else {
            memcpy(m->current_data, app->target_data, MAX_TAG_BLOCK_SIZE);
        }
    }, true);
    iso15693_writer_worker_stop(app->worker);
    iso15693_writer_worker_start(app->worker, app->current_mode, app->target_block, app->full_tag_data, app->lock_status_data, app->target_data, app->afi_value, app->dsfid_value, iso15693_worker_callback, app);
}

static bool iso15693_custom_event_callback(void* context, uint32_t event) {
    Iso15693App* app = context;
    if(event == Iso15693AppEventWriteSuccess || event == Iso15693AppEventReadSuccess) {
        notification_message(app->notifications, &sequence_success);
        
        
        uint16_t detected_count = iso15693_writer_worker_get_block_count(app->worker);
        uint8_t detected_size = iso15693_writer_worker_get_block_size(app->worker);
        
        if(detected_count > 0 && detected_count <= MAX_TAG_BLOCKS) {
            app->actual_block_count = detected_count;
        } else {
            app->actual_block_count = 28;
        }
        
        if(detected_size > 0 && detected_size <= MAX_TAG_BLOCK_SIZE) {
            app->actual_block_size = detected_size;
        } else {
            app->actual_block_size = 4;
        }

        if(app->current_mode == Iso15693WriterWorkerModeReadAll && app->save_to_file) {
            iso15693_save_to_file(app, app->file_path);
            with_view_model(app->writer_view, Iso15693Model* m, { 
                m->actual_block_count = app->actual_block_count;
                m->actual_block_size = app->actual_block_size;
                m->full_dump_ptr = app->full_tag_data;
                m->lock_status_ptr = app->lock_status_data;
            }, true);
        }
        if(app->current_mode == Iso15693WriterWorkerModeReadAll && !app->save_to_file) {
            with_view_model(app->writer_view, Iso15693Model* m, { 
                m->actual_block_count = app->actual_block_count;
                m->actual_block_size = app->actual_block_size;
                m->full_dump_ptr = app->full_tag_data;
                m->lock_status_ptr = app->lock_status_data;
            }, true);
        }
        if(app->current_mode == Iso15693WriterWorkerModeRead) {
            uint8_t* data = &app->full_tag_data[app->target_block * app->actual_block_size];
            memcpy(app->target_data, data, app->actual_block_size);
            with_view_model(app->writer_view, Iso15693Model* m, { 
                m->actual_block_count = app->actual_block_count;
                m->actual_block_size = app->actual_block_size;
                memcpy(m->current_data, data, app->actual_block_size); 
                m->current_address = app->target_block; 
                m->lock_status_ptr = app->lock_status_data;
            }, true);
        } else {
            
             with_view_model(app->writer_view, Iso15693Model* m, { 
                m->actual_block_count = app->actual_block_count;
                m->actual_block_size = app->actual_block_size;
            }, true);
        }
        with_view_model(app->writer_view, Iso15693Model* m, { m->state = Iso15693StateSuccess; }, true);
        return true;
    } else if(event == Iso15693AppEventWriteFail) {
        notification_message(app->notifications, &sequence_error);
        with_view_model(app->writer_view, Iso15693Model* m, { m->state = Iso15693StateFail; }, true);
        return true;
    } else if(event == Iso15693AppEventRetry) { iso15693_start_scan(app); return true; }
    else if(event == Iso15693AppEventExitToMenu) {
        iso15693_writer_worker_stop(app->worker);
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewSubmenu);
        app->current_view = Iso15693WriterViewSubmenu;
        return true;
    } else if(event == Iso15693AppEventSavePreview) {
        app->save_to_file = true;
        app->save_preview_mode = true;
        app->current_view = Iso15693ViewTextInput;
        text_input_reset(app->text_input);
        text_input_set_header_text(app->text_input, "Save Dump As:");
        snprintf(app->file_path, sizeof(app->file_path), "%s/", ISO_APP_DATA_PATH);
        text_input_set_result_callback(app->text_input, iso15693_text_input_callback, app, &app->file_path[strlen(ISO_APP_DATA_PATH) + 1], sizeof(app->file_path) - strlen(ISO_APP_DATA_PATH) - 1, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693ViewTextInput);
        return true;
    } else if(event == Iso15693AppEventFileNameEntered) {
        size_t path_len = strlen(app->file_path);
        if(path_len < 4 || strcmp(&app->file_path[path_len - 4], ".txt") != 0) {
            snprintf(&app->file_path[path_len], sizeof(app->file_path) - path_len, ".txt");
        }
        
        if(app->save_preview_mode) {
           
            iso15693_save_to_file(app, app->file_path);
            app->save_preview_mode = false;
            
            app->current_view = Iso15693WriterViewWriter;
            view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewWriter);
            
        } else {
            
            app->current_view = Iso15693WriterViewWriter;
            view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewWriter);
            iso15693_start_scan(app);
        }
        return true;
    }
    return false;
}

static bool iso15693_navigation_callback(void* context) {
    Iso15693App* app = context;
    if(app->current_view == Iso15693WriterViewSubmenu) return false;
    if(app->current_view == Iso15693ViewTextInput) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventExitToMenu);
        return true;
    }
    view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventExitToMenu);
    return true;
}

static void iso15693_byte_input_done_callback(void* context) {
    Iso15693App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventExitToMenu);
}

static void iso15693_text_input_callback(void* context) {
    Iso15693App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, Iso15693AppEventFileNameEntered);
}

static void iso15693_submenu_callback(void* context, uint32_t index) {
    Iso15693App* app = context;
    if(index == Iso15693MenuSetData) {
        app->current_view = Iso15693WriterViewConfigAddress;
        byte_input_set_result_callback(app->byte_input, iso15693_byte_input_done_callback, NULL, app, app->target_data, MAX_TAG_BLOCK_SIZE);
        byte_input_set_header_text(app->byte_input, "Set Data (32B)");
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewConfigAddress);
        return;
    } else if(index == Iso15693MenuSetAddress) {
        app->current_view = Iso15693WriterViewConfigAddress;
        byte_input_set_result_callback(app->byte_input, iso15693_byte_input_done_callback, NULL, app, &app->target_block, 1);
        byte_input_set_header_text(app->byte_input, "Set Address (1B)");
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewConfigAddress);
        return;
    } else if(index == Iso15693MenuSetAFI) {
        app->current_view = Iso15693WriterViewConfigAddress;
        byte_input_set_result_callback(app->byte_input, iso15693_byte_input_done_callback, NULL, app, &app->afi_value, 1);
        byte_input_set_header_text(app->byte_input, "Set AFI (00-FF)");
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewConfigAddress);
        return;
    } else if(index == Iso15693MenuSetDSFID) {
        app->current_view = Iso15693WriterViewConfigAddress;
        byte_input_set_result_callback(app->byte_input, iso15693_byte_input_done_callback, NULL, app, &app->dsfid_value, 1);
        byte_input_set_header_text(app->byte_input, "Set DSFID (00-FF)");
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewConfigAddress);
        return;
    }

    if(index == Iso15693MenuWriteSingle) app->current_mode = Iso15693WriterWorkerModeWrite;
    else if(index == Iso15693MenuFormatAll) app->current_mode = Iso15693WriterWorkerModeFormat;
    else if(index == Iso15693MenuWriteAll) {
        DialogsFileBrowserOptions browser_options;
        dialog_file_browser_set_basic_options(&browser_options, ".txt", NULL);
        FuriString* path = furi_string_alloc();
        furi_string_set(path, ISO_APP_DATA_PATH);
        FuriString* filepath = furi_string_alloc();
        if(dialog_file_browser_show(app->dialogs, filepath, path, &browser_options)) {
            snprintf(app->file_path, sizeof(app->file_path), "%s", furi_string_get_cstr(filepath));
            furi_string_free(filepath);
            furi_string_free(path);
            iso15693_load_from_file(app, app->file_path);
            app->current_mode = Iso15693WriterWorkerModeWriteFull;
            app->current_view = Iso15693WriterViewWriter;
            view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewWriter);
            iso15693_start_scan(app);
        } else {
            furi_string_free(filepath);
            furi_string_free(path);
        }
        return;
    }
    else if(index == Iso15693MenuReadSingle) app->current_mode = Iso15693WriterWorkerModeRead;
    else if(index == Iso15693MenuReadDump) {
        app->current_mode = Iso15693WriterWorkerModeReadAll;
        app->save_to_file = false;
        app->save_preview_mode = false;
        app->current_view = Iso15693WriterViewWriter;
        view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewWriter);
        iso15693_start_scan(app);
        return;
    }
    
    else if(index == Iso15693MenuLock) app->current_mode = Iso15693WriterWorkerModeLock;
    else if(index == Iso15693MenuLockAll) app->current_mode = Iso15693WriterWorkerModeLockAll;
    else if(index == Iso15693MenuWriteAFI) app->current_mode = Iso15693WriterWorkerModeWriteAFI;
    else if(index == Iso15693MenuLockAFI) app->current_mode = Iso15693WriterWorkerModeLockAFI;
    else if(index == Iso15693MenuWriteDSFID) app->current_mode = Iso15693WriterWorkerModeWriteDSFID;
    else if(index == Iso15693MenuLockDSFID) app->current_mode = Iso15693WriterWorkerModeLockDSFID;

    app->current_view = Iso15693WriterViewWriter;
    view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewWriter);
    iso15693_start_scan(app);
}

static Iso15693App* iso15693_app_alloc() {
    Iso15693App* app = malloc(sizeof(Iso15693App));
    memset(app, 0, sizeof(Iso15693App));
    memset(app->target_data, 0xFF, 4);
    app->afi_value = 0x00;
    app->dsfid_value = 0x00;
    app->save_to_file = false;
    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->view_dispatcher = view_dispatcher_alloc();
    app->worker = iso15693_writer_worker_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, iso15693_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, iso15693_navigation_callback);
    app->submenu = submenu_alloc();
    submenu_set_header(app->submenu, "ISO 15693-3 NFC Tools");
    submenu_add_item(app->submenu, "Write Single Block", Iso15693MenuWriteSingle, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "FF to All Block", Iso15693MenuFormatAll, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Write Dump", Iso15693MenuWriteAll, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Read Single Block", Iso15693MenuReadSingle, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Read Dump", Iso15693MenuReadDump, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Lock Block", Iso15693MenuLock, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Lock All Blocks", Iso15693MenuLockAll, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Set AFI", Iso15693MenuSetAFI, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Write AFI", Iso15693MenuWriteAFI, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Lock AFI", Iso15693MenuLockAFI, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Set DSFID", Iso15693MenuSetDSFID, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Write DSFID", Iso15693MenuWriteDSFID, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Lock DSFID", Iso15693MenuLockDSFID, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Set Data", Iso15693MenuSetData, iso15693_submenu_callback, app);
    submenu_add_item(app->submenu, "Set Address", Iso15693MenuSetAddress, iso15693_submenu_callback, app);
    view_dispatcher_add_view(app->view_dispatcher, Iso15693WriterViewSubmenu, submenu_get_view(app->submenu));
    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, Iso15693WriterViewConfigAddress, byte_input_get_view(app->byte_input));
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, Iso15693ViewTextInput, text_input_get_view(app->text_input));
    app->writer_view = view_alloc();
    view_allocate_model(app->writer_view, ViewModelTypeLockFree, sizeof(Iso15693Model));
    view_set_draw_callback(app->writer_view, iso15693_draw_callback);
    view_set_input_callback(app->writer_view, iso15693_input_callback);
    view_set_context(app->writer_view, app);
    view_dispatcher_add_view(app->view_dispatcher, Iso15693WriterViewWriter, app->writer_view);
    return app;
}

static void iso15693_app_free(Iso15693App* app) {
    iso15693_writer_worker_free(app->worker);
    view_dispatcher_remove_view(app->view_dispatcher, Iso15693WriterViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, Iso15693WriterViewConfigAddress);
    view_dispatcher_remove_view(app->view_dispatcher, Iso15693ViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, Iso15693WriterViewWriter);
    submenu_free(app->submenu); byte_input_free(app->byte_input); text_input_free(app->text_input); view_free(app->writer_view);
    view_dispatcher_free(app->view_dispatcher); furi_record_close(RECORD_NOTIFICATION); furi_record_close(RECORD_DIALOGS); furi_record_close(RECORD_GUI);
    free(app);
}

int32_t iso15693_writer_app(void* p) {
    UNUSED(p);
    Iso15693App* app = iso15693_app_alloc();
    view_dispatcher_switch_to_view(app->view_dispatcher, Iso15693WriterViewSubmenu);
    view_dispatcher_run(app->view_dispatcher);
    iso15693_app_free(app);
    return 0;
}