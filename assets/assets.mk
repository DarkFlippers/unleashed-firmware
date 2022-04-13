ASSETS_DIR			:= $(PROJECT_ROOT)/assets
ASSETS_COMPILLER	:= $(PROJECT_ROOT)/scripts/assets.py
ASSETS_COMPILED_DIR	:= $(ASSETS_DIR)/compiled
ASSETS_SOURCE_DIR	:= $(ASSETS_DIR)/icons

ASSETS_SOURCES		+= $(shell find $(ASSETS_SOURCE_DIR) -type f -iname '*.png' -or -iname 'frame_rate')
ASSETS				+= $(ASSETS_COMPILED_DIR)/assets_icons.c

DOLPHIN_SOURCE_DIR			:= $(ASSETS_DIR)/dolphin
DOLPHIN_INTERNAL_OUTPUT_DIR	:= $(ASSETS_COMPILED_DIR)
DOLPHIN_EXTERNAL_OUTPUT_DIR	:= $(ASSETS_DIR)/resources/dolphin

PROTOBUF_SOURCE_DIR		:= $(ASSETS_DIR)/protobuf
PROTOBUF_COMPILER		:= $(PROJECT_ROOT)/lib/nanopb/generator/nanopb_generator.py
PROTOBUF_COMPILED_DIR	:= $(ASSETS_COMPILED_DIR)
PROTOBUF_SOURCES		:= $(shell find $(PROTOBUF_SOURCE_DIR) -type f -iname '*.proto')
PROTOBUF_FILENAMES		:= $(notdir $(addsuffix .pb.c,$(basename $(PROTOBUF_SOURCES))))
PROTOBUF				:= $(addprefix $(PROTOBUF_COMPILED_DIR)/,$(PROTOBUF_FILENAMES)) $(PROTOBUF_COMPILED_DIR)/protobuf_version.h
PROTOBUF_VERSION		:= $(shell git -C $(PROTOBUF_SOURCE_DIR) fetch --tags 2>/dev/null ; git -C $(PROTOBUF_SOURCE_DIR) describe --tags --abbrev=0 2>/dev/null || echo 'unknown')
PROTOBUF_MAJOR_VERSION	:= $(word 1,$(subst ., ,$(PROTOBUF_VERSION)))
PROTOBUF_MINOR_VERSION	:= $(word 2,$(subst ., ,$(PROTOBUF_VERSION)))
$(if $(PROTOBUF_MAJOR_VERSION),,$(error "Protobuf major version is not specified, $$PROTOBUF_VERSION=$(PROTOBUF_VERSION), please perform git fetch in assets/protobuf directory"))
$(if $(PROTOBUF_MINOR_VERSION),,$(error "Protobuf minor version is not specified, $$PROTOBUF_VERSION=$(PROTOBUF_VERSION), please perform git fetch in assets/protobuf directory"))

PROTOBUF_CFLAGS			+= -DPB_ENABLE_MALLOC

CFLAGS				+= -I$(ASSETS_COMPILED_DIR) $(PROTOBUF_CFLAGS)
C_SOURCES			+= $(wildcard $(ASSETS_COMPILED_DIR)/*.c)
