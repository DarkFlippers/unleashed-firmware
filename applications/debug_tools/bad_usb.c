#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <furi-hal-usb-hid.h>
#include <storage/storage.h>

#define TAG "BadUsb"
#define WORKER_TAG TAG "Worker"

typedef enum {
    EventTypeInput,
    EventTypeWorkerState,
} EventType;

typedef enum {
    WorkerStateDone,
    WorkerStateNoFile,
    WorkerStateScriptError,
    WorkerStateDisconnected,
} WorkerState;

typedef enum {
    AppStateWait,
    AppStateRunning,
    AppStateError,
    AppStateExit,
} AppState;

typedef enum {
    WorkerCmdStart = (1 << 0),
    WorkerCmdStop = (1 << 1),
} WorkerCommandFlags;

// Event message from worker
typedef struct {
    WorkerState state;
    uint16_t line;
} BadUsbWorkerState;

typedef struct {
    union {
        InputEvent input;
        BadUsbWorkerState worker;
    };
    EventType type;
} BadUsbEvent;

typedef struct {
    uint32_t defdelay;
    char msg_text[32];
    osThreadAttr_t thread_attr;
    osThreadId_t thread;
    osMessageQueueId_t event_queue;
} BadUsbParams;

typedef struct {
    char* name;
    uint16_t keycode;
} DuckyKey;

static const DuckyKey ducky_keys[] = {
    {"CTRL", KEY_MOD_LEFT_CTRL},
    {"CONTROL", KEY_MOD_LEFT_CTRL},
    {"SHIFT", KEY_MOD_LEFT_SHIFT},
    {"ALT", KEY_MOD_LEFT_ALT},
    {"GUI", KEY_MOD_LEFT_GUI},
    {"WINDOWS", KEY_MOD_LEFT_GUI},

    {"DOWNARROW", KEY_DOWN_ARROW},
    {"DOWN", KEY_DOWN_ARROW},
    {"LEFTARROW", KEY_LEFT_ARROW},
    {"LEFT", KEY_LEFT_ARROW},
    {"RIGHTARROW", KEY_RIGHT_ARROW},
    {"RIGHT", KEY_RIGHT_ARROW},
    {"UPARROW", KEY_UP_ARROW},
    {"UP", KEY_UP_ARROW},

    {"ENTER", KEY_ENTER},
    {"BREAK", KEY_PAUSE},
    {"PAUSE", KEY_PAUSE},
    {"CAPSLOCK", KEY_CAPS_LOCK},
    {"DELETE", KEY_DELETE},
    {"BACKSPACE", KEY_BACKSPACE},
    {"END", KEY_END},
    {"ESC", KEY_ESC},
    {"ESCAPE", KEY_ESC},
    {"HOME", KEY_HOME},
    {"INSERT", KEY_INSERT},
    {"NUMLOCK", KEY_NUM_LOCK},
    {"PAGEUP", KEY_PAGE_UP},
    {"PAGEDOWN", KEY_PAGE_DOWN},
    {"PRINTSCREEN", KEY_PRINT},
    {"SCROLLOCK", KEY_SCROLL_LOCK},
    {"SPACE", KEY_SPACE},
    {"TAB", KEY_TAB},
    {"MENU", KEY_APPLICATION},
    {"APP", KEY_APPLICATION},
};

static const char ducky_cmd_comment[] = {"REM"};
static const char ducky_cmd_delay[] = {"DELAY"};
static const char ducky_cmd_string[] = {"STRING"};
static const char ducky_cmd_defdelay_1[] = {"DEFAULT_DELAY"};
static const char ducky_cmd_defdelay_2[] = {"DEFAULTDELAY"};

static bool ducky_get_delay_val(char* param, uint32_t* val) {
    uint32_t delay_val = 0;
    if(sscanf(param, "%lu", &delay_val) == 1) {
        *val = delay_val;
        return true;
    }
    return false;
}

static bool ducky_string(char* param) {
    uint32_t i = 0;
    while(param[i] != '\0') {
        furi_hal_hid_kb_press(HID_ASCII_TO_KEY(param[i]));
        furi_hal_hid_kb_release(HID_ASCII_TO_KEY(param[i]));
        i++;
    }
    return true;
}

