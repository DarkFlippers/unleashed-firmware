#ifndef __SWD_PROBE_APP_H
#define __SWD_PROBE_APP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_speaker.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <dialogs/dialogs.h>
#include <input/input.h>
#include <storage/storage.h>
#include <dolphin/dolphin.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "usb_uart.h"

#define TAG "SWD"

/* short debug message */
#define DBGS(format) furi_log_print_format(FuriLogLevelDebug, TAG, "%s: " format, __FUNCTION__)
/* formatted debug message */
#define DBG(format, ...) \
    furi_log_print_format(FuriLogLevelDebug, TAG, "%s: " format, __FUNCTION__, __VA_ARGS__)
/* log message*/
#define LOG(...) furi_log_print_format(FuriLogLevelDefault, TAG, __VA_ARGS__)

#define COUNT(x) ((size_t)(sizeof(x) / sizeof((x)[0])))
#define ARRAY_SIZE(x) COUNT(x)

#define SWD_DELAY_US 0
#define TIMER_HZ 25
#define TIMEOUT 3
#define QUEUE_SIZE 8
#define IDLE_BITS 8
#define CLOCK_DELAY 0

#define MAX_FILE_LENGTH 128
#define SCRIPT_MAX_LINES 1000

typedef enum {
    ModePageScan = 0,
    ModePageFound = 1,
    ModePageDPRegs = 2,
    ModePageDPID = 3,
    ModePageAPID = 4,
    ModePageCount = 5,
    ModePageHexDump = 0x100,
    ModePageScript = 0x101,
    ModePageCoresight = 0x102,
} ModePages;

#define CDBGPWRUPREQ (1 << 28)
#define CDBGPWRUPACK (1 << 29)
#define CSYSPWRUPREQ (1 << 30)
#define CSYSPWRUPACK (1 << 31)
#define WDATAERR (1 << 7)
#define STICKYERR (1 << 5)
#define STAT_ERROR_FLAGS (WDATAERR | STICKYERR)

#define REG_IDCODE 0x00
#define REG_CTRLSTAT 0x01
#define REG_CTRLSTAT_BANK 0x00
#define REG_DLCR 0x01
#define REG_DLCR_BANK 0x01
#define REG_TARGETID 0x01
#define REG_TARGETID_BANK 0x02
#define REG_DLPIDR 0x01
#define REG_DLPIDR_BANK 0x03
#define REG_EVENTSTAT 0x01
#define REG_EVENTSTAT_BANK 0x04

#define REG_SELECT 0x02

#define MEMAP_CSW 0x00
#define MEMAP_TAR 0x04
#define MEMAP_DRW 0x0C
#define AP_IDR 0xFC
#define AP_BASE 0xF8

#define SCS_CPUID 0xE000ED00u
#define SCS_CPACR 0xE000ED88u
#define SCS_DHCSR 0xE000EDF0u
#define SCS_DHCSR_S_HALT (1u << 17)
#define SCS_DHCSR_C_MASKINTS (1u << 3)
#define SCS_DHCSR_C_STEP (1u << 2)
#define SCS_DHCSR_C_HALT (1u << 1)
#define SCS_DHCSR_C_DEBUGEN (1u << 0)
#define SCS_DHCSR_KEY 0xA05F0000u
#define SCS_DCRSR 0xE000EDF4u
#define SCS_DCRSR_RD 0x00000000u
#define SCS_DCRSR_WR 0x00010000u
#define SCS_DCRDR 0xE000EDF8u
#define SCS_DEMCR 0xE000EDFCu

typedef enum { KeyNone, KeyUp, KeyRight, KeyDown, KeyLeft, KeyOK } KeyCode;

typedef enum {
    EventTimerTick,
    EventKeyPress,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef struct {
    uint32_t ctrlstat;
    bool ctrlstat_ok;
    uint32_t dlcr;
    bool dlcr_ok;
    uint32_t dlpidr;
    bool dlpidr_ok;
    uint32_t dpidr;
    bool dpidr_ok;
    uint32_t eventstat;
    bool eventstat_ok;
    uint32_t select;
    bool select_ok;
    uint32_t targetid;
    bool targetid_ok;
} swd_dpreg_t;

typedef struct {
    bool ok;
    bool tested;
    uint8_t revision;
    uint16_t designer;
    uint8_t class;
    uint8_t variant;
    uint8_t type;
    uint32_t base;
} swd_apidr_info_t;

typedef struct {
    uint8_t revision;
    uint8_t partno;
    uint8_t version;
    uint16_t designer;
} swd_dpidr_info_t;

typedef struct {
    uint8_t revision;
    uint16_t partno;
    uint16_t designer;
} swd_targetid_info_t;

typedef struct sScriptContext ScriptContext;

typedef struct {
    Storage* storage;
    Gui* gui;
    DialogsApp* dialogs;
    NotificationApp* notification;

    FuriTimer* timer;
    UsbUart* uart;
    ViewPort* view_port;

    FuriMessageQueue* event_queue;
    FuriMutex* swd_mutex;
    FuriMutex* gui_mutex;

    swd_targetid_info_t targetid_info;
    swd_dpidr_info_t dpidr_info;
    swd_dpreg_t dp_regs;
    swd_apidr_info_t apidr_info[256];

    ScriptContext* script;
    ScriptContext* commandline;

    uint8_t timeout_overdue;
    uint32_t loop_count;
    uint8_t current_mask_id;
    uint32_t current_mask;
    uint8_t io_swc;
    uint8_t io_swd;
    uint8_t io_num_swc;
    uint8_t io_num_swd;
    int32_t detected_timeout;
    uint32_t swd_clock_delay;
    uint32_t swd_idle_bits;
    bool detected;
    bool detected_device;
    bool detected_notified;
    uint32_t mode_page;
    uint8_t ap_pos;
    uint8_t ap_scanned;

    uint32_t coresight_pos[16];
    uint32_t coresight_count[16];
    uint8_t coresight_level;
    uint32_t coresight_bases[16];

    uint32_t hex_addr;
    uint8_t hex_select;
    uint8_t hex_buffer[32];
    uint8_t hex_buffer_valid[8];

    char state_string[64];
    char script_detected[MAX_FILE_LENGTH];
    bool script_detected_executed;
} AppFSM;

struct sScriptContext {
    AppFSM* app;
    ScriptContext* parent;
    char filename[MAX_FILE_LENGTH];

    /* when used with string input */
    char line_data[128];
    uint64_t line_pos;

    /* when used with file input */
    File* script_file;

    uint64_t position;
    uint32_t selected_ap;
    uint32_t max_tries;
    uint32_t block_size;

    bool abort;
    bool restart;
    bool errors_ignore;
    bool status_ignore;
    bool goto_active;
    char goto_label[64];
};

typedef struct {
    const char* prefix;
    bool (*func)(ScriptContext* ctx);
} ScriptFunctionInfo;

uint8_t swd_read_memory(AppFSM* const ctx, uint8_t ap, uint32_t address, uint32_t* data);

#endif