LIB_DIR			= $(PROJECT_ROOT)/lib

# TODO: some places use lib/header.h includes, is it ok?
CFLAGS			+= -I$(LIB_DIR)

# Mlib containers
CFLAGS			+= -I$(LIB_DIR)/mlib

# U8G2 display library
U8G2_DIR		= $(LIB_DIR)/u8g2
CFLAGS			+= -I$(U8G2_DIR)
C_SOURCES		+= $(U8G2_DIR)/u8x8_d_st7565.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_d_setup.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_intersection.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_setup.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_d_memory.c
C_SOURCES		+= $(U8G2_DIR)/u8x8_cad.c
C_SOURCES		+= $(U8G2_DIR)/u8x8_byte.c
C_SOURCES		+= $(U8G2_DIR)/u8x8_gpio.c
C_SOURCES		+= $(U8G2_DIR)/u8x8_display.c
C_SOURCES		+= $(U8G2_DIR)/u8x8_setup.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_hvline.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_line.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_ll_hvline.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_circle.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_box.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_buffer.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_font.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_fonts.c
C_SOURCES		+= $(U8G2_DIR)/u8x8_8x8.c
C_SOURCES		+= $(U8G2_DIR)/u8g2_bitmap.c

FATFS_DIR		= $(LIB_DIR)/fatfs
C_SOURCES		+= $(FATFS_DIR)/ff.c
C_SOURCES		+= $(FATFS_DIR)/ff_gen_drv.c
C_SOURCES		+= $(FATFS_DIR)/diskio.c
C_SOURCES		+= $(FATFS_DIR)/option/unicode.c

# Little FS
LITTLEFS_DIR	= $(LIB_DIR)/littlefs
CFLAGS			+= -I$(LITTLEFS_DIR)
C_SOURCES		+= $(LITTLEFS_DIR)/lfs.c
C_SOURCES		+= $(LITTLEFS_DIR)/lfs_util.c

ifeq ($(APP_NFC), 1)
ST25RFAL002_DIR	= $(LIB_DIR)/ST25RFAL002
CFLAGS			+= -I$(ST25RFAL002_DIR)
CFLAGS			+= -I$(ST25RFAL002_DIR)/include
CFLAGS			+= -I$(ST25RFAL002_DIR)/source/st25r3916
C_SOURCES		+= $(wildcard $(ST25RFAL002_DIR)/*.c)
C_SOURCES		+= $(wildcard $(ST25RFAL002_DIR)/source/*.c)
C_SOURCES		+= $(wildcard $(ST25RFAL002_DIR)/source/st25r3916/*.c)

CFLAGS			+= -I$(LIB_DIR)/nfc_protocols
C_SOURCES		+= $(wildcard $(LIB_DIR)/nfc_protocols/*.c)
endif

# callback connector (C to CPP) library
CFLAGS			+= -I$(LIB_DIR)/callback-connector

# app template library
CFLAGS			+= -I$(LIB_DIR)/app-template

# add C scene template
CFLAGS			+= -I$(LIB_DIR)/app_scene_template

# fnv1a hash library
CFLAGS			+= -I$(LIB_DIR)/fnv1a-hash
C_SOURCES		+= $(LIB_DIR)/fnv1a-hash/fnv1a-hash.c

# onewire library
ONEWIRE_DIR		= $(LIB_DIR)/onewire
CFLAGS			+= -I$(ONEWIRE_DIR)
CPP_SOURCES		+= $(wildcard $(ONEWIRE_DIR)/*.cpp)

# cyfral library
CYFRAL_DIR		= $(LIB_DIR)/cyfral
CFLAGS			+= -I$(CYFRAL_DIR)
CPP_SOURCES		+= $(wildcard $(CYFRAL_DIR)/*.cpp)

# common apps api
CFLAGS			+= -I$(LIB_DIR)/common-api

# drivers
CFLAGS			+= -I$(LIB_DIR)/drivers
C_SOURCES		+= $(wildcard $(LIB_DIR)/drivers/*.c)

#version
CFLAGS			+= -I$(LIB_DIR)/version
C_SOURCES		+= $(LIB_DIR)/version/version.c

#file reader
CFLAGS			+= -I$(LIB_DIR)/file_reader
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/file_reader/*.cpp)

#irda lib
CFLAGS			+= -I$(LIB_DIR)/irda
C_SOURCES		+= $(wildcard $(LIB_DIR)/irda/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/irda/*/*.c)

#args lib
CFLAGS			+= -I$(LIB_DIR)/args
C_SOURCES		+= $(wildcard $(LIB_DIR)/args/*.c)

# SubGhz
C_SOURCES		+= $(wildcard $(LIB_DIR)/fl_subghz/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/fl_subghz/*/*.c)

#scened app template lib
CFLAGS			+= -I$(LIB_DIR)/app-scened-template
C_SOURCES		+= $(wildcard $(LIB_DIR)/app-scened-template/*.c)
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/app-scened-template/*.cpp)
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/app-scened-template/*/*.cpp)