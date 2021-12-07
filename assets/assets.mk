ASSETS_DIR			:= $(PROJECT_ROOT)/assets
ASSETS_COMPILLER	:= $(PROJECT_ROOT)/scripts/assets.py
ASSETS_COMPILED_DIR	:= $(ASSETS_DIR)/compiled
ASSETS_SOURCE_DIR	:= $(ASSETS_DIR)/icons

ASSETS_SOURCES		+= $(shell find $(ASSETS_SOURCE_DIR) -type f -iname '*.png' -or -iname 'frame_rate')
ASSETS				+= $(ASSETS_COMPILED_DIR)/assets_icons.c

PROTOBUF_SOURCE_DIR		:= $(ASSETS_DIR)/protobuf
PROTOBUF_COMPILER		:= $(PROJECT_ROOT)/lib/nanopb/generator/nanopb_generator.py
PROTOBUF_COMPILED_DIR	:= $(ASSETS_COMPILED_DIR)
PROTOBUF_SOURCES		:= $(shell find $(PROTOBUF_SOURCE_DIR) -type f -iname '*.proto')
#PROTOBUF_FILENAMES		:= $(notdir $(PROTOBUF))
PROTOBUF_FILENAMES		:= $(notdir $(addsuffix .pb.c,$(basename $(PROTOBUF_SOURCES))))
PROTOBUF				:= $(addprefix $(PROTOBUF_COMPILED_DIR)/,$(PROTOBUF_FILENAMES))
PROTOBUF_CFLAGS			+= -DPB_ENABLE_MALLOC

CFLAGS				+= -I$(ASSETS_COMPILED_DIR) $(PROTOBUF_CFLAGS)
C_SOURCES			+= $(wildcard $(ASSETS_COMPILED_DIR)/*.c)
