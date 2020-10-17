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