ASSETS_DIR			:= $(PROJECT_ROOT)/assets
ASSETS_COMPILLER	:= $(ASSETS_DIR)/assets.py
ASSETS_OUTPUT_DIR	:= $(ASSETS_DIR)/output
ASSETS_SOURCE_DIR	:= $(ASSETS_DIR)/icons

ASSETS_SOURCES		+= $(shell find $(ASSETS_SOURCE_DIR) -type f -iname '*.png' -or -iname 'frame_rate')
ASSETS				+= $(ASSETS_OUTPUT_DIR)/assets_icons.c

CFLAGS				+= -I$(ASSETS_OUTPUT_DIR)
C_SOURCES			+= $(ASSETS_OUTPUT_DIR)/assets_icons.c
