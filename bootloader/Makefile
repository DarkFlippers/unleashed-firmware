PROJECT_ROOT	= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)
PROJECT			= bootloader

include 		$(PROJECT_ROOT)/make/base.mk

CFLAGS			+= -I$(PROJECT_ROOT) -Itargets/api-hal-include
ASM_SOURCES		+= $(wildcard src/*.s)
C_SOURCES		+= $(wildcard src/*.c)
CPP_SOURCES		+= $(wildcard src/*.cpp)

TARGET			?= f6
TARGET_DIR		= targets/$(TARGET)
include			$(TARGET_DIR)/target.mk

include			$(PROJECT_ROOT)/make/git.mk
include			$(PROJECT_ROOT)/make/toolchain.mk
include			$(PROJECT_ROOT)/make/rules.mk
