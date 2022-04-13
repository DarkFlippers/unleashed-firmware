LIB_DIR			= $(PROJECT_ROOT)/lib

# TODO: some places use lib/header.h includes, is it ok?
CFLAGS			+= -I$(LIB_DIR)

# Mlib containers
CFLAGS			+= -I$(LIB_DIR)/mlib

# U8G2 display library
U8G2_DIR		= $(LIB_DIR)/u8g2
CFLAGS			+= -I$(U8G2_DIR)
C_SOURCES		+= $(U8G2_DIR)/u8g2_glue.c
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
CFLAGS			+= -I$(LITTLEFS_DIR) -DLFS_CONFIG=lfs_config.h
C_SOURCES		+= $(LITTLEFS_DIR)/lfs.c
C_SOURCES		+= $(LITTLEFS_DIR)/lfs_util.c

ST25RFAL002_DIR	= $(LIB_DIR)/ST25RFAL002
CFLAGS			+= -I$(ST25RFAL002_DIR)
CFLAGS			+= -I$(ST25RFAL002_DIR)/include
CFLAGS			+= -I$(ST25RFAL002_DIR)/source/st25r3916
C_SOURCES		+= $(wildcard $(ST25RFAL002_DIR)/*.c)
C_SOURCES		+= $(wildcard $(ST25RFAL002_DIR)/source/*.c)
C_SOURCES		+= $(wildcard $(ST25RFAL002_DIR)/source/st25r3916/*.c)

CFLAGS			+= -I$(LIB_DIR)/nfc_protocols
C_SOURCES		+= $(wildcard $(LIB_DIR)/nfc_protocols/*.c)

# callback connector (C to CPP) library
CFLAGS			+= -I$(LIB_DIR)/callback-connector

# app template library
CFLAGS			+= -I$(LIB_DIR)/app-template

# add C scene template
CFLAGS			+= -I$(LIB_DIR)/app_scene_template

# fnv1a hash library
CFLAGS			+= -I$(LIB_DIR)/fnv1a-hash
C_SOURCES		+= $(LIB_DIR)/fnv1a-hash/fnv1a-hash.c

# common apps api
CFLAGS			+= -I$(LIB_DIR)/common-api

# drivers
CFLAGS			+= -I$(LIB_DIR)/drivers
C_SOURCES		+= $(wildcard $(LIB_DIR)/drivers/*.c)

# IR lib
CFLAGS			+= -I$(LIB_DIR)/infrared/encoder_decoder
CFLAGS			+= -I$(LIB_DIR)/infrared/worker
C_SOURCES		+= $(wildcard $(LIB_DIR)/infrared/encoder_decoder/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/infrared/encoder_decoder/*/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/infrared/worker/*.c)

# SubGhz
C_SOURCES		+= $(wildcard $(LIB_DIR)/subghz/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/subghz/*/*.c)

#scened app template lib
CFLAGS			+= -I$(LIB_DIR)/app-scened-template
C_SOURCES		+= $(wildcard $(LIB_DIR)/app-scened-template/*.c)
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/app-scened-template/*.cpp)
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/app-scened-template/*/*.cpp)

# Toolbox
C_SOURCES		+= $(wildcard $(LIB_DIR)/toolbox/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/toolbox/*/*.c)
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/toolbox/*.cpp)
CPP_SOURCES		+= $(wildcard $(LIB_DIR)/toolbox/*/*.cpp)

# USB Stack
CFLAGS			+= -I$(LIB_DIR)/libusb_stm32/inc
C_SOURCES		+= $(wildcard $(LIB_DIR)/libusb_stm32/src/*.c)

# protobuf
CFLAGS			+= -I$(LIB_DIR)/nanopb
C_SOURCES		+= $(wildcard $(LIB_DIR)/nanopb/*.c)

# heatshrink
CFLAGS			+= -I$(LIB_DIR)/heatshrink
C_SOURCES		+= $(wildcard $(LIB_DIR)/heatshrink/*.c)

# Toolbox
CFLAGS			+= -I$(LIB_DIR)/flipper_file
C_SOURCES		+= $(wildcard $(LIB_DIR)/flipper_file/*.c)

# Flipper format
CFLAGS			+= -I$(LIB_DIR)/flipper_format
C_SOURCES		+= $(wildcard $(LIB_DIR)/flipper_format/*.c)

# Micro-ECC
CFLAGS			+= -I$(LIB_DIR)/micro-ecc
C_SOURCES		+= $(wildcard $(LIB_DIR)/micro-ecc/*.c)

# iButton and OneWire
C_SOURCES		+= $(wildcard $(LIB_DIR)/one_wire/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/one_wire/*/*.c)
C_SOURCES		+= $(wildcard $(LIB_DIR)/one_wire/*/*/*.c)

# microtar
CFLAGS			+= -I$(LIB_DIR)/microtar/src
C_SOURCES		+= $(wildcard $(LIB_DIR)/microtar/src/*.c)

# Update-related common code
C_SOURCES		+= $(wildcard $(LIB_DIR)/update_util/*.c)