static uint16_t ducky_get_keycode(char* param, bool accept_chars) {
    for(uint8_t i = 0; i < (sizeof(ducky_keys) / sizeof(ducky_keys[0])); i++) {
        if(strncmp(param, ducky_keys[i].name, strlen(ducky_keys[i].name)) == 0)
            return ducky_keys[i].keycode;
    }
    if((accept_chars) && (strlen(param) > 0)) {
        return (HID_ASCII_TO_KEY(param[0]) & 0xFF);
    }
    return 0;
}

static bool ducky_parse_line(string_t line, BadUsbParams* app) {
    //uint32_t line_len = string_size(line);
    char* line_t = (char*)string_get_cstr(line);
    bool state = false;

    // General commands
    if(strncmp(line_t, ducky_cmd_comment, strlen(ducky_cmd_comment)) == 0) {
        // REM - comment line
        return true;
    } else if(strncmp(line_t, ducky_cmd_delay, strlen(ducky_cmd_delay)) == 0) {
        // DELAY
        line_t = &line_t[args_get_first_word_length(line) + 1];
        uint32_t delay_val = 0;
        state = ducky_get_delay_val(line_t, &delay_val);
        if((state) && (delay_val > 0)) {
            // Using ThreadFlagsWait as delay function allows exiting task on WorkerCmdStop command
            if(osThreadFlagsWait(WorkerCmdStop, osFlagsWaitAny | osFlagsNoClear, delay_val) ==
               WorkerCmdStop)
                return true;
        }
        return state;
    } else if(
        (strncmp(line_t, ducky_cmd_defdelay_1, strlen(ducky_cmd_defdelay_1)) == 0) ||
        (strncmp(line_t, ducky_cmd_defdelay_2, strlen(ducky_cmd_defdelay_2)) == 0)) {
        // DEFAULT_DELAY
        line_t = &line_t[args_get_first_word_length(line) + 1];
        return ducky_get_delay_val(line_t, &app->defdelay);
    } else if(strncmp(line_t, ducky_cmd_string, strlen(ducky_cmd_string)) == 0) {
        // STRING
        if(app->defdelay > 0) {
            if(osThreadFlagsWait(WorkerCmdStop, osFlagsWaitAny | osFlagsNoClear, app->defdelay) ==
               WorkerCmdStop)
                return true;
        }
        line_t = &line_t[args_get_first_word_length(line) + 1];
        return ducky_string(line_t);
    } else {
        // Special keys + modifiers
        uint16_t key = ducky_get_keycode(line_t, false);
        if(key == KEY_NONE) return false;
        if((key & 0xFF00) != 0) {
            // It's a modifier key
            line_t = &line_t[args_get_first_word_length(line) + 1];
            key |= ducky_get_keycode(line_t, true);
        }
        if(app->defdelay > 0) {
            if(osThreadFlagsWait(WorkerCmdStop, osFlagsWaitAny | osFlagsNoClear, app->defdelay) ==
               WorkerCmdStop)
                return true;
        }
        furi_hal_hid_kb_press(key);
        furi_hal_hid_kb_release(key);
        return true;
    }
    return false;
}

