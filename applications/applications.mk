APP_DIR		= $(PROJECT_ROOT)/applications
LIB_DIR 	= $(PROJECT_ROOT)/lib

CFLAGS		+= -I$(APP_DIR)

# Use APP_* for autostart app
# Use BUILD_* for add app to build

APP_RELEASE ?= 0
ifeq ($(APP_RELEASE), 1)
APP_MENU = 1
APP_NFC  = 1
APP_POWER = 1
APP_BT = 1
APP_CLI = 1
BUILD_IRDA  = 1
APP_DOLPHIN = 1
BUILD_EXAMPLE_BLINK = 1
BUILD_EXAMPLE_UART_WRITE = 1
BUILD_EXAMPLE_INPUT_DUMP = 1
BUILD_CC1101 = 1
BUILD_LF_RFID = 1
BUILD_SPEAKER_DEMO = 1
BUILD_VIBRO_DEMO = 1
BUILD_SD_TEST = 1
BUILD_GPIO_DEMO = 1
BUILD_MUSIC_PLAYER = 1
BUILD_FLOOPPER_BLOOPPER = 1
BUILD_IBUTTON = 1
endif

APP_NFC ?= 0
ifeq ($(APP_NFC), 1)
APP_MENU	= 1
CFLAGS		+= -DAPP_NFC
C_SOURCES	+= $(wildcard $(APP_DIR)/nfc/*.c)
endif

APP_DOLPHIN ?= 0
ifeq ($(APP_DOLPHIN), 1)
APP_MENU	= 1
CFLAGS		+= -DAPP_DOLPHIN
C_SOURCES	+= $(wildcard $(APP_DIR)/dolphin/*.c)
endif

APP_POWER ?= 0
ifeq ($(APP_POWER), 1)
APP_GUI		= 1
APP_CLI		= 1
CFLAGS		+= -DAPP_POWER
C_SOURCES	+= $(wildcard $(APP_DIR)/power/*.c)
endif

APP_BT ?= 0
ifeq ($(APP_BT), 1)
APP_CLI		= 1
CFLAGS		+= -DAPP_BT
C_SOURCES	+= $(wildcard $(APP_DIR)/bt/*.c)
endif

APP_MENU ?= 0
ifeq ($(APP_MENU), 1)
CFLAGS += -DAPP_MENU
BUILD_MENU = 1
endif
BUILD_MENU ?= 0
ifeq ($(BUILD_MENU), 1)
APP_INPUT	= 1
APP_GUI		= 1
CFLAGS		+= -DBUILD_MENU
C_SOURCES	+= $(wildcard $(APP_DIR)/menu/*.c)
C_SOURCES	+= $(wildcard $(APP_DIR)/app-loader/*.c)
endif

APP_TEST	?= 0
ifeq ($(APP_TEST), 1)
CFLAGS		+= -DAPP_TEST
C_SOURCES	+= $(APP_DIR)/tests/furiac_test.c
C_SOURCES	+= $(APP_DIR)/tests/furi_record_test.c
C_SOURCES	+= $(APP_DIR)/tests/test_index.c
C_SOURCES	+= $(APP_DIR)/tests/minunit_test.c
C_SOURCES	+= $(APP_DIR)/tests/furi_valuemutex_test.c
C_SOURCES	+= $(APP_DIR)/tests/furi_pubsub_test.c
C_SOURCES	+= $(APP_DIR)/tests/furi_memmgr_test.c
C_SOURCES	+= $(APP_DIR)/tests/furi_value_expanders_test.c
C_SOURCES	+= $(APP_DIR)/tests/furi_event_test.c
endif

APP_EXAMPLE_BLINK ?= 0
ifeq ($(APP_EXAMPLE_BLINK), 1)
CFLAGS		+= -DAPP_EXAMPLE_BLINK
BUILD_EXAMPLE_BLINK = 1
endif
BUILD_EXAMPLE_BLINK ?= 0
ifeq ($(BUILD_EXAMPLE_BLINK), 1)
CFLAGS		+= -DBUILD_EXAMPLE_BLINK
C_SOURCES	+= $(APP_DIR)/examples/blink.c
APP_INPUT = 1
endif

APP_EXAMPLE_UART_WRITE ?= 0
ifeq ($(APP_EXAMPLE_UART_WRITE), 1)
CFLAGS		+= -DAPP_EXAMPLE_UART_WRITE
BUILD_EXAMPLE_UART_WRITE = 1
endif
BUILD_EXAMPLE_UART_WRITE ?= 0
ifeq ($(BUILD_EXAMPLE_UART_WRITE), 1)
CFLAGS		+= -DBUILD_EXAMPLE_UART_WRITE
C_SOURCES	+= $(APP_DIR)/examples/uart_write.c
endif

APP_EXAMPLE_IPC ?= 0
ifeq ($(APP_EXAMPLE_IPC), 1)
CFLAGS		+= -DAPP_EXAMPLE_IPC
BUILD_EXAMPLE_IPC = 1
endif
BUILD_EXAMPLE_IPC ?= 0
ifeq ($(BUILD_EXAMPLE_IPC), 1)
CFLAGS		+= -DBUILD_EXAMPLE_IPC
C_SOURCES	+= $(APP_DIR)/examples/ipc.c
endif

APP_EXAMPLE_INPUT_DUMP ?= 0
ifeq ($(APP_EXAMPLE_INPUT_DUMP), 1)
CFLAGS		+= -DAPP_EXAMPLE_INPUT_DUMP
BUILD_EXAMPLE_INPUT_DUMP = 1
endif
BUILD_EXAMPLE_INPUT_DUMP ?= 0
ifeq ($(BUILD_EXAMPLE_INPUT_DUMP), 1)
CFLAGS		+= -DBUILD_EXAMPLE_INPUT_DUMP
C_SOURCES	+= $(APP_DIR)/examples/input_dump.c
APP_INPUT = 1
endif

APP_EXAMPLE_QRCODE ?= 0
ifeq ($(APP_EXAMPLE_QRCODE), 1)
CFLAGS		+= -DAPP_EXAMPLE_QRCODE
BUILD_EXAMPLE_QRCODE = 1
endif
BUILD_EXAMPLE_QRCODE ?= 0
ifeq ($(BUILD_EXAMPLE_QRCODE), 1)
CFLAGS		+= -DBUILD_EXAMPLE_QRCODE
C_SOURCES	+= $(APP_DIR)/examples/u8g2_qrcode.c
C_SOURCES	+= $(LIB_DIR)/qrcode/qrcode.c
APP_DISPLAY = 1
endif

# deprecated
APP_EXAMPLE_DISPLAY ?= 0
ifeq ($(APP_EXAMPLE_DISPLAY), 1)
CFLAGS		+= -DAPP_EXAMPLE_DISPLAY
C_SOURCES	+= $(APP_DIR)/examples/u8g2_example.c
APP_DISPLAY = 1
endif

APP_EXAMPLE_FATFS ?= 0
ifeq ($(APP_EXAMPLE_FATFS), 1)
CFLAGS		+= -DAPP_EXAMPLE_FATFS
BUILD_EXAMPLE_FATFS = 1
endif
BUILD_EXAMPLE_FATFS ?= 0
ifeq ($(BUILD_EXAMPLE_FATFS), 1)
CFLAGS		+= -DBUILD_EXAMPLE_FATFS
C_SOURCES	+= $(APP_DIR)/examples/fatfs_list.c
APP_INPUT = 1
APP_DISPLAY = 1
endif

APP_CC1101 ?= 0
ifeq ($(APP_CC1101), 1)
CFLAGS		+= -DAPP_CC1101
BUILD_CC1101 = 1
endif
BUILD_CC1101 ?= 0
ifeq ($(BUILD_CC1101), 1)
CFLAGS		+= -DBUILD_CC1101
C_SOURCES	+= $(wildcard $(APP_DIR)/cc1101-workaround/*.c)
CPP_SOURCES	+= $(wildcard $(APP_DIR)/cc1101-workaround/*.cpp)
APP_INPUT = 1
APP_GUI = 1
APP_CLI = 1
endif

APP_LF_RFID ?= 0
ifeq ($(APP_LF_RFID), 1)
CFLAGS		+= -DAPP_LF_RFID
BUILD_LF_RFID = 1
endif
BUILD_LF_RFID ?= 0
ifeq ($(BUILD_LF_RFID), 1)
CFLAGS		+= -DBUILD_LF_RFID
C_SOURCES	+= $(wildcard $(APP_DIR)/lf-rfid/*.c)
CPP_SOURCES	+= $(wildcard $(APP_DIR)/lf-rfid/*.cpp)
APP_INPUT = 1
APP_GUI = 1
endif

APP_IRDA ?= 0
ifeq ($(APP_IRDA), 1)
CFLAGS		+= -DAPP_IRDA
BUILD_IRDA = 1
endif
BUILD_IRDA ?= 0
ifeq ($(BUILD_IRDA), 1)
CFLAGS		+= -DBUILD_IRDA
C_SOURCES	+= $(wildcard $(APP_DIR)/irda/*.c)
APP_INPUT = 1
APP_GUI = 1
endif

APP_SD_TEST ?= 0
ifeq ($(APP_SD_TEST), 1)
CFLAGS		+= -DAPP_SD_TEST
BUILD_SD_TEST = 1
endif
BUILD_SD_TEST ?= 0
ifeq ($(BUILD_SD_TEST), 1)
CFLAGS		+= -DBUILD_SD_TEST
CPP_SOURCES	+= $(wildcard $(APP_DIR)/sd-card-test/*.cpp)
APP_INPUT = 1
APP_GUI = 1
endif

APP_SPEAKER_DEMO ?= 0
ifeq ($(APP_SPEAKER_DEMO), 1)
CFLAGS		+= -DAPP_SPEAKER_DEMO
BUILD_SPEAKER_DEMO = 1
endif
BUILD_SPEAKER_DEMO ?= 0
ifeq ($(BUILD_SPEAKER_DEMO), 1)
CFLAGS		+= -DBUILD_SPEAKER_DEMO
C_SOURCES	+= $(wildcard $(APP_DIR)/coreglitch_demo_0/*.c)
APP_INPUT = 1
APP_GUI = 1
endif

BUILD_VIBRO_DEMO ?= 0
ifeq ($(BUILD_VIBRO_DEMO), 1)
CFLAGS		+= -DBUILD_VIBRO_DEMO
C_SOURCES	+= $(wildcard $(APP_DIR)/examples/vibro.c)
APP_INPUT = 1
endif

APP_GPIO_DEMO ?= 0
ifeq ($(APP_GPIO_DEMO), 1)
CFLAGS		+= -DAPP_GPIO_DEMO
BUILD_GPIO_DEMO = 1
endif
BUILD_GPIO_DEMO ?= 0
ifeq ($(BUILD_GPIO_DEMO), 1)
CFLAGS		+= -DBUILD_GPIO_DEMO
C_SOURCES	+= $(wildcard $(APP_DIR)/gpio-tester/*.c)
endif

APP_MUSIC_PLAYER ?= 0
ifeq ($(APP_MUSIC_PLAYER), 1)
CFLAGS		+= -DAPP_MUSIC_PLAYER
BUILD_MUSIC_PLAYER = 1
endif
BUILD_MUSIC_PLAYER ?= 0
ifeq ($(BUILD_MUSIC_PLAYER), 1)
CFLAGS		+= -DBUILD_MUSIC_PLAYER
C_SOURCES	+= $(wildcard $(APP_DIR)/music-player/*.c)
endif

APP_FLOOPPER_BLOOPPER ?= 0
ifeq ($(APP_FLOOPPER_BLOOPPER), 1)
CFLAGS		+= -DAPP_FLOOPPER_BLOOPPER
BUILD_FLOOPPER_BLOOPPER = 1
endif
BUILD_FLOOPPER_BLOOPPER ?= 0
ifeq ($(BUILD_FLOOPPER_BLOOPPER), 1)
CFLAGS		+= -DBUILD_FLOOPPER_BLOOPPER
C_SOURCES	+= $(wildcard $(APP_DIR)/floopper-bloopper/*.c)
endif

APP_IBUTTON ?= 0
ifeq ($(APP_IBUTTON), 1)
CFLAGS		+= -DAPP_IBUTTON
BUILD_IBUTTON = 1
endif
BUILD_IBUTTON ?= 0
ifeq ($(BUILD_IBUTTON), 1)
CFLAGS		+= -DBUILD_IBUTTON
CPP_SOURCES	+= $(APP_DIR)/ibutton/ibutton.cpp
endif

APP_SDNFC ?= 0
ifeq ($(APP_SDNFC), 1)
CFLAGS		+= -DAPP_SDNFC
BUILD_SDNFC = 1
endif
BUILD_SDNFC ?= 0
ifeq ($(BUILD_SDNFC), 1)
CFLAGS		+= -DBUILD_SDNFC
CPP_SOURCES	+= $(APP_DIR)/sdnfc/sdnfc.cpp
endif
# device drivers

APP_GUI	?= 0
ifeq ($(APP_GUI), 1)
CFLAGS		+= -DAPP_GUI
C_SOURCES	+= $(wildcard $(APP_DIR)/gui/*.c)
C_SOURCES	+= $(wildcard $(APP_DIR)/backlight-control/*.c)
endif

# deprecated
ifeq ($(APP_DISPLAY), 1)
CFLAGS		+= -DAPP_DISPLAY
C_SOURCES	+= $(APP_DIR)/display-u8g2/display-u8g2.c
endif

APP_INPUT	?= 0
ifeq ($(APP_INPUT), 1)
CFLAGS		+= -DAPP_INPUT
C_SOURCES	+= $(APP_DIR)/input/input.c
endif

APP_CLI ?= 0
ifeq ($(APP_CLI), 1)
APP_GUI		= 1
CFLAGS		+= -DAPP_CLI
C_SOURCES	+= $(wildcard $(APP_DIR)/cli/*.c)
endif
