#pragma once

#include <furi.h>
#include <furi_hal_spi.h>
#include <furi_hal_spi_config.h>
#include "spi_mem_app.h"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
#include <storage/storage.h>
#include <toolbox/path.h>
#include <toolbox/random_name.h>
#include "scenes/spi_mem_scene.h"
#include "lib/spi/spi_mem_worker.h"
#include "spi_mem_manager_icons.h"
#include "views/spi_mem_view_progress.h"
#include "views/spi_mem_view_detect.h"

#define TAG "SPIMem"
#define SPI_MEM_FILE_EXTENSION ".bin"
#define SPI_MEM_FILE_FOLDER EXT_PATH("spimem")
#define SPI_MEM_FILE_NAME_SIZE 100
#define SPI_MEM_TEXT_BUFFER_SIZE 128

typedef enum {
    SPIMemModeRead,
    SPIMemModeWrite,
    SPIMemModeCompare,
    SPIMemModeErase,
    SPIMemModeDelete,
    SPIMemModeUnknown
} SPIMemMode;

struct SPIMemApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    NotificationApp* notifications;
    FuriString* file_path;
    DialogsApp* dialogs;
    Storage* storage;
    File* file;
    Widget* widget;
    SPIMemWorker* worker;
    SPIMemChip* chip_info;
    found_chips_t found_chips;
    uint32_t chip_vendor_enum;
    SPIMemProgressView* view_progress;
    SPIMemDetectView* view_detect;
    TextInput* text_input;
    SPIMemMode mode;
    char text_buffer[SPI_MEM_TEXT_BUFFER_SIZE + 1];
};

typedef enum {
    SPIMemViewSubmenu,
    SPIMemViewDialogEx,
    SPIMemViewPopup,
    SPIMemViewWidget,
    SPIMemViewTextInput,
    SPIMemViewProgress,
    SPIMemViewDetect
} SPIMemView;

typedef enum {
    SPIMemCustomEventViewReadCancel,
    SPIMemCustomEventViewVerifySkip,
    SPIMemCustomEventTextEditResult,
    SPIMemCustomEventPopupBack
} SPIMemCustomEvent;