static void badusb_worker(void* context) {
    BadUsbParams* app = context;
    FURI_LOG_I(WORKER_TAG, "Init");
    File* script_file = storage_file_alloc(furi_record_open("storage"));
    BadUsbEvent evt;
    string_t line;
    uint32_t line_cnt = 0;
    string_init(line);
    if(storage_file_open(script_file, "/ext/badusb.txt", FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[16];
        uint16_t ret;
        uint32_t flags =
            osThreadFlagsWait(WorkerCmdStart | WorkerCmdStop, osFlagsWaitAny, osWaitForever);
        if(flags & WorkerCmdStart) {
            FURI_LOG_I(WORKER_TAG, "Start");
            do {
                ret = storage_file_read(script_file, buffer, 16);
                for(uint16_t i = 0; i < ret; i++) {
                    if(buffer[i] == '\n' && string_size(line) > 0) {
                        line_cnt++;
                        if(ducky_parse_line(line, app) == false) {
                            ret = 0;
                            FURI_LOG_E(WORKER_TAG, "Unknown command at line %lu", line_cnt);
                            evt.type = EventTypeWorkerState;
                            evt.worker.state = WorkerStateScriptError;
                            evt.worker.line = line_cnt;
                            osMessageQueuePut(app->event_queue, &evt, 0, osWaitForever);
                            break;
                        }
                        flags = osThreadFlagsGet();
                        if(flags == WorkerCmdStop) {
                            ret = 0;
                            break;
                        }
                        string_reset(line);
                    } else {
                        string_push_back(line, buffer[i]);
                    }
                }
            } while(ret > 0);
        }
    } else {
        FURI_LOG_E(WORKER_TAG, "Script file open error");
        evt.type = EventTypeWorkerState;
        evt.worker.state = WorkerStateNoFile;
        osMessageQueuePut(app->event_queue, &evt, 0, osWaitForever);
    }
    string_reset(line);
    string_clear(line);

    furi_hal_hid_kb_release_all();
    storage_file_close(script_file);
    storage_file_free(script_file);

    FURI_LOG_I(WORKER_TAG, "End");
    evt.type = EventTypeWorkerState;
    evt.worker.state = WorkerStateDone;
    osMessageQueuePut(app->event_queue, &evt, 0, osWaitForever);

    furi_hal_hid_kb_release_all();

    osThreadExit();
}

static void bad_usb_render_callback(Canvas* canvas, void* ctx) {
    BadUsbParams* app = (BadUsbParams*)ctx;

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Bad USB test");

    if(strlen(app->msg_text) > 0) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 0, 62, app->msg_text);
    }
}

static void bad_usb_input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    BadUsbEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

int32_t bad_usb_app(void* p) {
    BadUsbParams* app = furi_alloc(sizeof(BadUsbParams));
    app->event_queue = osMessageQueueNew(8, sizeof(BadUsbEvent), NULL);
    furi_check(app->event_queue);
    ViewPort* view_port = view_port_alloc();

    UsbMode usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_set_config(UsbModeHid);

    view_port_draw_callback_set(view_port, bad_usb_render_callback, app);
    view_port_input_callback_set(view_port, bad_usb_input_callback, app->event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    app->thread = NULL;
    app->thread_attr.name = "bad_usb_worker";
    app->thread_attr.stack_size = 2048;
    app->thread = osThreadNew(badusb_worker, app, &app->thread_attr);
    bool worker_running = true;
    AppState app_state = AppStateWait;
    snprintf(app->msg_text, sizeof(app->msg_text), "Press [OK] to start");
    view_port_update(view_port);

    BadUsbEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(app->event_queue, &event, NULL, osWaitForever);

        if(event_status == osOK) {
            if(event.type == EventTypeInput) {
                if(event.input.type == InputTypeShort && event.input.key == InputKeyBack) {
                    if(worker_running) {
                        osThreadFlagsSet(app->thread, WorkerCmdStop);
                        app_state = AppStateExit;
                    } else
                        break;
                }

                if(event.input.type == InputTypeShort && event.input.key == InputKeyOk) {
                    if(worker_running) {
                        app_state = AppStateRunning;
                        osThreadFlagsSet(app->thread, WorkerCmdStart);
                        snprintf(app->msg_text, sizeof(app->msg_text), "Running...");
                        view_port_update(view_port);
                    }
                }
            } else if(event.type == EventTypeWorkerState) {
                FURI_LOG_I(TAG, "ev: %d", event.worker.state);
                if(event.worker.state == WorkerStateDone) {
                    worker_running = false;
                    if(app_state == AppStateExit)
                        break;
                    else if(app_state == AppStateRunning) {
                        //done
                        app->thread = osThreadNew(badusb_worker, app, &app->thread_attr);
                        worker_running = true;
                        app_state = AppStateWait;
                        snprintf(app->msg_text, sizeof(app->msg_text), "Press [OK] to start");
                        view_port_update(view_port);
                    }
                } else if(event.worker.state == WorkerStateNoFile) {
                    app_state = AppStateError;
                    snprintf(app->msg_text, sizeof(app->msg_text), "File not found!");
                    view_port_update(view_port);
                } else if(event.worker.state == WorkerStateScriptError) {
                    app_state = AppStateError;
                    snprintf(
                        app->msg_text,
                        sizeof(app->msg_text),
                        "Error at line %u",
                        event.worker.line);
                    view_port_update(view_port);
                }
            }
        }
    }
    furi_hal_usb_set_config(usb_mode_prev);

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);

    osMessageQueueDelete(app->event_queue);
    free(app);

    return 0;
}
