#include "u8g2/u8g2.h"
#include "fatfs/ff.h"
#include "flipper_v2.h"
#include <stdio.h>

extern uint8_t BSP_SD_Init();

// TODO currently we have small stack, so it will be static
FuriRecordSubscriber* furi_log;
#define STR_BUFFER_SIZE 128
char str_buffer[STR_BUFFER_SIZE];
uint8_t line_current = 0;
uint16_t line_position = 0;

// TODO this should be in the target driver
FATFS SD_FatFs;
char SD_Path[4];

typedef enum {
    EventTypeStart,
    EventTypeKey,
} AppEventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    AppEventType type;
} AppEvent;

static void event_cb(const void* value, void* ctx) {
    QueueHandle_t event_queue = (QueueHandle_t)ctx;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *(InputEvent*)value;
    xQueueSend(event_queue, (void*)&event, 0);
}

void fatfs_list(void* p) {
    const uint8_t line_size = 10;
    const uint8_t lines_on_display = 6;

    uint8_t bsp_result;
    FRESULT result;
    DIR dir;
    FILINFO fno;
    AppEvent event;

    QueueHandle_t event_queue = xQueueCreate(2, sizeof(AppEvent));

    furi_log = get_default_log();
    fuprintf(furi_log, "[fatfs_list] app start\n");

    FuriRecordSubscriber* fb_record =
        furi_open_deprecated("u8g2_fb", false, false, NULL, NULL, NULL);
    if(fb_record == NULL) {
        fuprintf(furi_log, "[fatfs_list] cannot create fb record\n");
        furiac_exit(NULL);
    }

    PubSub* event_record = furi_open("input_events");
    if(event_record == NULL) {
        fuprintf(furi_log, "[fatfs_list] cannot open input_events record\n");
        furiac_exit(NULL);
    }
    PubSubItem* subscription = subscribe_pubsub(event_record, event_cb, event_queue);
    if(subscription == NULL) {
        fuprintf(furi_log, "[fatfs_list] cannot register input_events callback\n");
        furiac_exit(NULL);
    }

    bsp_result = BSP_SD_Init();

    if(bsp_result != 0) {
        fuprintf(furi_log, "[fatfs_list] SD card init error\n");
        furiac_exit(NULL);
    }

    result = f_mount(&SD_FatFs, (TCHAR const*)SD_Path, 1);

    if(result != FR_OK) {
        fuprintf(furi_log, "[fatfs_list] SD card mount error\n");
        furiac_exit(NULL);
    }

    // ok, now we can work with sd card

    // send start event
    event.type = EventTypeStart;
    xQueueSend(event_queue, (void*)&event, 0);

    while(1) {
        if(xQueueReceive(event_queue, (void*)&event, portMAX_DELAY)) {
            // process buttons event
            if(event.type == EventTypeKey) {
                // button pressed
                if(event.value.input.state == true) {
                    if(event.value.input.input == InputUp && line_position > 0) {
                        line_position--;
                    }
                    if(event.value.input.input == InputDown) {
                        line_position++;
                    }
                }
            }

            line_current = 1;

            // open root dir
            result = f_opendir(&dir, "");

            while(1) {
                // read a directory item
                result = f_readdir(&dir, &fno);

                if(result != FR_OK) {
                    // cannot read dir
                    break;
                }

                if(fno.fname[0] == 0) {
                    // Break on end of dir
                    break;
                }

                // draw files on display
                if(line_current > line_position &&
                   line_current <= (line_position + lines_on_display)) {
                    if(fno.fattrib & AM_DIR) {
                        snprintf(str_buffer, STR_BUFFER_SIZE, "DIR %s\n", fno.fname);
                    } else {
                        snprintf(str_buffer, STR_BUFFER_SIZE, "FIL %s\n", fno.fname);
                    }
                    fuprintf(furi_log, str_buffer);
                }

                line_current++;
            }

            result = f_closedir(&dir);

            furi_commit(fb_record);
        }
    }

    furiac_exit(NULL);
}
