MAKEFILE_DIR	:= $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
PROJECT_ROOT	:= $(abspath $(MAKEFILE_DIR)/..)

include 		$(PROJECT_ROOT)/make/base.mk
include 		$(PROJECT_ROOT)/make/freertos-heap.mk
include			$(PROJECT_ROOT)/assets/assets.mk
include			$(PROJECT_ROOT)/core/core.mk
include 		$(PROJECT_ROOT)/applications/applications.mk
include			$(PROJECT_ROOT)/lib/lib.mk

CFLAGS			+= -I$(PROJECT_ROOT) -Itargets/furi_hal_include
CFLAGS			+= -Werror -Wno-address-of-packed-member
CPPFLAGS		+= -Werror

include			$(PROJECT_ROOT)/make/defaults.mk
TARGET_DIR		= targets/$(TARGET)
include			$(TARGET_DIR)/target.mk

ifeq ($(RAM_EXEC), 0)
PROJECT			:= firmware
else
PROJECT			:= updater
endif

include			$(PROJECT_ROOT)/make/git.mk
include			$(PROJECT_ROOT)/make/toolchain.mk
include			$(PROJECT_ROOT)/make/rules.mk
