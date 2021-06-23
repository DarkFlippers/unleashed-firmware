ASSETS_DIR			:= $(PROJECT_ROOT)/assets
ASSETS_COMPILLER	:= $(PROJECT_ROOT)/scripts/assets.py
ASSETS_COMPILED_DIR	:= $(ASSETS_DIR)/compiled
ASSETS_SOURCE_DIR	:= $(ASSETS_DIR)/icons

ASSETS_SOURCES		+= $(shell find $(ASSETS_SOURCE_DIR) -type f -iname '*.png' -or -iname 'frame_rate')
ASSETS				+= $(ASSETS_COMPILED_DIR)/assets_icons.c

CFLAGS				+= -I$(ASSETS_COMPILED_DIR)
C_SOURCES			+= $(ASSETS_COMPILED_DIR)/assets_icons.c
